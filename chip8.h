#include <stdint.h>
typedef struct VM
{
    uint8_t RAM[4096];        // Memória 4KB
    uint16_t PC;              // Program Counter
    uint8_t V[16];            // Registradores de propósito geral
    uint8_t SP;               // Stack Pointer
    uint16_t I;               // Registrador de índice
    uint16_t stack[16];       // Pilha
    uint8_t DISPLAY[64 * 32]; // Tela
} VM;

void VM_Inicializar(VM *vm, uint16_t pc_inicial);
void VM_CarregarROM(VM *vm,
                    char *arq_rom,
                    uint16_t pc_inicial);
void VM_ExecutarInstrucao(VM *vm);
void VM_ImprimirRegistradores(VM *vm);