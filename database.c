#include "database.h"

#include "game_screen.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

void print_entry(Entry e) {
    printf("%s\t%d\t%d\t%d\t%d\t%d", e.name, e.score_data.score, e.score_data.longest_chain, e.score_data.placed_pieces, e.width, e.height);
}

// TODO kivenni?
void debug_entries(Entries entries) {
    printf("Entries len: %d\n", entries.len);
    for (int i = 0; i < entries.len; i++) {
        print_entry(entries.list[i]);
        printf("\n");
    }
}

static bool write_entries(Entries entries) {
    FILE* fp = fopen("scores.txt", "w");

    if (fp == NULL) {
        printf("File megnyitasa sikertelen.\n");
        return false;
    }

    for (int i = 0; i < entries.len; i++) {
        Entry e = entries.list[i];
        fprintf(fp, "%s\t%d\t%d\t%d\t%d\t%d\n", e.name, e.score_data.score, e.score_data.longest_chain, e.score_data.placed_pieces, e.width, e.height);
    }

    fclose(fp);
}
bool add_entry(Entries* entries, Entry new_entry) {
    Entry* new_list = (Entry*)malloc((entries->len + 1) * sizeof(Entry));

    for (int i = 0; i < entries->len; i++) {
        new_list[i] = entries->list[i];
    }

    new_list[entries->len] = new_entry;
    // TODO ez megy?
    entries->len++;
    free(entries->list);

    entries->list = new_list;
}
// hamisat ad vissza, ha hiba történt
bool read_entries(Entries* entries) {
    // Ha nem találja?
    FILE* fp = fopen("scores.txt", "r");
    if (fp == NULL) {
        // TODO itt mi van?
        // hogyan tudom megkülönböztetni, hogy hiba volt a fájl megnyitásakor, vagy nem létezik?
        // cppreferencen valami err flaget említenek

        // Ha nincs ilyen file visszaad egy üres listát?

        entries->len = 0;
        entries->list = NULL;
        return true;
    }

    int len = 0;
    Entry* list = NULL;

    Entry current_entry;
    while (fscanf(fp, "%s", current_entry.name) != EOF) {
        // feltételezzük, hogy a fájl helyes-> működnek a beolvasások
        fscanf(fp, "%d", &current_entry.score_data.score);
        fscanf(fp, "%d", &current_entry.score_data.longest_chain);
        fscanf(fp, "%d", &current_entry.score_data.placed_pieces);
        fscanf(fp, "%d", &current_entry.width);
        fscanf(fp, "%d", &current_entry.height);

        Entry* new_list = (Entry*)malloc((len + 1) * sizeof(Entry));

        for (int i = 0; i < len; i++) {
            new_list[i] = list[i];
        }

        new_list[len] = current_entry;
        len++;
        free(list);

        list = new_list;
    }

    entries->len = len;
    // TODO ez nem feltétlen kell,
    free(entries->list);
    entries->list = list;

    fclose(fp);
    return true;
}

// az adatbázishoz hozzáad egy elemet, ha ilyen nevű még nem szerepelt
// ha szerepelt, felülírja a pontszámát
bool insert_entry(Entry new_entry) {
    Entries entries;
    bool success = read_entries(&entries);
    if (!success) return false;

    for (int i = 0; i < entries.len; i++) {
        // ha ezzel a névvel már szerepel entry, csak átírjuk
        if (strcmp(new_entry.name, entries.list[i].name) == 0) {
            // TODO befejezni
        }
    }

    success = add_entry(&entries, new_entry);
}
