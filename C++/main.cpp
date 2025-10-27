#include "chip8.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "defs.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0] << " <arquivo_rom>" << std::endl;
        return 1;
    }

    Chip8 chip8;

    chip8.VM_inicializar(0x200);

    chip8.VM_CarregarROM(argv[1], 0x200);

#ifdef DEBUG
    chip8.VM_ImprimirRegistradores();
#endif

#ifdef DEBUG
    chip8.VM_ImprimirRegistradores();
#endif
    for (int i = 0; i < 1000; ++i)
        chip8.VM_ExecutarInstrucao();
    chip8.printDisplay();
    chip8.runSDL();
    return 0;
}
