#ifndef GBPPU_H
#define GBPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gbMEM.h"
#include <vector>

#define SCALE 4
//Screen dimension constants
#define SCREEN_WIDTH (160 * SCALE)
#define SCREEN_HEIGHT (144 * SCALE)

class gbPPU{
    private:
        gbMEM* MEM;
        uint8_t* dMEM;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        uint8_t Vram[160][144];

    public:
        gbPPU(gbMEM* memory, SDL_Renderer* rend, SDL_Texture* textu);
        void drawLine();
        void updatePPU(int cycles);

        void renderFrame();

    private:
        void drawBackground();
        void drawWindow();
        void drawSprites();
};

#endif /* GBPPU_H */