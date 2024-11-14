
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game_screen.h"

// TODO bevinni az sdl-es import file-ba, mint globál konstans
const int WINDOW_WIDTH_2 = 1000;
const int WINDOW_HEIGHT_2 = 1000;

// TODO ez dinamikus legyen majd?
const int BLOCK_SIZE = 70;
// megjegyzés: y trükkös, felülről számolja az SDL

// TODO átvinni game_render.c-be

// TODO ezt lehetne jobban?
// vagy akár rosszabban: belerakni a block_size-ot, hogy lehessen dinamikusan állítani

// alapvető infók amik általában kellenek minden komponens rajzolásához
typedef struct CommonRenderData {
    int board_width, board_height;
    int board_width_px, board_height_px;
    int origin_x, origin_y;
} CommonRenderData;

// kiszámítja a rajzoláshoz gyakran használt adatokat
CommonRenderData generate_common_render_data(GameState *game_state) {
    CommonRenderData render_data;
    // tábla szélessége
    render_data.board_width = game_state->board_width;
    render_data.board_height = game_state->board_height;

    // tábla szélessége pixelben
    render_data.board_width_px = render_data.board_width * BLOCK_SIZE;
    render_data.board_height_px = render_data.board_height * BLOCK_SIZE;

    // tábla bal alsó sarkának koordinátái pixelben
    render_data.origin_x = WINDOW_HEIGHT_2 / 2 - render_data.board_width_px / 2;
    render_data.origin_y = WINDOW_HEIGHT_2 / 2 + render_data.board_height_px / 2;

    return render_data;
}
// lerajzol a táblára egy blokkot x y cellába
static void render_block(SDL_Renderer *renderer, CommonRenderData rd, Block block, int x, int y) {
    Block current_block = block;
    // ha empty, nincs renderelés
    if (current_block == EMPTY) return;

    int r = 0, g = 0, b = 0;
    switch (block) {
        case RED:
            r = 255;
            break;

        case GREEN:
            g = 255;
            break;

        case BLUE:
            b = 255;
            break;

        case YELLOW:
            r = 255;
            g = 255;
            break;

        default:
            // mindent megnéztünk
            printf("Invalid block state in 'render_block'!");
            break;
    }
    int circle_middle_x = (rd.origin_x + BLOCK_SIZE / 2) + BLOCK_SIZE * x;
    int circle_middle_y = (rd.origin_y - BLOCK_SIZE / 2) - BLOCK_SIZE * y;
    int radius = (BLOCK_SIZE / 2) * 0.9;  // TODO wild casting kiszedése
    filledCircleRGBA(renderer, circle_middle_x, circle_middle_y, radius, r, g, b, 255);
}

// kirajzolja a táblát háttérrel, rácsvonalakkal, és a már lerakott részekkel
static void render_board(SDL_Renderer *renderer, CommonRenderData rd, Block **board) {
    // tábla háttere
    boxRGBA(renderer, rd.origin_x, rd.origin_y, rd.origin_x + rd.board_width_px, rd.origin_y - rd.board_height_px, 127, 127, 127, 255);

    // tényleges tartalom kirajzolása
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            render_block(renderer, rd, board[x][y], x, y);
        }
    }

    // rács kirajzolása
    for (int x = 1; x <= rd.board_width - 1; x++) {
        lineRGBA(renderer, rd.origin_x + BLOCK_SIZE * x, rd.origin_y, rd.origin_x + BLOCK_SIZE * x, rd.origin_y - rd.board_height_px, 0, 0, 0, 127);
    }

    for (int y = 1; y <= rd.board_height - 1; y++) {
        lineRGBA(renderer, rd.origin_x, rd.origin_y - BLOCK_SIZE * y, rd.origin_x + rd.board_width_px, rd.origin_y - BLOCK_SIZE * y, 0, 0, 0, 127);
    }

    // TODO összekötéseket a lerakott részek közt
}

// TODO GameState miért pointer? a többinek annak kéne lennie? Érték szerint fine átvenni, kb 30 byte
// Bár itt ezt változtatjuk
// A rajzolóknál érthető az érték szerinti, mert nem változtatjuk
void render_game(SDL_Renderer *renderer, GameState *game_state) {
    // TODO nem mindig kell újrarajzolni mindent. De majd csak akkor, ha optimalizálok?
    SDL_RenderClear(renderer);

    CommonRenderData render_data = generate_common_render_data(game_state);
    // fehár háttér
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH_2, WINDOW_HEIGHT_2, 255, 255, 255, 255);

    // tábla kirajzolása: csak a már lerakott részek
    render_board(renderer, render_data, game_state->board);

    // TODO score, chain, stb kirajzolása

    // jelenleg aktív rész kirajzolása
    Piece ac = game_state->active_piece;
    render_block(renderer, render_data, ac.block1, ac.x1, ac.y1);
    render_block(renderer, render_data, ac.block2, ac.x2, ac.y2);

    SDL_RenderPresent(renderer);
}

// TODO fura lehet hogy a renderer megjelenik a hard_drop-ban
// valójában csak a lockos függvényekben, ami így nem meglepő
// TODO ez így fura, hogy itt van?
// TODO nem örülök, hogy mik ennek a függvénynek az argumensei
// pl a game_state csak azért van benne, hogy a magasság kiolvasható legyen
// kirajzolja az animációhoz szükséges félkész blokkokat
void render_animation_block(SDL_Renderer *renderer, GameState *game_state, int x, int y, double radius_mult) {
    CommonRenderData rd = generate_common_render_data(game_state);
    // TODO ezt a szürkét blob változózni? majd a szépség runon
    int r = 127;
    int g = 127;
    int b = 127;
    int circle_middle_x = (rd.origin_x + BLOCK_SIZE / 2) + BLOCK_SIZE * x;
    int circle_middle_y = (rd.origin_y - BLOCK_SIZE / 2) - BLOCK_SIZE * y;
    int radius = (BLOCK_SIZE / 2) * 0.9 * radius_mult;  // TODO wild casting kiszedése? Itt is lehet kéne, amikor van radius_mult
    filledCircleRGBA(renderer, circle_middle_x, circle_middle_y, radius, r, g, b, 255);
}