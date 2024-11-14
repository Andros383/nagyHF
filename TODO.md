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

Átnézni az egész import structure-t
game_screen-ben van benne a Block pl, de game_loop-ot is elég includeolni a game_render-ből?

Lehet ha csak a rendert veszem át a primitives nem is kell?

MINDEN INCLUDE "*.h" ÉS NEM "*.c" ALAKÚ

Külön másolva ne nyíljon meg konzol ablak
ez elméletben csak az mwindows vagy hasonló flag állítása



---
KÉRDÉSEK
memóriakezelésből a returnt hogyan?
hibakezelést hogyan, amikor nem azt akarom, hogy a visszatérési érték a "volt-e hiba foglalás közben" legyen
mennyire kell fancynek kinéznie (valszeg meg tudok oldani egy slick kinézetet)
berakni a renderert glob változóba? már túl sok helyen jelenik meg random
timereket indítsak külön minden grav szintre, vagy csak százalékozzam?
---
kivinni az egyszer használatos, kis konstansokat a fájlok elejére?

valahogy felírni, hogy miért kell pl a natural_gavitybe az upkick? (azért hogy fel tudja állítani)

MEGJEGYZÉS: talán nem is kell a lerakásban nullázni, ha csak minden más helyen nullázom?
de akkor kevésbé érthető
inkább legyen érthető

nagyon wacky alt megoldás a delay előtt felsetuppolni egy event loopot, ami minden eventet, kivéve a delay végén érkező, saját user eventet felfog

de szerintem érthetőbb csinálni egy enable_events változót