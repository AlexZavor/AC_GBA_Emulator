#ifndef GBEMULATOR_H
#define GBEMULATOR_H

#include "SDL.h"
#include "game.h"

// Initialize Game boy emulator.
void gbEmulator_init(SDL_Renderer* render, SDL_Event* event);
void gbEmulator_deinit();
// Hand control over to the game boy emulator, returns on an exit
int gbEmulator_run();
// Insert Cartage into gb emulator
int gbEmulator_insertCart(game* game);

#endif /* GBEMULATOR_H */