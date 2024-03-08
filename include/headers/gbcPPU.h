#ifndef GBCPPU_H
#define GBCPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gbMEM.h"
#include <vector>
#include "globals.h"

class gbcPPU{
    private:
        gbMEM* MEM;
        uint8_t* dMEM;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        uint8_t Vram[160][144];

    public:
        gbcPPU(gbMEM* memory, SDL_Renderer* rend, SDL_Texture* textu);
        void drawLine();
        void updatePPU(int cycles);

        void renderFrame();

    private:
        void drawBackground();
        void drawWindow();
        void drawSprites();
};

#endif /* GBCPPU_H */