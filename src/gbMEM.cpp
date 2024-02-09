#include "gbMEM.h"
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
16kB switchable ROM bank |
--------------------------- 4000   |= 32kB Cartrigbe
16kB ROM bank #0 |
--------------------------- 0000 --
*/

gbMEM::gbMEM() {
    initMem();
}


// read/write commands to check for cartrage opperation (not active yet)
uint8_t gbMEM::read(uint16_t address){return 0;}
void gbMEM::write(uint16_t address, uint8_t data){}
void gbMEM::orWrite(uint16_t address, uint8_t data){}
void gbMEM::andWrite(uint16_t address, uint8_t data){}


bool gbMEM::insertCart(std::string game){
std::streampos size;
	std::ifstream file2(game, std::ios::in | std::ios::binary | std::ios::ate);
	if (file2.is_open())
	{
		size = file2.tellg();
		cartrage = new char[(int)size];
		file2.seekg(0, std::ios::beg);
		file2.read(cartrage, size);
		file2.close();
		for (int d = 0; d < 0x8000; d++) {
			MEM[d] = cartrage[d];
		}
		// if (!setMBC(MEM[0x0147])) {
		// 	std::cout << "Unrecognized MBC - ";
		// 	printf("%.2X\n", MEM[0x0147]);
		// 	return false;
		// }
		// else {
		// 	setBanks(MEM[0x0148]);
		// 	std::cout << "loading save - ";
		// 	setRam(MEM[0x0149], Game, MEM);
		// 	std::cout << "Cartrage Loaded - ";
		// 	printf("%.2X\n", MEM[0x0147]);
		// }
	}
	else {
		printf("unable to load Cartrage!\n");
		return false;
	}
    
    //Print out Title of Game Cartrage! EPIC!
    printf("loaded Cartrage - ");
    for (int i = 0x134; i < 0x142; i++) {
        std::cout << ((char)MEM[i]);
    }
    printf("\n");
	return true;
}

void gbMEM::initMem() {
    MEM[0xFF01] = 0x00;
	MEM[0xFF02] = 0x7E;
	MEM[0xFF04] = 0x00;
	MEM[0xFF05] = 0x00;
	MEM[0xFF06] = 0x00;
	MEM[0xFF07] = 0xF8;
	MEM[0xFF0F] = 0x00;
	MEM[0xFF10] = 0x80;
	MEM[0xFF11] = 0x00;
	MEM[0xFF12] = 0x00;
	MEM[0xFF13] = 0x00;
	MEM[0xFF14] = 0x00;
	MEM[0xFF16] = 0x00;
	MEM[0xFF17] = 0x00;
	MEM[0xFF18] = 0x00;
	MEM[0xFF19] = 0x00;
	MEM[0xFF1A] = 0x00;
	MEM[0xFF1B] = 0x00;
	MEM[0xFF1C] = 0x00;
	MEM[0xFF1E] = 0x00;
	MEM[0xFF20] = 0x00;
	MEM[0xFF23] = 0x00;
	MEM[0xFF24] = 0x00;
	MEM[0xFF25] = 0x00;
	MEM[0xFF26] = 0x00;
	MEM[0xFF40] = 0x91;
	MEM[0xFF41] = 0x81;
	MEM[0xFF44] = 0x00;
	MEM[0xFF46] = 0xFF;
	MEM[0xFF47] = 0xFC;
	MEM[0xFF48] = 0xFF;
	MEM[0xFF49] = 0xFF;
}
