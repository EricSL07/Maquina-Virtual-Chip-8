#include "../lib/chip8.hpp"
#include "../lib/defs.hpp"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Uso: " << argv[0] << " <arquivo_rom> <fps> <escala>" << std::endl;
        return 1;
    }

    int Hz = atoi(argv[2]);    // Recebe a velocidade em Hz como parâmetro
    int escala = atoi(argv[3]); // Recebe a escala de renderização como parâmetro

    Chip8 chip8;

    chip8.VM_inicializar(0x200);

    chip8.VM_CarregarROM(argv[1], 0x200);

#ifdef DEBUG
    chip8.VM_ImprimirRegistradores();
#endif

#ifdef DEBUG
    chip8.VM_ImprimirRegistradores();
#endif
    chip8.rodarSDL(Hz, escala); // Deve receber a velocidade em Hz como parâmetro e receber a escala de renderização
    return 0;
}
