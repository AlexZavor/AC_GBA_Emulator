#ifndef GBCPPU_H
#define GBCPPU_H

#include "stdint.h"
#include "SDL.h"

void gbcPPU_init(SDL_Renderer* rend);
void gbcPPU_deinit();
void gbcPPU_drawLine();
uint32_t gbcPPU_updatePPU(int cycles);

void gbcPPU_renderFrame();

#endif /* GBCPPU_H */