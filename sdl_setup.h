#ifndef SDL_SETUP_H
#define SDL_SETUP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// játékos nevének maximum hossza, az input_text függvénynek kell
#define MAX_NAME_LEN (20 + 1)

void sdl_init(int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **font);
int input_text(char *dest, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, TTF_Font *font, SDL_Renderer *renderer);
void sdl_close(SDL_Window **pwindow, SDL_Renderer **prenderer, TTF_Font **pfont);
#endif