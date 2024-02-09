#ifndef GBPPU_H
#define GBPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gbMEM.h"

class gbPPU{
    private:
        gbMEM* MEM;
        SDL_Renderer* renderer;
    public:
        gbPPU(gbMEM* MEM){
            MEM = MEM;
        };
};

#endif /* GBPPU_H */