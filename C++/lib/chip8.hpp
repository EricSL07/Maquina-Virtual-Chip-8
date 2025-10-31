#include <cstdint>
#include <array>
#include <string>
#include <SDL2/SDL.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

class Chip8
{
private:
    uint8_t memoria[4096];            // memória do CHIP-8
    uint16_t pc;                      // contador de programa
    uint8_t registradores[16];        // registradores V0..VF
    uint8_t ponteiro_pilha;           // ponteiro da pilha (SP)
    uint16_t indiceI;                 // registrador I
    uint16_t pilha[16];               // pilha de retorno
    uint8_t tela[64 * 32];            // buffer de vídeo (DISPLAY)
    uint8_t temporizador_delay;       // delay timer
    uint8_t temporizador_som;         // sound timer
    bool deveDesenhar;                // flag indicando que a nova imagem deve ser desenhada
    uint8_t teclas[16];               // estado das 16 teclas do CHIP-8
    bool aguardando_tecla = false;    // espera por tecla (FX0A)
    uint8_t reg_aguardando_tecla = 0; // registrador a preencher quando a tecla for pressionada

public:
    Chip8();
    ~Chip8() = default;

    void VM_inicializar(uint16_t pc_inicial);
    void VM_CarregarROM(char *arq_rom, uint16_t pc_inicial);
    void VM_ExecutarInstrucao();
    void VM_ImprimirRegistradores();
    void rodarSDL();
    void tickTimers();
    // friend para permitir que a camada SDL acesse os internos para renderização/entrada
    friend void rodar_loop_sdl(Chip8 &vm);
};