# Máquina Virtual CHIP-8

Emulador da máquina virtual **CHIP-8** implementado em C e C++. O projeto está dividido em duas versões:

- **`C/`** – implementação básica em C (sem interface gráfica), útil para aprendizado e depuração da CPU.
- **`C++/`** – implementação completa em C++ com interface gráfica via **SDL2**, suporte a entrada de teclado e temporizadores.

---

## O que é CHIP-8?

CHIP-8 é uma linguagem de máquina virtual criada na década de 1970, originalmente projetada para facilitar o desenvolvimento de jogos em microcomputadores de 8 bits. Até hoje é amplamente utilizada como projeto de aprendizado para construção de emuladores.

### Especificações

| Componente           | Descrição                            |
|----------------------|--------------------------------------|
| Memória              | 4 KB (4096 bytes)                    |
| Registradores        | 16 registradores de 8 bits (V0–VF)   |
| Registrador I        | Registrador de índice de 16 bits     |
| Contador de programa | 16 bits, começa em `0x200`           |
| Pilha                | 16 níveis de profundidade            |
| Tela                 | 64 × 32 pixels (monocromático)       |
| Temporizadores       | Delay timer e sound timer (60 Hz)    |
| Teclado              | 16 teclas hexadecimais (0–F)         |

---

## Versão C (básica)

Localizada em `C/`, contém os seguintes arquivos:

| Arquivo     | Descrição                                     |
|-------------|-----------------------------------------------|
| `chip8.h`   | Definição da struct `VM` e protótipos         |
| `chip8.c`   | Implementação parcial das instruções          |
| `defs.h`    | Definições auxiliares                         |
| `main.c`    | Ponto de entrada – carrega e executa uma ROM  |

### Compilação (versão C)

```bash
cd C
gcc -o chip8 main.c chip8.c
```

### Uso (versão C)

```bash
./chip8 <arquivo_rom>
```

---

## Versão C++ com SDL2 (completa)

Localizada em `C++/`, contém uma implementação completa do conjunto de instruções CHIP-8 com renderização gráfica.

### Estrutura

```
C++/
├── src/
│   ├── main.cpp         # Ponto de entrada
│   ├── chip8.cpp        # Implementação da CPU CHIP-8
│   └── chip8_sdl.cpp    # Loop principal SDL2 (renderização e entrada)
├── lib/
│   ├── chip8.hpp        # Declaração da classe Chip8
│   └── defs.hpp         # Definições auxiliares
├── ROMs/                # ROMs para teste
├── c8games/             # Jogos CHIP-8
└── Makefile             # Sistema de build
```

### Dependências

- Compilador C++17 (`g++`)
- [SDL2](https://www.libsdl.org/)

**Instalação do SDL2 no Linux (Ubuntu/Debian):**
```bash
sudo apt install libsdl2-dev
```

**Instalação do SDL2 no macOS:**
```bash
brew install sdl2
```

### Compilação (versão C++)

```bash
cd C++
make
```

### Uso (versão C++)

```bash
./chip8 <arquivo_rom> <fps> <escala>
```

| Parâmetro       | Descrição                                                        |
|-----------------|------------------------------------------------------------------|
| `<arquivo_rom>` | Caminho para o arquivo de ROM CHIP-8 (`.ch8`)                    |
| `<fps>`         | Velocidade da CPU em Hz (instruções por segundo). Ex: `600`      |
| `<escala>`      | Fator de escala da janela. Ex: `10` resulta em janela 640×320    |

**Exemplo:**
```bash
./chip8 c8games/PONG 600 10
```

Também é possível usar o Makefile:
```bash
make run-rom ROM=c8games/PONG
```

### Limpeza

```bash
make clean
```

---

## Mapeamento de Teclado

O CHIP-8 utiliza 16 teclas hexadecimais. O mapeamento para o teclado moderno é:

| CHIP-8 | Teclado |   | CHIP-8 | Teclado |
|--------|---------|---|--------|---------|
| `1`    | `1`     |   | `2`    | `2`     |
| `3`    | `3`     |   | `C`    | `4`     |
| `4`    | `Q`     |   | `5`    | `W`     |
| `6`    | `E`     |   | `D`    | `R`     |
| `7`    | `A`     |   | `8`    | `S`     |
| `9`    | `D`     |   | `E`    | `F`     |
| `A`    | `Z`     |   | `0`    | `X`     |
| `B`    | `C`     |   | `F`    | `V`     |

Pressione **Esc** para sair.

---

## Instruções implementadas

A versão C++ implementa o conjunto completo de instruções CHIP-8:

| Opcode   | Mnemônico         | Descrição                                      |
|----------|-------------------|------------------------------------------------|
| `00E0`   | CLS               | Limpa a tela                                   |
| `00EE`   | RET               | Retorna de sub-rotina                          |
| `1NNN`   | JP addr           | Salta para endereço NNN                        |
| `2NNN`   | CALL addr         | Chama sub-rotina em NNN                        |
| `3XNN`   | SE Vx, byte       | Pula se Vx == NN                               |
| `4XNN`   | SNE Vx, byte      | Pula se Vx != NN                               |
| `5XY0`   | SE Vx, Vy         | Pula se Vx == Vy                               |
| `6XNN`   | LD Vx, byte       | Carrega NN em Vx                               |
| `7XNN`   | ADD Vx, byte      | Vx = Vx + NN                                   |
| `8XY0-E` | Aritm./Lógica     | Operações entre registradores                  |
| `9XY0`   | SNE Vx, Vy        | Pula se Vx != Vy                               |
| `ANNN`   | LD I, addr        | I = NNN                                        |
| `BNNN`   | JP V0, addr       | Salta para NNN + V0                            |
| `CXNN`   | RND Vx, byte      | Vx = rand() & NN                               |
| `DXYN`   | DRW Vx, Vy, n     | Desenha sprite N bytes na posição (Vx, Vy)     |
| `EX9E`   | SKP Vx            | Pula se tecla Vx pressionada                   |
| `EXA1`   | SKNP Vx           | Pula se tecla Vx não pressionada               |
| `FX07`   | LD Vx, DT         | Vx = delay timer                               |
| `FX0A`   | LD Vx, K          | Aguarda tecla e armazena em Vx                 |
| `FX15`   | LD DT, Vx         | delay timer = Vx                               |
| `FX18`   | LD ST, Vx         | sound timer = Vx                               |
| `FX1E`   | ADD I, Vx         | I = I + Vx                                     |
| `FX29`   | LD F, Vx          | I aponta para sprite do dígito Vx              |
| `FX33`   | LD B, Vx          | Armazena BCD de Vx em I, I+1, I+2             |
| `FX55`   | LD [I], Vx        | Salva V0..Vx na memória a partir de I          |
| `FX65`   | LD Vx, [I]        | Lê memória a partir de I em V0..Vx             |

---

## Licença

Este projeto é de uso educacional e livre para estudo e modificação.
