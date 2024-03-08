#include "gbcPPU.h"

gbcPPU::gbcPPU(gbMEM *memory, SDL_Renderer *rend, SDL_Texture *textu) {
    MEM = memory;
    dMEM = memory->MEM;
    renderer = rend;
	texture = textu;
}

void gbcPPU::drawLine() {
}

void gbcPPU::updatePPU(int cycles) {
}

void gbcPPU::renderFrame() {
}

void gbcPPU::drawBackground() {
}

void gbcPPU::drawWindow() {
}

void gbcPPU::drawSprites() {
}
