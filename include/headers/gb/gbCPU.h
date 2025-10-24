#ifndef GBCPU_H
#define GBCPU_H
#include "stdint.h"

struct reg {
    struct {
        union {
            struct {
                unsigned char f;
                unsigned char a;
            };
            unsigned short af;
        };
    };

    struct {
        union {
            struct {
                unsigned char c;
                unsigned char b;
            };
            unsigned short bc;
        };
    };

    struct {
        union {
            struct {
                unsigned char e;
                unsigned char d;
            };
            unsigned short de;
        };
    };

    struct {
        union {
            struct {
                unsigned char l;
                unsigned char h;
            };
            unsigned short hl;
        };
    };

    unsigned short sp;
    unsigned short pc;
};

void gbCPU_init();
void gbCPU_deinit();
//Returns number of cycles taken to execute
uint8_t gbCPU_instruction();
void gbCPU_printInstruction();

void gbCPU_timers(uint8_t clock);
int gbCPU_interrupts(int cycles);

void gbCPU_setColor();

#endif /* GBMEM_H */