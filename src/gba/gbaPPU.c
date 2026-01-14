#include "gba/gbaPPU.h"
#include "gba/gbaMEM.h"
#include "gba/bitwise.h"
#include "stdio.h"
#include "globals.h"

static SDL_Renderer* renderer;
static SDL_Texture* texture;
extern uint8_t* Vram;
extern uint16_t* Pallet;

#define IO_BASE_ADDR (0x4000000)
#define PALLET_BASE_ADDR (0x5000000)
#define VRAM_BASE_ADDR (0x6000000)

void gbaPPU_init(SDL_Renderer* rend){
    renderer = rend;
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GBA_SCREEN_WIDTH, GBA_SCREEN_HEIGHT);
}

void gbaPPU_deinit(){
	SDL_DestroyTexture(texture);
}

void gbaPPU_update(int cycles){
    uint16_t y = cycles / 1232;
    uint16_t x = (cycles % 1232)/4;
    static uint32_t status = 0x0000;
    gba_write16(IO_BASE_ADDR + 6, y);// update line count
    status &= ~(0b0011);
    if(x > 240){
        set_bit(&status, 1, true);
        // H-blank
    }
    if(y >= 160){
        set_bit(&status, 0, true);
        // V-blank
    }
    gba_write16(IO_BASE_ADDR + 4, status);
}

uint32_t bgr555_to_argb(uint16_t color){
    uint32_t b = (color>>10)&0x1F;
    uint32_t g = (color>>5)&0x1F;
    uint32_t r = (color)&0x1F;
    return 0xFF000000 | (b<<3) | (g<<11) | (r<<19);
}

void gbaPPU_renderFrame(){
	// Credit to DOOMReboot on Github for the pixel pusher system! https://github.com/DOOMReboot
	// The Back Buffer texture may be stored with an extra bit of width (pitch) on the video card in order to properly
    // align it in VRAM should the width not lie on the correct memory boundary (usually four bytes).
    int32_t pitch = 0;

    // This will hold a pointer to the memory position in VRAM where our Back Buffer texture lies
    uint32_t* pixelBuffer = NULL;

    // Lock the memory in order to write our Back Buffer image to it
    if (!SDL_LockTexture(texture, NULL, (void**)&pixelBuffer, &pitch))
    {
        // The pitch of the Back Buffer texture in VRAM must be divided by four bytes
        // as it will always be a multiple of four
        pitch /= sizeof(uint32_t);

        // dump_vram();

        // printf("%.4X\n", Pallet[0]);

        // Draw frame to texture
        for (uint32_t y = 0; y < (GBA_SCREEN_HEIGHT); y++){
			for(uint32_t x = 0; x < (GBA_SCREEN_WIDTH); x++){
                uint32_t addr = ((y)*(GBA_SCREEN_WIDTH)) + x;
                uint8_t pal = (*(Vram+addr));
                uint16_t color = Pallet[pal];
				pixelBuffer[addr] = bgr555_to_argb(color);
			}
		}
        
		// Unlock the texture in VRAM and send to renderer!
        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }
}