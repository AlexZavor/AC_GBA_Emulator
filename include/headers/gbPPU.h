#ifndef GBPPU_H
#define GBPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gbMEM.h"
#include <vector>

class gbPPU{
    private:
        gbMEM* MEM;
        uint8_t* dMEM;
        SDL_Renderer* renderer;

        uint8_t currentLine[160];

    public:
        gbPPU(gbMEM* memory, SDL_Renderer* rend);
        void drawLine(uint8_t line);

    private:
        void drawBackground();
        void drawWindow();
        void drawSprites();
};

#endif /* GBPPU_H */