#ifndef GAME_H
#define GAME_H

#include "tinydir.h"
#include "globals.h"
#include <vector>
#include <string>

// Struct for a "Game" containing relevent information
typedef struct {
    GB_sys system;
    std::string name;
    bool has_save;
} game;

// Load All Games from Global "GAME_DIR" Folder into Roms list
void game_loadGames(std::vector<game>* Roms);

// Remove save data from game
void game_removeSave(game* game);

#endif // GAME_H