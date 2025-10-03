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
        }
        else if (inst == 0x00EE)
        { // RET - Return
            PC = stack[--SP];
            std::cout << "Return" << std::endl;
        }
        PC += 2;
        break;

    case 0x6: // LD Vx, byte
        V[X] = NN;
        std::cout << "V" << std::hex << static_cast<int>(X) << " = 0x" << static_cast<int>(NN) << std::endl;
        PC += 2;
        break;

    case 0xa:
        I = NNN;
        PC += 2;
        break;

    case 0xd:
        const int DISPLAY_WIDTH = 64;
        const int DISPLAY_HEIGHT = 32;

        uint8_t xcoord = V[X] % DISPLAY_WIDTH;
        uint8_t ycoord = V[Y] % DISPLAY_HEIGHT;

        for (int row = 0; row < N; row++)
        {
            uint8_t bits = RAM[I + row];
            uint8_t cy = (ycoord + row) % DISPLAY_HEIGHT;

            for (int col = 0; col < 8; col++)
            {
                uint8_t cx = (xcoord + col) % DISPLAY_WIDTH;
                uint8_t curr_col = DISPLAY[cy * cx];
                col = bits & (0x01 << 7 - col);
                if (col > 0)
                {
                    if (curr_col > 0)
                    {
                        DISPLAY[cy * cx] = 0;
                        V[0xF] = 1;
                    }
                    else {
                        DISPLAY[cy * DISPLAY_WIDTH + cx] = 1;
                    }
                }
                if (cx == DISPLAY_WIDTH -1)
                {
                    break;
                }
                
            }
            if (cy == DISPLAY_HEIGHT - 1)
            {
                break;
            }
            //update_display();
        }
        
        

    default:
        std::cout << "Grupo não identificado! Instrução: 0x" << std::hex << inst << std::endl;
        std::exit(1);
        break;
    }
}

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