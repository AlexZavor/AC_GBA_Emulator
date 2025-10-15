#include "gbc/gbcEmulator.h"

#include "inputData.h"
// #include "gb/gbAPU.h"
#include "gb/gbMEM.h"
#include "gb/gbCPU.h"
#include "gbc/gbcPPU.h"
#include "timer.h"

static SDL_Renderer* renderer;
static gbMEM* MEM;
static gbCPU* CPU;
static gbcPPU* PPU; 
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
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    PPU = new gbcPPU(MEM, renderer);
    // gbAPU().APU_setMEM(MEM);
}

void gbcEmulator_deinit(){
    delete(MEM);
    delete(CPU);
    delete(PPU);
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
        MEM->MEM[0xFF44] = line;
        while (cycle_count > 0)
        {   
            //Update joypad
            switch (MEM->MEM[0xFF00] & 0x30) {
            case 0x10:
                MEM->MEM[0xFF00] = inputButtons;
                break;
            case 0x20:
                MEM->MEM[0xFF00] = inputDpad;
                break;
            default:
                MEM->MEM[0xFF00] = 0x3F;
                break;
            }
            // Run CPU until finish line
            // CPU->printInstruction();
            uint8_t cycles = (CPU->instruction())/(1+(speed?1:0));
            cycle_count -= PPU->updatePPU(cycle_count);
            cycles += (CPU->interrupts(cycles))/(1+(speed?1:0));
            CPU->timers(cycles);

            cycle_count -= cycles;

            // Speed check
            if(((MEM->MEM[0xFF4D]&0x80)>>7) != (MEM->MEM[0xFF4D]&0x01)){
                MEM->MEM[0xFF4D] ^= 0x80;
                speed = MEM->MEM[0xFF4D]&0x80;
            }
        }
        //Draw line
        PPU->drawLine();
    }
    PPU->renderFrame();
    SDL_RenderPresent(renderer);
    timer_end();
    timer_buff();
    }
}

int gbcEmulator_insertCart(game* game) {
    if(MEM->insertCart(game)) {
        CPU->setColor();
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}