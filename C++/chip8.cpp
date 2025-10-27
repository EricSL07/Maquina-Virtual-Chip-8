#include "chip8.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

Chip8::Chip8()
{
    PC = 0x200; // endereço inicial típico do CHIP-8
    I = 0;
    SP = 0;
    delay_timer = 0;
    sound_timer = 0;
    FlagDesenhar = false;
    std::memset(key, 0, sizeof(key));
    std::memset(V, 0, sizeof(V));
    std::memset(RAM, 0, sizeof(RAM));
    std::memset(DISPLAY, 0, sizeof(DISPLAY));
    std::memset(stack, 0, sizeof(stack));
}

void Chip8::VM_inicializar(uint16_t pc_inicial)
{
    PC = pc_inicial;
}

void Chip8::VM_CarregarROM(char *arq_rom, uint16_t pc_inicial)
{
    FILE *rom = fopen(arq_rom, "rb");
    fseek(rom, 0, SEEK_END);
    int tam_rom = ftell(rom);
    rewind(rom);

    fread(&RAM[pc_inicial], 1, tam_rom, rom);

    fclose(rom);
}

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

static void update_texture_from_display(SDL_Texture *tex, const uint8_t display[])
{
    const int width = 64, height = 32;
    uint32_t pixels[width * height];
    // branco e preto (format RGBA8888)
    const uint32_t on = 0xFFFFFFFF;  // white
    const uint32_t off = 0x000000FF; // black

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            pixels[y * width + x] = display[y * width + x] ? on : off;
        }
    }
    // pitch = width * 4 bytes
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

    const int cycles_per_frame = 10;     // ajuste conforme necessário
    const uint32_t frame_ms = 1000 / 60; // 60Hz
    uint32_t last_tick = SDL_GetTicks();

    while (running)
    {
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
                    key[mapped] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                }
            }
        }
        if (!running)
            break;

        // executa N instruções por frame (CPU speed)
        for (int i = 0; i < cycles_per_frame; ++i)
        {
            VM_ExecutarInstrucao();
        }

        // temporização: atualiza timers a 60Hz
        uint32_t now = SDL_GetTicks();
        if (now - last_tick >= frame_ms)
        {
            tickTimers();
            // redesenha apenas se algo mudou
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
            last_tick = now;
        }

        // pequena pausa para não consumir 100% CPU
        SDL_Delay(1);
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
    uint16_t inst = (RAM[PC] << 8) | RAM[PC + 1];

    std::cout << "Instrução: 0x" << std::hex << std::setw(4) << std::setfill('0') << inst << std::endl;
    PC += 2;

    uint8_t grupo = inst >> 12;
    uint8_t X = (inst & 0x0F00) >> 8;
    uint8_t Y = (inst & 0x00F0) >> 4;
    uint8_t N = inst & 0x000F;
    uint8_t NN = inst & 0x00FF;
    uint16_t NNN = inst & 0x0FFF;

    switch (grupo)
    {
    case 0x0:
        if (inst == 0x00E0) // CLS: Clear the display
        {
            for (int i = 0; i < 64 * 32; i++)
            {
                DISPLAY[i] = 0;
                FlagDesenhar = true;
            }
            break;
        }
        else if (inst == 0x00EE) // RET: Return from subroutine
        {
            if (SP > 0)
                SP--;
            PC = stack[SP];
            break;
        }
        else
        {
            // Ignora outros 0x0NNN (não implementados)
            std::cout << "Opcode 0x0NNN não suportado: 0x" << std::hex << inst << std::dec << "\n";
        }
        break;

    case 0x1:
        PC = NNN;
        break;

    case 0x6: // LD Vx, byte
        V[X] = NN;
        break;

    case 0xa: // define o valor de I para o endereço de NNN.
        I = NNN;
        break;

    case 0xd: // Draw sprite at (Vx, Vy) with N bytes of sprite data starting at I
    {
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
        break;
    }

    case 0x7: // adiciona o valor NN ao registrador VX
        V[X] = V[X] + NN;
        break;

    default:
        std::cout << "Grupo não identificado! Instrução: 0x" << std::hex << inst << std::endl;
        std::exit(1);
        break;
    }
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
