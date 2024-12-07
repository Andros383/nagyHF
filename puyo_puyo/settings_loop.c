#include "settings_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debugmalloc.h"
#include "render.h"

// a + és - gombok rajzolásának segítésére
// kirajzol egy 50*50-es gombot, egy karakterrel a közepén
// a button_text itt vagy "+" vagy "-"
// x, y a bal felső sarok koordinátái
// outline_code a keret színe
static void render_button(CommonRenderData rd, char *button_text, int x, int y, SDL_Color outline_color) {
    boxRGBA(rd.renderer, x, y, x + 50, y + 50, outline_color.r, outline_color.g, outline_color.b, outline_color.a);
    // közepét fehérre kell színezni, mert a + nem tölti ki a négyzetet
    boxRGBA(rd.renderer, x + 2, y + 2, x + 50 - 2, y + 50 - 2, 255, 255, 255, 255);
    // kézzel középre állított
    render_text_block(rd, button_text, x + 17, y + 10, COLOR_TRANSPARENT);
}
// Magasság és szélesség szövegek kirajzolása
static void render_width_height_labels(CommonRenderData rd, int board_width, int board_height) {
    char label[50 + 1];
    sprintf(label, "Szélesség: %2d", board_width);
    render_text_block(rd, label, 500, 200, COLOR_TRANSPARENT);

    sprintf(label, "Magasság: %2d", board_height);
    render_text_block(rd, label, 850, 200, COLOR_TRANSPARENT);
}
// kirajzolja első belépéskor a képernyőt
static void starting_render(CommonRenderData rd, int board_width, int board_height) {
    // fehér háttér
    boxRGBA(rd.renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 255, 255, 255, 255);

    // Magasság és Szélesség kirajzolása
    render_width_height_labels(rd, board_width, board_height);

    // Szélesség + - gombjai
    render_button(rd, "-", 500, 125, COLOR_BLACK);
    render_button(rd, "+", 684, 125, COLOR_BLACK);

    // Magasság + - gombjai
    render_button(rd, "-", 850, 125, COLOR_BLACK);
    render_button(rd, "+", 1016, 125, COLOR_BLACK);

    // Kilépő gomb
    render_button(rd, "<", 125, 125, COLOR_BLACK);

    // Irányítások, pontszámítás
    render_text_block(rd, "Irányítások", 701, 300, COLOR_TRANSPARENT);

    render_text_block(rd, "Balra:", 500, 350, COLOR_TRANSPARENT);
    render_text_block(rd, "Jobbra:", 500, 400, COLOR_TRANSPARENT);
    render_text_block(rd, "Lassú leejtés:", 500, 450, COLOR_TRANSPARENT);
    render_text_block(rd, "Gyors lerakás:", 500, 500, COLOR_TRANSPARENT);
    render_text_block(rd, "Forgatás 1:", 500, 550, COLOR_TRANSPARENT);
    render_text_block(rd, "Forgatás 2:", 500, 600, COLOR_TRANSPARENT);
    render_text_block(rd, "Újraindítás:", 500, 650, COLOR_TRANSPARENT);
    render_text_block(rd, "Menüből kilépés:", 500, 700, COLOR_TRANSPARENT);
    render_text_block(rd, "Program lezárása:", 500, 750, COLOR_TRANSPARENT);

    render_text_block(rd, "A", 850, 350, COLOR_TRANSPARENT);
    render_text_block(rd, "D", 850, 400, COLOR_TRANSPARENT);
    render_text_block(rd, "S", 850, 450, COLOR_TRANSPARENT);
    render_text_block(rd, "W", 850, 500, COLOR_TRANSPARENT);
    render_text_block(rd, "I", 850, 550, COLOR_TRANSPARENT);
    render_text_block(rd, "O", 850, 600, COLOR_TRANSPARENT);
    render_text_block(rd, "ESC", 850, 650, COLOR_TRANSPARENT);
    render_text_block(rd, "ESC", 850, 700, COLOR_TRANSPARENT);
    render_text_block(rd, "Ablak lezárása", 850, 750, COLOR_TRANSPARENT);

    render_text_block(rd, "Minden törlési lépés után kapott pontszám:", 422, 850, COLOR_TRANSPARENT);
    render_text_block(rd, "2^(lánclépés-1) * (10 * törölt csoportok + törölt blokkok)", 278, 900, COLOR_TRANSPARENT);
}

// gombok újrarajzolása az alapján, hogy rajta van-e és volt-e az egér
// visszaadja, rajzolt-e
static bool rerender_plus_minus_button(CommonRenderData rd, char *button_text, int x, int y, bool *selected, bool in) {
    if (*selected != in) {
        if (in)
            render_button(rd, button_text, x, y, COLOR_RED);
        else
            render_button(rd, button_text, x, y, COLOR_BLACK);
        *selected = in;
        return true;
    }
    return false;
}

int settings_loop(SDL_Renderer *renderer, TTF_Font *font, int *board_width, int *board_height) {
    // nem fogja használni, csak a renderer és font változókat
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    // kezdőképernyő kirajzolása
    starting_render(rd, *board_width, *board_height);
    SDL_RenderPresent(renderer);

    // bal egérgomb le van-e nyomva
    bool held_left = false;

    // gombok ki vannak-e választva
    bool selected_width_plus = false;
    bool selected_width_minus = false;
    bool selected_height_plus = false;
    bool selected_height_minus = false;
    bool selected_back = false;

    int return_code = -1;
    bool quit = false;
    SDL_Event event;
    while (SDL_WaitEvent(&event) && !quit) {
        bool draw = false;
        switch (event.type) {
            case SDL_MOUSEMOTION: {
                // ha az egér az egyik gombon van, a keret pirosra színezése
                int x = event.motion.x;
                int y = event.motion.y;

                // minden gombnak az y koordinátája egy helyen van, azaz elég egyszer tesztelni rá
                bool in_y_range = 125 <= y && y <= 175;

                bool in_width_minus = 500 <= x && x <= 550 && in_y_range;
                bool in_width_plus = 684 <= x && x <= 734 && in_y_range;
                bool in_height_minus = 850 <= x && x <= 900 && in_y_range;
                bool in_height_plus = 1016 <= x && x <= 1066 && in_y_range;
                bool in_back = 125 <= x && x <= 175 && in_y_range;

                // ha bármelyik igaz, akkor a rajzolja le
                draw = draw | rerender_plus_minus_button(rd, "-", 500, 125, &selected_width_minus, in_width_minus);
                draw = draw | rerender_plus_minus_button(rd, "+", 684, 125, &selected_width_plus, in_width_plus);
                draw = draw | rerender_plus_minus_button(rd, "-", 850, 125, &selected_height_minus, in_height_minus);
                draw = draw | rerender_plus_minus_button(rd, "+", 1016, 125, &selected_height_plus, in_height_plus);
                draw = draw | rerender_plus_minus_button(rd, "<", 125, 125, &selected_back, in_back);
            } break;
            case SDL_MOUSEBUTTONDOWN:
                // bal kattintásra növeli/csökkenti a magasságot/szélességet
                if (event.button.button == SDL_BUTTON_LEFT && !held_left) {
                    held_left = true;
                    int x = event.motion.x;
                    int y = event.motion.y;

                    bool in_y_range = 125 <= y && y <= 175;
                    bool in_width_minus = 500 <= x && x <= 550 && in_y_range;
                    bool in_width_plus = 684 <= x && x <= 734 && in_y_range;
                    bool in_height_minus = 850 <= x && x <= 900 && in_y_range;
                    bool in_height_plus = 1016 <= x && x <= 1066 && in_y_range;
                    bool in_back = 125 <= x && x <= 175 && in_y_range;

                    if (in_width_minus)
                        (*board_width)--;
                    if (in_width_plus)
                        (*board_width)++;
                    if (in_height_minus)
                        (*board_height)--;
                    if (in_height_plus)
                        (*board_height)++;

                    if (in_back) {
                        quit = true;
                        return_code = 1;
                    }

                    // ha bármelyikben benne volt, újrarajzolja
                    draw = draw | in_width_minus;
                    draw = draw | in_width_plus;
                    draw = draw | in_height_minus;
                    draw = draw | in_height_plus;

                    if (*board_width < 2) *board_width = 2;
                    if (*board_width > 20) *board_width = 20;
                    if (*board_height < 2) *board_height = 2;
                    if (*board_height > 40) *board_height = 40;

                    render_width_height_labels(rd, *board_width, *board_height);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                // felengedni csak egyszer lehet
                if (event.button.button == SDL_BUTTON_LEFT)
                    held_left = false;
                break;
            case SDL_KEYDOWN:
                // ESC-re kilép a játékképernyőre
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return_code = 1;
                    quit = true;
                }
                break;
            case SDL_QUIT:
                // kilép X lenyomására
                return_code = 0;
                quit = true;
                break;
            default:
                break;
        }
        if (draw) SDL_RenderPresent(renderer);
    }
    return return_code;
}