
// TODO ide hogy a fenébe nem kell SDL2 import??
#include "game_render.h"

#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debugmalloc.h"
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

// block-ban megkapja a blokk színét, ami alapján beállítja az rgb-t a színnek megfelelő értékekre
static void set_RGB_from_block(Block block, int *r, int *g, int *b) {
    // alapvetően fekete
    *r = 0;
    *g = 0;
    *b = 0;

    switch (block) {
        case RED:
            *r = 255;
            break;

        case GREEN:
            *g = 255;
            break;

        case BLUE:
            *b = 255;
            break;

        case YELLOW:
            *r = 255;
            *g = 255;
            break;

        default:
            // 0, 0, 0-t ad vissza, teljesen fekete, könnyen észrevehető ha baj van
            // TODO valaminek itt kell, hogy a háttér legyen?
            break;
    }
}

// lerajzol a táblára egy blokkot x y cellába
static void render_block_on_board(CommonRenderData rd, Block block, int x, int y) {
    // TODO ezt egyáltalán használom?
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
            printf("Invalid block state in 'render_block_on_board'!");
            break;
    }
    int circle_middle_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
    int circle_middle_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;
    int radius = (rd.block_size / 2) * 0.9;  // TODO wild casting kiszedése
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius, 0, 0, 0, 255);
    filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius - 1, r, g, b, 255);
}

// kirajzolja a táblát háttérrel, rácsvonalakkal, és a már lerakott részekkel
void render_board(CommonRenderData rd, Block **board) {
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
                filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius, 0, 0, 0, 255);
            }
        }
    }

    int connector_thickness = rd.block_size / 4;
    // legyen páros, hogy jól nézzen ki
    if (connector_thickness % 2 != 0) connector_thickness++;
    // vízszintes összekötővonalak
    for (int x = 0; x < rd.board_width - 1; x++) {
        for (int y = 0; y < rd.board_height; y++) {
            // ha a jelenlegi és a tőle jobbra lévő megegyezik, de nem EMPTY
            if (board[x][y] == board[x + 1][y] && board[x][y] != EMPTY) {
                // TODO jobb változónév
                int mid_x = (rd.origin_x + rd.block_size / 2) + rd.block_size * x;
                int mid_y = (rd.origin_y - rd.block_size / 2) - rd.block_size * y;

                // alapozás
                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x + rd.block_size, mid_y, connector_thickness, 0, 0, 0, 255);
                // TODO rgb glob változó hogy ne foglalja le?
                int r, g, b;
                set_RGB_from_block(board[x][y], &r, &g, &b);
                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x + rd.block_size, mid_y, connector_thickness - 2, r, g, b, 255);
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

                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x, mid_y - rd.block_size, connector_thickness, 0, 0, 0, 255);
                int r, g, b;
                set_RGB_from_block(board[x][y], &r, &g, &b);
                thickLineRGBA(rd.renderer, mid_x, mid_y, mid_x, mid_y - rd.block_size, connector_thickness - 2, r, g, b, 255);
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

                int r, g, b;
                set_RGB_from_block(board[x][y], &r, &g, &b);
                filledCircleRGBA(rd.renderer, circle_middle_x, circle_middle_y, radius - 1, r, g, b, 255);
            }
        }
    }
}

// rajzol egy szövegdobozt a megadott kerettel
// outline_color_code állítja a keret színét, azért kód, mert csak három fajta keretszínt használok a programban
// 0: nincs, 1: fekete, 2: piros
void render_text_block(CommonRenderData rd, char *text, int x, int y, int outline_color_code) {
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

    // háttérszín változói
    // TODO egy db = 0? r= g= b= a= 0;
    int r = 0, g = 0, b = 0, a = 0;
    // ha 0 a code, akkor maradhat csupa 0
    switch (outline_color_code) {
        case 0:
            break;
        case 1:
            a = 255;
            break;
        case 2:
            r = 255;
            a = 255;
            break;
        default:
            printf("Invalid color code\n");
            break;
    }

    boxRGBA(rd.renderer, hova.x - 2, hova.y - 2, hova.x + hova.w + 1, hova.y + hova.h + 1, r, g, b, a);
    // printf("%s:\n %d %d %d %d -> %d\n", text, hova.x, hova.y, hova.x + hova.w, hova.y + hova.h, hova.w);
    // printf("%s outline:\n %d %d %d %d\n", text, hova.x - 2, hova.y - 2, hova.x + hova.w + 1, hova.y + hova.h + 1);

    // TODO valszeg ezeket nem kell felszabadítani mindig
    // vagy csak a tábla törlését átgondolom
    SDL_RenderCopy(rd.renderer, felirat_t, NULL, &hova);
    SDL_FreeSurface(felirat);
    SDL_DestroyTexture(felirat_t);
}

// megrajzolja a következő részt és egy téglalapot köré
void render_queue(CommonRenderData rd, GameState *game_state) {
    // átméretezésnél is maradjon hasonló helyen
    int block_size = 80;

    // soron következő részek kirajzolása
    int top_left_x = rd.origin_x + rd.board_width_px + block_size / 2;
    int top_left_y = rd.origin_y - rd.board_height_px + block_size / 2;

    // kézzel állított értékek
    // első rész
    // keret
    boxRGBA(rd.renderer, top_left_x, top_left_y, top_left_x + block_size, top_left_y + block_size * 2, 0, 0, 0, 255);
    boxRGBA(rd.renderer, top_left_x + 2, top_left_y + 2, top_left_x + block_size - 2, top_left_y + block_size * 2 - 2, 255, 255, 255, 255);

    // TODO alapból filled-et használok, vagy csak normált?
    // a blokkok színezéséhez ideiglenesen használt változók
    int r, g, b;
    // felső
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8, 0, 0, 0, 255);

    set_RGB_from_block(game_state->queue[0].block1, &r, &g, &b);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8 - 1, r, g, b, 255);

    // alsó
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8, 0, 0, 0, 255);

    set_RGB_from_block(game_state->queue[0].block2, &r, &g, &b);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8 - 1, r, g, b, 255);

    // TODO ha a 0.9-et átírom, itt a 2.5-ököt is

    // második rész lejjebb van
    top_left_y += block_size * 2.5;

    boxRGBA(rd.renderer, top_left_x, top_left_y, top_left_x + block_size, top_left_y + block_size * 2, 0, 0, 0, 255);
    boxRGBA(rd.renderer, top_left_x + 2, top_left_y + 2, top_left_x + block_size - 2, top_left_y + block_size * 2 - 2, 255, 255, 255, 255);

    // felső
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8, 0, 0, 0, 255);

    set_RGB_from_block(game_state->queue[1].block1, &r, &g, &b);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size / 2, block_size / 2 * 0.8 - 1, r, g, b, 255);

    // alsó
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8, 0, 0, 0, 255);

    set_RGB_from_block(game_state->queue[1].block2, &r, &g, &b);
    filledCircleRGBA(rd.renderer, top_left_x + block_size / 2, top_left_y + block_size * 3 / 2, block_size / 2 * 0.8 - 1, r, g, b, 255);
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
        render_block_on_board(rd, ac.block1, ac.x1, ac.y1);
        render_block_on_board(rd, ac.block2, ac.x2, ac.y2);
    }

    // TODO sprintf max hosszos változata? többi helyen is
    // leghosszabb string ami lehet
    char score_label[] = "1000000000";
    sprintf(score_label, "%010d", game_state->score_data.score);

    render_queue(rd, game_state);

    // TODO legyen középen
    // TODO legyen a táblához relatív jó helyen
    // TODO táblához képest álljon?

    // jelenleg középen, tábla alja - 10
    render_text_block(rd, score_label, 720, rd.origin_y + 10, 0);

    // TODO ezek limitelése?
    // ez ~40 perc játékidő 4-es pps-el
    char placed_pieces_label[] = "Lerakott részek: 10000";
    sprintf(placed_pieces_label, "Lerakott részek: %5d", game_state->score_data.placed_pieces);

    render_text_block(rd, placed_pieces_label, 100, 800, 0);

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
