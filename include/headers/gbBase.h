#ifndef GBBASE_H
#define GBBASE_H

#include <stdbool.h>
#include <stdio.h>
#include "SDL2/SDL.h"
#include "inputData.h"

class gbBase{
    public:
        SDL_Surface* runFrame(inputData input);
        bool insertCart(FILE* game);
};

#endif /* GBBASE_H */