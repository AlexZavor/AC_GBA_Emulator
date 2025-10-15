#include "gb/gbEmulator.h"

#include "inputData.h"
// #include "gb/gbAPU.h"
#include "gb/gbMEM.h"
#include "gb/gbCPU.h"
#include "gb/gbPPU.h"
#include "timer.h"

static SDL_Renderer* renderer;
static SDL_Texture* texture;
gbMEM* MEM;
gbCPU* CPU;
gbPPU* PPU; 
static inputData input;
static SDL_Event* e;

uint8_t inputButtons;
uint8_t inputDpad;

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
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    PPU = new gbPPU(MEM, renderer);
    // gbAPU().APU_setMEM(MEM);
}

void gbEmulator_deinit(){
    SDL_DestroyTexture(texture);
    delete(MEM);
    delete(CPU);
    delete(PPU);
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
            uint8_t cycles = CPU->instruction();
            cycles += CPU->interrupts(cycles);
            CPU->timers(cycles);

            cycle_count -= cycles;
            PPU->updatePPU(cycle_count);
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

int gbEmulator_insertCart(game* game) {
    if(MEM->insertCart(game)) {
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}