#include <SDL2/SDL.h>

#define FPS_ALVO 60
#define TEMPO_FRAME (1000/FPS_ALVO)

int main(int argc, char** argv){
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* janela = SDL_CreateWindow(
        "Exemplo SDL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        janela, -1, SDL_RENDERER_ACCELERATED
    );

    SDL_Event evento;
    int executando = 1;

    int posx = 380;
    int posy = 280;
    int velocidade = 5;

    while (executando)
    {
        Uint32 t_inicio = SDL_GetTicks();
        while (SDL_PollEvent(&evento))
        {
            if(evento.type == SDL_QUIT){
                executando = 0;
            }
            if (evento.type == SDL_KEYDOWN)
            {
                if (evento.key.keysym.sym == SDLK_ESCAPE)
                {
                    executando = 0;
                }
                
            }
            
        }

        const Uint8* teclado = SDL_GetKeyboardState(NULL);

        if(teclado[SDL_SCANCODE_UP])
            posy -= 5;
        if (teclado[SDL_SCANCODE_DOWN])
            posy += 5;
        if (teclado[SDL_SCANCODE_LEFT])
            posx -= 5;
        if (teclado[SDL_SCANCODE_RIGHT])
            posx += 5;

        if(posx < 0)
            posx = 0;
        if (posy < 0)
            posy = 0;
        if (posx + 40 > 800)
            posx = 760;
        if (posy + 40 > 600)
            posy = 560;

        SDL_SetRenderDrawColor(renderer,
            0,0,0,255);
        SDL_RenderClear(renderer);

        SDL_Rect caixa = {posx, posy, 40, 40};

        SDL_SetRenderDrawColor(renderer, 
        255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &caixa);

        SDL_RenderPresent(renderer);

        int duracao = SDL_GetTicks() - t_inicio;
        if (duracao < TEMPO_FRAME)
        {
            SDL_Delay(TEMPO_FRAME - duracao);
        }
        
        
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(janela);
    SDL_Quit();
    
}