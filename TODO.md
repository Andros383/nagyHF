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

// README.md másolata
mwindows flag kikapcsolja a printf-et, de nyílik konzol?

src file és SDL2.dll-t nem tölti fel

Mivel nem vagyok biztos a teljes SDL2-es paritásban a projektet feltöltés előtt át kéne másolni codeblocks-ba

Angol típusnevek, magyar dokumentáció? Majd doxygennel generálni, labvezzel egyeztetni

excalidraw workspace-en van lerajzolva amit le kell rajzolni, nem rakom fel githubra, mert minek,úgy is csak főgépen dolgozok rajta
//