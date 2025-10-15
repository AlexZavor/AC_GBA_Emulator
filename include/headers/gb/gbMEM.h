#ifndef GBMEM_H
#define GBMEM_H
#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "game.h"


typedef struct {
    union{
        struct{
            uint8_t low0;
            uint8_t high0;
        };
        uint16_t color0;
    };
    union{
        struct{
            uint8_t low1;
            uint8_t high1;
        };
        uint16_t color1;
    };
    union{
        struct{
            uint8_t low2;
            uint8_t high2;
        };
        uint16_t color2;
    };
    union{
        struct{
            uint8_t low3;
            uint8_t high3;
        };
        uint16_t color3;
    };
}pallet;

class gbMEM{
    public:
        uint8_t MEM[0x10000];
        bool flag = 0;

        uint8_t Vram[0x4000];
        uint8_t VramBank = 1;

        pallet BGColorPallet[8];
        pallet OBJColorPallet[8];

    private:
        char* cartage;
        char* ram;
        
        uint8_t Wram[0x8000];
        uint8_t WramBank = 1;

        typedef enum{
		NONE,
		MBC1,
		MBC2,
        MBC3,
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
        bool rumble;

        
        // if color game boy
        bool color = false;

        // Name of current game
        game* game_p;

    public:
        gbMEM();
        ~gbMEM();
        // read with cartage operations
        // uint8_t read(uint16_t address);
        // write with cartage operations
        void write(uint16_t address, uint8_t data);
        // write data | mem with cartage operations
        void orWrite(uint16_t address, uint8_t data);
        // write data & mem with cartage operations
        void andWrite(uint16_t address, uint8_t data);

        // insert and save a cart to memory
        bool insertCart(game* g);

        // set memory as for a game boy color        
        void setColor();
        // Swap Wram bank
        bool swapWramBank(uint8_t bank);
        // Swap Vram bank
        bool swapVramBank(uint8_t bank);
        bool saveVram();
        
        // GBC vram DMA transfer functions
        bool vRamDMAFull(uint16_t size);
        bool vramDMALine();


    private:
        // initialize starting memory (mostly from nintendo logo)
        void initMem();
        // Load what MBC the cartage is
        bool setMBC(uint8_t code);
        // Set number of banks from code
        bool setBanks(uint8_t code);
        // Set in number of ram banks, and load ram if needed.
        bool setRam(uint8_t code);
        // Save the game before you leave!
        bool saveRam();
};

#endif /* GBMEM_H */