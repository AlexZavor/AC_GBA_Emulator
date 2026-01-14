#include "gba/gbaEmulator.h"

#include "inputData.h"
#include "gba/gbaMEM.h"
#include "gba/gbaCPU.h"
#include "gba/gbaPPU.h"
#include "gba/gbaDebug.h"
#include "timer.h"

static SDL_Renderer* renderer;
static inputData input;
static SDL_Event* e;

// function call for window resize
extern void set_window_size(int w, int h);


void gbaEmulator_init(SDL_Renderer* render, SDL_Event* event) {
    renderer = render;
	resetInputData(&input);
    e = event;
    // init mem, cpu, and ppu
    gbaMEM_init();
    gbaCPU_init();
    gbaPPU_init(render);
}

void gbaEmulator_deinit(){
    gbaMEM_deinit();
    gbaCPU_deinit();
    gbaPPU_deinit();
}

int gbaEmulator_run() {
    set_window_size(GBA_SCREEN_WIDTH*SCALE, GBA_SCREEN_HEIGHT*SCALE);
    while(!input.quit){
        timer_start();
        readInput(&input, e);

        int cycle = 0x7E*1232;
        while( cycle < 280896){
            // one frame
            static int ret = 0;
            gba_debug(e);
            ret = gbaCPU_instruction();
            if(ret < 0){
                input.quit = true;
                break;
            }
            cycle += ret;
            gbaPPU_update(cycle);
            readInput(&input, e);//for debug
            if(input.quit) break;
        }
        
        gbaPPU_renderFrame();
        SDL_RenderPresent(renderer);
        timer_end();
        timer_buff();
    }
    set_window_size(SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE);
    return 0;
}

int gbaEmulator_insertCart(game* game) {
    if(gbaMEM_insertCart(game)) {
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}