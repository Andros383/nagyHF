#include "game_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
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

// TODO egy blokk renderelését kiszervezni függvénybe? vagy az sok

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
static void render_game(SDL_Renderer *renderer, GameState *game_state) {
    // TODO nem mindig kell újrarajzolni mindent. De majd csak akkor, ha optimalizálok?
    SDL_RenderClear(renderer);

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
//*******************
//* ^ game_render.c *
//* v game_logic.c  *
//*******************

// mindegyik függvény változtatja a játékállapotot így vagy úgy, azaz pointerként veszik át a game_state-et
// TODO ezeket okosabban? talán meg lehetne oldani a mozgásokat

// visszaadja, hogy innen indulva a bejáratlan részeken mekkora csoportosulásban van a szín
int group_size(GameState *game_state, bool **visited, Block searched_color, int x, int y) {
    // szebb ha itt veszem fel a szélcheckeket

    // megpróbált lelépni a tábláról
    if (x < 0 || game_state->board_width <= x) return 0;
    if (y < 0 || game_state->board_height <= y) return 0;

    // nem a keresett szín
    if (game_state->board[x][y] != searched_color) return 0;
    // már megnéztük
    if (visited[x][y]) return 0;
    // 1-ről kezd, mert magát beleszámolja
    int size = 1;
    visited[x][y] = true;

    // mind a négy irányban szétnéz
    size += group_size(game_state, visited, searched_color, x, y + 1);
    size += group_size(game_state, visited, searched_color, x + 1, y);
    size += group_size(game_state, visited, searched_color, x, y - 1);
    size += group_size(game_state, visited, searched_color, x - 1, y);

    return size;
}

// TODO kéne neki gravot hívnia utána?
//  törli a négy vagy nagyobb csoportokat, és visszaadja, volt-e törlés
bool pop_groups(GameState *game_state) {
    // TODO mit kéne, ha nem tudja lefoglalni?
    // nem tudom szépen visszaadni értékként, hogy volt-e pop és hogy sikeres volt a memóriafoglalás

    // TODO összevetni a külön írt setup fügvénnyel
    //  TODO null checkek, free!!!
    //   megnéztük-e már az adott cellát

    int width = game_state->board_width;
    int height = game_state->board_height;

    // TODO kieemelni a lefoglalást?

    // megnézte-e már a bejárás
    bool **visited = (bool **)malloc(width * sizeof(bool *));
    for (int i = 0; i < game_state->board_width; i++) {
        visited[i] = (bool *)malloc(height);
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            visited[x][y] = false;
        }
    }

    // TODO itt sokat sokszor újraszámol
    // pontosabban a hármas vagy alatta lévő csoportokat újraszámolja, ami szerintem nem annyira horror
    // nem optimalizálok ha nem lassú

    // volt-e csoport törlése
    bool pop = false;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // nem üres és nem látogatott, elkezdi a bejárást innen
            if (!visited[x][y] && game_state->board[x][y] != EMPTY) {
                // TODO valami jobb név
                int size = group_size(game_state, visited, game_state->board[x][y], x, y);
                // ebben az állapotban a visited pontosan tartalmazza, hogy melyik cellák vannak benne a vizsgált csoportban
                for (int x = 0; x < width; x++) {
                    for (int y = 0; y < height; y++) {
                        // ha a csoportméret elég nagy, törli az elemeket amiket bejárt
                        if (size >= 4 && visited[x][y]) {
                            game_state->board[x][y] = EMPTY;
                            pop = true;
                        }
                        // egyben nullázza is a tömböt
                        visited[x][y] = false;
                    }
                }
            }
        }
    }

    for (int x = 0; x < width; x++) {
        free(visited[x]);
    }
    free(visited);

    return pop;
}
// TODO végigstaticozni
// Leejti a táblán a lebegő részeket
void gravity(GameState *game_state) {
    // oszloponként végigmenve
    for (int x = 0; x < game_state->board_width; x++) {
        // lentről felfelé
        for (int y = 1; y < game_state->board_height; y++) {
            // ha talál egy blokkot ami alatt üres hely van
            if (game_state->board[x][y - 1] == EMPTY && game_state->board[x][y] != EMPTY) {
                // megkeresi a legalacsonyabb üres helyet, és ledobja oda
                for (int search_y = 0; search_y < game_state->board_height; search_y++) {
                    if (game_state->board[x][search_y] == EMPTY) {
                        game_state->board[x][search_y] = game_state->board[x][y];
                        game_state->board[x][y] = EMPTY;
                    }
                }
            }
        }
    }
}

// ténylegesen a táblára helyezi az aktív részt, majd rendezi a táblát (grav, chain)
void lock_active_piece(GameState *game_state) {
    // TODO ezek a sorok nagyon hosszúak
    // aktív rész lerakása a táblára
    game_state->board[game_state->active_piece.x1][game_state->active_piece.y1] = game_state->active_piece.block1;
    game_state->board[game_state->active_piece.x2][game_state->active_piece.y2] = game_state->active_piece.block2;

    // TODO new piece alapból a jó helyre rakja le? Queue-ba jól tegye bele
    //  új rész behozása
    game_state->active_piece = game_state->queue[0];
    game_state->active_piece.x1 = game_state->board_width / 2;
    game_state->active_piece.x2 = game_state->board_width / 2;
    game_state->active_piece.y1 = game_state->board_height - 1;
    game_state->active_piece.y2 = game_state->board_height - 2;

    // TODO rövidebbre a sorokat?
    //  be tud-e jönni az aktív rész
    bool block_1_allowed = game_state->board[game_state->active_piece.x1][game_state->active_piece.y1] == EMPTY;
    bool block_2_allowed = game_state->board[game_state->active_piece.x2][game_state->active_piece.y2] == EMPTY;

    if (!(block_1_allowed && block_2_allowed)) {
        printf("Game Over");
        // TODO game over megcsinálása
    }

    game_state->queue[0] = game_state->queue[1];
    game_state->queue[1] = gen_rand_piece();

    gravity(game_state);

    // pop_groups igaz, ha törlődött csoport ebben a lépésben
    // ha törlődött, akkor gravitációt aktiváljuk, majd megint töröljük a csoportokat
    while (pop_groups(game_state)) {
        gravity(game_state);
    }
}

// az aktív részt letolja, amennyire tudja, majd le is rakja
void hard_drop(GameState *game_state) {
    Piece piece = game_state->active_piece;

    bool successful_downpush = true;
    // akkor végzünk, ha már nem lehet letolni
    while (successful_downpush) {
        successful_downpush = false;
        if ((piece.y1 - 1 >= 0) && (piece.y2 - 1 >= 0)) {
            bool b1 = game_state->board[piece.x1][piece.y1 - 1] == EMPTY;
            bool b2 = game_state->board[piece.x2][piece.y2 - 1] == EMPTY;
            if (b1 && b2) {
                // csak a másoltat toljuk
                piece.y1--;
                piece.y2--;
                successful_downpush = true;
            }
        }
    }

    // átmásolja a tolt részeket
    game_state->active_piece.y1 = piece.y1;
    game_state->active_piece.y2 = piece.y2;

    // TODO lockolni a részt
    lock_active_piece(game_state);
}
// úgy változtatja a játékállást, hogy az aktív rész eggyel lefele mozogjon
void soft_drop(GameState *game_state) {
    // TODO b1 és b2 elhagyása?

    Piece piece = game_state->active_piece;

    // megjegyz: külön ifbe kell rakni, hogy ne próbáljon a cella ürességnél kiindexelni

    // nem a padlón vannak
    if ((piece.y1 - 1 >= 0) && (piece.y2 - 1 >= 0)) {
        // nem próbálnak meg már foglalt cellára mozogni
        bool b1 = game_state->board[piece.x1][piece.y1 - 1] == EMPTY;
        bool b2 = game_state->board[piece.x2][piece.y2 - 1] == EMPTY;
        if (b1 && b2) {
            game_state->active_piece.y1--;
            game_state->active_piece.y2--;
        }
    }
    // TODO lock, de nem biztos hogy ez fogja vezérelni, inkább majd az auto grav meg a hard drop
}
void move_left(GameState *game_state) {
    Piece piece = game_state->active_piece;

    // nem a tábla szélén vannak
    if ((piece.x1 - 1 >= 0) && (piece.x2 - 1 >= 0)) {
        // nem próbálnak meg már foglalt cellára mozogni
        bool b1 = (game_state->board[piece.x1 - 1][piece.y1] == EMPTY);
        bool b2 = (game_state->board[piece.x2 - 1][piece.y2] == EMPTY);
        if (b1 && b2) {
            game_state->active_piece.x1--;
            game_state->active_piece.x2--;
        }
    }
}
void move_right(GameState *game_state) {
    Piece piece = game_state->active_piece;

    // nem a tábla szélén vannak
    if ((piece.x1 + 1 < game_state->board_width) && (piece.x2 + 1 < game_state->board_width)) {
        // nem próbálnak meg már foglalt cellára mozogni
        bool b1 = (game_state->board[piece.x1 + 1][piece.y1] == EMPTY);
        bool b2 = (game_state->board[piece.x2 + 1][piece.y2] == EMPTY);
        if (b1 && b2) {
            // nem a lemásoltat állítjuk
            game_state->active_piece.x1++;
            game_state->active_piece.x2++;
        }
    }
}

// TODO itt sok a másolt kód, más-más néven van ugyan az elmentve, de legalább érthető?

void rotate_cw(GameState *game_state) {
    Piece piece = game_state->active_piece;
    // ha kész van a rész forgatásával return-ol

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
            // TODO ez fontos, hogy külön részben legyen
            // az, hogy a 2-es blokk tud mozogni balra feltételezhető, mert a táblaszélesség alsó limitje 2 vagy nagyobb
            // be tudja rúgni
            if (game_state->board[piece.x2 - 1][piece.y2] == EMPTY) {
                game_state->active_piece.x1 = piece.x1;
                game_state->active_piece.y1 = piece.y1 - 1;
                game_state->active_piece.x2 = piece.x2 - 1;
                game_state->active_piece.y2 = piece.y2;
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
        // ezt csak meg tudja csinálni, felette gravitáció miatt nem lehet blokk
        game_state->active_piece.x1 = piece.x1 + 1;
        game_state->active_piece.y1 = piece.y1 + 1;
        // ez sem fog kiindexelni, mert feljebb nem tud jutni
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
            // tud rúgni
            if (game_state->board[piece.x2 + 1][piece.y2] == EMPTY) {
                game_state->active_piece.x1 = piece.x1;
                game_state->active_piece.y1 = piece.y1 + 1;
                game_state->active_piece.x2 = piece.x2 + 1;
                game_state->active_piece.y2 = piece.y2;
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
        // TODO felrugásokra limit
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
            // felfele mindig tud rúgni
            game_state->active_piece.x1 = piece.x1 - 1;
            game_state->active_piece.y1 = piece.y1;
            game_state->active_piece.x2 = piece.x2;
            game_state->active_piece.y2 = piece.y2 + 1;
        } else {
            // nem kell rúgnia
            game_state->active_piece.x1 = hova_x;
            game_state->active_piece.y1 = hova_y;
        }

        return;
    }
    // TODO ezt kivenni
    printf("Ide nem kéne eljutni.");
}
void rotate_ccw(GameState *game_state) {
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
            // be tudja rúgni
            if (game_state->board[piece.x2 + 1][piece.y2] == EMPTY) {
                game_state->active_piece.x1 = piece.x1;
                game_state->active_piece.y1 = piece.y1 - 1;
                game_state->active_piece.x2 = piece.x2 + 1;
                game_state->active_piece.y2 = piece.y2;
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
        // TODO kick limit

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
            // felfele mindig tud rúgni
            game_state->active_piece.x1 = piece.x1 + 1;
            game_state->active_piece.y1 = piece.y1;
            game_state->active_piece.x2 = piece.x2;
            game_state->active_piece.y2 = piece.y2 + 1;
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
            // tud rúgni
            if (game_state->board[piece.x2 - 1][piece.y2] == EMPTY) {
                game_state->active_piece.x1 = piece.x1;
                game_state->active_piece.y1 = piece.y1 + 1;
                game_state->active_piece.x2 = piece.x2 - 1;
                game_state->active_piece.y2 = piece.y2;
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
        // csak felforgatja, mindig meg tudja csinálni

        game_state->active_piece.x1 = piece.x1 - 1;
        game_state->active_piece.y1 = piece.y1 + 1;

        return;
    }
    // TODO ezt kivenni
    printf("Ide nem kéne eljutni.");
}

int game_loop(SDL_Renderer *renderer, GameState *game_state) {
    // TODO ezt kitörölni
    // tábla manuális állítása
    // game_state->board[3][3] = GREEN;

    // TODO mindenféle dolog, pl ne mindig rajzolja újra, számolja hogy mikor megy jobbra, stb

    // controls temporary le vannak írva a controls.md-be, nem lesz váloztatható, csak megjegyezzem
    SDL_Event event;
    // kilépés mindig megy
    while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)

                {
                    case SDLK_w:
                        hard_drop(game_state);
                        break;
                    case SDLK_a:
                        move_left(game_state);
                        break;
                    case SDLK_s:
                        // TODO talán ide kéne, hogy a grav ne hasson ilyenkor azonnal
                        soft_drop(game_state);
                        break;
                    case SDLK_d:
                        move_right(game_state);
                        break;
                    case SDLK_i:
                        rotate_cw(game_state);
                        break;
                    case SDLK_o:
                        rotate_ccw(game_state);
                        break;
                    case SDLK_ESCAPE:
                        // TODO újraindítani a játékot
#define GYORSKILEP
#ifdef GYORSKILEP
                        SDL_Quit();
#endif
                        break;

                    default:
                        // TODO
                        break;
                }
                break;

            case SDL_KEYUP:
                // TODO állítani hogy fel van-oe emelve
                break;
            default:
                break;
        }
        // TODO nem kell mindig
        // am de
        render_game(renderer, game_state);
    }
    // TODO ez csak placeholder return code
    return -1;
}