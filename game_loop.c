#include "game_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
bool pop_groups(SDL_Renderer *renderer, GameState *game_state) {
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

    // TODO átgondolni ez jó-e, hogy csak itt lerenderelem
    // elméletben nem tud visszamenni a main event loopba, azaz tuti nem fog közben mozogni a rész
    render_game(renderer, game_state);

    if (pop) {
        for (int pop_anim_count = 0; pop_anim_count < 3; pop_anim_count++) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    if (clear[x][y]) {
                        render_animation_block(renderer, game_state, x, y, (1.0 / 3.0) * pop_anim_count);
                    }
                }
            }
            SDL_RenderPresent(renderer);
            SDL_Delay(200);
        }

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
void lock_active_piece(SDL_Renderer *renderer, GameState *game_state, int *nat_grav, int *upkick, bool *enable_events) {
    // gravitáció kikapcsolása, mert időbe kerül, mire az animáció lejátszódik
    *enable_events = false;

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

    // részre vonatkozó anti-stalling számlálók törlése
    *nat_grav = 0;
    *upkick = 0;

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
    while (pop_groups(renderer, game_state)) {
        gravity(game_state);
    }

    // TODO rendszerezni a pontkezelő függvényeket?
    // talán saját fájlba a pont, sajátba a game_state, sajátba a render függvényeket
    // és akkor egymást hivogatják
    game_state->score_data.placed_pieces++;

    // gravitáció visszakapcsolása
    *enable_events = true;
}

// az aktív részt letolja, amennyire tudja, majd le is rakja
void hard_drop(SDL_Renderer *renderer, GameState *game_state, int *nat_grav, int *upkick, bool *enable_events) {
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
    lock_active_piece(renderer, game_state, nat_grav, upkick, enable_events);
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
void natural_gravity(SDL_Renderer *renderer, GameState *game_state, int *nat_grav, int *upkick, bool *enable_events) {
    // lejjebb mozgatja a részt
    if (!soft_drop(game_state)) {
        // ha nem mozgott lejjebb, növeli a számlálót
        if (*nat_grav >= 3) {
            // ha elérte a határt, lehelyezi
            lock_active_piece(renderer, game_state, nat_grav, upkick, enable_events);
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

// TODO itt sok a másolt kód, más-más néven van ugyan az elmentve, de legalább érthető?

void rotate_cw(SDL_Renderer *renderer, GameState *game_state, int *nat_grav, int *upkick) {
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
void rotate_ccw(SDL_Renderer *renderer, GameState *game_state, int *nat_grav, int *upkick) {
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

Uint32 gravity_timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);
    return ms; /* ujabb varakozas */
}

int game_loop(SDL_Renderer *renderer, GameState *game_state) {
    // TODO mindenféle dolog, pl ne mindig rajzolja újra, számolja hogy mikor megy jobbra, stb

    // controls temporary le vannak írva a controls.md-be, nem lesz váloztatható, csak megjegyezzem
    // TODO am ez 0, ha nem sikerült
    // szóval ezt is hibakezelni?

    // TODO csak akkor, ha még nem nyomtak le irányítási gombot
    //  50 fps, de lehetne timer indítgatásokkal is
    // gravitáció időzítője
    SDL_TimerID id = SDL_AddTimer(20, gravity_timer, NULL);

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

    // kilépés mindig megy
    while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
        // ha nem fogad bemenetet, eldobja, várja a következőt
        if (!enable_events) continue;
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)

                {
                    case SDLK_w:
                        hard_drop(renderer, game_state, &nat_grav, &upkick, &enable_events);
                        break;
                    case SDLK_a:
                        move_left(game_state);
                        break;
                    case SDLK_s:
                        // TODO talán ide kéne, hogy a grav ne hasson ilyenkor azonnal
                        // mmint ha a soft drop-ot megnyomtam, akkor ne rakhassa egyből lejjebb a gravitáció
                        // de ha nagyon szívás megoldani kihagyom
                        // amikor fontos, már nem vehető észre, amikor észrevehető, nem baj
                        soft_drop(game_state);
                        break;
                    case SDLK_d:
                        move_right(game_state);
                        break;
                    case SDLK_i:
                        rotate_cw(renderer, game_state, &nat_grav, &upkick);
                        break;
                    case SDLK_o:
                        rotate_ccw(renderer, game_state, &nat_grav, &upkick);
                        break;
                    case SDLK_ESCAPE:
                        // TODO újraindítani a játékot
                        break;
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
            case SDL_USEREVENT:

                if (timer_counter % 50 == 0) {
                    natural_gravity(renderer, game_state, &nat_grav, &upkick, &enable_events);
                    timer_counter = 0;
                }
                timer_counter++;

                break;
            default:
                break;
        }
        // TODO nem kell mindig
        // am de
        render_game(renderer, game_state);
    }
    // időzítő leállítása
    SDL_RemoveTimer(id);

    // TODO ez csak placeholder return code
    return -1;
}