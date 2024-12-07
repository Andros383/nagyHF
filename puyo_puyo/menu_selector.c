#include "menu_selector.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "debugmalloc.h"
#include "game_loop.h"
#include "game_over_loop.h"
#include "leaderboard_loop.h"
#include "render.h"
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
    // játéktábla lefoglalása
    game_state->board = (Block **)malloc(board_width * sizeof(Block *));
    if (game_state->board == NULL) {
        printf("init_game_state: sikertelen memoriafoglalas\n");
        return false;
    }

    for (int x = 0; x < board_width; x++) {
        game_state->board[x] = (Block *)malloc(board_height * sizeof(Block));
        if (game_state->board[x] == NULL) {
            printf("init_game_state: sikertelen memoriafoglalas\n");
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

    // tábla nullázása
    for (int x = 0; x < board_width; x++) {
        for (int y = 0; y < board_height; y++) {
            game_state->board[x][y] = EMPTY;
        }
    }
    // magasság és szélesség beállítása
    game_state->board_width = board_width;
    game_state->board_height = board_height;

    // aktív rész generálása
    game_state->active_piece = gen_rand_piece();

    // aktív rész koordinátáit be kell állítani
    game_state->active_piece.x1 = board_width / 2;
    game_state->active_piece.x2 = board_width / 2;
    game_state->active_piece.y1 = board_height - 1;
    game_state->active_piece.y2 = board_height - 2;

    // soron következő részek generálása
    game_state->queue[0] = gen_rand_piece();
    game_state->queue[1] = gen_rand_piece();

    // pontszámok nullázása
    game_state->score_data.longest_chain = 0;
    game_state->score_data.placed_pieces = 0;
    game_state->score_data.score = 0;

    return true;
}
// felszabadítja a játékállást, pontosabban a játéktáblát
static void free_game_state(GameState *game_state, int board_width, int board_height) {
    for (int i = 0; i < board_width; i++) {
        free(game_state->board[i]);
    }
    free(game_state->board);
}
// felállítja a játéktáblát, valamint az első képernyőt kirajzolja
static int game_setup(SDL_Renderer *renderer, TTF_Font *font, int board_width, int board_height, ScoreData *score_data) {
    srand(time(NULL));  // randomizálás beállítása

    // játékállás felvétele
    GameState game_state;

    bool success = init_game_state(&game_state, board_width, board_height);
    if (!success) return -1;

    // rajzoláshoz szükséges adatok
    CommonRenderData rd = init_common_render_data(renderer, font, &game_state);

    // első képernyő megrajzolása
    // fehér háttér
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 255, 255, 255, 255);

    // menügombok
    render_text_block(rd, "Beállítások", 1300, 500, COLOR_BLACK);
    render_text_block(rd, "Ranglista", 1320, 600, COLOR_BLACK);

    // játékállás kirajzolása
    render_game(rd, &game_state);

    // játék indítása
    int return_code = game_loop(rd, &game_state);

    // a pontszámok kimentése a fő függvénybe
    *score_data = game_state.score_data;

    // játékállás (tábla) felszabadítása
    free_game_state(&game_state, board_width, board_height);

    return return_code;
}

// különböző képernyők közötti választás return_code alapján
// -1: kilépés hiba miatt
// 0:  kilépés X-el
// 1:  játékképernyő
// 2:  beállítások
// 3:  ranglista
// 4:  játék vége képernyő
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
                // ha valami ismeretlen kódot kap, jelzi
                printf("menu_selector_loop: ismeretlen visszateresi kod\n");
                return_code = -1;
                break;
        }
    }

    if (return_code == -1)
        printf("Hiba tortent, program leallitasa.\n");
    else
        printf("A program hiba nelkul kilepett.\n");
}