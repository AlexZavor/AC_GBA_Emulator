#ifndef GBCPU_H
#define GBCPU_H
#include <stdint.h>
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
        //Main CPU Registers, A,B,C,D,E,F,H,L,pc,sp.
        //also adressible through common combinations like HL or AF
        struct reg registers;

    public:
        gbCPU(gbMEM* memory);
        //Returns number of cycles taken to execute
        uint8_t instruction();
        void printInstruction();

        void timers();
        int interrupts();
    
    private:
        void initCpu();
        //Stack operations for you know... stack
        void PushStack(uint8_t data);
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