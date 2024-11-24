#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "menu_selector.h"
#include "render.h"

int game_loop(CommonRenderData rd, GameState *game_state);
#endif