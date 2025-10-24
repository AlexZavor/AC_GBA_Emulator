#ifndef GAME_H
#define GAME_H

#include "tinydir.h"
#include "globals.h"
#include "stdbool.h"

#define ROMS_MAX 256

// Struct for a "Game" containing relevent information
typedef struct {
    GB_sys system;
    char* name;
    bool has_save;
} game;

typedef struct {
    game* games[ROMS_MAX];
    int length;
} game_holder;

// Load All Games from Global "GAME_DIR" Folder into Roms list
void game_loadGames(game_holder* Roms);

// Remove save data from game
void game_removeSave(game* game);

#endif // GAME_H