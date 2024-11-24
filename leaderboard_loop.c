#include "leaderboard_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "debugmalloc.h"
#include "render.h"

#define SHOWN_NAMES 15
// egy eredmény kirajzolása
static void render_entry(CommonRenderData rd, int y, Entry e) {
    render_text_block(rd, e.name, 100, y, COLOR_TRANSPARENT);

    // többi adat nem string formátumú
    char tmp[50 + 1];

    sprintf(tmp, "%d", e.score_data.score);
    render_text_block(rd, tmp, 500, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d", e.score_data.longest_chain);
    render_text_block(rd, tmp, 800, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d", e.score_data.placed_pieces);
    render_text_block(rd, tmp, 1000, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d x %d", e.width, e.height);
    render_text_block(rd, tmp, 1200, y, COLOR_TRANSPARENT);
}
// kirajzolja a legjobb pontszámokat a rendezési tömb alapján
// ha a rendezési tömbben -1 van, már nincs megjelenítendő pontszám
static void render_entries(CommonRenderData rd, Entries entries, int sorted[SHOWN_NAMES]) {
    // a pontszámok helyének törlése
    boxRGBA(rd.renderer, 0, 175, 1600, 1000, 255, 255, 255, 255);

    int y = 200;
    for (int i = 0; i < SHOWN_NAMES; i++) {
        int pos = sorted[i];
        // a többi adat innentől szemét, mert nincs elég eredmény a ranglistán
        if (pos == -1) break;

        Entry e = entries.array[pos];
        render_entry(rd, y, e);
        y += 50;
    }
}

// kirajzolja a gombokat, pirossal azt, ami alapján éppen szortírozva van
// 0: pontszám
// 1: leghosszabb lánc
// 2: lerakott részek
static void render_buttons(CommonRenderData rd, int sorted_by) {
    // vissza gomb koordinátái a rajzoláshoz
    int x_back = 25;
    int y_back = 25;
    boxRGBA(rd.renderer, x_back, y_back, x_back + 50, y_back + 50, 0, 0, 0, 255);
    boxRGBA(rd.renderer, x_back + 2, y_back + 2, x_back + 50 - 2, y_back + 50 - 2, 255, 255, 255, 255);
    render_text_block(rd, "<", x_back + 17, y_back + 10, COLOR_TRANSPARENT);

    // nincs kerete a gombnak, ha nem kattintható
    render_text_block(rd, "Név", 100, 100, COLOR_TRANSPARENT);
    render_text_block(rd, "Táblaméret", 1200, 100, COLOR_TRANSPARENT);

    SDL_Color point_color = COLOR_BLACK;
    SDL_Color chain_color = COLOR_BLACK;
    SDL_Color pieces_color = COLOR_BLACK;

    // a szortírozott statisztika mindig piros keretű
    switch (sorted_by) {
        case 0:
            point_color = COLOR_RED;
            break;
        case 1:
            chain_color = COLOR_RED;
            break;
        case 2:
            pieces_color = COLOR_RED;
            break;
        default:
            break;
    }
    render_text_block(rd, "Pontszám", 500, 100, point_color);
    render_text_block(rd, "Max lánc", 800, 100, chain_color);
    render_text_block(rd, "Részek", 1000, 100, pieces_color);
}

// újrarajzolja a kiválasztás szerint a címkét, és beállítja, hogy rajta van-e az egér
// visszaadja, rajzolt-e
static bool rerender_label(CommonRenderData rd, char *label, int x, int y, bool *selected, bool in) {
    if (*selected != in) {
        if (in)
            render_text_block(rd, label, x, y, COLOR_RED);
        else
            render_text_block(rd, label, x, y, COLOR_BLACK);

        *selected = in;
        return true;
    }
    return false;
}

// szortírozza az elemeket mindhárom szempont alapján, és elmenti a sorted_ids-be
// 0: pontszám, 1: leghosszabb lánc, 2: lerakott részek
// visszaadja, hogy sikeres volt-e a szortírozás
static bool sort_entries(Entries entries, int sorted_ids[3][SHOWN_NAMES]) {
    // kimenet nullázása
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < SHOWN_NAMES; j++) {
            sorted_ids[i][j] = -1;
        }
    }

    // ha nincs mit szortírozni, azonnal visszatér üres kimenettel
    if (entries.len == 0) {
        printf("sort_entries: ures scores.txt, nincs szortirozas\n");
        return true;
    }

    // ide kerül mind a három fajta szortírozás
    // egy-egy részeredményből annyi átkerül sorted_ids[]-be, amennyi kifér a képernyőre
    int *temp_sort = (int *)malloc(entries.len * sizeof(int));
    if (temp_sort == NULL) {
        printf("sort_entries: memoriafoglalasi hiba\n");
        return false;
    }

    // alap feltöltés (0, 1, 2...)
    for (int i = 0; i < entries.len; i++) {
        temp_sort[i] = i;
    }

    // növekvő sorrendbe rendezések buborékrendezéssel

    // pontszám
    for (int i = 0; i < entries.len; i++) {
        for (int j = i + 1; j < entries.len; j++) {
            int i_val = entries.array[temp_sort[i]].score_data.score;
            int j_val = entries.array[temp_sort[j]].score_data.score;
            if (i_val < j_val) {
                int tempindex = temp_sort[i];
                temp_sort[i] = temp_sort[j];
                temp_sort[j] = tempindex;
            }
        }
    }
    // átmásolás a kimenetbe
    for (int i = 0; i < entries.len && i < SHOWN_NAMES; i++) {
        sorted_ids[0][i] = temp_sort[i];
    }

    // lánchossz
    for (int i = 0; i < entries.len; i++) {
        for (int j = i + 1; j < entries.len; j++) {
            int i_val = entries.array[temp_sort[i]].score_data.longest_chain;
            int j_val = entries.array[temp_sort[j]].score_data.longest_chain;
            if (i_val < j_val) {
                int tempindex = temp_sort[i];
                temp_sort[i] = temp_sort[j];
                temp_sort[j] = tempindex;
            }
        }
    }
    // átmásolás a kimenetbe
    for (int i = 0; i < entries.len && i < SHOWN_NAMES; i++) {
        sorted_ids[1][i] = temp_sort[i];
    }

    // lerakott részek
    for (int i = 0; i < entries.len; i++) {
        for (int j = i + 1; j < entries.len; j++) {
            int i_val = entries.array[temp_sort[i]].score_data.placed_pieces;
            int j_val = entries.array[temp_sort[j]].score_data.placed_pieces;
            if (i_val < j_val) {
                int tempindex = temp_sort[i];
                temp_sort[i] = temp_sort[j];
                temp_sort[j] = tempindex;
            }
        }
    }
    // átmásolás a kimenetbe
    for (int i = 0; i < entries.len && i < SHOWN_NAMES; i++) {
        sorted_ids[2][i] = temp_sort[i];
    }

    free(temp_sort);
    return true;
}
int leaderboard_loop(SDL_Renderer *renderer, TTF_Font *font) {
    // nem fogja használni, csak a renderer és font változókat
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    Entries entries = new_entries();

    bool success = read_entries(&entries);
    if (!success) return -1;

    int sorted_by = 0;

    // mindhárom szortírozás szerinti sorrend
    int sorted_ids[3][SHOWN_NAMES];

    success = sort_entries(entries, sorted_ids);
    if (!success) {
        return -1;
    }

    // első képernyő kirajzolása
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 255, 255, 255, 255);
    render_entries(rd, entries, sorted_ids[sorted_by]);
    render_buttons(rd, sorted_by);

    SDL_RenderPresent(renderer);

    // gombok ki vannak-e választva
    bool selected_score = false;
    bool selected_chain = false;
    bool selected_pieces = false;
    bool selected_back = false;

    // bal egérgomb le van-e nyomva
    bool held_left = false;

    bool quit = false;
    int return_code = -1;
    SDL_Event event;
    while (SDL_WaitEvent(&event) && !quit) {
        bool draw = false;
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // ESC-re visszalép a játékképernyőre
                    return_code = 1;
                    quit = true;
                }

                break;
            case SDL_MOUSEMOTION: {
                // ha az egér az egyik gombon van, a keret pirosra színezése
                int x = event.motion.x;
                int y = event.motion.y;

                // minden gombnak az y koordinátája egy helyen van, azaz elég egyszer tesztelni rá
                bool in_y_range = 100 <= y && y <= 132;

                bool in_score = 500 <= x && x <= 644 && in_y_range;
                bool in_chain = 800 <= x && x <= 944 && in_y_range;
                bool in_pieces = 1000 <= x && x <= 1108 && in_y_range;

                // pirossal rajzolja ki, "válassza ki", vagy ha rajta van a kurzor, vagy ha ez alapján szortírozunk
                draw = draw | rerender_label(rd, "Pontszám", 500, 100, &selected_score, in_score | (sorted_by == 0));
                draw = draw | rerender_label(rd, "Max lánc", 800, 100, &selected_chain, in_chain | (sorted_by == 1));
                draw = draw | rerender_label(rd, "Részek", 1000, 100, &selected_pieces, in_pieces | (sorted_by == 2));

                // vissza gombot külön kell kezelni, mert más a helye és a magassága is
                bool in_back = (25 <= x && x <= 75) && (25 <= y && y <= 75);
                SDL_Color back_outline_color = COLOR_BLACK;
                if (selected_back != in_back) {
                    if (in_back)
                        back_outline_color = COLOR_RED;
                    else
                        back_outline_color = COLOR_BLACK;
                    selected_back = in_back;

                    // vissza gomb koordinátái a rajzoláshoz
                    int x_back = 25;
                    int y_back = 25;
                    boxRGBA(rd.renderer, x_back, y_back, x_back + 50, y_back + 50, back_outline_color.r, back_outline_color.g, back_outline_color.b, back_outline_color.a);
                    boxRGBA(rd.renderer, x_back + 2, y_back + 2, x_back + 50 - 2, y_back + 50 - 2, 255, 255, 255, 255);
                    render_text_block(rd, "<", x_back + 17, y_back + 10, COLOR_TRANSPARENT);
                    draw = true;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                // bal egérgombbal gombok kattintása
                if (event.button.button == SDL_BUTTON_LEFT && !held_left) {
                    held_left = true;
                    int x = event.motion.x;
                    int y = event.motion.y;

                    bool in_y_range = 100 <= y && y <= 132;

                    bool in_score = 500 <= x && x <= 644 && in_y_range;
                    bool in_chain = 800 <= x && x <= 944 && in_y_range;
                    bool in_pieces = 1000 <= x && x <= 1108 && in_y_range;
                    bool in_back = (25 <= x && x <= 75) && (25 <= y && y <= 75);

                    // kattintásra a szortírozási preferencia változik
                    if (in_score) sorted_by = 0;
                    if (in_chain) sorted_by = 1;
                    if (in_pieces) sorted_by = 2;
                    // kilépés vissza gombbal
                    if (in_back) {
                        return_code = 1;
                        quit = true;
                    }

                    // gombok újrarajzolása
                    draw = draw | in_score;
                    draw = draw | in_chain;
                    draw = draw | in_pieces;

                    render_buttons(rd, sorted_by);
                    render_entries(rd, entries, sorted_ids[sorted_by]);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                // felengedni csak egyszer lehet
                if (event.button.button == SDL_BUTTON_LEFT)
                    held_left = false;
                break;
            case SDL_QUIT:
                // kilépés X gombbal
                quit = true;
                return_code = 0;
                break;
            default:
                break;
        }
        if (draw) SDL_RenderPresent(renderer);
    }

    free(entries.array);

    return return_code;
}