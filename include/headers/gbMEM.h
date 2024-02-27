#ifndef GBMEM_H
#define GBMEM_H
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

class gbMEM{
    public:
        uint8_t MEM[0x10000];
        bool flag = 0;

    private:
        char* cartrage;
        char* ram;

        typedef enum{
		NONE,
		MBC1,
		MBC2,
		MBC5,
	    }MBC;

        MBC cartMBC;
        bool RAMEnabled;
        uint16_t bank;
        uint16_t banks;
        uint8_t ramBank;
        uint8_t ramBanks;
        bool battery;
        bool RAM;
        bool timer;

        // Name of current game
        std::string game;

    public:
        gbMEM();
        ~gbMEM();
        // read with cartrage operations
        // uint8_t read(uint16_t address);
        // write with cartrage operations
        void write(uint16_t address, uint8_t data);
        // wrtie data | mem with cartrage operations
        void orWrite(uint16_t address, uint8_t data);
        // write data & mem with cartrage operations
        void andWrite(uint16_t address, uint8_t data);

        // insert and save a cart to memory
        bool insertCart(std::string game);

    private:
        // initalize starting memory (mostly from nintendo logo)
        void initMem();
        // Load what MBC the cartrage is
        bool setMBC(uint8_t code);
        // Set number of banks from code
        bool setBanks(uint8_t code);
        // Set in number of ram banks, and load ram if needed.
        bool setRam(uint8_t code);
        // Save the game before you leave!
        bool saveRam();
};

#endif /* GBMEM_H */