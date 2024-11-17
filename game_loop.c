#include "game_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game_piece_rotations.h"
#include "game_render.h"
#include "game_screen.h"

// TODO egybevenni a stallokat?
// csak egy stall counter lenne

//  mindegyik függvény változtatja a játékállapotot így vagy úgy, azaz pointerként veszik át a game_state-et
//  TODO ezeket okosabban? talán meg lehetne oldani a mozgásokat

// első híváskor visited-nek üresnek kell lennie
// amikor visszatér, a visitedben benne lesz, hogy melyik cellák vannak az adott csoportban
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
bool pop_groups(CommonRenderData rd, GameState *game_state) {
    // TODO mit kéne, ha nem tudja lefoglalni?
    // nem tudom szépen visszaadni értékként, hogy volt-e pop és hogy sikeres volt a memóriafoglalás

    // TODO összevetni a külön írt setup fügvénnyel
    //  TODO null checkek, free!!!
    //   megnéztük-e már az adott cellát

    int width = game_state->board_width;
    int height = game_state->board_height;

    // TODO kieemelni a lefoglalást?

    // temp változó, járt-e már ott a bejárás
    bool **visited = (bool **)malloc(width * sizeof(bool *));
    for (int i = 0; i < game_state->board_width; i++) {
        visited[i] = (bool *)malloc(height);
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            visited[x][y] = false;
        }
    }

    // egy lépés végén a törlendő cellák
    // animáció miatt kell lementeni
    bool **clear = (bool **)malloc(width * sizeof(bool *));
    for (int i = 0; i < game_state->board_width; i++) {
        clear[i] = (bool *)malloc(height);
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            clear[x][y] = false;
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
                        // ha a csoportméret elég nagy, felírja, miket kell törölnie
                        if (size >= 4 && visited[x][y]) {
                            clear[x][y] = true;
                            pop = true;
                        }
                        // egyben nullázza is a tömböt
                        visited[x][y] = false;
                    }
                }
            }
        }
    }

    if (pop) {
        // TODO első híváskor van egy nagyon kicsi kör, meg kéne lennie egy megállásnak a teljes törléskor is
        for (int pop_anim_count = 1; pop_anim_count <= 3; pop_anim_count++) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    if (clear[x][y]) {
                        render_animation_block(rd, x, y, (1.0 / 3.0) * pop_anim_count);
                    }
                }
            }
            // TODO delay legyen többszöröse minden gravitációnak, hogy ne legyen fura +1 lépés
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

    for (int x = 0; x < width; x++) {
        free(visited[x]);
    }
    free(visited);

    for (int x = 0; x < width; x++) {
        free(clear[x]);
    }
    free(clear);

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
void lock_active_piece(CommonRenderData rd, GameState *game_state, int *nat_grav, int *upkick) {
    // TODO utánanézni ez jó-e, mert fura hogy reference-et ad át
    // gravitáció kikapcsolása, mert időbe kerül, mire az animáció lejátszódik
    SDL_Event lock_input_event;
    lock_input_event.type = SDL_USEREVENT + 1;
    SDL_PushEvent(&lock_input_event);

    // TODO ezek a sorok nagyon hosszúak
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

    // pop_groups igaz, ha törlődött csoport ebben a lépésben
    // ha törlődött, akkor gravitációt aktiváljuk, majd megint töröljük a csoportokat
    while (pop_groups(rd, game_state)) {
        gravity(game_state);
        render_game(rd, game_state);
    }

    // TODO new piece alapból a jó helyre rakja le? Queue-ba jól tegye bele
    //  új rész behozása
    game_state->active_piece = game_state->queue[0];
    game_state->active_piece.x1 = game_state->board_width / 2;
    game_state->active_piece.x2 = game_state->board_width / 2;
    game_state->active_piece.y1 = game_state->board_height - 1;
    game_state->active_piece.y2 = game_state->board_height - 2;

    // részre vonatkozó anti-stalling számlálók törlése
    *nat_grav = 0;
    *upkick = 0;

    // TODO rövidebbre a sorokat?
    //  be tud-e jönni az aktív rész, ha nem -> Game Over
    bool block_1_allowed = game_state->board[game_state->active_piece.x1][game_state->active_piece.y1] == EMPTY;
    bool block_2_allowed = game_state->board[game_state->active_piece.x2][game_state->active_piece.y2] == EMPTY;

    if (!(block_1_allowed && block_2_allowed)) {
        printf("Game Over\n");
        // TODO game over megcsinálása
    }

    game_state->queue[0] = game_state->queue[1];
    game_state->queue[1] = gen_rand_piece();

    // TODO rendszerezni a pontkezelő függvényeket?
    // talán saját fájlba a pont, sajátba a game_state, sajátba a render függvényeket
    // és akkor egymást hivogatják
    game_state->score_data.placed_pieces++;

    // be kell hozni a billentyűzet eventjeit, különben az event be és kikapcsolása nem hatna rájuk
    // feltételezésem szerint csak wait event-kor kerülnek rá ténylegesen az SDL event sorra
    SDL_PumpEvents();

    // gravitáció visszakapcsolása
    lock_input_event.type = SDL_USEREVENT + 2;
    SDL_PushEvent(&lock_input_event);
}

// az aktív részt letolja, amennyire tudja, majd le is rakja
void hard_drop(CommonRenderData rd, GameState *game_state, int *nat_grav, int *upkick) {
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
    lock_active_piece(rd, game_state, nat_grav, upkick);
}

// úgy változtatja a játékállást, hogy az aktív rész eggyel lefele mozogjon
// visszaadja, hogy lejjebb mozgott-e a rész
bool soft_drop(GameState *game_state) {
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
            return true;
        }
    }
    // TODO lock, de nem biztos hogy ez fogja vezérelni, inkább majd az auto grav meg a hard drop
    return false;
}
// TODO talán a nat_grav és az upkicket berakni a game_state-be, vagy csak flag structot létrehozni?
void natural_gravity(CommonRenderData rd, GameState *game_state, int *nat_grav, int *upkick) {
    // lejjebb mozgatja a részt
    if (!soft_drop(game_state)) {
        // ha nem mozgott lejjebb, növeli a számlálót
        if (*nat_grav >= 3) {
            // ha elérte a határt, lehelyezi
            lock_active_piece(rd, game_state, nat_grav, upkick);
        }
        (*nat_grav)++;
    }
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

// TODO lehet csak egy timer, aminek paramjába van, hogy hanyadik user event?
Uint32 left_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 10;
    SDL_PushEvent(&ev);
    return ms; /* ujabb varakozas */
}
Uint32 right_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 11;
    SDL_PushEvent(&ev);
    return ms; /* ujabb varakozas */
}
Uint32 soft_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT + 12;
    SDL_PushEvent(&ev);
    return ms; /* ujabb varakozas */
}
Uint32 gravity_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);
    return ms; /* ujabb varakozas */
}

int game_loop(CommonRenderData rd, GameState *game_state) {
    // TODO mindenféle dolog, pl ne mindig rajzolja újra, számolja hogy mikor megy jobbra, stb

    // controls temporary le vannak írva a controls.md-be, nem lesz váloztatható, csak megjegyezzem

    // TODO am ez 0, ha nem sikerült
    // szóval ezt is hibakezelni?
    // TODO csak akkor, ha még nem nyomtak le irányítási gombot
    //  50 fps, de lehetne timer indítgatásokkal is
    // gravitáció időzítője
    SDL_TimerID id = SDL_AddTimer(20, gravity_timer, NULL);
    // TODO valamiér a gravitáció egyszer mindig meghívódik az elején
    // emiatt belépéskor a tábla kétszer lesz lerajzolva

    // hányszor volt gravity_event
    // egyenlőre így lesz, maradékkal nézve hogy ritkítsam
    // de lehet hogy majd csak mindig új időzítőket indítok pieceszám váltásnál
    int timer_counter = 0;

    // fogad-e bemenetek a loop
    bool enable_events = true;

    SDL_Event event;

    // TODO mindkettő végére egy count?
    //  hányszor került feljebb a rész forgatás által
    int upkick = 0;
    // hányszor próbálta már a gravitáció lerakni a részt
    int nat_grav = 0;

    // TODO a switchekben meg változónév felvevésekor konzisztens sorrendbe legyenek a gombok nézve
    //  pl left right sodt drop

    // TODO ez helyett timer_id_soft?
    // kezdőértéknek jó a 0?
    // TODO le kell kezelni, ha a timer nem helyes? (0-t ad vissza)
    // persze létrehozáskor
    SDL_TimerID timerid_left;
    SDL_TimerID timerid_right;
    SDL_TimerID timerid_soft = 0;

    bool held_left = false;
    bool held_right = false;
    bool held_soft = false;

    bool held_hard = false;
    bool held_cw = false;
    bool held_ccw = false;

    bool selected_settings = false;
    bool selected_toplist = false;

    int return_code = -1;
    bool quit = false;
    // kilépés mindig megy
    while (SDL_WaitEvent(&event) && event.type != SDL_QUIT && !quit) {
        // le kell-e rajzolni az egész képernyőt
        bool draw = false;

        // gravitáció fogadásának külön állítása
        // ha a switchben lenne, nem lehetne visszakapcsolni
        if (event.type == SDL_USEREVENT + 1) {
            enable_events = false;
            continue;
        }
        if (event.type == SDL_USEREVENT + 2) {
            enable_events = true;
            continue;
        }
        // ha nem fogad bemenetet, eldobja, várja a következőt
        // ha másra nem, gravitáció eventekre megy
        // TODO szebben megoldani?
        // csak a keydown eventeket dobja ki, mert csak azok bajosak
        // hogy ne tudja forgatni a részeket
        // bemozgatni a switchbe a checket
        if (!enable_events && event.type == SDL_KEYDOWN) {
            continue;
        }

        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)

                {
                    case SDLK_w:
                        if (!held_hard) {
                            hard_drop(rd, game_state, &nat_grav, &upkick);
                            held_hard = true;
                            draw = true;
                        }
                        break;
                    case SDLK_a:
                        if (!held_left) {
                            move_left(game_state);
                            held_left = true;
                            timerid_left = SDL_AddTimer(150, left_timer, NULL);
                            draw = true;
                        }
                        break;
                    case SDLK_s:
                        // TODO talán ide kéne, hogy a grav ne hasson ilyenkor azonnal
                        // mmint ha a soft drop-ot megnyomtam, akkor ne rakhassa egyből lejjebb a gravitáció
                        // de ha nagyon szívás megoldani kihagyom
                        // amikor fontos, már nem vehető észre, amikor észrevehető, nem baj
                        if (!held_soft) {
                            soft_drop(game_state);
                            held_soft = true;
                            // időzítő elindítása
                            timerid_soft = SDL_AddTimer(150, soft_timer, NULL);
                            draw = true;
                        }
                        break;
                    case SDLK_d:
                        if (!held_right) {
                            move_right(game_state);
                            held_right = true;
                            timerid_right = SDL_AddTimer(150, right_timer, NULL);
                            draw = true;
                        }
                        break;
                    case SDLK_i:
                        if (!held_cw) {
                            rotate_cw(game_state, &upkick);
                            held_cw = true;
                            draw = true;
                        }
                        break;
                    case SDLK_o:
                        if (!held_ccw) {
                            rotate_ccw(game_state, &upkick);
                            held_ccw = true;
                            draw = true;
                        }
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
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        held_hard = false;
                        break;
                    case SDLK_a:
                        held_left = false;
                        SDL_RemoveTimer(timerid_left);
                        break;
                    case SDLK_s:
                        held_soft = false;
                        SDL_RemoveTimer(timerid_soft);
                        break;
                    case SDLK_d:
                        held_right = false;
                        SDL_RemoveTimer(timerid_right);
                        break;
                    case SDLK_i:
                        held_cw = false;
                        break;
                    case SDLK_o:
                        held_ccw = false;
                        break;
                }
                break;
            case SDL_USEREVENT:  // gravitáció
                // TODO gravitáció állítása
                if (timer_counter % 500 == 0) {
                    natural_gravity(rd, game_state, &nat_grav, &upkick);
                    timer_counter = 0;
                    draw = true;
                }
                timer_counter++;

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
                        // szürke keret, ha ki van választva
                        render_text_block(rd, "Beállítások", 1300, 500, 0xFF0000FF);
                    } else {
                        render_text_block(rd, "Beállítások", 1300, 500, 0x000000FF);
                    }
                    selected_settings = in_settings;
                    // ha változott, ki kell rajzolni
                    SDL_RenderPresent(rd.renderer);
                }
                if (selected_toplist != in_toplist) {
                    if (in_toplist) {
                        render_text_block(rd, "Ranglista", 1320, 600, 0xFF0000FF);
                    } else {
                        render_text_block(rd, "Ranglista", 1320, 600, 0x000000FF);
                    }
                    selected_toplist = in_toplist;
                    SDL_RenderPresent(rd.renderer);
                }
            } break;
            case SDL_MOUSEBUTTONDOWN: {
                // TODO ez csak bármi egérgomb lenyomása
                int x = event.motion.x;
                int y = event.motion.y;
                bool in_settings = 1298 <= x && x <= 1499 && 498 <= y && y <= 533;
                bool in_toplist = 1318 <= x && x <= 1483 && 598 <= y && y <= 633;
                if (in_settings) {
                    quit = true;
                    return_code = 1;
                }
                if (in_toplist) {
                    quit = true;
                    return_code = 2;
                }
            } break;
            default:
                break;
        }

        // TODO ez nem igaz: teljes táblarajzolás, azaz meghívja a RenderClear()-t
        if (draw)
            render_game(rd, game_state);
    }

    // TODO minden időzítő leállítása
    // időzítő leállítása
    SDL_RemoveTimer(id);

    // TODO ez csak placeholder return code
    return return_code;
}

// saját usereventek:
// SDL_USEREVENT: gravitációért felelős event
// SDL_USEREVENT + 1: leállítja a bemenetek kezelését, kivéve SDL_USEREVENT + 2-t
// SDL_USEREVENT + 2: folytatja az eventek kezelését
// SDL_USEREVENT + 10: folyamatos balra lépés event
// SDL_USEREVENT + 11: folyamatos jobbra lépés event
// SDL_USEREVENT + 12: folyamatos soft drop event

// mindig amikor ki akar lépni az event loopból egy kódrész beállítja a quit-et igazra, és a return_code-ot arra, ami a kilépéshez megfelel

// return code-ok:
// 0: felhasználó kilép x-el
// 1: beállítások menü
// 2: toplista menü
// 3: Game Over screen
// -1: memóriafoglalás / egyéb hiba