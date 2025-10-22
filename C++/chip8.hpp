#include <cstdint>
#include <array>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

class Chip8
{
private:
    uint8_t RAM[4096];
    uint16_t PC;
    uint8_t V[16];
    uint8_t SP;
    uint16_t I;
    uint16_t stack[16];
    uint8_t DISPLAY[64 * 32];

public:
    Chip8();
    ~Chip8() = default;

    void VM_inicializar(uint16_t pc_inicial);
    void VM_CarregarROM(char *arq_rom, uint16_t pc_inicial);
    void VM_ExecutarInstrucao();
    void VM_ImprimirRegistradores();
    void printDisplay();
    void runSDL();
};