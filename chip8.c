#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

void VM_Inicializar(VM *vm, uint16_t pc_inicial)
{
    vm->PC = pc_inicial;
}

void VM_CarregarROM(VM *vm,
                    char *arq_rom,
                    uint16_t pc_inicial)
{

    FILE *rom = fopen(arq_rom, "rb");
    fseek(rom, 0, SEEK_END);
    int tam_rom = ftell(rom);
    rewind(rom);

    fread(&vm->RAM[pc_inicial], 1, tam_rom, rom);

    fclose(rom);
}
void VM_ExecutarInstrucao(VM *vm)
{
    uint16_t inst = (vm->RAM[vm->PC] << 8) | vm->RAM[vm->PC + 1];

    printf("Instrução: 0x%04x\n", inst);
    vm->PC + 2;
    uint8_t grupo = inst >> 12;
    uint8_t X = (inst & 0x0F00) >> 8;
    uint8_t Y = (inst & 0x00F0) >> 4;
    uint8_t N = inst & 0x000F;
    uint8_t NN = inst & 0x00FF;
    uint8_t NNN = inst & 0x0FFF;

    switch (grupo)
    {
        case 0:
            if(inst == 0x00E0){
                for (int i = 0; i < 64; i++)
                {
                    vm->DISPLAY[i] = 0;
                }
                break;
            }
            case 6:
                vm->V[X] = NN;
                break;

    default:
        printf("Grupo não identificado! Instrução: 0x%04X\n", inst);
        exit(1);
    }
}

void VM_ImprimirRegistradores(VM *vm)
{
    printf("PC: 0x%04X I: 0x%04X SP: 0x%02X\n", vm->PC, vm->I, vm->SP);
    for (int i = 0; i < 16; i++)
    {
        printf("V[%X]: 0x%02X ", i, vm->V[i]);
    }
    printf("\n");
}