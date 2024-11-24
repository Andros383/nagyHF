#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "menu_selector.h"
#include "sdl_setup.h"

// eltárolja egy játékmenetről elmentett információkat
typedef struct Entry {
    char name[MAX_NAME_LEN];
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
Entries new_entries();
#endif