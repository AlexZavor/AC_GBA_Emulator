#ifndef GBCPU_H
#define GBCPU_H
#include <stdint.h>
#include <cstring>
#include "gbMEM.h"

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

class gbCPU{
    private:
        gbMEM* MEM;
        uint8_t* dMEM;
        //Main CPU Registers, A,B,C,D,E,F,H,L,sp,pc.
        //also adressible through common combinations like HL or AF
        struct reg registers;

        
        bool IME; //Interrupt Master Enable
        bool preIME; //used because IME only returns after one instruction
        bool halted; //Shows if the CPU is Halted
        uint8_t DMA;
        uint8_t DIV;
        int lineprogress;

    public:
        gbCPU(gbMEM* memory);
        //Returns number of cycles taken to execute
        uint8_t instruction();
        void printInstruction();

        void timers(uint8_t clock);
        int interrupts(int cycles);

        void setColor();
    
    private:
        //set registers to correct values
        void initCpu();
        //Push data onto the stack
        void PushStack(uint8_t data);
        //Pop data off stack
        uint8_t PopStack();
        //Set of extra instructions
        uint8_t CBPrefix();
        //Flag setting functions
        void setZ(bool set);
        void setN(bool set);
        void setH(bool set);
        void setC(bool set);
        //for error Handeling
        void Failure(int code);

};

#endif /* GBMEM_H */