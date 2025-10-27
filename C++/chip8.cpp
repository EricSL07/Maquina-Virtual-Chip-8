#include "chip8.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

static const uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
{
    PC = 0x200; // endereço inicial típico do CHIP-8
    I = 0;
    SP = 0;
    delay_timer = 0;
    sound_timer = 0;
    FlagDesenhar = false;
    waitingForKey = false;
    waitingRegister = 0;
    std::memset(key, 0, sizeof(key));
    std::memset(V, 0, sizeof(V));
    std::memset(RAM, 0, sizeof(RAM));
    std::memset(DISPLAY, 0, sizeof(DISPLAY));
    std::memset(stack, 0, sizeof(stack));
    // carregar fontset na memória em 0x50
    std::memcpy(&RAM[0x50], chip8_fontset, sizeof(chip8_fontset));
}

void Chip8::VM_inicializar(uint16_t pc_inicial)
{
    PC = pc_inicial;
}

void Chip8::VM_CarregarROM(char *arq_rom, uint16_t pc_inicial)
{
    FILE *rom = fopen(arq_rom, "rb");
    if (!rom)
    {
        perror("fopen");
        return;
    }
    fseek(rom, 0, SEEK_END);
    long tam_rom = ftell(rom);
    if (tam_rom <= 0)
    {
        fclose(rom);
        return;
    }
    rewind(rom);
    size_t readn = fread(&RAM[pc_inicial], 1, (size_t)tam_rom, rom);
    (void)readn;
    fclose(rom);
}

// Mapeamento de scancodes SDL para as teclas do CHIP-8
static int map_sdl_scancode_to_chip8(SDL_Scancode sc)
{
    switch (sc)
    {
    case SDL_SCANCODE_X:
        return 0x0;
    case SDL_SCANCODE_1:
        return 0x1;
    case SDL_SCANCODE_2:
        return 0x2;
    case SDL_SCANCODE_3:
        return 0x3;
    case SDL_SCANCODE_Q:
        return 0x4;
    case SDL_SCANCODE_W:
        return 0x5;
    case SDL_SCANCODE_E:
        return 0x6;
    case SDL_SCANCODE_A:
        return 0x7;
    case SDL_SCANCODE_S:
        return 0x8;
    case SDL_SCANCODE_D:
        return 0x9;
    case SDL_SCANCODE_Z:
        return 0xA;
    case SDL_SCANCODE_C:
        return 0xB;
    case SDL_SCANCODE_4:
        return 0xC;
    case SDL_SCANCODE_R:
        return 0xD;
    case SDL_SCANCODE_F:
        return 0xE;
    case SDL_SCANCODE_V:
        return 0xF;
    default:
        return -1;
    }
}

// Atualiza a textura 64x32 a partir do array DISPLAY (formato RGBA8888)
static void update_texture_from_display(SDL_Texture *tex, const uint8_t display[])
{
    const int width = 64, height = 32;
    uint32_t pixels[width * height];
    const uint32_t on = 0xFFFFFFFF;  // white
    const uint32_t off = 0x000000FF; // black

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            pixels[y * width + x] = display[y * width + x] ? on : off;
        }
    }
    SDL_UpdateTexture(tex, NULL, pixels, width * sizeof(uint32_t));
}

void Chip8::runSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init erro: %s\n", SDL_GetError());
        return;
    }

    const int scale = 10; // ajuste (10 -> janela 640x320)
    const int win_w = 64 * scale;
    const int win_h = 32 * scale;

    SDL_Window *window = SDL_CreateWindow("CHIP-8",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          win_w, win_h, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow erro: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer erro: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    // textura 64x32 (uma célula por texel)
    SDL_Texture *tex = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                         64, 32);

    bool running = true;
    SDL_Event event;

    // timing configuration inspired by sdl2.c: keep a fixed 60 FPS frame time
    const int cpuHz = 600;                                  // objetivo de instruções por segundo (ajuste 400..800)
    const double cycles_per_frame_d = double(cpuHz) / 60.0; // média de instruções por frame
    double cycle_acc = 0.0;
    const uint32_t frame_ms = 1000 / 60; // 60Hz

    while (running)
    {
        // início do frame (timing)
        Uint32 t_inicio = SDL_GetTicks();

        // processa eventos e teclado
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    running = false;
                    break;
                }
                int mapped = map_sdl_scancode_to_chip8(event.key.keysym.scancode);
                if (mapped >= 0)
                {
                    if (event.type == SDL_KEYDOWN)
                    {
                        key[mapped] = 1;
                        if (waitingForKey)
                        {
                            V[waitingRegister] = mapped;
                            waitingForKey = false;
                            PC += 2; // avança a instrução FX0A
                        }
                    }
                    else
                    {
                        key[mapped] = 0;
                    }
                }
            }
        }
        if (!running)
            break;

        // calcular quantas instruções executar neste frame (acumulador para fracionário)
        cycle_acc += cycles_per_frame_d;
        int to_run = (int)cycle_acc;
        if (to_run <= 0)
            to_run = 1;
        cycle_acc -= to_run;

        for (int i = 0; i < to_run; ++i)
        {
            VM_ExecutarInstrucao();
        }

        // atualiza timers e redesenha (uma vez por frame)
        tickTimers();
        if (FlagDesenhar)
        {
            update_texture_from_display(tex, DISPLAY);
            SDL_RenderClear(renderer);
            // desenha e escala automaticamente (dest rect maior)
            SDL_Rect dst = {0, 0, win_w, win_h};
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_RenderPresent(renderer);
            FlagDesenhar = false;
        }

        // fim do frame: espere o tempo restante para manter ~60 FPS
        Uint32 duracao = SDL_GetTicks() - t_inicio;
        if (duracao < frame_ms)
            SDL_Delay(frame_ms - duracao);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Chip8::printDisplay()
{
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            std::cout << (DISPLAY[y * 64 + x] ? "█" : " ");
        }
        std::cout << '\n';
    }
}

void Chip8::VM_ExecutarInstrucao()
{
    // Se estamos aguardando tecla, não execute instruções até receber uma
    if (waitingForKey)
    {
        return;
    }

    uint16_t inst = (RAM[PC] << 8) | RAM[PC + 1];
    // Não incrementamos PC aqui; cada case faz a atualização correta.
    uint8_t X = (inst & 0x0F00) >> 8;
    uint8_t Y = (inst & 0x00F0) >> 4;
    uint8_t N = inst & 0x000F;
    uint8_t NN = inst & 0x00FF;
    uint16_t NNN = inst & 0x0FFF;

    switch (inst & 0xF000)
    {
    case 0x0000:
        if (inst == 0x00E0)
        { // CLS
            std::memset(DISPLAY, 0, sizeof(DISPLAY));
            FlagDesenhar = true;
            PC += 2;
        }
        else if (inst == 0x00EE)
        { // RET
            if (SP > 0)
            {
                SP--;
                PC = stack[SP];
            }
            else
            {
                PC += 2; // fallback
            }
        }
        else
        {
            // 0x0NNN ignored
            PC += 2;
        }
        break;

    case 0x1000: // JP addr
        PC = NNN;
        break;

    case 0x2000: // CALL addr
        stack[SP] = PC + 2;
        SP = (SP + 1) & 0x0F; // keep within 0..15
        PC = NNN;
        break;

    case 0x3000: // SE Vx, byte
        if (V[X] == NN)
            PC += 4;
        else
            PC += 2;
        break;

    case 0x4000: // SNE Vx, byte
        if (V[X] != NN)
            PC += 4;
        else
            PC += 2;
        break;

    case 0x5000: // SE Vx, Vy  (only if low nibble == 0)
        if ((inst & 0x000F) == 0)
        {
            if (V[X] == V[Y])
                PC += 4;
            else
                PC += 2;
        }
        else
        {
            PC += 2;
        }
        break;

    case 0x6000: // LD Vx, byte
        V[X] = NN;
        PC += 2;
        break;

    case 0x7000: // ADD Vx, byte
        V[X] = (V[X] + NN) & 0xFF;
        PC += 2;
        break;

    case 0x8000: // arithmetic and logic
        switch (inst & 0x000F)
        {
        case 0x0:
            V[X] = V[Y];
            break;
        case 0x1:
            V[X] |= V[Y];
            break;
        case 0x2:
            V[X] &= V[Y];
            break;
        case 0x3:
            V[X] ^= V[Y];
            break;
        case 0x4:
        { // ADD Vx, Vy
            uint16_t sum = V[X] + V[Y];
            V[0xF] = (sum > 0xFF) ? 1 : 0;
            V[X] = sum & 0xFF;
            break;
        }
        case 0x5:
        { // SUB Vx, Vy
            V[0xF] = (V[X] > V[Y]) ? 1 : 0;
            V[X] = (V[X] - V[Y]) & 0xFF;
            break;
        }
        case 0x6: // SHR Vx {, Vy} - common variant: shift Vx right by 1, VF = least significant bit
            V[0xF] = V[X] & 0x1;
            V[X] >>= 1;
            break;
        case 0x7:
        { // SUBN Vx, Vy
            V[0xF] = (V[Y] > V[X]) ? 1 : 0;
            V[X] = (V[Y] - V[X]) & 0xFF;
            break;
        }
        case 0xE: // SHL Vx {, Vy}
            V[0xF] = (V[X] >> 7) & 0x1;
            V[X] = (V[X] << 1) & 0xFF;
            break;
        default:
            break;
        }
        PC += 2;
        break;

    case 0x9000: // SNE Vx, Vy
        if ((inst & 0x000F) == 0)
        {
            if (V[X] != V[Y])
                PC += 4;
            else
                PC += 2;
        }
        else
        {
            PC += 2;
        }
        break;

    case 0xA000: // LD I, addr
        I = NNN;
        PC += 2;
        break;

    case 0xB000: // JP V0, addr
        PC = NNN + V[0];
        break;

    case 0xC000:
    { // RND Vx, byte
        uint8_t rnd = (uint8_t)(rand() & 0xFF);
        V[X] = rnd & NN;
        PC += 2;
        break;
    }

    case 0xD000:
    { // DRW Vx, Vy, nibble
        const uint8_t DISPLAY_WIDTH = 64;
        const uint8_t DISPLAY_HEIGHT = 32;

        uint8_t xcoord = V[X] % DISPLAY_WIDTH;
        uint8_t ycoord = V[Y] % DISPLAY_HEIGHT;

        V[0xF] = 0;

        for (uint8_t row = 0; row < N; row++)
        {
            uint8_t bits = RAM[I + row];
            uint8_t cy = (ycoord + row) % DISPLAY_HEIGHT;

            for (uint8_t col = 0; col < 8; col++)
            {
                uint8_t cx = (xcoord + col) % DISPLAY_WIDTH;
                uint8_t curr_col = DISPLAY[cy * DISPLAY_WIDTH + cx];
                uint8_t pixel_sprite = ((bits >> (7 - col)) & 1);
                if (pixel_sprite)
                {
                    if (curr_col)
                    {
                        DISPLAY[cy * DISPLAY_WIDTH + cx] = 0;
                        V[0xF] = 1;
                    }
                    else
                    {
                        DISPLAY[cy * DISPLAY_WIDTH + cx] = 1;
                    }
                }
            }
        }
        FlagDesenhar = true;
        PC += 2;
        break;
    }

    case 0xE000:
    {
        switch (inst & 0x00FF)
        {
        case 0x009E: // SKP Vx
            if (key[V[X]])
                PC += 4;
            else
                PC += 2;
            break;
        case 0x00A1: // SKNP Vx
            if (!key[V[X]])
                PC += 4;
            else
                PC += 2;
            break;
        default:
            PC += 2;
            break;
        }
        break;
    }

    case 0xF000:
    {
        switch (inst & 0x00FF)
        {
        case 0x0007: // LD Vx, delay_timer
            V[X] = delay_timer;
            PC += 2;
            break;
        case 0x000A: // LD Vx, K (wait for key)
            // set waiting state and return without advancing PC
            waitingForKey = true;
            waitingRegister = X;
            // do NOT change PC; next cycles will return early while waitingForKey==true
            break;
        case 0x0015: // LD delay_timer, Vx
            delay_timer = V[X];
            PC += 2;
            break;
        case 0x0018: // LD sound_timer, Vx
            sound_timer = V[X];
            PC += 2;
            break;
        case 0x001E: // ADD I, Vx
            I = (I + V[X]) & 0x0FFF;
            PC += 2;
            break;
        case 0x0029: // LD F, Vx (set I to location of sprite for digit Vx)
            I = 0x50 + (V[X] * 5);
            PC += 2;
            break;
        case 0x0033: // LD B, Vx (BCD)
            RAM[I] = V[X] / 100;
            RAM[I + 1] = (V[X] / 10) % 10;
            RAM[I + 2] = V[X] % 10;
            PC += 2;
            break;
        case 0x0055: // LD [I], V0..Vx
            for (int i = 0; i <= X; ++i)
                RAM[I + i] = V[i];
            PC += 2;
            break;
        case 0x0065: // LD V0..Vx, [I]
            for (int i = 0; i <= X; ++i)
                V[i] = RAM[I + i];
            PC += 2;
            break;
        default:
            PC += 2;
            break;
        }
        break;
    }

    default:
        PC += 2;
        break;
    } // end switch
}

void Chip8::VM_ImprimirRegistradores()
{
    printf("PC: 0x%04X I: 0x%04X SP: 0x%02X\n", PC, I, SP);
    for (int i = 0; i < 16; i++)
    {
        printf("V[%X]: 0x%02X ", i, V[i]);
    }
    printf("\n");
}

void Chip8::tickTimers()
{
    if (delay_timer > 0)
        --delay_timer;
    if (sound_timer > 0)
    {
        --sound_timer;
        if (sound_timer == 1)
            std::cout << '\a'; // beep ASCII
    }
}
