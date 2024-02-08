#ifndef GBMEM_H
#define GBMEM_H
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

class gbMEM{
    private:
        uint8_t MEM[0x10000];
        
        char* cartrage;
        char* ram;
    public:
        gbMEM();
        uint8_t qRead(uint16_t address);
        uint8_t read(uint16_t address);
        void qWrite(uint16_t address, uint8_t data);
        void write(uint16_t address, uint8_t data);
        void orQWrite(uint16_t address, uint8_t data);
        void orWrite(uint16_t address, uint8_t data);
        void andQWrite(uint16_t address, uint8_t data);
        void andWrite(uint16_t address, uint8_t data);

        bool insertCart(std::string game);

    private:
        void initMem();
};

#endif /* GBMEM_H */