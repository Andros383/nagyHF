#include "database.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debugmalloc.h"
#include "menu_selector.h"

// Visszaad egy üres entries típust
Entries new_entries() {
    Entries entries;
    entries.array = NULL;
    entries.len = 0;
    return entries;
}

// felülírja a scores.txt-t a kapott pontszámokkal
static bool write_entries(Entries entries) {
    FILE* fp = fopen("scores.txt", "w");

    if (fp == NULL) {
        printf("write_entries: file megnyitasa sikertelen\n");
        return false;
    }

    for (int i = 0; i < entries.len; i++) {
        Entry e = entries.array[i];
        fprintf(fp, "%s\t%d\t%d\t%d\t%d\t%d\n", e.name, e.score_data.score, e.score_data.longest_chain, e.score_data.placed_pieces, e.width, e.height);
    }

    fclose(fp);
    return true;
}

// hozzáad egy pontszámot az entries végéhez
// hamisat ad vissza, ha hiba történt
static bool add_entry(Entries* entries, Entry new_entry) {
    Entry* new_array = (Entry*)malloc((entries->len + 1) * sizeof(Entry));

    if (new_array == NULL) {
        printf("add_entry: sikertelen memoriafoglalas\n");
        // ne is lehessen használni
        return false;
    }

    for (int i = 0; i < entries->len; i++) {
        new_array[i] = entries->array[i];
    }

    new_array[entries->len] = new_entry;
    entries->len++;

    free(entries->array);

    entries->array = new_array;
    return true;
}

// beolvassa a scores.txt-ből a pontszámokat, és beleírja a kapott entries-be
// hamisat ad vissza, ha hiba történt
// elméletben nem történhet hiba, mert ha nem talál fájlt entries-t ad vissza
bool read_entries(Entries* entries) {
    FILE* fp = fopen("scores.txt", "r");
    if (fp == NULL) {
        printf("read_entries(): scores.txt nem talalhato, feltehetoen nem volt elmentett eredmeny\n");
        // nincs még fájl, visszaad egy üres array-t
        entries->len = 0;
        entries->array = NULL;
        return true;
    }

    Entry current_entry;

    while (true) {
        char c;
        int index = 0;

        // ha az első karatker EOF, akkor vége a fájlnak, kilép a ciklusból
        if (fscanf(fp, "%c", &c) == EOF) break;

        // innentől nem kell nézni az EOF-eket, mert a program nem vállalkozik helytelen fájlok kezelésére

        // név beolvasása, karakterenként, mert lehet benne szóköz is
        while (c != '\t') {
            current_entry.name[index] = c;
            index++;
            fscanf(fp, "%c", &c);
        }
        current_entry.name[index] = '\0';

        // többi adat beolvasása
        fscanf(fp, "%d", &current_entry.score_data.score);
        fscanf(fp, "%d", &current_entry.score_data.longest_chain);
        fscanf(fp, "%d", &current_entry.score_data.placed_pieces);
        fscanf(fp, "%d", &current_entry.width);
        fscanf(fp, "%d", &current_entry.height);

        // beolvasott eredmény hozzáadása az entries-hez
        bool success = add_entry(entries, current_entry);
        if (!success) return false;

        // utolsó enter beolvasása, hogy az új sor elején legyen a kurzor
        fscanf(fp, "%c", &c);
    }

    fclose(fp);
    return true;
}

// a scores.txt-hez hozzáad egy elemet, ha ilyen nevű még nem szerepelt
// ha szerepelt, felülírja a pontszámát
// hamisat ad vissza, ha hiba történt
bool insert_entry(Entry new_entry) {
    Entries entries = new_entries();
    bool success = read_entries(&entries);

    if (!success) return false;

    bool found = false;
    for (int i = 0; i < entries.len; i++) {
        // ha ezzel a névvel már szerepel entry, csak frissítjük az új értékre
        if (strcmp(new_entry.name, entries.array[i].name) == 0) {
            entries.array[i] = new_entry;
            found = true;
        }
    }

    // ha nem találtuk, hozzáadjuk a arrayához
    if (!found) {
        success = add_entry(&entries, new_entry);
        if (!success) {
            // fel kell szabadítani az eddig lefoglalt memóriát
            free(entries.array);
            return false;
        }
    }

    // új tömb beírása
    write_entries(entries);

    free(entries.array);
    return true;
}
