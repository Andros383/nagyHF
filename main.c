#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>  //TODO ezt kivenni, mert nem fog a konzolra írni
#include <stdlib.h>

#include "database.h"
#include "debugmalloc.h"
#include "game_screen.h"
#include "sdl_setup.h"
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1000;

int main(int argc, char *argv[]) {
    Entries e;
    read_entries(&e);
    debug_entries(e);
    //
    return 0;
    // TODO kivenni, meg nézni, megy-e

    // sdl setup
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    sdl_init(WINDOW_WIDTH, WINDOW_HEIGHT, &window, &renderer, &font);

    menu_selector_loop(renderer, font);

    /* az elvegzett rajzolasok a kepernyore */
    // SDL_RenderPresent(renderer);

    // /* varunk a kilepesre */
    // SDL_Event ev;
    // while (SDL_WaitEvent(&ev) && ev.type != SDL_QUIT) {
    //     /* SDL_RenderPresent(renderer); - MacOS Mojave esetén */
    // }

    // /* ablak bezarasa */
    // SDL_Quit();

    // TODO ide kell ez vagy sem?
    SDL_Quit();

    printf("Sikeres kilepes.\nAzert -1 a return code, mert a loop valaszto fuggveny nincs kesz.");
    return 0;
}
