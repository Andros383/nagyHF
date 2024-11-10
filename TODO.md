Minden file includeolja a saját headerjét
Minden memória kilépéskor szabaduljon fel, akkor is, ha hiba miatt lépünk ki
Kell-e minden include
Dokumentáció nyelve? Függvények nyelve?

mi static, mi nem? mit kéne kintről is elérni?

megjelölni, hogy minden SDL infoc-ről van

game logic és game render-re átnevezni?

valahogy lekezelni jól a "nem kéne idejutni" cuccokat? pl a case-ek defaultjában

kivenni a #def-es teszteket, debugokat, gyors kilépéseket

struct defeknél az első nem kell
pl
typedef struct CommonRenderData {
    int board_width, board_height;
    int board_width_px, board_height_px;
    int origin_x, origin_y;
} CommonRenderData;

helyett

typedef struct {
    int board_width, board_height;
    int board_width_px, board_height_px;
    int origin_x, origin_y;
} CommonRenderData;

Megnézni az eredeti változat meg e között mi a különbség