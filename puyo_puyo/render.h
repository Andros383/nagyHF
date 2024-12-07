#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include "menu_selector.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1000

// színek definiálása
// valójában nem az SDL_Color típussal rajzolom ki, csak azzal veszem át az adatot a függvényekben
#define COLOR_BLACK \
    (SDL_Color) { 0, 0, 0, 255 }
#define COLOR_TRANSPARENT \
    (SDL_Color) { 0, 0, 0, 0 }
#define COLOR_RED \
    (SDL_Color) { 255, 0, 0, 255 }
#define COLOR_GREEN \
    (SDL_Color) { 0, 255, 0, 255 }
#define COLOR_BLUE \
    (SDL_Color) { 0, 0, 255, 255 }
#define COLOR_YELLOW \
    (SDL_Color) { 255, 255, 0, 255 }
#define COLOR_WHITE \
    (SDL_Color) { 255, 255, 255, 255 }
#define COLOR_GRAY \
    (SDL_Color) { 188, 188, 188, 255 }

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
void render_text_block(CommonRenderData rd, char *text, int x, int y, SDL_Color outline_color);
void render_board(CommonRenderData rd, Block **board);
void render_block_on_board(CommonRenderData rd, Block block, int x, int y, double radius_mult);
#endif