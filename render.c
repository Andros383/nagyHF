
#include "render.h"

#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debugmalloc.h"
#include "menu_selector.h"

// kiszámítja a rajzoláshoz gyakran használt adatokat
CommonRenderData init_common_render_data(SDL_Renderer *renderer, TTF_Font *font, GameState *game_state) {
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    // tábla szélessége
    rd.board_width = game_state->board_width;
    rd.board_height = game_state->board_height;

    // egy blokk mérete pixelben a maximális szélesség és magasság alapján
    int proposed_size_width = 500 / rd.board_width;
    int proposed_size_height = 800 / rd.board_height;

    // a kettő közül a kisebb
    rd.block_size = proposed_size_width < proposed_size_height ? proposed_size_width : proposed_size_height;

    // tábla méretei pixelben
    rd.board_width_px = rd.board_width * rd.block_size;
    rd.board_height_px = rd.board_height * rd.block_size;

    // tábla bal alsó sarkának koordinátái pixelben
    rd.origin_x = WINDOW_WIDTH / 2 - rd.board_width_px / 2;
    rd.origin_y = WINDOW_HEIGHT / 2 + rd.board_height_px / 2;

    return rd;
}

// visszaadja a blokk színének RGBA értékeit
static SDL_Color get_color_from_block(Block block) {
    switch (block) {
        case RED:
            return COLOR_RED;
            break;
        case GREEN:
            return COLOR_GREEN;
            break;
        case BLUE:
            return COLOR_BLUE;
            break;
        case YELLOW:
            return COLOR_YELLOW;
            break;
        case EMPTY:
            return COLOR_GRAY;
            break;
        default:
            // átlátszó ha érvénytelen lenne
            return COLOR_TRANSPARENT;
            break;
    }
}

// lerajzol a táblára egy blokkot az (x, y) cellába a megadott méret szorzóval
// méret szorzó az animációhoz kell
void render_block_on_board(CommonRenderData rd, Block block, int x, int y, double radius_mult) {
    SDL_Color color = get_color_from_block(block);
    int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
    int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
    int radius = (rd.block_size / 2) * 0.9 * radius_mult;
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius, 0, 0, 0, 255);
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius - 1, color.r, color.g, color.b, color.a);
}

// kirajzolja a táblát háttérrel, rácsvonalakkal, és a már lerakott részekkel
void render_board(CommonRenderData rd, Block **board) {
    SDL_Renderer *renderer = rd.renderer;

    // tábla kerete, mert nincs thickRectangle
    boxRGBA(renderer, rd.origin_x - 3, rd.origin_y + 3, rd.origin_x + rd.board_width_px + 3, rd.origin_y - rd.board_height_px - 3, 0, 0, 0, 255);
    // tábla háttere -> minesweeper szürke
    boxRGBA(renderer, rd.origin_x, rd.origin_y, rd.origin_x + rd.board_width_px, rd.origin_y - rd.board_height_px, 188, 188, 188, 255);

    // rács kirajzolása
    for (int x = 1; x <= rd.board_width - 1; x++) {
        lineRGBA(renderer, rd.origin_x + rd.block_size * x, rd.origin_y, rd.origin_x + rd.block_size * x, rd.origin_y - rd.board_height_px, 0, 0, 0, 127);
    }

    for (int y = 1; y <= rd.board_height - 1; y++) {
        lineRGBA(renderer, rd.origin_x, rd.origin_y - rd.block_size * y, rd.origin_x + rd.board_width_px, rd.origin_y - rd.block_size * y, 0, 0, 0, 127);
    }

    // lerakott blokkok rajzolása

    // fekete alapozókörök
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            if (board[x][y] != EMPTY) {
                int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
                int radius = (rd.block_size / 2) * 0.9;
                filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius, 0, 0, 0, 255);
            }
        }
    }

    int connector_thickness = rd.block_size / 4;
    // legyen páros, hogy jól nézzen ki
    if (connector_thickness % 2 != 0) connector_thickness++;
    // vízszintes összekötővonalak
    for (int x = 0; x < rd.board_width - 1; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            // ha a jelenlegi és a tőle jobbra lévő megegyezik, de nem EMPTY
            if (board[x][y] == board[x + 1][y] && board[x][y] != EMPTY) {
                int mid_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int mid_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;

                // alapozás
                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x + rd.block_size, mid_y, connector_thickness, 0, 0, 0, 255);
                SDL_Color color = get_color_from_block(board[x][y]);
                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x + rd.block_size, mid_y, connector_thickness - 2, color.r, color.g, color.b, color.a);
            }
        }
    }

    // függőleges összekötővonalak
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height - 1; y++) {
            if (board[x][y] == board[x][y + 1] && board[x][y] != EMPTY) {
                int mid_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int mid_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;

                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x, mid_y - rd.block_size, connector_thickness, 0, 0, 0, 255);

                SDL_Color color = get_color_from_block(board[x][y]);
                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x, mid_y - rd.block_size, connector_thickness - 2, color.r, color.g, color.b, color.a);
            }
        }
    }

    // körök kitöltése
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            if (board[x][y] != EMPTY) {
                int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
                int radius = (rd.block_size / 2) * 0.9;

                SDL_Color color = get_color_from_block(board[x][y]);
                filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius - 1, color.r, color.g, color.b, color.a);
            }
        }
    }
}

// rajzol egy szövegdobozt outline_color színű kerettel
void render_text_block(CommonRenderData rd, char *text, int x, int y, SDL_Color outline_color) {
    // Source: InfoC https://infoc.eet.bme.hu/sdl/#5
    TTF_Font *font = rd.font;

    SDL_Rect hova = {10, 10, 0, 0};
    SDL_Surface *felirat = TTF_RenderUTF8_Shaded(font, text, (SDL_Color){0, 0, 0}, (SDL_Color){255, 255, 255});
    SDL_Texture *felirat_t = SDL_CreateTextureFromSurface(rd.renderer, felirat);
    hova.x = x;
    hova.y = y;
    hova.w = felirat->w;
    hova.h = felirat->h;

    boxRGBA(rd.renderer, hova.x - 2, hova.y - 2, hova.x + hova.w + 1, hova.y + hova.h + 1, outline_color.r, outline_color.g, outline_color.b, outline_color.a);

    // debug: kiírrja a kiírt szöveg pozícióját, szélességét, magasságát
    // printf("%s:\n %d %d %d %d -> %d\n", text, hova.x, hova.y, hova.x + hova.w, hova.y + hova.h, hova.w);

    SDL_RenderCopy(rd.renderer, felirat_t, NULL, &hova);
    SDL_FreeSurface(felirat);
    SDL_DestroyTexture(felirat_t);
}

// megrajzolja a következő részeket
static void render_queue(CommonRenderData rd, GameState *game_state) {
    // átméretezésnél is maradjon hasonló helyen
    int block_size = 80;

    // soron következő részek kirajzolása
    int top_left_x = rd.origin_x + rd.board_width_px + block_size / 2;
    int top_left_y = rd.origin_y - rd.board_height_px + block_size / 2;

    // blokkok színezésére használt változó
    SDL_Color color;

    // kézzel állított értékek

    // első rész
    // keret
    boxRGBA(rd.renderer, top_left_x, top_left_y, top_left_x + block_size, top_left_y + block_size * 2, 0, 0, 0, 255);
    boxRGBA(rd.renderer, top_left_x + 2, top_left_y + 2, top_left_x + block_size - 2, top_left_y + block_size * 2 - 2, 255, 255, 255, 255);

    // felső
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8, 0, 0, 0, 255);
    color = get_color_from_block(game_state->queue[0].block1);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8 - 1, color.r, color.g, color.b, color.a);

    // alsó
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8, 0, 0, 0, 255);
    color = get_color_from_block(game_state->queue[0].block2);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8 - 1, color.r, color.g, color.b, color.a);

    // második rész
    // lejjebb van
    top_left_y += block_size * 2.5;

    boxRGBA(rd.renderer, top_left_x, top_left_y, top_left_x + block_size, top_left_y + block_size * 2, 0, 0, 0, 255);
    boxRGBA(rd.renderer, top_left_x + 2, top_left_y + 2, top_left_x + block_size - 2, top_left_y + block_size * 2 - 2, 255, 255, 255, 255);

    // felső
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8, 0, 0, 0, 255);
    color = get_color_from_block(game_state->queue[1].block1);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8 - 1, color.r, color.g, color.b, color.a);

    // alsó
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8, 0, 0, 0, 255);
    color = get_color_from_block(game_state->queue[1].block2);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8 - 1, color.r, color.g, color.b, color.a);
}

void render_game(CommonRenderData rd, GameState *game_state) {
    SDL_Renderer *renderer = rd.renderer;

    // fehér háttér, csak ott, ahol a játék játszódik, hogy a gombokat ne rajzolja felül
    // game_setup a fehér hátteret rakni az egész ablakra
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH - 350, WINDOW_HEIGHT, 255, 255, 255, 255);

    // tábla kirajzolása: csak a már lerakott részek
    render_board(rd, game_state->board);

    // jelenleg aktív rész kirajzolása
    Piece ac = game_state->active_piece;
    // csak akkor rajzolja ki, ha nem invalid
    if (!(ac.x1 == -1 || ac.y1 == -1 || ac.x2 == -1 || ac.y2 == -1)) {
        render_block_on_board(rd, ac.block1, ac.x1, ac.y1, 1.0);
        render_block_on_board(rd, ac.block2, ac.x2, ac.y2, 1.0);
    }

    // soron következő részek kirajzolása
    render_queue(rd, game_state);

    // pontszámok kirajzolása
    char label[50 + 1];
    sprintf(label, "%010d", game_state->score_data.score);
    render_text_block(rd, label, 720, rd.origin_y + 10, COLOR_TRANSPARENT);
    // 720 középen van az általános szöveghosszokra

    sprintf(label, "Leghosszabb lánc: %4d", game_state->score_data.longest_chain);
    render_text_block(rd, label, 100, 750, COLOR_TRANSPARENT);

    sprintf(label, "Lerakott részek: %5d", game_state->score_data.placed_pieces);
    render_text_block(rd, label, 100, 800, COLOR_TRANSPARENT);

    SDL_RenderPresent(renderer);
}
