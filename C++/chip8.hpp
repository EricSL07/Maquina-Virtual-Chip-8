#include <cstdint>
#include <array>
#include <string>

class Chip8
{
private:
    std::array<uint8_t, 4096> RAM;
    uint16_t PC;
    std::array<uint8_t, 16> V;
    uint8_t SP;
    uint16_t I;
    std::array<uint16_t, 16> stack;
    std::array<uint8_t, 128 * 64> DISPLAY;


public:
    Chip8();
    ~Chip8() = default;

    void VM_inicializar(uint16_t pc_inicial);
    bool VM_CarregarROM(std::string arq_rom, uint16_t pc_inicial);
    void VM_ExecutarInstrucao();
    void VM_ImprimirRegistradores();
    // void VM_ImprimirDisplayConsole();
};