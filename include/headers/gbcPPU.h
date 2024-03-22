#ifndef GBCPPU_H
#define GBCPPU_H

#include "stdint.h"
#include "SDL.h"
#include "gbMEM.h"
#include <vector>
#include "globals.h"

typedef struct {
    union{
        struct{
            uint8_t low0;
            uint8_t high0;
        };
        uint16_t color0;
    };
    union{
        struct{
            uint8_t low1;
            uint8_t high1;
        };
        uint16_t color1;
    };
    union{
        struct{
            uint8_t low2;
            uint8_t high2;
        };
        uint16_t color2;
    };
    union{
        struct{
            uint8_t low3;
            uint8_t high3;
        };
        uint16_t color3;
    };
}pallet;

class gbcPPU{
    private:
        gbMEM* MEM;
        uint8_t* dMEM;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        uint8_t BGPriority[160];
        uint8_t line[160];
        uint16_t Vram[160][144];
        
        pallet BGColorPallet[8];
        pallet OBJColorPallet[8];

        uint8_t lastDMA;

    public:
        gbcPPU(gbMEM* memory, SDL_Renderer* rend, SDL_Texture* textu);
        void drawLine();
        uint32_t updatePPU(int cycles);

        void renderFrame();

    private:
        void drawBackground();
        void drawWindow();
        void drawSprites();
};

#endif /* GBCPPU_H */