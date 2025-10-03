#include "chip8.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0] << " <arquivo_rom>" << std::endl;
        return 1;
    }

    Chip8 chip8;

    if (!chip8.VM_CarregarROM(argv[1], 0x200))
    {
        std::cerr << "Erro ao carregar ROM!" << std::endl;
        return 1;
    }

#ifdef DEBUG
    chip8.VM_ImprimirRegistradores();
#endif

    std::cout << "Executando CHIP-8... Pressione Ctrl+C para parar" << std::endl;

    for (int i = 0; i < 10; i++)
    { // Limitado para teste
        chip8.VM_ExecutarInstrucao();

#ifdef DEBUG
        chip8.VM_ImprimirRegistradores();
#endif
    }

    return 0;
}
