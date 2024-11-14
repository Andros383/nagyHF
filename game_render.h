#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "game_screen.h"

void render_game(SDL_Renderer *renderer, GameState *game_state);
void render_animation_block(SDL_Renderer *renderer, GameState *game_state, int x, int y, double radius_mult);
#endif