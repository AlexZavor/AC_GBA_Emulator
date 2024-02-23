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
uint8_t gbMEM::read(uint16_t address){
	// if (address < 0xC000)
	// {
	// 	// //Cartrage ram
	// 	// if (RAMEnabled) {
	// 	// 	return ram[address - 0xA000 + (0x4000 * rambank)];
	// 	// }
	// 	// else {
	// 	// 	return 0;
	// 	// 	std::cout << "ram not enabled" << std::endl;
	// 	// }
	// }
	// else if (address < 0xD000);
	// else if (address < 0xE000)
	// {
	// 	//Switching Work Ram (GBC)
	// 	//return Wram[(address - 0xC000) + (WramBank * 0x1000)];
	// }
	// else if (address < 0xFE00);

	return MEM[address];

}
void gbMEM::write(uint16_t address, uint8_t data){
	if (address < 0x8000) {
		//Writing to cartrage. probably a register
		// std::cout << "cartrage write" << std::endl;
		switch (cartMBC) {
		case MBC::NONE:
			//std::cout << "probably tetris being annoying" << std::endl;
			break;
		case MBC::MBC1:
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
				if (bank == 0) {
					bank = 1;
				}
				if (banks < 16) {
					bank &= 0b00001111;
				}if (banks < 8) {
					bank &= 0b00000111;
				}if (banks < 4) {
					bank &= 0b00000011;
				}
				// printf("Bank swap - %d\n", bank);
				std::memcpy(MEM + 0x4000, cartrage + ((long)bank * (long)0x4000), 0x4000);
				// for (int byte = 0; byte < 0x4000; byte++)
				// {
				// 	MEM[0x4000 + byte] = cartrage[((long)bank * (long)0x4000) + byte];
				// }
			}
			else if (address < 0x6000) {
				//RAM Bank Number
				// rambank = data;
				// if (rambank > rambanks) {
				// 	rambank = 0;
				// }
			}
			break;
		default:
			break;
		}
		return;
	}
	// else if (address < 0xC000)
	// {
	// 	//Cartrage ram
	// 	// if (RAMEnabled) {
	// 	// 	ram[(address - 0xA000) + (0x4000 * rambank)];
	// 	// }
	// 	// else {
	// 	// 	//Do nothing
	// 	// 	std::cout << "ram not enabled" << std::endl;
	// 	// }
	// }
	// else if (address < 0xD000);
	// else if (address < 0xE000)
	// {
	// 	//Switching Work Ram (GBC)
	// 	// Wram[(address - 0xC000) + (WramBank * 0x1000)] = data;
	// }
	// else if (address < 0xFE00);
	else{
		MEM[address] = data;
	}
	
}
void gbMEM::orWrite(uint16_t address, uint8_t data){MEM[address] |= data;}
void gbMEM::andWrite(uint16_t address, uint8_t data){MEM[address] &= data;}


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
		memcpy(MEM, cartrage, 0x8000);
		// for (int d = 0; d < 0x8000; d++) {
		// 	MEM[d] = cartrage[d];
		// }
		if (!setMBC(MEM[0x0147])) {
			std::cout << "Unrecognized MBC - ";
			printf("%.2X\n", MEM[0x0147]);
			return false;
		}
		else {
			setBanks(MEM[0x0148]);
			// std::cout << "loading save - ";
			// setRam(MEM[0x0149], Game, MEM);
			// std::cout << "Cartrage Loaded - ";
			// printf("%.2X\n", MEM[0x0147]);
		}
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

bool gbMEM::setMBC(uint8_t code) {
	switch (code)
	{
	case 0x00:
		cartMBC = MBC::NONE;
		printf("MBC - NONE\n");
		return true;
	case 0x01:
		cartMBC = MBC::MBC1;
		printf("MBC - MBC1\n");
		return true;
	default:
		return false;
	}
}
bool gbMEM::setBanks(uint8_t code) {
	banks = 0x0002;
	banks = banks << (code);
	return true;
}