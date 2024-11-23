#ifndef GAME_OVER_LOOP_H
#define GAME_OVER_LOOP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include "game_screen.h"

int game_over_loop(SDL_Renderer *renderer, TTF_Font *font, ScoreData score_data, int width, int height);
#endif