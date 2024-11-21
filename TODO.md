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

minden függvénynek kell-e minden változó, amit megkap

név egységesítés
pl a jelző (timer, flag, nemtommi) mindig az elején vagy a végén legyen

rd.rendererek kinyírása
vagy nemtom, refactor check

megj: minden ami rajzol, csak CommonRenderData-t kap
ha függ az, hogy mit rajzol a játékállástól, akkor a game_state-et is megkapja, hogy kiolvassa belőle

initben font betöltött-e

RGBA helyett Color-t használni, ahol értelmesebb

renderel-e felesleges dolgot
pl amikor a block renderelésnél az üreseket is kirenderelte, csak a háttérszínnel

memóriafoglalásnál fprintf(stderr) cuccok

hosszú nyomásnál ha animáció előtt le volt nyomva, majd felengedte,
és újra lenyomja animáció vége előtt, és ez után rögtön vége az animációnak, nem kezd el arra menni, amíg a billentyűzetes auto repeat le nem nyomja

minden case rész végén van break

dobozok margója mindig be vagy mindig kifele szóljon

debugmalloc lehet csak mainre szól?
mindenhol includeolni?

ESC a játékképernyőn legyen újraindítás

magasság szélesség állításakor bal gomb lenyomva, egér mozgatással gyorsan számol
megcsinálni egy held_down változót

print bounds check?

forgatást unblock átnézni
a "VÁLTOZTATÁS" helyeket átnézni, de elméletben OK minden?
mert a >=-ket megnéztem

minden color-t RGBA-ra cserélni
segédfüggvény ami kirajzol blokk alapján AKÁRHOVA egy kört, ezt használja a gridre rajzoló

lehet-e függvényt a használata után deklarálni, ha be van magába includeolva a fájl?

malloc() elé hogy konvertálja a jó típusú pointerré

(int*)malloc(), (int*) ne maradjon le

backgroundot átnevezni borderre ahol valójában keret

switchek defaultjában kiírni a hibát, ha baj, akkor returnolni, ha nem, akkor meg a kódban van a baj, nem kell kilépni szerintem, mert jó kódot írtam

--- KÉRDÉSEK 2
valid a stringkezelésem? az sprintf()-es mágia
jó-e a debugmalloc behozása?
Hibát hogyan kéne jeleznem? Elég csak errorra kiírni? info-cn megnézni
hogyan tudom megkülönböztetni, hogy hiba volt a fájl megnyitásakor, vagy nem létezik?

errornál a felszabadítások ellenőrzése

egységesíteni a hibaüzeneteket

ugye amit nem sikerült lefoglalni nem kell felszabadítani

duplás tömbjeimnél van ilyen helyzet

MINDEN MALLOC ELLENŐRZÉS, VAN-E POINTER CAST ÉS TYPEOF

minden switchnek minden ága le van breakezve

kivenni az esces kilépést

https://infoc.eet.bme.hu/ea02/#6
kéne egy strict ISO C -pedantic flag?
BEÁLLÍTANI CODEBLOCKSBA IS

https://infoc.eet.bme.hu/scanf/
scanf problémákat 

height width sorrend?

debugmalloc nem szól amikor nem szabadítom fel teszt után az entries list mezejét

mindenhol nézem-e ahol van visszatérési érték

kommentes összefoglaló a fileok működéséről

bevenni a megállító jeleket átnézésben a for ciklus feltételbe?
pl az insert_entry-nél az átnézőben ami átírja az értékeket, hogy lépjen ki
akkor már inkább break