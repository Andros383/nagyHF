#ifndef LEADERBOARD_LOOP_H
#define LEADERBOARD_LOOP_H

#include "game_screen.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct Entry {
    char name[50 + 1];
    // TODO rövidebb? vagy megoldani ...-al
    ScoreData score_data;
    int width, height;
} Entry;
typedef struct Entries {
    int len;
    Entry* list;
} Entries;

bool read_entries(Entries* list);
bool insert_entry(Entry new_entry);
void debug_entries(Entries entries);
#endif