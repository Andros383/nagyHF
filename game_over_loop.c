#include "game_over_loop.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "game_render.h"
#include "game_screen.h"
#include "sdl_setup.h"

// TODO mindenek ugyan az a neve legyen
static void start_render(CommonRenderData rd, ScoreData score_data, int width, int height) {
    // TODO koordinátákkal
    // játéktábla látható marad, de el lesz sötétítve
    boxRGBA(rd.renderer, 0, 0, 1600, 1000, 0, 0, 0, 100);

    // fehér háttér, hogy picit több kerete legyen a szövegnek

    // ablak közepén
    render_text_block(rd, "Játék vége!", 701, 150, COLOR_WHITE);
    render_text_block(rd, "Név", 773, 250, COLOR_WHITE);

    render_text_block(rd, "Statisztikák", 100, 250, COLOR_WHITE);

    char label[50 + 1];
    sprintf(label, "Pontszám: %d", score_data.score);
    render_text_block(rd, label, 100, 300, COLOR_WHITE);

    sprintf(label, "Leghosszabb lánc: %d", score_data.longest_chain);
    render_text_block(rd, label, 100, 350, COLOR_WHITE);

    sprintf(label, "Lerakott részek: %d", score_data.placed_pieces);
    render_text_block(rd, label, 100, 400, COLOR_WHITE);

    sprintf(label, "Táblaméret:  %d x %d", width, height);
    render_text_block(rd, label, 100, 450, COLOR_WHITE);

    render_text_block(rd, "Ha nem szeretnéd elmenteni / felülírni a pontszámodat, indíts új játékot ESC-el.", 100, 950, COLOR_WHITE);
}

int game_over_loop(SDL_Renderer *renderer, TTF_Font *font, ScoreData score_data, int width, int height) {
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    // bool quit = false;
    // SDL_Event event;

    start_render(rd, score_data, width, height);
    SDL_RenderPresent(rd.renderer);

    // TODO max_name
    char name[MAX_NAME_LEN];

    // loopban, hogy ha üres nevet kapna kérje újra
    while (true) {
        SDL_Rect r = {650, 650, 300, 32};

        int return_code = input_text(name, r, COLOR_WHITE, COLOR_BLACK, font, renderer);
        switch (return_code) {
            case -1:
                // hibával lépett ki a program
                return -1;
                break;
            case 0:
                // felhasználó lezárta az ablakot
                return 0;
                break;
            case 1:
                // adatot nem kell elmenteni ESC miatt, új játék kezdődik
                return 1;
                break;
            case 2:
                // ha a név menő üres, de entert nyomott a felhasználó, újra bekéri a nevet
                if (strlen(name) == 0) {
                    continue;
                }

                Entry entry;
                strcpy(entry.name, name);
                entry.score_data = score_data;
                entry.width = width;
                entry.height = height;

                // eredmény adatbázisba beillesztése
                bool success = insert_entry(entry);
                if (!success) return -1;

                // ha név került beírásra, megmutatja a toplistán
                return 3;
                break;

            default:
                // invalid visszatérési érték
                return -1;
                break;
        }
    }
}