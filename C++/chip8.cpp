#include "chip8.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>

Chip8::Chip8()
{
    VM_inicializar(0x200);
}

void Chip8::VM_inicializar(uint16_t pc_inicial)
{
    PC = pc_inicial;
    I = 0;
    SP = 0;

    // Limpar arrays
    RAM.fill(0);
    V.fill(0);
    stack.fill(0);
    DISPLAY.fill(0);

    std::cout << "CHIP-8 inicializado com PC = 0x" << std::hex << pc_inicial << std::endl;
}

bool Chip8::VM_CarregarROM(std::string arq_rom, uint16_t pc_inicial)
{
    std::ifstream arquivo(arq_rom, std::ios::binary);

    if (!arquivo.is_open())
    {
        std::cerr << "Erro ao abrir arquivo: " << arq_rom << std::endl;
        return false;
    }

    arquivo.seekg(0, std::ios::end);
    size_t tam_rom = arquivo.tellg();
    arquivo.seekg(0, std::ios::beg);

    if (tam_rom > (4096 - static_cast<size_t>(pc_inicial)))
    {
        std::cerr << "ROM muito grande!" << std::endl;
        return false;
    }

    arquivo.read(reinterpret_cast<char *>(&RAM[pc_inicial]), tam_rom);

    if (!arquivo)
    {
        std::cerr << "Erro ao ler arquivo!" << std::endl;
        return false;
    }

    std::cout << "ROM carregada: " << arq_rom << " (" << tam_rom << " bytes)" << std::endl;
    return true;
}

void Chip8::VM_ExecutarInstrucao()
{
    uint16_t inst = (RAM[PC] << 8) | RAM[PC + 1];

    std::cout << "Instrução: 0x" << std::hex << std::setw(4) << std::setfill('0') << inst << std::endl;

    uint8_t grupo = inst >> 12;
    uint8_t X = (inst & 0x0F00) >> 8;
    uint8_t Y = (inst & 0x00F0) >> 4;
    uint8_t N = inst & 0x000F;
    uint8_t NN = inst & 0x00FF;
    uint16_t NNN = inst & 0x0FFF;

    switch (grupo)
    {
    case 0x0:
        if (inst == 0x00E0)
        { // CLS - Clear display
            DISPLAY.fill(0);
            std::cout << "Display limpo" << std::endl;
            PC += 2;
        }
        else if (inst == 0x00EE)
        { // RET - Return
            // POP address from stack and jump to it. Do NOT add 2 after RET because
            // the return address stored on the stack is already the correct next PC.
            PC = stack[--SP];
            std::cout << "Return" << std::endl;
        }
        else
        {
            // 0x0NNN - SYS addr (ignored on modern interpreters)
            PC += 2;
        }
        break;

    case 0x1:
        PC = NNN;

    case 0x6: // LD Vx, byte
        V[X] = NN;
        std::cout << "V" << std::hex << static_cast<int>(X) << " = 0x" << static_cast<int>(NN) << std::endl;
        PC += 2;
        break;

    case 0xa: // define o valor de I para o endereço de NNN.
        I = NNN;
        PC += 2;
        break;

    case 0xd: // Draw sprite at (Vx, Vy) with N bytes of sprite data starting at I
    {
        
        uint8_t xcoord = V[X] % DISPLAY_WIDTH;
        uint8_t ycoord = V[Y] % DISPLAY_HEIGHT;

        V[0xF] = 0; // reset VF (collision flag)
        for (int row = 0; row < N; row++)
        {
            uint8_t bits = RAM[I + row];
            uint8_t cy = (ycoord + row) % DISPLAY_HEIGHT;

            for (int col = 0; col < 8; col++)
            {
                uint8_t cx = (xcoord + col) % DISPLAY_WIDTH;
                uint8_t bit = (bits >> (7 - col)) & 0x1;
                if (bit)
                {
                    size_t idx = static_cast<size_t>(cy) * DISPLAY_WIDTH + cx;
                    if (DISPLAY[idx])
                    {
                        V[0xF] = 1;
                    }
                    DISPLAY[idx] ^= 1;
                }
            }
        }
        PC += 2;
    }
    break;

    case 0x7: // adiciona o valor NN ao registrador VX
        V[X] = V[X] + NN;
        PC += 2;
        break;

    // case 0xff:
    
    //     const uint8_t DISPLAY_WIDTH = 128;
    //     const uint8_t DISPLAY_HEIGHT = 64;
    //     PC += 2;
    //     break;

    default:
        std::cout << "Grupo não identificado! Instrução: 0x" << std::hex << inst << std::endl;
        std::exit(1);
        break;
    }
}

// void Chip8::VM_ImprimirDisplayConsole()
// {
//     const int WIDTH = 64;
//     const int HEIGHT = 32;

//     // Posicionar o cursor no topo para redesenhar in-place (requere terminal ANSI)
//     std::cout << "\x1b[H";

//     for (int y = 0; y < HEIGHT; ++y)
//     {
//         for (int x = 0; x < WIDTH; ++x)
//         {
//             size_t idx = static_cast<size_t>(y) * WIDTH + x;
//             // caractere para pixel ligado/desligado (ajuste conforme fonte/terminal)
//             std::cout << (DISPLAY[idx] ? "█" : " ");
//         }
//         std::cout << '\n';
//     }
//     std::cout << std::flush;
// }

void Chip8::VM_ImprimirRegistradores()
{
    // Salvar estado atual da stream
    auto flags = std::cout.flags();

    std::cout << "\n╔══════════════════════════════════╗" << std::endl;
    std::cout << "║        CHIP-8 REGISTRADORES      ║" << std::endl;
    std::cout << "╠══════════════════════════════════╣" << std::endl;

    // Registradores principais
    std::cout << "║ PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << PC
              << "  I: 0x" << std::setw(4) << std::setfill('0') << I
              << "  SP: 0x" << std::setw(2) << std::setfill('0') << static_cast<int>(SP)
              << " ║" << std::endl;

    std::cout << "╠══════════════════════════════════╣" << std::endl;

    // Registradores V em matriz 4x4
    for (int row = 0; row < 4; row++)
    {
        std::cout << "║ ";
        for (int col = 0; col < 4; col++)
        {
            int i = row * 4 + col;
            std::cout << "V" << std::hex << i << ":0x"
                      << std::setw(2) << std::setfill('0') << static_cast<int>(V[i]) << " ";
        }
        std::cout << "║" << std::endl;
    }

    std::cout << "╚══════════════════════════════════╝" << std::endl;

    // Restaurar estado da stream
    std::cout.flags(flags);
}