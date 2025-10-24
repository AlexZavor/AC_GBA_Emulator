#include "gbc/gbcEmulator.h"

#include "inputData.h"
// #include "gb/gbAPU.h"
#include "gb/gbMEM.h"
#include "gb/gbCPU.h"
#include "gbc/gbcPPU.h"
#include "timer.h"

static SDL_Renderer* renderer;
static inputData input;
static SDL_Event* e;

static uint8_t inputButtons;
static uint8_t inputDpad;

// Read relevant input for the GB Emulator.
void gbcEmulator_input(){ 
    readInput(&input, e);
    
    //Formatting input data
    inputButtons = 0x1F - (input.start<<3) - (input.sel<<2) - (input.B<<1) - input.A;
    inputDpad    = 0x2F - (input.down<<3) - (input.up<<2) - (input.left<<1) - input.right;
}

void gbcEmulator_init(SDL_Renderer* render, SDL_Event* event) {
    renderer = render;
	resetInputData(&input);
    e = event;
    // init mem, cpu, and ppu
    gbMEM_init();
    gbMEM_setColor();
    gbCPU_init();
    gbCPU_setColor();
    gbcPPU_init(render);
    // gbAPU().APU_setMEM(MEM);
}

void gbcEmulator_deinit(){
    gbMEM_deinit();
    gbCPU_deinit();
    gbcPPU_deinit();
}

int gbcEmulator_run() {
    while(true){
    timer_start();
    gbcEmulator_input();
    if(input.quit) return 0;

    for (uint8_t line = 0; line < 154; line++)
    {
        static int cycle_count = 0;
        static bool speed = 0 ;
        cycle_count += 456;
        gb_write(0xFF44, line);
        while (cycle_count > 0)
        {   
            //Update joypad
            switch (gb_read(0xFF00) & 0x30) {
            case 0x10:
                gb_write(0xFF00, inputButtons);
                break;
            case 0x20:
                gb_write(0xFF00, inputDpad);
                break;
            default:
                gb_write(0xFF00, 0x3F);
                break;
            }
            // Run CPU until finish line
            // CPU->printInstruction();
            uint8_t cycles = (gbCPU_instruction())/(1+(speed?1:0));
            cycle_count -= gbcPPU_updatePPU(cycle_count);
            cycles += (gbCPU_interrupts(cycles))/(1+(speed?1:0));
            gbCPU_timers(cycles);

            cycle_count -= cycles;

            // Speed check
            if(((gb_read(0xFF4D)&0x80)>>7) != (gb_read(0xFF4D)&0x01)){
                gb_xorWrite(0xFF4D, 0x80);
                speed = gb_read(0xFF4D)&0x80;
            }
        }
        //Draw line
        gbcPPU_drawLine();
    }
    gbcPPU_renderFrame();
    SDL_RenderPresent(renderer);
    timer_end();
    timer_buff();
    }
}

int gbcEmulator_insertCart(game* game) {
    if(gbMEM_insertCart(game)) {
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}