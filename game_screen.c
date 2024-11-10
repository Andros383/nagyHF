#include "game_screen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game_loop.h"

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
            fprintf(stderr, "Sikertelen memoriafoglalas @ game_state->board[i]\n");
            // TODO ezen a ponton az előző sikeres foglalások le vannak foglalva, kezelni kell?
            // TODO IGEN, kezelni kell
            return false;
        }

        // Nullázás
        for (int y = 0; y < board_height; y++) {
            game_state->board[x][y] = EMPTY;
        }

        // TODO ez az indexelés a jó? mmint op precedence
        // elm igen, de -> és a [] egy szinten van
        // https://en.cppreference.com/w/c/language/operator_precedence
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
void game_setup(SDL_Renderer *renderer, int board_width, int board_height) {
    // TODO ez majd egy nagyobb loop, mindig a game_loop-ot indítja, return code alapján vált képernyőt
    // többi menüből kilépve mindig újraindítja a game_loop-ot, lefoglalja a dolgokat.

    srand(time(NULL));  // randomizálás beállítása
    GameState game_state;

    bool success = init_game_state(&game_state, board_width, board_height);

    if (success) {
        int return_code = game_loop(renderer, &game_state);
#define TESZT
#ifdef TESZT
        printf("return_code: %d\n", return_code);
#endif
    }

    // tábla felszabadítása
    free_game_state(&game_state, board_width, board_height);
}