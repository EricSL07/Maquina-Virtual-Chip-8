#include <stdint.h>

struct Chip8 {
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t gfx[64 * 32];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    uint8_t sp;
    uint8_t key[16];
} chip8_t; 

