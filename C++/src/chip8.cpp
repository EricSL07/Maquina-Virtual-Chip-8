#include "./lib/chip8.hpp"

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
    pc = 0x200; // endereço inicial típico do CHIP-8
    indiceI = 0;
    ponteiro_pilha = 0;
    temporizador_delay = 0;
    temporizador_som = 0;
    deveDesenhar = false;
    aguardando_tecla = false;
    reg_aguardando_tecla = 0;
    std::memset(teclas, 0, sizeof(teclas));
    std::memset(registradores, 0, sizeof(registradores));
    std::memset(memoria, 0, sizeof(memoria));
    std::memset(tela, 0, sizeof(tela));
    std::memset(pilha, 0, sizeof(pilha));
    // carregar fontset na memória em 0x50
    std::memcpy(&memoria[0x50], chip8_fontset, sizeof(chip8_fontset));
}

void Chip8::VM_inicializar(uint16_t pc_inicial)
{
    pc = pc_inicial;
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
    size_t readn = fread(&memoria[pc_inicial], 1, (size_t)tam_rom, rom);
    (void)readn;
    fclose(rom);
}

void Chip8::VM_ExecutarInstrucao()
{
    // Se estamos aguardando tecla, não execute instruções até receber uma
    if (aguardando_tecla)
    {
        return;
    }

    uint16_t inst = (memoria[pc] << 8) | memoria[pc + 1];
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
            std::memset(tela, 0, sizeof(tela));
            deveDesenhar = true;
            pc += 2;
        }
        else if (inst == 0x00EE)
        { // RET
            if (ponteiro_pilha > 0)
            {
                ponteiro_pilha--;
                pc = pilha[ponteiro_pilha];
            }
            else
            {
                pc += 2; // fallback
            }
        }
        else
        {
            // 0x0NNN ignored
            pc += 2;
        }
        break;

    case 0x1000: // JP addr
        pc = NNN;
        break;

    case 0x2000: // CALL addr
        pilha[ponteiro_pilha] = pc + 2;
        ponteiro_pilha = (ponteiro_pilha + 1) & 0x0F; // manter em 0..15
        pc = NNN;
        break;

    case 0x3000: // SE Vx, byte
        if (registradores[X] == NN)
            pc += 4;
        else
            pc += 2;
        break;

    case 0x4000: // SNE Vx, byte
        if (registradores[X] != NN)
            pc += 4;
        else
            pc += 2;
        break;

    case 0x5000: // SE Vx, Vy  (only if low nibble == 0)
        if ((inst & 0x000F) == 0)
        {
            if (registradores[X] == registradores[Y])
                pc += 4;
            else
                pc += 2;
        }
        else
        {
            pc += 2;
        }
        break;

    case 0x6000: // LD Vx, byte
        registradores[X] = NN;
        pc += 2;
        break;

    case 0x7000: // ADD Vx, byte
        registradores[X] = (registradores[X] + NN) & 0xFF;
        pc += 2;
        break;

    case 0x8000: // arithmetic and logic
        switch (inst & 0x000F)
        {
        case 0x0:
            registradores[X] = registradores[Y];
            break;
        case 0x1:
            registradores[X] |= registradores[Y];
            break;
        case 0x2:
            registradores[X] &= registradores[Y];
            break;
        case 0x3:
            registradores[X] ^= registradores[Y];
            break;
        case 0x4:
        { // ADD Vx, Vy
            uint16_t sum = registradores[X] + registradores[Y];
            registradores[0xF] = (sum > 0xFF) ? 1 : 0;
            registradores[X] = sum & 0xFF;
            break;
        }
        case 0x5:
        { // SUB Vx, Vy
            registradores[0xF] = (registradores[X] > registradores[Y]) ? 1 : 0;
            registradores[X] = (registradores[X] - registradores[Y]) & 0xFF;
            break;
        }
        case 0x6: // SHR Vx {, Vy} - common variant: shift Vx right by 1, VF = least significant bit
            registradores[0xF] = registradores[X] & 0x1;
            registradores[X] >>= 1;
            break;
        case 0x7:
        { // SUBN Vx, Vy
            registradores[0xF] = (registradores[Y] > registradores[X]) ? 1 : 0;
            registradores[X] = (registradores[Y] - registradores[X]) & 0xFF;
            break;
        }
        case 0xE: // SHL Vx {, Vy}
            registradores[0xF] = (registradores[X] >> 7) & 0x1;
            registradores[X] = (registradores[X] << 1) & 0xFF;
            break;
        default:
            break;
        }
        pc += 2;
        break;

    case 0x9000: // SNE Vx, Vy
        if ((inst & 0x000F) == 0)
        {
            if (registradores[X] != registradores[Y])
                pc += 4;
            else
                pc += 2;
        }
        else
        {
            pc += 2;
        }
        break;

    case 0xA000: // LD I, addr
        indiceI = NNN;
        pc += 2;
        break;

    case 0xB000: // JP V0, addr
        pc = NNN + registradores[0];
        break;

    case 0xC000:
    { // RND Vx, byte
        uint8_t rnd = (uint8_t)(rand() & 0xFF);
        registradores[X] = rnd & NN;
        pc += 2;
        break;
    }

    case 0xD000:
    { // DRW Vx, Vy, nibble
        const uint8_t DISPLAY_WIDTH = 64;
        const uint8_t DISPLAY_HEIGHT = 32;

        uint8_t xcoord = registradores[X] % DISPLAY_WIDTH;
        uint8_t ycoord = registradores[Y] % DISPLAY_HEIGHT;

        registradores[0xF] = 0;

        for (uint8_t row = 0; row < N; row++)
        {
            uint8_t bits = memoria[indiceI + row];
            uint8_t cy = (ycoord + row) % DISPLAY_HEIGHT;

            for (uint8_t col = 0; col < 8; col++)
            {
                uint8_t cx = (xcoord + col) % DISPLAY_WIDTH;
                uint8_t curr_col = tela[cy * DISPLAY_WIDTH + cx];
                uint8_t pixel_sprite = ((bits >> (7 - col)) & 1);
                if (pixel_sprite)
                {
                    if (curr_col)
                    {
                        tela[cy * DISPLAY_WIDTH + cx] = 0;
                        registradores[0xF] = 1;
                    }
                    else
                    {
                        tela[cy * DISPLAY_WIDTH + cx] = 1;
                    }
                }
            }
        }
        deveDesenhar = true;
        pc += 2;
        break;
    }

    case 0xE000:
    {
        switch (inst & 0x00FF)
        {
        case 0x009E: // SKP Vx
            if (teclas[registradores[X]])
                pc += 4;
            else
                pc += 2;
            break;
        case 0x00A1: // SKNP Vx
            if (!teclas[registradores[X]])
                pc += 4;
            else
                pc += 2;
            break;
        default:
            pc += 2;
            break;
        }
        break;
    }

    case 0xF000:
    {
        switch (inst & 0x00FF)
        {
        case 0x0007: // LD Vx, delay_timer
            registradores[X] = temporizador_delay;
            pc += 2;
            break;
        case 0x000A: // LD Vx, K (wait for key)
            // set waiting state and return without advancing PC
            aguardando_tecla = true;
            reg_aguardando_tecla = X;
            // do NOT change PC; next cycles will return early while waitingForKey==true
            break;
        case 0x0015: // LD delay_timer, Vx
            temporizador_delay = registradores[X];
            pc += 2;
            break;
        case 0x0018: // LD sound_timer, Vx
            temporizador_som = registradores[X];
            pc += 2;
            break;
        case 0x001E: // ADD I, Vx
            indiceI = (indiceI + registradores[X]) & 0x0FFF;
            pc += 2;
            break;
        case 0x0029: // LD F, Vx (set I to location of sprite for digit Vx)
            indiceI = 0x50 + (registradores[X] * 5);
            pc += 2;
            break;
        case 0x0033: // LD B, Vx (BCD)
            memoria[indiceI] = registradores[X] / 100;
            memoria[indiceI + 1] = (registradores[X] / 10) % 10;
            memoria[indiceI + 2] = registradores[X] % 10;
            pc += 2;
            break;
        case 0x0055: // LD [I], V0..Vx
            for (int i = 0; i <= X; ++i)
                memoria[indiceI + i] = registradores[i];
            pc += 2;
            break;
        case 0x0065: // LD V0..Vx, [I]
            for (int i = 0; i <= X; ++i)
                registradores[i] = memoria[indiceI + i];
            pc += 2;
            break;
        default:
            pc += 2;
            break;
        }
        break;
    }

    default:
        pc += 2;
        break;
    } // end switch
}

void Chip8::VM_ImprimirRegistradores()
{
    printf("PC: 0x%04X I: 0x%04X SP: 0x%02X\n", pc, indiceI, ponteiro_pilha);
    for (int i = 0; i < 16; i++)
    {
        printf("V[%X]: 0x%02X ", i, registradores[i]);
    }
    printf("\n");
}

void Chip8::tickTimers()
{
    if (temporizador_delay > 0)
        --temporizador_delay;
    if (temporizador_som > 0)
    {
        --temporizador_som;
        if (temporizador_som == 1)
            std::cout << '\a'; // beep ASCII
    }
}

void Chip8::rodarSDL()
{
    rodar_loop_sdl(*this);
}
