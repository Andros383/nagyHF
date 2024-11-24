#include "game_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debugmalloc.h"
#include "game_piece_rotations.h"
#include "menu_selector.h"
#include "render.h"

// rekurzív bejárás
// első híváskor a visited tömb minden eleme legyen hamis
// visszatérési értéke a csoport mérete
// visszatéréskor a visited tömbben megjelöli a bejárt csoport elemeit
static int explore_group(GameState *game_state, bool **visited, Block searched_color, int x, int y) {
    // megpróbált lelépni a tábláról
    if (x < 0 || game_state->board_width <= x) return 0;
    if (y < 0 || game_state->board_height <= y) return 0;

    // nem a keresett szín
    if (game_state->board[x][y] != searched_color) return 0;
    // már megnéztük
    if (visited[x][y]) return 0;

    // 1-ről kezd, itt számolja az éppen bejárt elemet
    int size = 1;
    visited[x][y] = true;

    // mind a négy irányban keresi a többi elemet
    size += explore_group(game_state, visited, searched_color, x, y + 1);
    size += explore_group(game_state, visited, searched_color, x + 1, y);
    size += explore_group(game_state, visited, searched_color, x, y - 1);
    size += explore_group(game_state, visited, searched_color, x - 1, y);

    return size;
}

//  törli a négy vagy nagyobb csoportokat, és visszaadja, volt-e törlés
// a cleared_groups és a cleared_blocks változókba belerakja a törölt csoportok és a törölt blokkok számát
// visszatérési értékek:
// -1: hiba történt
// 0:  nem történt hiba, nem volt csoport törlés
// 1:  nem történt hiba, történt csoport törlés
static int pop_groups(CommonRenderData rd, GameState *game_state, int chain_length, int *cleared_groups, int *cleared_blocks) {
    int width = game_state->board_width;
    int height = game_state->board_height;

    // használt tömbök lefoglalása
    // ha nem sikerül egy foglalás, a visszatérési kód -1

    // temp változó, járt-e már ott a bejárás
    bool **visited = (bool **)malloc(width * sizeof(bool *));

    if (visited == NULL) {
        printf("pop_groups: sikertelen memoriafoglalas\n");
        return -1;
    }

    for (int x = 0; x < game_state->board_width; x++) {
        visited[x] = (bool *)malloc(height * sizeof(bool));

        // ha nem sikerült lefoglalni az oszlopot
        if (visited[x] == NULL) {
            // előző oszlopok felszabadítása
            for (int i = 0; i < x; i++) {
                free(visited[i]);
            }
            // eredeti tömb felszabadítása
            free(visited);

            printf("pop_groups: sikertelen memoriafoglalas\n");
            return -1;
        }
    }

    // egy lépés végén a törlendő cellák
    // animáció miatt kell lementeni
    bool **clear = (bool **)malloc(width * sizeof(bool *));

    if (clear == NULL) {
        // egész visited tömb felszabadítása
        for (int x = 0; x < width; x++) {
            free(visited[x]);
        }
        free(visited);

        printf("pop_groups: sikertelen memoriafoglalas\n");
        return -1;
    }

    for (int x = 0; x < game_state->board_width; x++) {
        clear[x] = (bool *)malloc(height * sizeof(bool));

        // ha nem sikerült lefoglalni az oszlopot
        if (clear[x] == NULL) {
            // előző oszlopok felszabadítása
            for (int i = 0; i < x; i++) {
                free(clear[i]);
            }

            // tömb felszabadítása
            free(clear);

            // egész visited tömb felszabadítása
            for (int x = 0; x < width; x++) {
                free(visited[x]);
            }
            free(visited);

            printf("pop_groups: sikertelen memoriafoglalas\n");
            return -1;
        }
    }

    // tömbök nullázása
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            visited[x][y] = false;
            clear[x][y] = false;
        }
    }

    // volt-e csoport törlése
    bool pop = false;

    // pontozáshoz számolja a törölt csoportokat és blokkokat
    *cleared_groups = 0;
    *cleared_blocks = 0;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // ha az adott csoportot már töröltük, akkor nem járjuk újra be
            // ha nem kell törölni bejárhatjuk, a pontszámon nem változtat
            // lehet többször járjuk be a 3 vagy kisebb csoportokat, de így elég két tömb
            if (!clear[x][y] && game_state->board[x][y] != EMPTY) {
                int size = explore_group(game_state, visited, game_state->board[x][y], x, y);
                // ebben az állapotban a visited pontosan tartalmazza, hogy melyik cellák vannak benne a vizsgált csoportban

                // ilyenkor töröltünk csoportot
                if (size >= 4) (*cleared_groups)++;

                for (int x = 0; x < width; x++) {
                    for (int y = 0; y < height; y++) {
                        // ha a csoportméret elég nagy, felírja, miket kell törölnie
                        if (size >= 4 && visited[x][y]) {
                            clear[x][y] = true;
                            pop = true;

                            (*cleared_blocks)++;
                        }

                        // a csoportmérettől függetlenül törli a visited elemeit a következő bejáráshoz
                        visited[x][y] = false;
                    }
                }
            }
        }
    }

    // törlés közbeni rajzolások és elemek törlése a tábláról
    if (pop) {
        // jelenlegi lánclépés megjelenítése
        char label[50 + 1];
        sprintf(label, "%2d-LÁNC", chain_length);
        render_text_block(rd, label, rd.origin_x - 150, 125, COLOR_BLACK);

        // pop animáicó
        for (int pop_anim_count = 1; pop_anim_count <= 3; pop_anim_count++) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    if (clear[x][y]) {
                        render_block_on_board(rd, EMPTY, x, y, (1.0 / 3.0) * pop_anim_count);
                    }
                }
            }
            // animáció megjelenítése
            SDL_RenderPresent(rd.renderer);
            SDL_Delay(200);
        }
        SDL_Delay(200);

        // az elemek tényleges törlése a tábláról
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                if (clear[x][y]) {
                    game_state->board[x][y] = EMPTY;
                }
            }
        }
    }

    // tömbök felszabadítása
    for (int x = 0; x < width; x++) {
        free(visited[x]);
    }
    free(visited);

    for (int x = 0; x < width; x++) {
        free(clear[x]);
    }
    free(clear);

    return pop ? 1 : 0;
}
// az összes blokkot leejti a tábla aljára
static void gravity(GameState *game_state) {
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

// ténylegesen a táblára helyezi az aktív részt, majd rendezi a táblát (gravitáció, csoporttörlések)
// behozza az új elemeket, növeli a pontszámot
// ha nem tud bejönni az új rész, vége a játéknak
// -1: hiba történt
// 0:  sikeres részlerakás
// 1:  új rész nem tud bejönni, játék vége
static int lock_active_piece(CommonRenderData rd, GameState *game_state, int *nat_grav_count, int *upkick_count) {
    // gravitáció kikapcsolása, mert időbe kerül, mire az animáció lejátszódik
    SDL_Event lock_input_event;
    lock_input_event.type = SDL_USEREVENT + 1;
    SDL_PushEvent(&lock_input_event);

    // aktív rész lerakása a táblára
    game_state->board[game_state->active_piece.x1][game_state->active_piece.y1] = game_state->active_piece.block1;
    game_state->board[game_state->active_piece.x2][game_state->active_piece.y2] = game_state->active_piece.block2;

    // aktív rész eltüntetése, hogy ne rajzolja ki
    game_state->active_piece.x1 = -1;
    game_state->active_piece.y1 = -1;
    game_state->active_piece.x2 = -1;
    game_state->active_piece.y2 = -1;

    // tábla rendezése
    gravity(game_state);
    render_game(rd, game_state);

    // Csoportok törlése több lépésben

    int cleared_groups = 0;
    int cleared_blocks = 0;
    int chain_length = 0;

    // 1: törölt csoportokat
    // 0: nem volt törlés
    // -1: memóraifoglalási hiba
    int return_code = pop_groups(rd, game_state, chain_length + 1, &cleared_groups, &cleared_blocks);
    if (return_code == -1) return -1;

    // (chain_length + 1)-et kap, mert ha van lánc, akkor kirajzolja a lánc hosszát

    while (return_code != 0) {
        chain_length++;
        int mult = 1;

        // 2 hatvány kiszámítása a pontszámításhoz
        for (int i = 0; i < chain_length - 1; i++) {
            mult *= 2;
        }

        // hozzáadott pontszám: 2^([lánchossz]-1) * (10*[törölt csoportok a lépésben] + [törölt blokkok a lépésben]);
        // picit erősen pontozza a hosszú láncokat, de nem baj
        int added_score = mult * (10 * cleared_groups + cleared_blocks);
        game_state->score_data.score += added_score;

        // gravitációval rendezés
        gravity(game_state);
        render_board(rd, game_state->board);

        // következő lánclépésre a csoportok törlése
        return_code = pop_groups(rd, game_state, chain_length + 1, &cleared_groups, &cleared_blocks);
        if (return_code == -1) return -1;
    }

    // max lánchossz változtatása
    if (chain_length > game_state->score_data.longest_chain) {
        game_state->score_data.longest_chain = chain_length;
    }
    // lerakott részek növelése
    game_state->score_data.placed_pieces++;

    //  új rész behozása
    game_state->active_piece = game_state->queue[0];
    game_state->active_piece.x1 = game_state->board_width / 2;
    game_state->active_piece.x2 = game_state->board_width / 2;
    game_state->active_piece.y1 = game_state->board_height - 1;
    game_state->active_piece.y2 = game_state->board_height - 2;

    // soron következő elemek léptetése
    game_state->queue[0] = game_state->queue[1];
    game_state->queue[1] = gen_rand_piece();

    // részre vonatkozó anti-stalling számlálók törlése
    *nat_grav_count = 0;
    *upkick_count = 0;

    //  be tud-e jönni az aktív rész, ha nem -> Game Over
    bool block_1_allowed = game_state->board[game_state->active_piece.x1][game_state->active_piece.y1] == EMPTY;
    bool block_2_allowed = game_state->board[game_state->active_piece.x2][game_state->active_piece.y2] == EMPTY;

    if (!(block_1_allowed && block_2_allowed)) {
        return 1;
    }

    // be kell hozni a billentyűzet eventjeit, különben az event be és kikapcsolása nem hatna rájuk
    SDL_PumpEvents();

    // gravitáció visszakapcsolása
    lock_input_event.type = SDL_USEREVENT + 2;
    SDL_PushEvent(&lock_input_event);

    return 0;
}

// az aktív részt letolja, amennyire tudja, majd le is rakja
// visszatérési érték:
// -1: hiba
// 0:  sikeres lejjebblépés
// 1:  játék vége
static int hard_drop(CommonRenderData rd, GameState *game_state, int *nat_grav_count, int *upkick_count) {
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

    // átmásolja a tolt részt
    game_state->active_piece.y1 = piece.y1;
    game_state->active_piece.y2 = piece.y2;

    // lerakja a részt
    int return_code = lock_active_piece(rd, game_state, nat_grav_count, upkick_count);
    // hiba történt
    if (return_code == -1) return -1;
    // játék vége
    if (return_code == 1) return 1;

    return 0;
}

// aktív részt eggyel lefele mozgatja
// visszaadja, hogy lejjebb mozgott-e a rész
static bool soft_drop(GameState *game_state) {
    Piece piece = game_state->active_piece;
    // megjegyz: külön ifbe kell rakni, hogy ne próbáljon a cella üresség megnézésénél kiindexelni a tábláról
    // nem a padlón vannak
    if ((piece.y1 - 1 >= 0) && (piece.y2 - 1 >= 0)) {
        // nem próbálnak meg már foglalt cellára mozogni
        bool b1 = game_state->board[piece.x1][piece.y1 - 1] == EMPTY;
        bool b2 = game_state->board[piece.x2][piece.y2 - 1] == EMPTY;
        if (b1 && b2) {
            game_state->active_piece.y1--;
            game_state->active_piece.y2--;
            return true;
        }
    }
    return false;
}
// gravitáció miatt eggyel lejjebb lép az aktív rész
// visszatérési értékek:
// -1: hiba
// 0:  sikeres lejjebblépés
// 1:  játék vége
static int natural_gravity(CommonRenderData rd, GameState *game_state, int *nat_grav_count, int *upkick_count) {
    // lejjebb mozgatja a részt
    if (!soft_drop(game_state)) {
        // ha nem mozgott lejjebb, növeli a számlálót
        if (*nat_grav_count >= 3) {
            // ha elérte a határt, lehelyezi
            int return_code = lock_active_piece(rd, game_state, nat_grav_count, upkick_count);
            // hiba történt
            if (return_code == -1) return -1;
            // játék vége
            if (return_code == 1) return 1;
        }
        (*nat_grav_count)++;
    }
    return 0;
}
// balra mozgatja az aktív részt
static void move_left(GameState *game_state) {
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
// jobbra mozgatja az aktív részt
static void move_right(GameState *game_state) {
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
// időzítők a folyamatos lenyomáshoz
// Source: InfoC https://infoc.eet.bme.hu/sdl/#3
static Uint32 left_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 10;
    SDL_PushEvent(&ev);
    return ms;
}
static Uint32 right_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 11;
    SDL_PushEvent(&ev);
    return ms;
}
static Uint32 soft_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 12;
    SDL_PushEvent(&ev);
    return ms;
}
// gravitáció időzítője
// real_ms tárolja, hogy milyen gyorsan esik le
// viszont a valós késleltetés nem lehet több, mint 1000ms, vagy kisebb, mint 150ms
static Uint32 gravity_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);

    int *real_ms = (int *)param;

    // real_ms 1120-ról kezd, azaz az első percben még nem változik a gravitáció
    if (*real_ms > 150) *real_ms = *real_ms - 2;
    int delay = *real_ms;

    if (delay > 1000) delay = 1000;
    if (delay < 150) delay = 150;

    return delay;
}

int game_loop(CommonRenderData rd, GameState *game_state) {
    // gravitáció számlálója, időzítője
    int real_ms = 1120;
    SDL_TimerID timerid_gravity = SDL_AddTimer(1000, gravity_timer, &real_ms);

    //  hányszor került feljebb a rész forgatás által
    int upkick_count = 0;
    // hányszor próbálta már a gravitáció lerakni a részt
    int nat_grav_count = 0;

    // hosszú lenyomáshoz időzítők
    SDL_TimerID timerid_left;
    SDL_TimerID timerid_right;
    SDL_TimerID timerid_soft;

    // irányító gombok le vannak-e nyomva
    bool held_left = false;
    bool held_right = false;
    bool held_soft = false;

    bool held_hard = false;
    bool held_cw = false;
    bool held_ccw = false;

    // menügombok ki vannak-e választva
    bool selected_settings = false;
    bool selected_toplist = false;

    // fogad-e bemenetet a loop
    bool enable_movement = true;

    int return_code = -1;
    bool quit = false;
    SDL_Event event;
    while (SDL_WaitEvent(&event) && !quit) {
        // le kell-e rajzolni az egész képernyőt
        bool draw = false;

        // gravitáció fogadásának külön állítása
        if (event.type == SDL_USEREVENT + 1) {
            enable_movement = false;
            continue;
        }
        if (event.type == SDL_USEREVENT + 2) {
            enable_movement = true;
            continue;
        }

        if (!enable_movement) {
            // kikapcsol mindenféle mozgást, a gravitációt, és az ismétlődő mozgásokat az animáció idejére
            Uint32 disabled_events[5] = {SDL_KEYDOWN, SDL_USEREVENT, SDL_USEREVENT + 10, SDL_USEREVENT + 11, SDL_USEREVENT + 12};
            Uint32 type = event.type;
            bool skip = false;
            for (int i = 0; i < 5; i++) {
                if (type == disabled_events[i]) skip = true;
            }
            // mostani event kihagyása
            if (skip) continue;
        }

        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        // hard drop
                        if (!held_hard) {
                            int hd_return_code = hard_drop(rd, game_state, &nat_grav_count, &upkick_count);
                            if (hd_return_code == -1) {
                                // hiba
                                quit = true;
                                return_code = -1;
                            }
                            if (hd_return_code == 1) {
                                // játék vége képernyő
                                quit = true;
                                return_code = 4;
                            }
                            held_hard = true;
                            draw = true;
                        }
                        break;
                    case SDLK_a:
                        // balra mozgatás
                        if (!held_left) {
                            move_left(game_state);
                            held_left = true;
                            timerid_left = SDL_AddTimer(150, left_timer, NULL);
                            draw = true;
                        }
                        break;
                    case SDLK_d:
                        // jobbra mozgatás
                        if (!held_right) {
                            move_right(game_state);
                            held_right = true;
                            timerid_right = SDL_AddTimer(150, right_timer, NULL);
                            draw = true;
                        }
                        break;
                    case SDLK_s:
                        // lefele mozgatás
                        if (!held_soft) {
                            soft_drop(game_state);
                            held_soft = true;
                            // időzítő elindítása
                            timerid_soft = SDL_AddTimer(150, soft_timer, NULL);
                            draw = true;
                        }
                        break;
                    case SDLK_i:
                        // forgatás 1
                        if (!held_cw) {
                            rotate_cw(game_state, &upkick_count);
                            held_cw = true;
                            draw = true;
                        }
                        break;
                    case SDLK_o:
                        // forgatás 2
                        if (!held_ccw) {
                            rotate_ccw(game_state, &upkick_count);
                            held_ccw = true;
                            draw = true;
                        }
                        break;
                    case SDLK_ESCAPE:
                        // játék újraindítása
                        return_code = 1;
                        quit = true;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYUP:
                // felengedni csak egyszer lehet
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        held_hard = false;
                        break;
                    case SDLK_a:
                        held_left = false;
                        SDL_RemoveTimer(timerid_left);
                        break;
                    case SDLK_d:
                        held_right = false;
                        SDL_RemoveTimer(timerid_right);
                        break;
                    case SDLK_s:
                        held_soft = false;
                        SDL_RemoveTimer(timerid_soft);
                        break;
                    case SDLK_i:
                        held_cw = false;
                        break;
                    case SDLK_o:
                        held_ccw = false;
                        break;
                }
                break;
            case SDL_USEREVENT:;
                // gravitáció
                int nd_return_code = natural_gravity(rd, game_state, &nat_grav_count, &upkick_count);
                if (nd_return_code == -1) {
                    // hiba
                    quit = true;
                    return_code = -1;
                }
                if (nd_return_code == 1) {
                    // játék vége képernyő
                    quit = true;
                    return_code = 4;
                }
                draw = true;
                break;
            case SDL_USEREVENT + 10:  // hosszú lenyomáskor automatikus mozgások
                move_left(game_state);
                draw = true;
                break;
            case SDL_USEREVENT + 11:
                move_right(game_state);
                draw = true;
                break;
            case SDL_USEREVENT + 12:
                soft_drop(game_state);
                draw = true;
                break;
            case SDL_MOUSEMOTION: {
                // egérmozgatás alapján a gombok színezése
                int x = event.motion.x;
                int y = event.motion.y;
                bool in_settings = 1298 <= x && x <= 1499 && 498 <= y && y <= 533;
                bool in_toplist = 1318 <= x && x <= 1483 && 598 <= y && y <= 633;
                // amikor nem egyezik a rajzolt állapot a valóssal, akkor újra kell rajzolni
                if (selected_settings != in_settings) {
                    if (in_settings) {
                        // piros keret, ha ki van választva
                        render_text_block(rd, "Beállítások", 1300, 500, COLOR_RED);
                    } else {
                        render_text_block(rd, "Beállítások", 1300, 500, COLOR_BLACK);
                    }
                    selected_settings = in_settings;
                    SDL_RenderPresent(rd.renderer);
                }
                if (selected_toplist != in_toplist) {
                    if (in_toplist) {
                        render_text_block(rd, "Ranglista", 1320, 600, COLOR_RED);
                    } else {
                        render_text_block(rd, "Ranglista", 1320, 600, COLOR_BLACK);
                    }
                    selected_toplist = in_toplist;
                    SDL_RenderPresent(rd.renderer);
                }
            } break;
            case SDL_MOUSEBUTTONDOWN:
                // nem nézi a hosszú lenyomást, mert azonnal képernyőt vált
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int x = event.motion.x;
                    int y = event.motion.y;
                    bool in_settings = 1298 <= x && x <= 1499 && 498 <= y && y <= 533;
                    bool in_toplist = 1318 <= x && x <= 1483 && 598 <= y && y <= 633;
                    if (in_settings) {
                        // beállítások menü
                        return_code = 2;
                        quit = true;
                    }
                    if (in_toplist) {
                        // toplista menü
                        return_code = 3;
                        quit = true;
                    }
                }
                break;
            case SDL_QUIT:
                // játék lezárása X-el
                return_code = 0;
                quit = true;
                break;
            default:
                break;
        }
        if (draw)
            render_game(rd, game_state);
    }

    // időzítők leállítása
    SDL_RemoveTimer(timerid_gravity);
    SDL_RemoveTimer(timerid_left);
    SDL_RemoveTimer(timerid_right);
    SDL_RemoveTimer(timerid_soft);

    return return_code;
}

// saját usereventek:
// SDL_USEREVENT: gravitációért felelős event
// SDL_USEREVENT + 1: leállítja a bemenetek kezelését, kivéve SDL_USEREVENT + 2-t
// SDL_USEREVENT + 2: folytatja az eventek kezelését
// SDL_USEREVENT + 10: folyamatos balra lépés event
// SDL_USEREVENT + 11: folyamatos jobbra lépés event
// SDL_USEREVENT + 12: folyamatos soft drop event

// mindig amikor ki akar lépni az event loopból egy kódrész beállítja a quit-et igazra, és a return_code-ot arra, ami a kilépésnek megfelel

// return code-ok:
// 0: felhasználó kilép x-el
// 1: újraindítás
// 2: beállítások menü
// 3: toplista menü
// 4: Game Over screen
// -1: memóriafoglalás / egyéb hiba