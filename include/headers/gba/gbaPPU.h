#ifndef GBAPPU_H
#define GBAPPU_H

#include "SDL.h"

void gbaPPU_init(SDL_Renderer* rend);
void gbaPPU_deinit();

void gbaPPU_renderFrame();

void gbaPPU_update(int cycles);

#endif /* GBAPPU_H */