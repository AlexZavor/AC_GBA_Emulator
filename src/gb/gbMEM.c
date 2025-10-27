#include "gb/gbMEM.h"

// #include <fstream>
#include "fcntl.h"
#include "unistd.h"
#include "globals.h"
#include "stdio.h"
#include "string.h"
#ifdef WIN32

#else
#include <sys/mman.h>
#endif

/*
Interrupt Enable Register
--------------------------- FFFF
Internal RAM
--------------------------- FF80
Empty but unusable for I/O
--------------------------- FF4C
I/O ports
--------------------------- FF00
Empty but unusable for I/O
--------------------------- FEA0
Sprite Attrib Memory (OAM)
--------------------------- FE00
Echo of 8kB Internal RAM
--------------------------- E000
8kB Internal RAM
--------------------------- C000
8kB switchable RAM bank
--------------------------- A000
8kB Video RAM
--------------------------- 8000 --
16kB switchable ROM bank 		   |
--------------------------- 4000   |= 32kB Cartridge
16kB ROM bank #0 				   |
--------------------------- 0000 --
*/

// ----------- cartridge data -----------
static game* game_p;
typedef enum{
	NONE,
	MBC1,
	MBC2,
	MBC3,
	MBC5,
}MBC;
static MBC cartMBC;
static uint16_t bank;
static uint16_t banks;
static uint8_t ramBank;
static uint8_t ramBanks;
static bool RAMEnabled;
static bool battery;
static bool RAM;
static bool timer;
static bool rumble;

// System Memory
uint8_t MEM[0x10000];

static uint8_t Vram[0x4000];
static uint8_t VramBank = 1;

static uint8_t* cartridge;
static int cart_fd;
static struct stat  cart_sb;
static uint8_t* ram;

// GBC variables
bool color = false;
static pallet BGColorPallet[8];
static pallet OBJColorPallet[8];
static uint8_t Wram[0x8000];
static uint8_t WramBank = 1;



// ============== Helper functions ==============
// sets inital values in the memory and for the cartridge
void initMem() {
	WramBank = 1;
	cartMBC = NONE;
	RAMEnabled = false;
	bank = 1;
	banks = 2;
	ramBank = 0;
	ramBanks = 0;
	battery = false;
	RAM = false;
	timer = false;
	rumble = false;

	MEM[0xFF02] = 0x7E;
	MEM[0xFF07] = 0xF8;
	MEM[0xFF10] = 0x80;
	MEM[0xFF26] = 0x0F;
	MEM[0xFF40] = 0x91;
	MEM[0xFF41] = 0x81;
	MEM[0xFF46] = 0xFF;
	MEM[0xFF47] = 0xFC;
	MEM[0xFF48] = 0xFF;
	MEM[0xFF49] = 0xFF;
}

bool setMBC(uint8_t code) {
	switch (code)
	{
	// No MBC
	case 0x00:
		cartMBC = NONE;
		return true;
	// MBC 1
	case 0x01:
		cartMBC = MBC1;
		return true;
	case 0x02:
		cartMBC = MBC1;
		RAM = true;
		return true;
	case 0x03:
		cartMBC = MBC1;
		RAM = true;
		battery = true;
		return true;
	// MBC 2 - Unimplemented
	case 0x05:
		cartMBC = MBC2;
		return false;
	case 0x06:
		cartMBC = MBC2;
		battery = true;
		return false;
	// MBC 3 - Unimplemented
	case 0x0F:
		cartMBC = MBC3;
		timer = true;
		battery = true;
		return false;
	case 0x10:
		cartMBC = MBC3;
		timer = true;
		RAM = true;
		battery = true;
		return false;
	case 0x11:
		cartMBC = MBC3;
		return false;
	case 0x12:
		cartMBC = MBC3;
		RAM = true;
		return false;
	case 0x13:
		cartMBC = MBC3;
		RAM = true;
		battery = true;
		return false;
	// MBC 5
	case 0x19:
		cartMBC = MBC5;
		return true;
	case 0x1A:
		cartMBC = MBC5;
		RAM = true;
		return true;
	case 0x1B:
		cartMBC = MBC5;
		RAM = true;
		battery = true;
		return true;
	case 0x1C:
		cartMBC = MBC5;
		rumble = true;
		return true;
	case 0x1D:
		cartMBC = MBC5;
		rumble = true;
		RAM = true;
		return true;
	case 0x1E:
		cartMBC = MBC5;
		rumble = true;
		RAM = true;
		battery = true;
		return true;
	// Error
	default:
		return false;
	}
}

bool setBanks(uint8_t code) {
	banks = 0x0002;
	banks <<= (code);
	return true;
}

bool setRam(uint8_t code) {
	switch (code)
	{
	case 0x00:
		RAM = false;
		break;
	case 0x02:
		ramBanks = 1;
		break;
	case 0x03:
		ramBanks = 4;
		break;
	case 0x04:
		ramBanks = 16;
		break;
	case 0x05:
		ramBanks = 8;
		break;
	default:
		return false;
	}
	ram = (uint8_t*)calloc((int)ramBanks * 0x2000, sizeof(uint8_t));
	if (ramBanks && battery) {
		printf("Loading save file - ");
		char save_path[512];
		sprintf(save_path, "%s%s.SAV", SAVE_DIR, game_p->name);
		int fd = open(save_path, O_RDWR);
		if (fd > 0)
		{
			lseek(fd, 0, SEEK_SET);
			read(fd, ram, ramBanks*0x2000);
			close(fd);
			memcpy(MEM + 0xA000, ram + (ramBank*0x2000), 0x2000);
			printf("save loaded!\n");
		}
		else {
			printf("Save failed to load.\n");
		}
	}
	return true;
}

bool saveRam() {
	if (ramBanks > 0 && battery) {
		memcpy(ram + (ramBank * 0x2000), MEM + 0xA000, 0x2000);
		char save_path[512];
		sprintf(save_path, "%s%s.SAV", SAVE_DIR, game_p->name);
		int fd = open(save_path, O_CREAT|O_RDWR|O_TRUNC);
		if (fd > 0)
		{
			lseek(fd, 0, SEEK_SET);
			write(fd, ram, ramBanks*0x2000);
			close(fd);
			printf("Game Saved\n");
			game_p->has_save = true;
			return 1;
		}
		else {
			game_p->has_save = false;
			printf("Game could not save\n");
			return 0;
		}
	}
	return 1;
}

// ============== GB MEM functions ===============

void gbMEM_init() {
	initMem();
	memset(BGColorPallet,0xFF, sizeof(BGColorPallet));
	memset(OBJColorPallet,0xFF, sizeof(OBJColorPallet));
	memset(Vram, 0x00, sizeof(Vram));
	color = false;
}

void gbMEM_deinit()
{
	saveRam();
	memset(MEM, 0, 0x10000); // unload the cartridge and memory
	if(ram != NULL)
		free(ram);
	if(cart_fd >0){
		#ifdef WIN32
		free(cartridge);
		#else
		munmap(cartridge, cart_sb.st_size);
		#endif
		close(cart_fd);
	}
	game_p = NULL;
}

int gbMEM_insertCart(game* g){
	char filepath[512];
	sprintf(filepath, "%s%s", GAME_DIR, g->name);
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
		game_p = g;
		memcpy(MEM, cartridge, 0x8000);
		if (!setMBC(MEM[0x0147])) {
			printf("\nUnrecognized MBC - %.2X\n", MEM[0x0147]);
			return false;
		}
		else {
			setBanks(MEM[0x0148]);
			setRam(MEM[0x0149]);
		}
	}
	else {
		printf("Unable to load cartridge!\n");
		return false;
	}
	
	//Print out Title of Game cartridge! EPIC!
	char title[15];
	memcpy(title, &MEM[0x134], 14);
	printf("Loaded Cartridge - %s\n", title);
	return true;
}

void gbMEM_colorWriteChecks(uint16_t address, uint8_t data){
	MEM[address] = data;
	if(address == 0xFF55){
		// GBC dma transfer
		if(data & 0x80){
			// H-blank DMA
			printf("H-blank dma\n");
			MEM[0xFF55] = 0xFF;
		} else {
			// Instant DMA
			uint16_t size = ((data & 0x7f) + 1);
			gbMEM_vRamDMAFull(size * 0x10);
			// time += size;
			MEM[0xFF55] = 0xFF;
		}
		return;
	}
	else if(address == 0xFF69){
		// BG color pallets
        pallet* pal = BGColorPallet + ((MEM[0xFF68] & 0x38) >> 3);
        switch (MEM[0xFF68] & 0x07) {
        case 0:
            pal->low0 = MEM[0xFF69];
            break;
        case 1:
            pal->high0 = MEM[0xFF69];
            break;
        case 2:
            pal->low1 = MEM[0xFF69];
            break;
        case 3:
            pal->high1 = MEM[0xFF69];
            break;
        case 4:
            pal->low2 = MEM[0xFF69];
            break;
        case 5:
            pal->high2 = MEM[0xFF69];
            break;
        case 6:
            pal->low3 = MEM[0xFF69];
            break;
        case 7:
            pal->high3 = MEM[0xFF69];
            break;
        }
        if(MEM[0xFF68] & 0x80){
            MEM[0xFF68]++;
            MEM[0xFF68] &= 0b10111111;
        }
	}
	else if(address == 0xFF6B){
		// OBJ color pallets
        pallet* pal = OBJColorPallet + ((MEM[0xFF6A] & 0x38) >> 3);
        switch (MEM[0xFF6A] & 0x07)
        {
        case 0:
            pal->low0 = MEM[0xFF6B];
            break;
        case 1:
            pal->high0 = MEM[0xFF6B];
            break;
        case 2:
            pal->low1 = MEM[0xFF6B];
            break;
        case 3:
            pal->high1 = MEM[0xFF6B];
            break;
        case 4:
            pal->low2 = MEM[0xFF6B];
            break;
        case 5:
            pal->high2 = MEM[0xFF6B];
            break;
        case 6:
            pal->low3 = MEM[0xFF6B];
            break;
        case 7:
            pal->high3 = MEM[0xFF6B];
            break;
        }
        if(MEM[0xFF6A] & 0x80){
            MEM[0xFF6A]++;
            MEM[0xFF6A] &= 0b10111111;
        }
	}
	else if(address == 0xFF4F){
		// Vram check
		MEM[address] = (data | 0xE);
		gbMEM_swapVramBank(data);
		return;
	}
	else if(address == 0xFF70){
		// Wram check
		gbMEM_swapWramBank(data);
	}
}

void gbMEM_cartridgeWrite(uint16_t address, uint8_t data){
	// Writing to cartridge ROM. probably a register
	switch (cartMBC) {
	case NONE:
		//std::cout << "probably Tetris being annoying" << std::endl;
		break;
	case MBC1:
		if (address < 0x2000) {
			//Ram Enable
			if ((data & 0x0F) == 0x0A) {
				RAMEnabled = true;
			}
			else {
				RAMEnabled = false;
			}
		}
		else if (address < 0x4000) {
			//ROM Bank Number
			bank = data & 0x1F;
			// if (bank == 0) {
			// 	bank = 1;
			// }if (banks < 16) {
			// 	bank &= 0b00001111;
			// }if (banks < 8) {
			// 	bank &= 0b00000111;
			// }if (banks < 4) {
			// 	bank &= 0b00000011;
			// }
			// printf("Bank swap - %d\n", bank);
			memcpy(MEM + 0x4000, cartridge + ((long)bank * (long)0x4000), 0x4000);
		}
		else if (address < 0x6000) {
			//RAM Bank Number / upper bits
			if(ramBanks > 1){
				printf("Swap ram bank - MBC1\n");
				// Save old ram
				memcpy(ram + (ramBank * 0x2000), MEM + 0xA000, 0x2000);
				// Load new data
				ramBank = data;
				if (ramBank > ramBanks) {
					ramBank = 0;
				}
				memcpy(MEM + 0xA000, ram + (ramBank * 0x2000), 0x2000);
			}
			if(banks >= 64){
				printf("ERROR: upper bits change - MBC1\n");
			}
		}
		else if (address < 0x8000) {
			//Banking Mode Select
			printf("ERROR: Banking Mode Select - MBC1\n");
		}
		break;
	case MBC5:
		if (address < 0x2000) {
			//Ram Enable
			if ((data & 0x0F) == 0x0A) {
				RAMEnabled = true;
			}
			else {
				RAMEnabled = false;
			}
		}
		else if (address < 0x3000) {
			//ROM Bank Number
			bank &= 0xFF00;
			bank += data;
			bank %= banks;
			// printf("Swap rom bank - MBC5 %d\n", bank);
			memcpy(MEM + 0x4000, cartridge + ((long)bank * (long)0x4000), 0x4000);
		}
		else if (address < 0x4000) {
			//ROM Bank Number high bit
			bank &= 0x00FF;
			bank |= ((uint16_t)(data))<<8;
			memcpy(MEM + 0x4000, cartridge + ((long)bank * (long)0x4000), 0x4000);
		}
		else if (address < 0x6000) {
			//RAM Bank Number
			// printf("Swap ram bank - MBC5\n");
			// Save old ram
			memcpy(ram + (ramBank * 0x2000), MEM + 0xA000, 0x2000);
			// Load new data
			ramBank = data;
			memcpy(MEM + 0xA000, ram + (ramBank * 0x2000), 0x2000);
		}
		else if (address < 0x8000) {
			printf("ERROR: Nothing here - MBC5\n");
		}
		break;
	default:
		break;
	}
	return;
}

void gb_DMA(uint8_t dest){
	memcpy(MEM + 0xFE00, MEM + (dest << 8), 0x9F);
}

// ================ GBC Functions ==================

void gbMEM_setColor() {
	color = true;
	MEM[0xFF4C] = 0xC0;
	MEM[0xFF4D] = 0xfe;
	MEM[0xFF4F] = 0xfe;
	MEM[0xFF51] = 0xff;
	MEM[0xFF52] = 0xff;
	MEM[0xFF53] = 0xff;
	MEM[0xFF54] = 0xff;
	MEM[0xFF55] = 0xff;
	MEM[0xFF56] = 0x3e;
	MEM[0xFF6C] = 0x00;
	MEM[0xFF70] = 0xf8;
}

bool gbMEM_swapWramBank(uint8_t bank)
{
	// printf("Swap Wram bank - GBC\n");
	// Save old ram
	memcpy(Wram + (WramBank * 0x1000), MEM + 0xD000, 0x1000);
	// Load new data
	WramBank = bank;
	if (WramBank == 0) {
		WramBank = 1;
	}
	WramBank &= 0x7;
	memcpy(MEM + 0xD000, Wram + (WramBank * 0x1000), 0x1000);
	return true;
}

bool gbMEM_swapVramBank(uint8_t bank)
{
	// printf("Swap Vram bank - GBC\n");
	// Save old ram
	memcpy(Vram + (VramBank * 0x2000), MEM + 0x8000, 0x2000);
	// Load new data
	VramBank = (bank & 0x01);
	memcpy(MEM + 0x8000, Vram + (VramBank * 0x2000), 0x2000);
	return true;
}

bool gbMEM_saveVram()
{
	memcpy(Vram + (VramBank * 0x2000), MEM + 0x8000, 0x2000);
	return true;
}

bool gbMEM_vRamDMAFull(uint16_t size) {
	gbMEM_saveVram();
	uint16_t dest = ((uint16_t)MEM[0xFF51] << 8) + (uint16_t)MEM[0xFF52];
	dest &= 0x1FF0;
	dest |= 0x8000;
	uint16_t source = ((uint16_t)MEM[0xFF53] << 8) + (uint16_t)MEM[0xFF54];
	source &= 0xFFF0;

    memcpy(MEM + dest, MEM + source, size);
	return true;
}

uint8_t gb_vramRead(uint16_t address){
	return Vram[address];
}

// GBC get color
uint16_t gb_getBackColor(uint8_t pallet, int color){
	switch (color)
	{
	case 0:
		return BGColorPallet[pallet].color0;
	case 1:
		return BGColorPallet[pallet].color1;
	case 2:
		return BGColorPallet[pallet].color2;
	case 3:
		return BGColorPallet[pallet].color3;
	default:
		printf("invalid bgcolor pallet\n");
		return 0;
	}
}
uint16_t gb_getObjColor(uint8_t pallet, int color){
	switch (color)
	{
	case 1:
		return OBJColorPallet[pallet].color1;
	case 2:
		return OBJColorPallet[pallet].color2;
	case 3:
		return OBJColorPallet[pallet].color3;
	default:
		printf("invalid objcolor pallet\n");
		return 0;
	}
}