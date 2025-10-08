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
    // Limpa o terminal (uma vez) e posiciona o cursor no topo

#ifdef DEBUG
        chip8.VM_ImprimirRegistradores();
#endif
    chip8.printDisplay();
    return 0;
}
