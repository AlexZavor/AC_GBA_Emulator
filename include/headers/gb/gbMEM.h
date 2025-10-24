#ifndef GBMEM_H
#define GBMEM_H
#include "stdint.h"
#include "game.h"

extern uint8_t MEM[0x10000];
extern bool color;

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

void gbMEM_init();
void gbMEM_deinit();
int gbMEM_insertCart(game* cartridge);

// Memory reads, very speedy macros. I tried a lot of things and this is just the fastest.
#define gb_read(addr) (MEM[addr])
#define gb_read16(addr) (gb_read(addr) + (gb_read(addr + 1) << 8))
// write with cartridge operations
void gbMEM_cartridgeWrite(uint16_t address, uint8_t data);
void gbMEM_colorWriteChecks(uint16_t address, uint8_t data);
static inline void gb_write(uint16_t address, uint8_t data){
	if (address < 0x8000) {
		gbMEM_cartridgeWrite(address, data);
	}else if(color){
        gbMEM_colorWriteChecks(address, data);
    }else{
        MEM[address] = data;
    }
}
// write data | mem with cartridge operations
#define gb_orWrite(addr, data) (gb_write(addr, gb_read(addr)|data))
// write data & mem with cartridge operations
#define gb_andWrite(addr, data) (gb_write(addr, gb_read(addr)&data))
// write data ^ mem with cartridge operations
#define gb_xorWrite(addr, data) (gb_write(addr, gb_read(addr)^data))
// increment data at address
#define gb_inc(addr) (gb_write(addr, gb_read(addr)+1))
// start a dma transfer
void gb_DMA(uint8_t dest);

// GBC Functions
// set memory as for a game boy color        
void gbMEM_setColor();
// Swap Wram bank
bool gbMEM_swapWramBank(uint8_t bank);
// Swap Vram bank
bool gbMEM_swapVramBank(uint8_t bank);
bool gbMEM_saveVram();

// GBC vram DMA transfer functions
bool gbMEM_vRamDMAFull(uint16_t size);
bool gbMEM_vramDMALine();
uint8_t gb_vramRead(uint16_t address);

// GBC get color
uint16_t gb_getBackColor(uint8_t pallet, int color);
uint16_t gb_getObjColor(uint8_t pallet, int color);

#endif /* GBMEM_H */