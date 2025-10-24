#include "gb/gbEmulator.h"

#include "inputData.h"
// #include "gb/gbAPU.h"
#include "gb/gbMEM.h"
#include "gb/gbCPU.h"
#include "gb/gbPPU.h"
#include "timer.h"

static SDL_Renderer* renderer;
static inputData input;
static SDL_Event* e;

static uint8_t inputButtons;
static uint8_t inputDpad;

// Read relevant input for the GB Emulator.
void gbEmulator_input(){ 
    readInput(&input, e);
    
    //Formatting input data
    inputButtons = 0x1F - (input.start<<3) - (input.sel<<2) - (input.B<<1) - input.A;
    inputDpad    = 0x2F - (input.down<<3) - (input.up<<2) - (input.left<<1) - input.right;
}

void gbEmulator_init(SDL_Renderer* render, SDL_Event* event) {
    renderer = render;
	resetInputData(&input);
    e = event;
    // init mem, cpu, and ppu
    gbMEM_init();
    gbCPU_init();
    gbPPU_init(render);
    // gbAPU().APU_setMEM(MEM);
}

void gbEmulator_deinit(){
    gbMEM_deinit();
    gbCPU_deinit();
    gbPPU_deinit();
}

int gbEmulator_run() {
    while(true){
    timer_start();
    gbEmulator_input();
    if(input.quit) return 0;

    for (uint8_t line = 0; line < 154; line++)
    {
        static int cycle_count = 0; 
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
            uint8_t cycles = gbCPU_instruction();
            cycles += gbCPU_interrupts(cycles);
            gbCPU_timers(cycles);

            cycle_count -= cycles;
            gbPPU_updatePPU(cycle_count);
        }
        //Draw line
        gbPPU_drawLine();
    }
    gbPPU_renderFrame();
    SDL_RenderPresent(renderer);
    timer_end();
    timer_buff();
    }
}

int gbEmulator_insertCart(game* game) {
    if(gbMEM_insertCart(game)) {
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}