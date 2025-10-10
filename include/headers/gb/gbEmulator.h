#ifndef GBEMULATOR_H
#define GBEMULATOR_H

#include <stdbool.h>
#include <stdio.h>
#include "SDL.h"
#include "inputData.h"
#include "gb/gbAPU.h"
#include "gb/gbMEM.h"
#include "gb/gbCPU.h"
#include "gb/gbPPU.h"

class gbEmulator{
    private:
        gbMEM* MEM;
        gbCPU* CPU;
        gbPPU* PPU;
    public:
        //Constructor. should initialize things like memory and prepare for first instruction
        gbEmulator(SDL_Renderer* renderer, SDL_Texture* texture);
        //Deconstructor. saves game, deletes old things
        ~gbEmulator();
        //Calls the GB Emulator to run for one frame and return the output
        void runFrame(inputData input);
        //Inserts a cartage into the Game Boy
        bool insertCart(std::string game);

};

#endif /* GBEMULATOR_H */