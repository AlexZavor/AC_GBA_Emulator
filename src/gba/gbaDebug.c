#include "gba/gbaDebug.h"
#include "gba/gbaCPU.h"
#include "gba/gbaMEM.h"
#include "gba/bitwise.h"
#include "stdio.h"

#define R(r) get_reg(r)
#define PC get_reg(15)
#define LR get_reg(14)
#define SP get_reg(13)

#define N_FLAG ((CPSR >> 31) & 0x1)
#define Z_FLAG ((CPSR >> 30) & 0x1)
#define C_FLAG ((CPSR >> 29) & 0x1)
#define V_FLAG ((CPSR >> 28) & 0x1)

// Input for Single step stuff?
// static inputData input;
int print = false;

// Debug stuff
inline void gba_debug(SDL_Event *e){
    // gbaCPU_print_cycle();
    // if(PC == 0x0801019A)print = true;
    if(print){
        uint32_t instr = gba_read32(PC);
        uint32_t CPSR = get_cpsr();
        bool THUMB = get_bit(CPSR, 5);
        if(THUMB){
            printf("T\t - Code: 0x%.4X", instr&0xFFFF);
        }else{
            printf("\t - Code: 0x%.8X", instr);
        }
        printf(" \tPC: 0x%.8X   \t| R0-%.8X R1-%.8X R2-%.8X R3-%.8X - %c%c%c%c\n", PC, R(0), R(1), R(2), R(3), N_FLAG?'N':'-', Z_FLAG?'Z':'-', C_FLAG?'C':'-', V_FLAG?'V':'-');
    }
    // if(PC == 0x080101C2)print = false;
}