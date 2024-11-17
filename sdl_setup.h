#ifndef SDL_SETUP_H
#define SDL_SETUP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

void sdl_init(int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **font);

#endif