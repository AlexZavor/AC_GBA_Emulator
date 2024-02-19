#include "gbEmulator.h"

gbEmulator::gbEmulator(SDL_Renderer* renderer) {
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    PPU = new gbPPU(MEM, renderer);
}

void gbEmulator::runFrame(inputData input) {

    //Formatting input data
    uint8_t inputButtons = 0x1F - (input.start<<3) - (input.sel<<2) - (input.B<<1) - input.A;
    uint8_t inputDpad    = 0x2F - (input.down<<3) - (input.up<<2) - (input.left<<1) - input.right;
    
    for (uint8_t line = 0; line < 154; line++)
    {
        static int cyclecount = 0; 
        cyclecount = 456;
        while (cyclecount > 0)
        {   
            //Update joypad
            uint8_t inputSel = (MEM->MEM[0xFF00] & 0x30);
            if(inputSel == 0x30){
                MEM->MEM[0xFF00] = 0x3F;
            }else if(inputSel == 0x10){
                MEM->MEM[0xFF00] = inputButtons;
            }else if(inputSel == 0x20){
                MEM->MEM[0xFF00] = inputDpad;
            }
            // CPU->printInstruction();
            uint8_t cycles = CPU->instruction();
            cycles += CPU->interrupts(cycles);
            cyclecount -= cycles;
            CPU->timers(cycles);
        }
        //Draw line
        PPU->drawLine(line);
    }
    //return frame
    // printf("Frame!\n");
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