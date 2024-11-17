#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include "game_screen.h"
// alapvető infók amik általában kellenek minden komponens rajzolásához
typedef struct CommonRenderData {
    SDL_Renderer *renderer;
    TTF_Font *font;

    int board_width, board_height;
    int board_width_px, board_height_px;
    int origin_x, origin_y;
    int block_size;
} CommonRenderData;

CommonRenderData init_common_render_data(SDL_Renderer *renderer, TTF_Font *font, GameState *game_state);
void render_game(CommonRenderData rd, GameState *game_state);
void render_animation_block(CommonRenderData rd, int x, int y, double radius_mult);
void render_text_block(CommonRenderData rd, char *text, int x, int y, Uint32 outline_color);
#endif