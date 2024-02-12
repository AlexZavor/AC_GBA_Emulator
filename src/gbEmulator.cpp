#include "gbEmulator.h"

gbEmulator::gbEmulator(SDL_Renderer* renderer) {
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    PPU = new gbPPU(MEM, renderer);
}

void gbEmulator::runFrame(inputData input) {
    for (uint8_t line = 0; line < 154; line++)
    {
        static int cyclecount = 0; 
        cyclecount = 456;
        while (cyclecount > 0)
        {
            MEM->MEM[0xFF00] |= 0x3F;
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