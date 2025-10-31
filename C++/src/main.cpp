#include "./lib/chip8.hpp"
#include "./lib/defs.hpp"

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
    chip8.rodarSDL();
    return 0;
}
