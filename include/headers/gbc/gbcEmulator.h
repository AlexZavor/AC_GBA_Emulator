#ifndef GBCEMULATOR_H
#define GBCEMULATOR_H

#include "SDL.h"
#include "game.h"

// Initialize Game boy emulator.
void gbcEmulator_init(SDL_Renderer* render, SDL_Event* event);
// Deinitialize Game boy emulator.
void gbcEmulator_deinit();
// Hand control over to the game boy emulator, returns on an exit
int gbcEmulator_run();
// Insert Cartage into gb emulator
int gbcEmulator_insertCart(game* game);

#endif /* GBCEMULATOR_H */