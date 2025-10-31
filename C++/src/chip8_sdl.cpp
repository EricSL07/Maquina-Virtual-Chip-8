#include "../lib/chip8.hpp"
// Mapeia SDL_Keycode para tecla CHIP-8 (0x0 a 0xF)
static int mapear_tecla_sdl_para_chip8(SDL_Keycode k)
{
    switch (k)
    {
    case SDLK_x:
        return 0x0;
    case SDLK_1:
        return 0x1;
    case SDLK_2:
        return 0x2;
    case SDLK_3:
        return 0x3;
    case SDLK_q:
        return 0x4;
    case SDLK_w:
        return 0x5;
    case SDLK_e:
        return 0x6;
    case SDLK_a:
        return 0x7;
    case SDLK_s:
        return 0x8;
    case SDLK_d:
        return 0x9;
    case SDLK_z:
        return 0xA;
    case SDLK_c:
        return 0xB;
    case SDLK_4:
        return 0xC;
    case SDLK_r:
        return 0xD;
    case SDLK_f:
        return 0xE;
    case SDLK_v:
        return 0xF;
    default:
        return -1;
    }
}

// Atualiza a textura 64x32 a partir do array DISPLAY (formato RGBA8888)
static void atualizar_textura_de_tela(SDL_Texture *tex, const uint8_t display[])
{
    const int width = 64, height = 32;
    uint32_t pixels[width * height];
    const uint32_t on = 0xFFFFFFFF;  // white
    const uint32_t off = 0x000000FF; // black

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            pixels[y * width + x] = display[y * width + x] ? on : off;
        }
    }
    SDL_UpdateTexture(tex, NULL, pixels, width * sizeof(uint32_t));
}

void rodar_loop_sdl(Chip8 &vm)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init erro: %s\n", SDL_GetError());
        return;
    }

    const int escala = 20; // ajuste (10 -> janela 640x320)
    const int largura_janela = 64 * escala;
    const int altura_janela = 32 * escala;

    SDL_Window *window = SDL_CreateWindow("CHIP-8",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          largura_janela, altura_janela, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow erro: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer erro: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Texture *tex = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                         64, 32);

    bool executando = true;
    SDL_Event event;

    const int hz_cpu = 500;                                  // objetivo de instruções por segundo (ajuste 400..800)
    const double ciclos_por_frame_d = double(hz_cpu) / 60.0; // média de instruções por frame
    double acumulador_ciclos = 0.0;
    const uint32_t ms_por_frame = 1000 / 60; // 60Hz

    while (executando)
    {
        Uint32 t_inicio = SDL_GetTicks();

        // events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                executando = false;
                break;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    executando = false;
                    break;
                }

                int mapeado = mapear_tecla_sdl_para_chip8(event.key.keysym.sym);
                if (mapeado >= 0)
                {
                    if (event.type == SDL_KEYDOWN)
                    {
                        vm.teclas[mapeado] = 1;
                        if (vm.aguardando_tecla)
                        {
                            vm.registradores[vm.reg_aguardando_tecla] = mapeado;
                            vm.aguardando_tecla = false;
                            vm.pc += 2; // avança a instrução FX0A
                        }
                    }
                    else
                    {
                        vm.teclas[mapeado] = 0;
                    }
                }
            }
        }

        // calcular quantas instruções executar neste frame (acumulador para fracionário)
        acumulador_ciclos += ciclos_por_frame_d;
        int a_executar = (int)acumulador_ciclos;
        if (a_executar <= 0)
            a_executar = 1;
        acumulador_ciclos -= a_executar;

        for (int i = 0; i < a_executar; ++i)
        {
            vm.VM_ExecutarInstrucao();
        }

        // atualiza timers e redesenha (uma vez por frame)
        vm.tickTimers();
        if (vm.deveDesenhar)
        {
            atualizar_textura_de_tela(tex, vm.tela);
            SDL_RenderClear(renderer);
            SDL_Rect dst = {0, 0, largura_janela, altura_janela};
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_RenderPresent(renderer);
            vm.deveDesenhar = false;
        }

        Uint32 duracao = SDL_GetTicks() - t_inicio;
        if (duracao < ms_por_frame)
            SDL_Delay(ms_por_frame - duracao);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
