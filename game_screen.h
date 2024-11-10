#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
// egy block a legkisebb rész, a táblán 1x1-es cella
// minden állapotát felveszi
typedef enum Block {
    RED,
    GREEN,
    BLUE,
    YELLOW,
    EMPTY
} Block;

// TODO 0-tól indexelni? x0, x1, vagy így érthetőbb

// egy rész, azaz két blokk
// ha az előre látható queue-ban van, akkor a koordináták -1
// a 2. blokk körül fog forogni
typedef struct Piece {
    Block block1;
    int x1, y1;
    Block block2;
    int x2, y2;
} Piece;

// a ranglistán a neven kívül megjelenő adatok
// a név nem szükséges, mert csak a játék végén kérjük el
typedef struct ScoreData {
    int score;
    int placed_pieces;
    int longest_chain;
} ScoreData;

// minden ami egy játékállást reprezentál
typedef struct GameState {
    // a board bal alsó sarokból van számozva, felfelé nő y, jobbra nő x
    Block** board;
    int board_width;
    int board_height;
    Piece queue[2];  // fixen 2 a mérete
    Piece active_piece;
    ScoreData score_data;
} GameState;

void game_setup(SDL_Renderer* renderer, int board_width, int board_height);
Piece gen_rand_piece();
#endif