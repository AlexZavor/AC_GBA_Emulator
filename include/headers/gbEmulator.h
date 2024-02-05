#ifndef GBEMULATOR_H
#define GBEMULATOR_H

#include <stdbool.h>
#include <stdio.h>
#include "SDL2/SDL.h"
#include "inputData.h"
#include "gbMEM.h"
#include "gbCPU.h"
#include "gbPPU.h"

class gbEmulator{
    private:
        gbMEM* MEM = new gbMEM();
        gbCPU* CPU = new gbCPU();
        gbPPU* PPU = new gbPPU();
    public:
        //Calls the GB Emulator to run for one frame and return the output
        bool runFrame(inputData input, SDL_Renderer*){};
        //Inserts a cartrage into the Game Boy
        bool insertCart(FILE* game){};

        gbEmulator(){};
};

#endif /* GBEMULATOR_H */