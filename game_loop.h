#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "game_screen.h"
int game_loop(SDL_Renderer *renderer, GameState *game_state);
#endif