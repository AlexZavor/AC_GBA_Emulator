#ifndef GBCPPU_H
#define GBCPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gb/gbMEM.h"
#include <vector>
#include "globals.h"


class gbcPPU{

    private:
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        uint8_t BGPriority[160];
        uint8_t line[160];
        uint16_t Vram[160][144];
        

        uint8_t lastDMA;

    public:
        gbcPPU(SDL_Renderer* rend);
        void drawLine();
        uint32_t updatePPU(int cycles);

        void renderFrame();

    private:
        void drawBackground();
        void drawWindow();
        void drawSprites();
};

#endif /* GBCPPU_H */