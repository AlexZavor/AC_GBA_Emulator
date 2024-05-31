#include "gbEmulator.h"

gbEmulator::gbEmulator(SDL_Renderer* renderer, SDL_Texture* texture) {
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    PPU = new gbPPU(MEM, renderer, texture);
    gbAPU().APU_setMEM(MEM);
}

gbEmulator::~gbEmulator()
{
    gbAPU().APU_clearMEM();
    delete MEM;
    // delete CPU;
    // delete PPU;
}

void gbEmulator::runFrame(inputData input) {

    //Formatting input data
    uint8_t inputButtons = 0x1F - (input.start<<3) - (input.sel<<2) - (input.B<<1) - input.A;
    uint8_t inputDpad    = 0x2F - (input.down<<3) - (input.up<<2) - (input.left<<1) - input.right;
    
    for (uint8_t line = 0; line < 154; line++)
    {
        static int cyclecount = 0; 
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
            uint8_t cycles = CPU->instruction();
            cycles += CPU->interrupts(cycles);
            CPU->timers(cycles);

            cyclecount -= cycles;
            PPU->updatePPU(cyclecount);
        }
        //Draw line
        PPU->drawLine();
    }
    PPU->renderFrame();
}

bool gbEmulator::insertCart(std::string game) {
    if(MEM->insertCart(game)) {
        return true;
    }
    else {
        printf("ABORTING\n");
        return false;
    }
}