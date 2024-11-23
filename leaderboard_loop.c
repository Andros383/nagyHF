#include "leaderboard_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "game_render.h"
void render_entry(CommonRenderData rd, int y, Entry e) {
    render_text_block(rd, e.name, 200, y, COLOR_TRANSPARENT);

    // többi adat nem string formátumú
    char tmp[50 + 1];

    sprintf(tmp, "%d", e.score_data.score);
    render_text_block(rd, tmp, 500, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d", e.score_data.longest_chain);
    render_text_block(rd, tmp, 800, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d", e.score_data.placed_pieces);
    render_text_block(rd, tmp, 1000, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d", e.width);
    render_text_block(rd, tmp, 1200, y, COLOR_TRANSPARENT);

    sprintf(tmp, "%d", e.height);
    render_text_block(rd, tmp, 1400, y, COLOR_TRANSPARENT);
}
// TODO Entries felszabadítása ha kész
// kirajzolja a top 10 pontszámot a rendezési tömb alapján
// ha a tömbben -1 van, onnantól már nincs megjelenítendő pontszám
void render_entries(CommonRenderData rd, Entries entries, int sorted[10]) {
    // a pontszámok helyének törlése
    boxRGBA(rd.renderer, 0, 175, 1600, 1000, 255, 255, 255, 255);

    int y = 200;
    for (int i = 0; i < 10; i++) {
        int pos = sorted[i];
        // a többi adat innentől szemét, mert nincs elég érték a ranglistán
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
void render_buttons(CommonRenderData rd, int sorted_by) {
    // vissza gomb koordinátái a rajzoláshoz
    int x_back = 25;
    int y_back = 25;
    boxRGBA(rd.renderer, x_back, y_back, x_back + 50, y_back + 50, 0, 0, 0, 255);
    boxRGBA(rd.renderer, x_back + 2, y_back + 2, x_back + 50 - 2, y_back + 50 - 2, 255, 255, 255, 255);
    render_text_block(rd, "<", x_back + 17, y_back + 10, COLOR_TRANSPARENT);

    // nincs kerete a gombnak, ha nem kattintható
    render_text_block(rd, "Név", 200, 100, COLOR_TRANSPARENT);
    render_text_block(rd, "Szélesség", 1200, 100, COLOR_TRANSPARENT);
    render_text_block(rd, "Magasság", 1400, 100, COLOR_TRANSPARENT);

    // TODO ez lehet felesleges, csak első eventig?
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

// TODO ugyan az a név mint a settings loopból
void starting_render(CommonRenderData rd) {
    // TODO koordinátákkal
    boxRGBA(rd.renderer, 0, 0, 1600, 1000, 255, 255, 255, 255);
}

// újrarajzolja a kiválasztás szerint a címkét, és beállítja, hogy rajta van-e az egér
// visszaadja, rajzolt-e
bool rerender_label(CommonRenderData rd, char *label, int x, int y, bool *selected, bool in) {
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

bool sort_entries(Entries entries, int sorted_ids[3][10]) {
    // kimenet nullázása
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            sorted_ids[i][j] = -1;
        }
    }

    // ide kerül mind a három fajta szortírozás
    // egy-egy részeredmény első 10 eleme pedig átmásolódik a sorted_ids-be
    int *temp_sort = (int *)malloc(entries.len * sizeof(int));
    if (temp_sort == NULL) {
        return false;
    }

    // alap feltöltés (0, 1, 2...)
    for (int i = 0; i < entries.len; i++) {
        temp_sort[i] = i;
    }

    // növekvő sorrendbe rendezés

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
    for (int i = 0; i < entries.len && i < 10; i++) {
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
    for (int i = 0; i < entries.len && i < 10; i++) {
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
    for (int i = 0; i < entries.len && i < 10; i++) {
        sorted_ids[2][i] = temp_sort[i];
    }
    return true;
}
int leaderboard_loop(SDL_Renderer *renderer, TTF_Font *font) {
    // csak ezt a kettőt használja
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    starting_render(rd);

    Entries entries = new_entries();

    bool success = read_entries(&entries);
    if (!success) return -1;

    int sorted_by = 0;

    // sorted_ids[0]-ban a sorted_by = 0-hoz tartozó adat szerinti szortírozás sorrendje van (0-nál pontszám)
    int sorted_ids[3][10];

    sort_entries(entries, sorted_ids);

    render_entries(rd, entries, sorted_ids[sorted_by]);
    render_buttons(rd, sorted_by);

    // TODO ez nem biztos hogy kell
    SDL_RenderPresent(renderer);

    bool selected_score = false;
    bool selected_chain = false;
    bool selected_pieces = false;
    bool selected_back = false;

    bool held_left = false;

    bool quit = false;
    int return_code = -1;
    SDL_Event event;
    while (SDL_WaitEvent(&event) && !quit) {
        bool draw = false;
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // vissza a főmenübe
                    return_code = 1;
                    quit = true;
                }

                break;
            case SDL_MOUSEMOTION: {
                // ha az egér az egyik gombon van, a keret pirosra színezése int x = event.motion.x;
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
                quit = true;
                return_code = 0;
                break;
            default:
                // TODO
                break;
        }
        if (draw) SDL_RenderPresent(renderer);
    }

    free(entries.array);

    return return_code;
}