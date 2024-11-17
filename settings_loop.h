#ifndef SETTINGS_LOOP_H
#define SETTINGS_LOOP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

int settings_loop(SDL_Renderer *renderer, TTF_Font *font, int *board_width, int *board_height);

#endif