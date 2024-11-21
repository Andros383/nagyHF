#include "settings_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game_render.h"

// a + és - gombok rajzolásának segítésére
// kirajzol egy 50*50-es gombot, egy karakterrel a közepén
// a button_text itt vagy "+" vagy "-"
// x, y a bal felső sarok koordinátái
// border_outline_code a keret színe kód alapján, a render_text_block alapján
// 1: fekete
// 2: piros

static void render_button(CommonRenderData rd, char *button_text, int x, int y, int outline_color_code) {
    // TODO itt is lehetne egybevonni az = jeleket
    int r = 0, g = 0, b = 0, a = 0;

    switch (outline_color_code) {
        case 1:
            a = 255;
            break;
        case 2:
            r = 255;
            a = 255;
            break;
        default:
            printf("Invalid outline_color_code in settings_loop.c\n");
            break;
    }
    // keret
    boxRGBA(rd.renderer, x, y, x + 50, y + 50, r, g, b, a);
    // közepét fehérre kell színezni, mert a + nem tölti ki a négyzetet
    boxRGBA(rd.renderer, x + 2, y + 2, x + 50 - 2, y + 50 - 2, 255, 255, 255, 255);
    // kézzel középre állított
    render_text_block(rd, button_text, x + 17, y + 10, 0);
}
static void render_width_height_labels(CommonRenderData rd, int board_width, int board_height) {
    // alap állapot kirajzolása

    // TODO wacky string kezelés

    // két számjegy
    // felveszi a leghosszabb stringet, amit majd ki fog írni
    char width_label[] = "Szélesség: 10";
    // beállítja a szélességet
    sprintf(width_label, "Szélesség: %2d", board_width);

    char height_label[] = "Magasság: 10";
    sprintf(height_label, "Magasság: %2d", board_height);

    render_text_block(rd, width_label, 500, 200, 0);
    render_text_block(rd, height_label, 850, 200, 0);
}
// kirajzolja első belépéskor a képernyőt
static void render_starting_screen(CommonRenderData rd, int board_width, int board_height) {
    // TODO valami táblázatszerűvé csinálás? vonalakkal meg minden

    // TODO width és height behozása mind glob konstans
    // fehér háttér
    boxRGBA(rd.renderer, 0, 0, 1600, 1000, 255, 255, 255, 255);

    // alap állapot kirajzolása

    render_width_height_labels(rd, board_width, board_height);

    // Szélesség + - gombjai
    render_button(rd, "-", 500, 125, 1);
    render_button(rd, "+", 684, 125, 1);

    // Magasság + - gombjai
    render_button(rd, "-", 850, 125, 1);
    render_button(rd, "+", 1016, 125, 1);

    // Kilépő gomb
    render_button(rd, "<", 125, 125, 1);

    // Irányítások

    // ez legyen középen
    render_text_block(rd, "Irányítások", 1600 / 2 - 198 / 2, 300, 0);

    render_text_block(rd, "Balra:", 500, 350, 0);
    render_text_block(rd, "Jobbra:", 500, 400, 0);
    render_text_block(rd, "Lassú leejtés:", 500, 450, 0);
    render_text_block(rd, "Gyors lerakás:", 500, 500, 0);
    render_text_block(rd, "Forgatás 1:", 500, 550, 0);
    render_text_block(rd, "Forgatás 2:", 500, 600, 0);
    render_text_block(rd, "Újraindítás:", 500, 650, 0);
    render_text_block(rd, "Menüből kilépés:", 500, 700, 0);
    render_text_block(rd, "Program lezárása:", 500, 750, 0);

    // TODO forgatás 1 2 helyett ccw cw?

    render_text_block(rd, "A", 850, 350, 0);
    render_text_block(rd, "D", 850, 400, 0);
    render_text_block(rd, "S", 850, 450, 0);
    render_text_block(rd, "W", 850, 500, 0);
    render_text_block(rd, "I", 850, 550, 0);
    render_text_block(rd, "O", 850, 600, 0);
    render_text_block(rd, "ESC", 850, 650, 0);
    render_text_block(rd, "ESC", 850, 700, 0);
    render_text_block(rd, "Ablak lezárása", 850, 750, 0);

    // TODO pontszámítás leírása
}

// az alapján, hogy előzőleg rajta volt-e az egér, és hogy most rajta van-e az egér újrarajzolja a megfelelő gombot
// visszaadja, rajzolt-e
bool rerender_plus_minus_button(CommonRenderData rd, char *button_text, int x, int y, bool *selected, bool in) {
    if (*selected != in) {
        if (in)
            render_button(rd, button_text, x, y, 2);
        else
            render_button(rd, button_text, x, y, 1);
        *selected = in;
        return true;
    }
    return false;
}

int settings_loop(SDL_Renderer *renderer, TTF_Font *font, int *board_width, int *board_height) {
    // TODO szebb lenne külön függvényt írni neki?
    // nem fogja használni a táblamérethez kapcsolódó dolgokat
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    render_starting_screen(rd, *board_width, *board_height);

    SDL_RenderPresent(renderer);

    bool held_left = false;

    bool selected_width_plus = false;
    bool selected_width_minus = false;
    bool selected_height_plus = false;
    bool selected_height_minus = false;
    bool selected_back = false;

    // TODO in_[]-t kimozgatni eventek elé?
    SDL_Event event;
    while (SDL_WaitEvent(&event) && event.type != SDL_QUIT) {
        bool draw = false;
        switch (event.type) {
            case SDL_MOUSEMOTION: {
                // ha az egér az egyik gombon van, a keret pirosra színezése
                int x = event.motion.x;
                int y = event.motion.y;

                // minden gombnak az y koordinátája egy helyen van, azaz elég egyszer tesztelni rá
                bool in_y_range = 125 <= y && y <= 175;
                if (in_y_range) {
                    bool in_width_minus = 500 <= x && x <= 550;
                    bool in_width_plus = 684 <= x && x <= 734;
                    bool in_height_minus = 850 <= x && x <= 900;
                    bool in_height_plus = 1016 <= x && x <= 1066;
                    bool in_back = 125 <= x && x <= 175;

                    // ha bármelyik igaz, akkor a rajzolja le
                    draw = draw | rerender_plus_minus_button(rd, "-", 500, 125, &selected_width_minus, in_width_minus);
                    draw = draw | rerender_plus_minus_button(rd, "+", 684, 125, &selected_width_plus, in_width_plus);
                    draw = draw | rerender_plus_minus_button(rd, "-", 850, 125, &selected_height_minus, in_height_minus);
                    draw = draw | rerender_plus_minus_button(rd, "+", 1016, 125, &selected_height_plus, in_height_plus);
                    draw = draw | rerender_plus_minus_button(rd, "<", 125, 125, &selected_back, in_back);
                }
            }
                // TODO valami kilépés gomb? Innen könnyű lenne
            case SDL_MOUSEBUTTONDOWN:
                // bal gomb lenyomására ha gombon van, csökkenti / növeli az adatot
                if (event.button.button == SDL_BUTTON_LEFT && !held_left) {
                    held_left = true;
                    int x = event.motion.x;
                    int y = event.motion.y;

                    bool in_y_range = 125 <= y && y <= 175;
                    if (in_y_range) {
                        bool in_width_minus = 500 <= x && x <= 550;
                        bool in_width_plus = 684 <= x && x <= 734;
                        bool in_height_minus = 850 <= x && x <= 900;
                        bool in_height_plus = 1016 <= x && x <= 1066;
                        bool in_back = 125 <= x && x <= 175;

                        if (in_width_minus)
                            (*board_width)--;
                        if (in_width_plus)
                            (*board_width)++;
                        if (in_height_minus)
                            (*board_height)--;
                        if (in_height_plus)
                            (*board_height)++;

                        if (in_back) return 1;

                        // ha bármelyikben benne volt, rajzolja le
                        draw = draw | in_width_minus;
                        draw = draw | in_width_plus;
                        draw = draw | in_height_minus;
                        draw = draw | in_height_plus;

                        // TODO jelezze a felhasználónak?
                        // TODO ezek konstansokba?
                        //  maximum és minimum értékek
                        // TODO ezek csak randomak, megnézni jók-e, lehet e nagyobb range
                        if (*board_width < 2) *board_width = 2;
                        if (*board_width > 20) *board_width = 20;
                        if (*board_height < 2) *board_height = 2;
                        if (*board_height > 40) *board_height = 40;

                        // TODO ne mindig mindkettőt renderelje
                        render_width_height_labels(rd, *board_width, *board_height);
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                // TODO ez a komment a fő fájlba is
                // felengedni csak egyszer tudja
                if (event.button.button == SDL_BUTTON_LEFT) held_left = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // ha kap escape-et, akkor szabályosan kilép
                    return 1;
                }
                break;
            case SDL_QUIT:
                // kilép, mert a felhasználó lezárta az ablakot
                return 0;
                break;
            default:
                // TODO
                break;
        }
        if (draw) SDL_RenderPresent(renderer);
    }
    // ha lezárja az ablakot, akkor ide jut, kilép
    return -1;
}