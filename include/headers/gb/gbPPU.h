#ifndef GBPPU_H
#define GBPPU_H

#include "stdint.h"
#include "SDL.h"

void gbPPU_init(SDL_Renderer* rend);
void gbPPU_deinit();
void gbPPU_drawLine();
void gbPPU_updatePPU(int cycles);

void gbPPU_renderFrame();

#endif /* GBPPU_H */