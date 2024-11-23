#include "game_screen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "debugmalloc.h"
#include "game_loop.h"
#include "game_over_loop.h"
#include "game_render.h"
#include "leaderboard_loop.h"
#include "settings_loop.h"

// visszaad egy random nem üres blokkot
static Block gen_rand_block() {
    switch (rand() % 4) {
        case 0:
            return RED;
            break;
        case 1:
            return GREEN;
            break;
        case 2:
            return BLUE;
            break;
        case 3:
            return YELLOW;
            break;
        default:
            // ide nem juthat el
            fprintf(stderr, "Random block generated wrong value.");
            return EMPTY;
            break;
    }
}

// public, mert kell a locknak
// generál egy random részt, invalid koordinátákkal
Piece gen_rand_piece() {
    Piece p;
    p.block1 = gen_rand_block();
    p.block2 = gen_rand_block();

    // invalid koordináták, mert a felhasználó mondja meg
    // nem állítja be, mert át kéne venni a magasságot
    p.x1 = -1;
    p.x2 = -1;
    p.y1 = -1;
    p.y2 = -1;

    return p;
}

// beállítja a beadott game_state-et egy alapállásra
// minden számolt pont 0, az aktív rész és a queue már helyesen vannak kitöltve
// visszaadja, hogy sikeres volt-e
static bool init_game_state(GameState *game_state, int board_width, int board_height) {
    game_state->board = (Block **)malloc(board_width * sizeof(Block *));
    if (game_state->board == NULL) {
        fprintf(stderr, "Sikertelen memoriafoglalas @ game_state->board\n");
        return false;
    }

    for (int x = 0; x < board_width; x++) {
        game_state->board[x] = (Block *)malloc(board_height * sizeof(Block));
        if (game_state->board[x] == NULL) {
            // TODO stderr-re kell majd kirakni?
            fprintf(stderr, "Sikertelen memoriafoglalas @ game_state->board[i]\n");

            // előző oszlopok felszabadítása
            for (int i = 0; i < x; i++) {
                free(game_state->board[i]);
            }
            // egész tábla felszabadítása
            free(game_state->board);

            // nem sikerült inicializálni
            return false;
        }
    }

    // Nullázás
    for (int x = 0; x < board_width; x++) {
        for (int y = 0; y < board_height; y++) {
            game_state->board[x][y] = EMPTY;
        }
    }

    game_state->board_width = board_width;
    game_state->board_height = board_height;

    // random rész generálása
    game_state->active_piece = gen_rand_piece();

    // aktív rész koordinátáit be kell állítani
    game_state->active_piece.x1 = board_width / 2;
    game_state->active_piece.x2 = board_width / 2;
    game_state->active_piece.y1 = board_height - 1;
    game_state->active_piece.y2 = board_height - 2;

    // queue generálása
    game_state->queue[0] = gen_rand_piece();
    game_state->queue[1] = gen_rand_piece();

    game_state->score_data.longest_chain = 0;
    game_state->score_data.placed_pieces = 0;
    game_state->score_data.score = 0;

    // sikeres inicializálás
    return true;
}

static void free_game_state(GameState *game_state, int board_width, int board_height) {
    for (int i = 0; i < board_width; i++) {
        free(game_state->board[i]);
        // TODO ez az indexelés a jó? mmint op precedence
        // elm igen, de -> és a [] egy szinten van
        // https://en.cppreference.com/w/c/language/operator_precedence
    }
    free(game_state->board);
}
// TODO más a neve, mást csinál
// start_game_loop?
int game_setup(SDL_Renderer *renderer, TTF_Font *font, int board_width, int board_height, ScoreData *score_data) {
    // TODO vagy a többi init függvényét is itt megcsinálni, vagy ezt az initet is a game_loop.c-be rakni

    // TODO ez majd egy nagyobb loop, mindig a game_loop-ot indítja, return code alapján vált képernyőt
    // többi menüből kilépve mindig újraindítja a game_loop-ot, lefoglalja a dolgokat.

    srand(time(NULL));  // randomizálás beállítása

    // TODO pontosan a közepére, a renderben van debug rá

    GameState game_state;
    bool success = init_game_state(&game_state, board_width, board_height);

    if (!success) return -1;

    // rajzoláshoz szükséges adatok
    CommonRenderData rd = init_common_render_data(renderer, font, &game_state);

    // első állapot megrajzolása
    // TODO width és height behozása mind glob konstans
    // fehér háttér
    boxRGBA(renderer, 0, 0, 1600, 1000, 255, 255, 255, 255);

    // menügombok
    render_text_block(rd, "Beállítások", 1300, 500, COLOR_BLACK);
    render_text_block(rd, "Ranglista", 1320, 600, COLOR_BLACK);

    // játékállás kirajzolása
    render_game(rd, &game_state);
    int return_code = game_loop(rd, &game_state);

    // a pontszámok kimentése a fő függvénybe
    *score_data = game_state.score_data;

    // játékállás (tábla) felszabadítása
    free_game_state(&game_state, board_width, board_height);

    return return_code;
}

void menu_selector_loop(SDL_Renderer *renderer, TTF_Font *font) {
    int board_height = 12;
    int board_width = 6;
    // a game_setup függvény a játékból kilépéskor átmásolja ide a pontszám adatokat
    ScoreData score_data;

    int return_code = 1;

    while (!(return_code == 0 || return_code == -1)) {
        switch (return_code) {
            case 1:
                return_code = game_setup(renderer, font, board_width, board_height, &score_data);
                break;
            case 2:
                return_code = settings_loop(renderer, font, &board_width, &board_height);
                break;
            case 3:
                return_code = leaderboard_loop(renderer, font);
                break;
            case 4:
                return_code = game_over_loop(renderer, font, score_data, board_width, board_height);
                break;
            default:
                // ha valami ismeretlen kódot kap, újraindítja a játékot
                return_code = 1;
                break;
        }
    }

    if (return_code == -1) printf("Varatlan hiba tortent.");
}