#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "debugmalloc.h"
#include "menu_selector.h"
#include "render.h"
#include "sdl_setup.h"

int main(int argc, char *argv[]) {
    // printf átirányítása a log file-ba
    // stdout-ra kerülnek a hibaüzenetek
    // stderr-re ír a debugmalloc, ha szükséges bekapcsolható

    freopen("log.txt", "w", stdout);
    // freopen("debugmalloc_log.txt", "w", stderr);

    // sdl setup
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    sdl_init(WINDOW_WIDTH, WINDOW_HEIGHT, &window, &renderer, &font);

    // játék indítása
    menu_selector_loop(renderer, font);

    // sdl leállítása
    sdl_close(&window, &renderer, &font);
    return 0;
}
