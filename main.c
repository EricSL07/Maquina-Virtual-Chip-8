#include "chip8.h"
#include "defs.h"

int main(int argc, char **argv)
{
    VM vm;
    VM_Inicializar(&vm, 0x200); // Inicializa a VM (PC = 0x200)

    VM_CarregarROM(&vm, argv[1], 0x200); // Carrega a ROM de um arquivo passado como argumento pela linha de comando
#ifdef DEBUG
    VM_ImprimirRegistradores(&vm);
#endif
    while (1)
    {
        VM_ExecutarInstrucao(&vm);
// Executa uma instrução
#ifdef DEBUG
        VM_ImprimirRegistradores(&vm);
#endif
    }
}