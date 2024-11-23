#ifndef DATABASE_H
#define DATABASE_H

#include "game_screen.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

// eltárolja egy játékmenetről elmentett információkat
typedef struct Entry {
    char name[50 + 1];
    // TODO rövidebb? vagy megoldani ...-al
    ScoreData score_data;
    int width, height;
} Entry;

// dinamikus tömbben az elmentett pontszámok, és a tömb hossza
typedef struct Entries {
    int len;
    Entry* array;
} Entries;

bool read_entries(Entries* array);
bool insert_entry(Entry new_entry);
void debug_entries(Entries entries);
Entries new_entries();
#endif