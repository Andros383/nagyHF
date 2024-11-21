#include "game_piece_rotations.h"

#include <stdbool.h>
#include <stdio.h>

#include "debugmalloc.h"
#include "game_screen.h"

// TODO itt sok a másolt kód, más-más néven van ugyan az elmentve, de legalább érthető?

// külön fájlban van, mert nem tetszett mennyi helyet foglalt
// úgy változtatja a játékállást, hogy a rész egyet óramutató állásával megegyezően forogjon
// ha a forgatás nem lehetséges, eltolja a részt, vagy nem forgatja el
// megfelelő forgatásnál számolja, hogy ne tudjon végtelenszer felfelé forogni
void rotate_cw(GameState *game_state, int *upkick) {
    Piece piece = game_state->active_piece;
    // hogy ne legyen más meghívva, a piece forgatása után returnol
    // TODO inkább case?

    // TODO szétszedni több függvényre?
    //  TODO kick limit

    // block 1 block 2-höz képest:
    // felette -> jobbra lesz tőle, csak balra tudja elrúgni magát
    if (piece.y1 > piece.y2) {
        // ahova mozog a forgó rész
        int hova_x = piece.x1 + 1;
        int hova_y = piece.y1 - 1;

        bool kick = false;

        if (hova_x >= game_state->board_width) {
            // balra rúg fal miatt
            kick = true;
        } else if (game_state->board[hova_x][hova_y] != EMPTY) {
            // külön kell megnézni kiindexelés miatt
            // balra rúg a foglalt hely miatt
            kick = true;
        }

        if (kick) {
            // nem rúg le a tábláról
            if (piece.x2 - 1 >= 0) {
                // be tudja rúgni
                if (game_state->board[piece.x2 - 1][piece.y2] == EMPTY) {
                    game_state->active_piece.x1 = piece.x1;
                    game_state->active_piece.y1 = piece.y1 - 1;
                    game_state->active_piece.x2 = piece.x2 - 1;
                    game_state->active_piece.y2 = piece.y2;
                }
            }

        } else {
            // nem kell rúgnia, csak leforgatja
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
            // 2-es blokk nem is változik
        }
        return;
    }
    // balra
    if (piece.x1 < piece.x2) {
        // nincs hova_x mert nem kell nézni
        int hova_y = piece.y1 + 1;

        bool kick = false;
        // felrúgás miatt ki tud fogorni ilyenkor a tábláról
        // de csak a tábla teteje miatt tud rúgni
        if (hova_y >= game_state->board_height) {
            kick = true;
        }

        if (kick) {
            // rúg, ha szabad a hely
            if (game_state->board[piece.x2][piece.y2 - 1] == EMPTY) {
                game_state->active_piece.x1 = piece.x1 + 1;
                game_state->active_piece.y1 = piece.y1;
                game_state->active_piece.x2 = piece.x2;
                game_state->active_piece.y2 = piece.y2 - 1;
            }
        } else {
            // nem kell rúgnia
            game_state->active_piece.x1 = piece.x1 + 1;
            game_state->active_piece.y1 = piece.y1 + 1;
        }
        return;
    }
    // alatta -> jobbra tud rúgni
    if (piece.y1 < piece.y2) {
        int hova_x = piece.x1 - 1;
        int hova_y = piece.y1 + 1;

        bool kick = false;

        // TODO talán jobb lenne negáltat nézni? Mikor nem tud rúgni? Így biztosan érthető

        // jobbra rúg padló miatt
        if (hova_x < 0) {
            kick = true;
        } else if (game_state->board[hova_x][hova_y] != EMPTY) {
            // jobbra rúg foglalt hely miatt
            kick = true;
        }

        if (kick) {
            // nem rúg le a tábláról
            // ITT IS VOLT VÁLTOZTATÁS
            if (piece.x2 + 1 < game_state->board_width) {
                // tud rúgni
                if (game_state->board[piece.x2 + 1][piece.y2] == EMPTY) {
                    game_state->active_piece.x1 = piece.x1;
                    game_state->active_piece.y1 = piece.y1 + 1;
                    game_state->active_piece.x2 = piece.x2 + 1;
                    game_state->active_piece.y2 = piece.y2;
                }
            }
        } else {
            // nem kell rúgnia, leforgatja
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
        }
        return;
    }
    // jobbra -> felfele tud rúgni
    if (piece.x1 > piece.x2) {
        int hova_x = piece.x1 - 1;
        int hova_y = piece.y1 - 1;

        bool kick = false;
        if (hova_y < 0) {
            // felfele rúg padló miatt
            kick = true;
        } else if (game_state->board[hova_x][hova_y] != EMPTY) {
            // felfele rúg foglalt hely miatt
            kick = true;
        }

        if (kick) {
            // nem rúg le a tábláról
            // TODO ITT IS VOLT VÁLTOZTATÁS
            if (piece.y2 + 1 < game_state->board_height) {
                // ha még nem rúgott túl sokszor fel
                if (*upkick < 3) {
                    // számolja a felrúgásokat
                    (*upkick)++;

                    // felfele mindig tud rúgni
                    game_state->active_piece.x1 = piece.x1 - 1;
                    game_state->active_piece.y1 = piece.y1;
                    game_state->active_piece.x2 = piece.x2;
                    game_state->active_piece.y2 = piece.y2 + 1;
                }
            }
        } else {
            // nem kell rúgnia
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
        }

        return;
    }
    // TODO ezt kivenni, jelezni
    printf("Ide nem kéne eljutni.");
}
void rotate_ccw(GameState *game_state, int *upkick) {
    Piece piece = game_state->active_piece;
    // ha kész van a rész forgatásával return-ol

    // TODO kick limit

    // block 1 block 2-höz képest:
    // felette -> balra lesz tőle, csak jobbra tudja elrúgni magát
    if (piece.y1 > piece.y2) {
        // ahova mozog a forgó rész
        int hova_x = piece.x1 - 1;
        int hova_y = piece.y1 - 1;

        bool kick = false;

        if (hova_x < 0) {
            // jobbra rúg fal miatt
            kick = true;
        } else if (game_state->board[hova_x][hova_y] != EMPTY) {
            // jobbra rúg a foglalt hely miatt
            kick = true;
        }

        if (kick) {
            // nem rúg le a tábláról
            if (piece.x2 + 1 < game_state->board_width) {
                // be tudja rúgni
                if (game_state->board[piece.x2 + 1][piece.y2] == EMPTY) {
                    game_state->active_piece.x1 = piece.x1;
                    game_state->active_piece.y1 = piece.y1 - 1;
                    game_state->active_piece.x2 = piece.x2 + 1;
                    game_state->active_piece.y2 = piece.y2;
                }
            }
        } else {
            // nem kell rúgnia, csak leforgatja
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
        }
        return;
    }
    // balra -> felfele tud rúgni
    if (piece.x1 < piece.x2) {
        // TODO felrugásokra limit
        int hova_x = piece.x1 + 1;
        int hova_y = piece.y1 - 1;

        bool kick = false;

        if (hova_y < 0) {
            // felfele rúg padló miatt
            kick = true;
        } else if (game_state->board[hova_x][hova_y] != EMPTY) {
            // felfele rúg foglalt hely miatt
            kick = true;
        }

        if (kick) {
            // TODO ITT VOLT BÁLTOZTATÁS
            // nem rúg le a tábláról
            if (piece.y2 + 1 < game_state->board_height) {
                // ha még nem rúgott túl sokszor fel
                if (*upkick < 3) {
                    // számolja a felrúgásokat
                    (*upkick)++;

                    // felfele mindig tud rúgni
                    game_state->active_piece.x1 = piece.x1 + 1;
                    game_state->active_piece.y1 = piece.y1;
                    game_state->active_piece.x2 = piece.x2;
                    game_state->active_piece.y2 = piece.y2 + 1;
                }
            }

        } else {
            // nem kell rúgnia
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
        }

        return;
    }
    // alatta -> balra tud rúgni
    if (piece.y1 < piece.y2) {
        int hova_x = piece.x1 + 1;
        int hova_y = piece.y1 + 1;

        bool kick = false;

        // balra rúg fal miatt
        if (hova_x >= game_state->board_width) {
            kick = true;
        } else if (game_state->board[hova_x][hova_y] != EMPTY) {
            // balra rúg foglalt hely miatt
            kick = true;
        }

        if (kick) {
            // nem rúg le a tábláról
            if (piece.x2 - 1 >= 0) {
                // tud rúgni
                if (game_state->board[piece.x2 - 1][piece.y2] == EMPTY) {
                    game_state->active_piece.x1 = piece.x1;
                    game_state->active_piece.y1 = piece.y1 + 1;
                    game_state->active_piece.x2 = piece.x2 - 1;
                    game_state->active_piece.y2 = piece.y2;
                }
            }
        } else {
            // nem kell rúgnia, leforgatja
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
        }
        return;
    }
    // jobbra
    if (piece.x1 > piece.x2) {
        // nincs hova_x mert nem kell nézni
        int hova_y = piece.y1 + 1;

        bool kick = false;
        // felrúgás miatt ki tud fogorni ilyenkor a tábláról
        // de csak a tábla teteje miatt tud rúgni
        if (hova_y >= game_state->board_height) {
            kick = true;
        }

        if (kick) {
            // rúg, ha szabad a hely
            if (game_state->board[piece.x2][piece.y2 - 1] == EMPTY) {
                game_state->active_piece.x1 = piece.x1 - 1;
                game_state->active_piece.y1 = piece.y1;
                game_state->active_piece.x2 = piece.x2;
                game_state->active_piece.y2 = piece.y2 - 1;
            }
        } else {
            // nem kell rúgnia
            game_state->active_piece.x1 = piece.x1 - 1;
            game_state->active_piece.y1 = piece.y1 + 1;
        }
        return;
    }
    // TODO ezt kivenni
    printf("Ide nem kéne eljutni.");
}