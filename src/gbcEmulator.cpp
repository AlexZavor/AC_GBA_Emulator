#include "gbcEmulator.h"

gbcEmulator::gbcEmulator(SDL_Renderer* renderer, SDL_Texture* texture) {
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    CPU->setColor();
    PPU = new gbcPPU(MEM, renderer, texture);
    prevWbank = 1;
}

gbcEmulator::~gbcEmulator()
{
    delete MEM;
    // delete CPU;
    // delete PPU;
}

void gbcEmulator::runFrame(inputData input) {

    //Formatting input data
    uint8_t inputButtons = 0x1F - (input.start<<3) - (input.sel<<2) - (input.B<<1) - input.A;
    uint8_t inputDpad    = 0x2F - (input.down<<3) - (input.up<<2) - (input.left<<1) - input.right;
    
    for (uint8_t line = 0; line < 154; line++)
    {
        static int cyclecount = 0;
        static bool speed = 0 ;
        cyclecount += 456;
        MEM->MEM[0xFF44] = line;
        while (cyclecount > 0)
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
            cyclecount -= PPU->updatePPU(cyclecount);
            cycles += (CPU->interrupts(cycles))/(1+(speed?1:0));
            CPU->timers(cycles);

            cyclecount -= cycles;

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
}

bool gbcEmulator::insertCart(std::string game) {
    if(MEM->insertCart(game)) {
        CPU->setColor();
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}