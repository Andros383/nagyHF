#include "sdl_setup.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

// source : infoc
/* ablak megnyitasa */
void sdl_init(int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **font) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow("SDL peldaprogram", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szeles, magas, 0);
    if (window == NULL) {
        SDL_Log("Nem hozhato letre az ablak: %s", SDL_GetError());
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Nem hozhato letre a megjelenito: %s", SDL_GetError());
        exit(1);
    }

    // szöveg kiírásához előkészület
    TTF_Init();
    *font = TTF_OpenFont("fonts/CONSOLAB.TTF", 32);

    // TODO render törlése, lehet nem kell majd
    SDL_RenderClear(renderer);

    *pwindow = window;
    *prenderer = renderer;
}
