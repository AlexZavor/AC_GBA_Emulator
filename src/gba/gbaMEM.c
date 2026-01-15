#include "gba/gbaMEM.h"

#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#ifndef WIN32
#include <sys/mman.h>
#endif

// TODO: get the rest of the memory set up.
/*
Memory Map
General Internal Memory

  00000000-00003FFF   BIOS - System ROM         (16 KBytes)
  00004000-01FFFFFF   Not used
  02000000-0203FFFF   WRAM - On-board Work RAM  (256 KBytes) 2 Wait
  02040000-02FFFFFF   Not used
  03000000-03007FFF   WRAM - On-chip Work RAM   (32 KBytes)
  03008000-03FFFFFF   Not used
  04000000-040003FE   I/O Registers
  04000400-04FFFFFF   Not used

Internal Display Memory

  05000000-050003FF   BG/OBJ Palette RAM        (1 Kbyte)
  05000400-05FFFFFF   Not used
  06000000-06017FFF   VRAM - Video RAM          (96 KBytes)
  06018000-06FFFFFF   Not used
  07000000-070003FF   OAM - OBJ Attributes      (1 Kbyte)
  07000400-07FFFFFF   Not used

External Memory (Game Pak)

  08000000-09FFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 0
  0A000000-0BFFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 1
  0C000000-0DFFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 2
  0E000000-0E00FFFF   Game Pak SRAM    (max 64 KBytes) - 8bit Bus width
  0E010000-0FFFFFFF   Not used

Unused Memory Area

  10000000-FFFFFFFF   Not used (upper 4bits of address bus unused)
*/

// Internal Memory
static uint8_t* WRAM_low;    //0x2000000
#define WRAM_LOW_SIZE (0x3FFFF)
static uint8_t* WRAM_high;   //0x3000000
#define WRAM_HIGH_SIZE (0x7FFF)
static uint8_t* IO_Reg;      //0x4000000
#define IO_REG_SIZE (0x3FE)

// Video Memory
uint8_t* Pallet;   //0x5000000
#define PALLET_SIZE (0x3FF)
uint8_t* Vram;      //0x6000000
#define VRAM_SIZE (0x17FFF)

// Cartridge Data
static game* game_p;
static int cart_fd;
static struct stat  cart_sb;
static uint8_t* cartridge;      //0x8000000
static uint8_t* cartridge_sram; //0xE000000

// pointer with read/write data included
typedef struct{
    void* p;
    bool r,w,v;
} ptr_t;

// Helper function for void pointer checking
void check_and_clear(void* data){
    uint8_t* ptr = *(uint8_t**)data;
    if(ptr != NULL){
        free(ptr);
        ptr = NULL;
    }
}

// Initallsize memory sections (thats a lot of malloc)
void gbaMEM_init(){
    cartridge_sram = (uint8_t*)malloc(0xFFFF);
    WRAM_low = (uint8_t*)calloc(WRAM_LOW_SIZE, sizeof(uint8_t)); // 256 KBytes
    WRAM_high = (uint8_t*)calloc(WRAM_HIGH_SIZE, sizeof(uint8_t)); // 32 KBytes
    IO_Reg = (uint8_t*)calloc(IO_REG_SIZE, sizeof(uint8_t));
    Pallet = (uint8_t*)calloc(PALLET_SIZE, sizeof(uint8_t));
    Vram = (uint8_t*)calloc(VRAM_SIZE, sizeof(uint8_t));
}

// Free memory
void gbaMEM_deinit(){
    if(cart_fd > 0){
		#ifdef WIN32
		free(cartridge);
		#else
		munmap(cartridge, cart_sb.st_size);
		#endif
		close(cart_fd);
        cart_fd = 0;
    }
    // Save Sram before clear
    check_and_clear(&cartridge_sram);
    check_and_clear(&WRAM_low);
    check_and_clear(&WRAM_high);
    check_and_clear(&IO_Reg);
    check_and_clear(&Pallet);
    check_and_clear(&Vram);
}

// Load cartridge into data (mmap/char* for linux/win)
int gbaMEM_insertCart(game *g){
    game_p = g;
	char filepath[512];
	sprintf(filepath, "%s%s", g->location, g->name);
	cart_fd = open(filepath, O_RDONLY);
	if (cart_fd >= 0)
	{
		fstat(cart_fd, &cart_sb);
		#ifdef WIN32
		cartridge = malloc(cart_sb.st_size);
		lseek(cart_fd, 0, SEEK_SET);
		read(cart_fd, cartridge, cart_sb.st_size);
		#else
		cartridge = mmap(NULL, cart_sb.st_size, PROT_READ,
			MAP_PRIVATE, cart_fd, 0);
		#endif
	}
	else {
		printf("Unable to load cartridge!\n");
		return false;
	}
	
	//Print out Title of Game cartridge! EPIC!
	char title[13];
	memcpy(title, &cartridge[0xA0], 12);
    title[12] = '\0';
	printf("Loaded Cartridge - %s, %ld bytes\n", title, cart_sb.st_size);
	return true;
}

//==================== Read/Write ====================

ptr_t get_pointer(uint32_t addr){
    ptr_t pointer = {0, false, false, false};
    switch ((addr & 0xFF000000) >> 24){
    case 0x2:// Low WRAM
        if((addr - 0x2000000) < WRAM_LOW_SIZE){
            pointer.p = (void*)(WRAM_low + (addr - 0x2000000));
            pointer.w = true;
            pointer.r = true;
            pointer.v = true;
        }
        break;
    case 0x3:// High WRAM
        if((addr - 0x3000000) < WRAM_HIGH_SIZE){
            pointer.p = (void*)(WRAM_high + (addr - 0x3000000));
            pointer.w = true;
            pointer.r = true;
            pointer.v = true;
        }
        break;
    case 0x4:// IO Registers
        if((addr - 0x4000000) < IO_REG_SIZE){
            // printf("Read I/O - %.3X\n",addr - 0x4000000);
            pointer.p = (void*)(IO_Reg + (addr - 0x4000000));
            pointer.w = true;
            pointer.r = true;
            pointer.v = true;
        }
        break;
    case 0x5:// Pallet memory
        if((addr - 0x5000000) < PALLET_SIZE){
            pointer.p = (void*)(Pallet + (addr - 0x5000000));
            pointer.w = true;
            pointer.r = true;
            pointer.v = true;
        }
        break;
    case 0x6:// Vram
        if((addr - 0x6000000) < VRAM_SIZE){
            pointer.p = (void*)(Vram + (addr - 0x6000000));
            pointer.w = true;
            pointer.r = true;
            pointer.v = true;
        }
        break;
    case 0x8:// Cartridge
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
        if(((addr & 0xFFFFFF)) < cart_sb.st_size){
            // is inside cartridge area
            pointer.p = (void*)(cartridge+(addr-0x08000000));
            pointer.r = true;
            pointer.v = true;
        }
        break;
    default:
        break;
    }
    return pointer;
}

uint32_t gba_read32(uint32_t addr){
    addr&=~0x1;//half-word align.
    ptr_t pointer = get_pointer(addr);
    if(pointer.v && pointer.r){
        return *(uint32_t*)pointer.p;
    }
    printf("could not read addr - %.8X (%s)\n", addr, pointer.v?("can't read"):("not valid"));
    return -1;
}
void gba_write32(uint32_t addr, uint32_t data){
    ptr_t pointer = get_pointer(addr);
    if(pointer.v && pointer.w){
        *(uint32_t*)pointer.p = data;
    }else{
        printf("could not write addr - %.8X (%s)\n", addr, pointer.v?("ROM"):("not valid"));
        exit(0);
    }
}
void gba_write16(uint32_t addr, uint16_t data){
    ptr_t pointer = get_pointer(addr);
    if(pointer.v && pointer.w){
        *(uint16_t*)pointer.p = data;
    }else
        printf("could not write addr h-word - %.8X (%s)\n", addr, pointer.v?("ROM"):("not valid"));
}
void gba_write8(uint32_t addr, uint8_t data){
    ptr_t pointer = get_pointer(addr);
    if(pointer.v && pointer.w){
        *(uint8_t*)pointer.p = data;
    }else
        printf("could not write addr byte - %.8X (%s)\n", addr, pointer.v?("ROM"):("not valid"));
}

// memory functions using virtual data pointers
void gba_memset(uint32_t dest, uint32_t data, uint32_t words){
    ptr_t dest_p = get_pointer(dest);
    if(dest_p.v && dest_p.w){
        memset(dest_p.p, data, words*sizeof(uint32_t));
    }else{
        printf("bad dest pointer memset\n");
    }
}
void gba_memcpy(uint32_t dest, uint32_t src, uint32_t words){
    ptr_t dest_p = get_pointer(dest);
    ptr_t src_p = get_pointer(src);
    if(dest_p.v && dest_p.w && src_p.v && src_p.r){
        memcpy(dest_p.p, src_p.p, words*sizeof(uint32_t));
    }else{
        printf("bad pointer memcpy\n");
    }
}

// ================== Debug Functions ================
void dump_vram(){
    FILE* f = fopen("dump.log", "wb");
    if (f != NULL) {
        fwrite(Vram, 1, VRAM_SIZE, f);
        fclose(f);
    }
}

void print_mem(uint32_t start_addr, int words){
    for(int i = 0; i < words; i++){
        printf("\t\t%.8X\n",gba_read32(start_addr+(4*i)));
    }
}