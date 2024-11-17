
// TODO ide hogy a fenébe nem kell SDL2 import??
#include "game_render.h"

#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game_screen.h"

// TODO bevinni az sdl-es import file-ba, mint globál konstans
const int WINDOW_WIDTH_2 = 1600;
const int WINDOW_HEIGHT_2 = 1000;

// TODO ez dinamikus legyen majd?
// const int BLOCK_SIZE = 70;
// megjegyzés: y trükkös, felülről számolja az SDL

// TODO átvinni game_render.c-be

// TODO ezt lehetne jobban?
// vagy akár rosszabban: belerakni a block_size-ot, hogy lehessen dinamikusan állítani

// kiszámítja a rajzoláshoz gyakran használt adatokat
CommonRenderData init_common_render_data(SDL_Renderer *renderer, TTF_Font *font, GameState *game_state) {
    CommonRenderData rd;
    rd.renderer = renderer;
    rd.font = font;

    // tábla szélessége
    rd.board_width = game_state->board_width;
    rd.board_height = game_state->board_height;

    // egy blokk mérete
    int proposed_size_width = 500 / rd.board_width;
    int proposed_size_height = 800 / rd.board_height;

    // a kettő közül a kisebb
    rd.block_size = proposed_size_width < proposed_size_height ? proposed_size_width : proposed_size_height;

    // tábla szélessége pixelben
    rd.board_width_px = rd.board_width * rd.block_size;
    rd.board_height_px = rd.board_height * rd.block_size;

    // tábla bal alsó sarkának koordinátái pixelben
    rd.origin_x = WINDOW_WIDTH_2 / 2 - rd.board_width_px / 2;
    rd.origin_y = WINDOW_HEIGHT_2 / 2 + rd.board_height_px / 2;

    // alapból 420-840
    return rd;
}

// TODO jobb név, render_block-ot megoldani ezzel
static Uint32 get_color(Block block) {
    switch (block) {
        case RED:
            return 0xFF0000FF;
            break;

        case GREEN:
            return 0x00FF00FF;
            break;

        case BLUE:
            return 0x0000FFFF;
            break;

        case YELLOW:
            return 0xFFFF00FF;
            break;
        case EMPTY:
            // TODO visszaadni a színét elm 188, 188, 188, 255
            return 0;
            break;
        default:
            // mindent megnéztünk
            printf("Invalid block state in 'get_color'!");
            // ilyenkor mivel az átlátszóság 0 semmit se rajzol
            return 0;
            break;
    }
}
// lerajzol a táblára egy blokkot x y cellába
static void render_block(CommonRenderData rd, Block block, int x, int y) {
    // TODO Color-ra átírni
    Block current_block = block;
    // ha empty, nincs renderelés
    if (current_block == EMPTY) return;

    int r = 0, g = 0, b = 0;
    switch (block) {
        case RED:
            r = 255;
            break;

        case GREEN:
            g = 255;
            break;

        case BLUE:
            b = 255;
            break;

        case YELLOW:
            r = 255;
            g = 255;
            break;

        default:
            // mindent megnéztünk
            printf("Invalid block state in 'render_block'!");
            break;
    }
    int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
    int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
    int radius = (rd.block_size / 2) * 0.9;  // TODO wild casting kiszedése
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius, 0, 0, 0, 255);
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius - 1, r, g, b, 255);
}

// kirajzolja a táblát háttérrel, rácsvonalakkal, és a már lerakott részekkel
static void render_board(CommonRenderData rd, Block **board) {
    SDL_Renderer *renderer = rd.renderer;

    // tábla kerete, mert nincs thickRectangle
    boxRGBA(renderer, rd.origin_x - 3, rd.origin_y + 3, rd.origin_x + rd.board_width_px + 3, rd.origin_y - rd.board_height_px - 3, 0, 0, 0, 255);
    // tábla háttere -> minesweeper szürke
    boxRGBA(renderer, rd.origin_x, rd.origin_y, rd.origin_x + rd.board_width_px, rd.origin_y - rd.board_height_px, 188, 188, 188, 255);

    // rács kirajzolása
    for (int x = 1; x <= rd.board_width - 1; x++) {
        lineRGBA(renderer, rd.origin_x + rd.block_size * x, rd.origin_y, rd.origin_x + rd.block_size * x, rd.origin_y - rd.board_height_px, 0, 0, 0, 127);
    }

    for (int y = 1; y <= rd.board_height - 1; y++) {
        lineRGBA(renderer, rd.origin_x, rd.origin_y - rd.block_size * y, rd.origin_x + rd.board_width_px, rd.origin_y - rd.block_size * y, 0, 0, 0, 127);
    }

    // tényleges tartalom kirajzolása

    // fekete alapozókörök
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            if (board[x][y] != EMPTY) {
                int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
                int radius = (rd.block_size / 2) * 0.9;  // TODO wild casting kiszedése
                filledCircleColor(rd.renderer, circle_middle_x, circle_middle_y, radius, 0x000000FF);
            }
        }
    }

    int connector_thickness = rd.block_size / 4;
    // legyen páros, hogy jól nézzen ki
    if (connector_thickness % 2 != 0) connector_thickness++;
    // vízszintes összekötővonalak
    for (int x = 0; x < rd.board_width - 1; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            if (board[x][y] == board[x + 1][y] && board[x][y] != EMPTY) {
                // TODO jobb változónév
                int mid_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int mid_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;

                // alapozás
                thickLineColor(rd.renderer, mid_x, mid_y, mid_x + rd.block_size, mid_y, connector_thickness, 0x000000FF);
                thickLineColor(rd.renderer, mid_x, mid_y, mid_x + rd.block_size, mid_y, connector_thickness - 2, get_color(board[x][y]));
            }
        }
    }

    // függőleges összekötővonalak
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height - 1; y++) {
            if (board[x][y] == board[x][y + 1] && board[x][y] != EMPTY) {
                // TODO jobb változónév
                int mid_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int mid_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;

                thickLineColor(rd.renderer, mid_x, mid_y, mid_x, mid_y - rd.block_size, connector_thickness, 0x000000FF);
                thickLineColor(rd.renderer, mid_x, mid_y, mid_x, mid_y - rd.block_size, connector_thickness - 2, get_color(board[x][y]));
            }
        }
    }

    // körök kitöltése
    for (int x = 0; x < rd.board_width; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            if (board[x][y] != EMPTY) {
                int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
                int radius = (rd.block_size / 2) * 0.9;  // TODO wild casting kiszedése
                filledCircleColor(rd.renderer, circle_middle_x, circle_middle_y, radius - 1, get_color(board[x][y]));
            }
        }
    }
}

// rajzol egy szövegdobozt a megadott kerettel, fehér színnel
// ha a keret 0x00000000 értelem szerűen átlátszóság miatt nem kap keretet
void render_text_block(CommonRenderData rd, char *text, int x, int y, Uint32 outline_color) {
    // Source: infoc

    // TODO egész refactor, hogy ne kelljen mindig megnyitni
    // valszeg csak lesz egy nagy render struktúrám amiben MINDEN adat benne lesz
    // csak legyen kész először is a programom valahogyan
    // refaktor lehet utána
    TTF_Font *font = rd.font;

    SDL_Rect hova = {10, 10, 0, 0};
    SDL_Surface *felirat = TTF_RenderUTF8_Shaded(font, text, (SDL_Color){0, 0, 0}, (SDL_Color){255, 255, 255});
    SDL_Texture *felirat_t = SDL_CreateTextureFromSurface(rd.renderer, felirat);
    hova.x = x;
    hova.y = y;
    hova.w = felirat->w;
    hova.h = felirat->h;

    boxColor(rd.renderer, hova.x - 2, hova.y - 2, hova.x + hova.w + 1, hova.y + hova.h + 1, outline_color);
    // printf("%s:\n %d %d %d %d -> %d\n", text, hova.x, hova.y, hova.x + hova.w, hova.y + hova.h, hova.w);
    // printf("%s outline:\n %d %d %d %d\n", text, hova.x - 2, hova.y - 2, hova.x + hova.w + 1, hova.y + hova.h + 1);

    // TODO valszeg ezeket nem kell felszabadítani mindig
    // vagy csak a tábla törlését átgondolom
    SDL_RenderCopy(rd.renderer, felirat_t, NULL, &hova);
    SDL_FreeSurface(felirat);
    SDL_DestroyTexture(felirat_t);
}
// TODO GameState miért pointer? a többinek annak kéne lennie? Érték szerint fine átvenni, kb 30 byte
// Bár itt ezt változtatjuk
// A rajzolóknál érthető az érték szerinti, mert nem változtatjuk
void render_game(CommonRenderData rd, GameState *game_state) {
    SDL_Renderer *renderer = rd.renderer;
    // // TODO nem mindig kell újrarajzolni mindent. De majd csak akkor, ha optimalizálok?
    // TODO RenderClear-t átgondolni kell-e, hogy "feltölti-e szeméttel", vagy csak pixeleket számol, milyen színűek
    // azaz ha clear nélkül nyomok sok rajzolást akkor több helyet foglal-e le
    // SDL_RenderClear(renderer);

    // fehér háttér, csak ott, ahol a játék játszódik
    // init fogja először a fehér hátteret rakni az egész ablakra
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH_2 - 350, WINDOW_HEIGHT_2, 255, 255, 255, 255);

    // tábla kirajzolása: csak a már lerakott részek
    render_board(rd, game_state->board);

    // TODO score, chain, stb kirajzolása

    // jelenleg aktív rész kirajzolása
    Piece ac = game_state->active_piece;
    // csak akkor rajzolja ki, ha nem invalid
    if (!(ac.x1 == -1 || ac.y1 == -1 || ac.x2 == -1 || ac.y2 == -1)) {
        render_block(rd, ac.block1, ac.x1, ac.y1);
        render_block(rd, ac.block2, ac.x2, ac.y2);
    }

    SDL_RenderPresent(renderer);
}

// TODO fura lehet hogy a renderer megjelenik a hard_drop-ban
// valójában csak a lockos függvényekben, ami így nem meglepő
// TODO ez így fura, hogy itt van?
// TODO nem örülök, hogy mik ennek a függvénynek az argumensei
// pl a game_state csak azért van benne, hogy a magasság kiolvasható legyen
// kirajzolja az animációhoz szükséges félkész blokkokat
void render_animation_block(CommonRenderData rd, int x, int y, double radius_mult) {
    // TODO ezt a szürkét glob változózni? majd a szépség runon
    int r = 188;
    int g = 188;
    int b = 188;
    int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
    int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
    int radius = (rd.block_size / 2) * 0.9 * radius_mult;  // TODO wild casting kiszedése? Itt is lehet kéne, amikor van radius_mult
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius, r, g, b, 255);
    SDL_RenderPresent(rd.renderer);
}
