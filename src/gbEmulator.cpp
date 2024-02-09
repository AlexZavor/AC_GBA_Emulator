#include "gbEmulator.h"

gbEmulator::gbEmulator() {
    MEM = new gbMEM();
    CPU = new gbCPU(MEM);
    PPU = new gbPPU(MEM);
}

void gbEmulator::runFrame(inputData input, SDL_Renderer* renderer) {
    for (uint8_t line = 0; line < 154; line++)
    {
        static int cyclecount = 0; 
        cyclecount += 456;
        while (cyclecount > 0)
        {
            // CPU->printInstruction();
            cyclecount -= CPU->instruction();
        }
        //Draw line
    }
    //return frame
    // printf("Frame!\n");
}

bool gbEmulator::insertCart(std::string game) {
    MEM->insertCart(game);
    return true;
}