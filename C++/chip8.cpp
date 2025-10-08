#include "chip8.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <thread>

Chip8::Chip8()
{
    // PC = 0;
    // I = 0;
    // SP = 0;

    // for (auto &r : V)
    //     r = 0;

    // for (auto &b : RAM)
    //     b = 0;

    // for (auto &p : DISPLAY)
    //     p = 0;
    // for (auto &s : stack)
    //     s = 0;
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

void Chip8::printDisplay()
{
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            if (DISPLAY[y * 64 + x] == 1)
            {
                std::cout << "█";
            }
            else
            {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
}

void Chip8::VM_ExecutarInstrucao()
{
    uint16_t inst = (RAM[PC] << 8) | RAM[PC + 1];

    std::cout << "Instrução: 0x" << std::hex << std::setw(4) << std::setfill('0') << inst << std::endl;
    PC += 2;
    std::this_thread::sleep_for(std::chrono::seconds(1));

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
            }
            break;
        }
        else if (inst == 0x00EE) // RET: Return from subroutine
        {
            if (SP > 0)
                SP--;
            PC = stack[SP];
            break;
        } else {
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

        for (uint8_t row = 0; row <= N; row++)
        {
            uint8_t bits = RAM[I + row];
            uint8_t cy = (ycoord + row) % DISPLAY_HEIGHT;

            for (uint8_t col = 0; col < 8; col++)
            {
                uint8_t cx = (xcoord + col) % DISPLAY_WIDTH;
                uint8_t curr_col = DISPLAY[cx, cy];
                col = (bits & (0x01 & (7 - col))) ;
                if (col > 0)
                {
                    if (curr_col > 0){
                        DISPLAY[cx, cy] = 0;
                        V[0xF] = 1;
                    }else {
                        DISPLAY[cx, cy] = 1;
                    }
                if (cx == DISPLAY_WIDTH - 1)
                {
                    break;
                }
            if (cy == DISPLAY_HEIGHT - 1)
            {
                break;
            }
                }
            }
        }
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