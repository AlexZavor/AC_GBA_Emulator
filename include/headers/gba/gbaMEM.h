#ifndef GBAMEM_H
#define GBAMEM_H

#include "game.h"
#include "stdint.h"

// Init and Deinit the memory
void gbaMEM_init();
void gbaMEM_deinit();
// Load memory from rom file
int gbaMEM_insertCart(game* cartridge);

// Read And Write functions
uint32_t gba_read32(uint32_t addr);
#define gba_read16(addr) ((uint16_t)gba_read32(addr))
#define gba_read8(addr) ((uint8_t)gba_read32(addr))
void gba_write32(uint32_t addr, uint32_t data);
void gba_write16(uint32_t addr, uint16_t data);
void gba_write8(uint32_t addr, uint8_t data);

// functions for Sys calls
void gba_memset(uint32_t dest, uint32_t data, uint32_t words);
void gba_memcpy(uint32_t dest, uint32_t src, uint32_t words);

// Debug
void dump_vram();

#endif /* GBAMEM_H */