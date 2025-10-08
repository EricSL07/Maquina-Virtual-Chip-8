#include "chip8.hpp"
#include <iostream>
#include <thread>
#include <chrono>

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
    // Limpa o terminal (uma vez) e posiciona o cursor no topo
    std::cout << "\x1b[2J\x1b[H";

    const int CYCLES_PER_FRAME = 10;                           // instruções por frame
    const auto FRAME_DURATION = std::chrono::milliseconds(16); // ~60 FPS
    while (true)
    {
        for (int c = 0; c < CYCLES_PER_FRAME; ++c)
        {
            chip8.VM_ExecutarInstrucao();
        }

#ifdef DEBUG
        chip8.VM_ImprimirRegistradores();
#endif

        // chip8.VM_ImprimirDisplayConsole();

        std::this_thread::sleep_for(FRAME_DURATION);
    }

    return 0;
}
