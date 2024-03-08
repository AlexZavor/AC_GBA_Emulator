#ifndef GBCEMULATOR_H
#define GBCEMULATOR_H

#include <stdbool.h>
#include <stdio.h>
#include "SDL.h"
#include "inputData.h"
#include "gbMEM.h"
#include "gbCPU.h"
#include "gbPPU.h"

class gbcEmulator{
    private:
        gbMEM* MEM;
        gbCPU* CPU;
        gbPPU* PPU;
    public:
        //Constructor. should initialize things like memory and prepare for first instruction
        gbcEmulator(SDL_Renderer* renderer, SDL_Texture* texture);
        //Deconstructor. saves game, deletes old things
        ~gbcEmulator();
        //Calls the GB Emulator to run for one frame and return the output
        void runFrame(inputData input);
        //Inserts a cartrage into the Game Boy
        bool insertCart(std::string game);

};

#endif /* GBCEMULATOR_H */