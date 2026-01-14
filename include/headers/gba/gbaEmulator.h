#ifndef GBAEMULATOR_H
#define GBAEMULATOR_H

#include "SDL.h"
#include "game.h"

// Initialize Game boy emulator.
void gbaEmulator_init(SDL_Renderer* render, SDL_Event* event);
// Deinitialize Game boy emulator.
void gbaEmulator_deinit();
// Insert Cartage into gb emulator
int gbaEmulator_insertCart(game* game);
// Hand control over to the game boy emulator, returns on an exit
int gbaEmulator_run();

#endif /* GBAEMULATOR_H */