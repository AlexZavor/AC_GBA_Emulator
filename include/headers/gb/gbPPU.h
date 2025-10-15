#ifndef GBPPU_H
#define GBPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gb/gbMEM.h"
#include <vector>
#include "globals.h"

class gbPPU{
    private:
        gbMEM* MEM;
        uint8_t* dMEM;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        uint8_t Vram[160][144];

    public:
        gbPPU(gbMEM* memory, SDL_Renderer* rend);
        void drawLine();
        void updatePPU(int cycles);

        void renderFrame();

    private:
        void drawBackground();
        void drawWindow();
        void drawSprites();
};

#endif /* GBPPU_H */