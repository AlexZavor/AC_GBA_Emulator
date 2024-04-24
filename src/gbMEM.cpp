
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
	memset(BGColorPallet,0xFF, sizeof(BGColorPallet));
	memset(OBJColorPallet,0xFF, sizeof(OBJColorPallet));
	memset(Vram, 0x00, sizeof(Vram));
	color = false;
}

gbMEM::~gbMEM()
{
	saveRam();
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
				}if (banks < 16) {
					bank &= 0b00001111;
				}if (banks < 8) {
					bank &= 0b00000111;
				}if (banks < 4) {
					bank &= 0b00000011;
				}
				// printf("Bank swap - %d\n", bank);
				std::memcpy(MEM + 0x4000, cartrage + ((long)bank * (long)0x4000), 0x4000);
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
		case MBC::MBC5:
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
				std::memcpy(MEM + 0x4000, cartrage + ((long)bank * (long)0x4000), 0x4000);
			}
			else if (address < 0x4000) {
				//ROM Bank Number high bit
				bank &= 0x00FF;
				bank |= ((uint16_t)(data))<<8;
				std::memcpy(MEM + 0x4000, cartrage + ((long)bank * (long)0x4000), 0x4000);
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
	else if(address == 0xFF55){
		// GBC dma transfer
		if(data & 0x80){
			// H-blank DMA
			printf("H-blank dma\n");
			MEM[0xFF55] = 0xFF;
		} else {
			// Instant DMA
			// printf("more DMA\n");
			uint16_t size = ((data & 0x7f) + 1);
			vRamDMAFull(size * 0x10);
			// time += size;
			MEM[0xFF55] = 0xFF;
		}
		// MEM[address] = data;
	}
	else if(address == 0xFF69){
		// BG color pallets
		MEM[address] = data;
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
		MEM[address] = data;
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
		swapVramBank(data);
	}
	else if(address == 0xFF70){
		// Wram check
		MEM[address] = data;
		swapWramBank(data);
	}
	else{
		MEM[address] = data;
	}
}
void gbMEM::orWrite(uint16_t address, uint8_t data){
	write(address, MEM[address] | data);
}
void gbMEM::andWrite(uint16_t address, uint8_t data){
	write(address, MEM[address] & data);
}

void gbMEM::setColor() {
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

bool gbMEM::insertCart(std::string game){
	std::streampos size;
	std::ifstream file2(game, std::ios::in | std::ios::binary | std::ios::ate);
	if (file2.is_open())
	{
		size = file2.tellg();
		cartrage = new char[(int)size];
		this->game = game.erase(game.size() - 3, game.size() - 1).erase(0,5);
		file2.seekg(0, std::ios::beg);
		file2.read(cartrage, size);
		file2.close();
		memcpy(MEM, cartrage, 0x8000);
		if (!setMBC(MEM[0x0147])) {
			std::cout << "\n Unrecognized MBC - ";
			printf("%.2X\n", MEM[0x0147]);
			return false;
		}
		else {
			setBanks(MEM[0x0148]);
			setRam(MEM[0x0149]);
		}
	}
	else {
		printf("\n Unable to load Cartrage!\n");
		return false;
	}
    
    //Print out Title of Game Cartrage! EPIC!
    printf("\n loaded Cartrage - ");
    for (int i = 0x134; i < 0x142; i++) {
        std::cout << ((char)MEM[i]);
    }
    printf("\n");
	return true;
}

void gbMEM::initMem() {
	WramBank = 1;
	cartMBC = MBC::NONE;
	RAMEnabled = false;
	bank = 1;
	banks = 2;
	ramBank = 0;
	ramBanks = 0;
	battery = false;
	RAM = false;
	timer = false;
	rumble = false;

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
	// No MBC
	case 0x00:
		cartMBC = MBC::NONE;
		return true;
	// MBC 1
	case 0x01:
		cartMBC = MBC::MBC1;
		return true;
	case 0x02:
		cartMBC = MBC::MBC1;
		RAM = true;
		return true;
	case 0x03:
		cartMBC = MBC::MBC1;
		RAM = true;
		battery = true;
		return true;
	// MBC 2 - Unimplemented
	case 0x05:
		cartMBC = MBC::MBC2;
		return false;
	case 0x06:
		cartMBC = MBC::MBC2;
		battery = true;
		return false;
	// MBC 3 - Unimplemented
	case 0x0F:
		cartMBC = MBC::MBC3;
		timer = true;
		battery = true;
		return false;
	case 0x10:
		cartMBC = MBC::MBC3;
		timer = true;
		RAM = true;
		battery = true;
		return false;
	case 0x11:
		cartMBC = MBC::MBC3;
		return false;
	case 0x12:
		cartMBC = MBC::MBC3;
		RAM = true;
		return false;
	case 0x13:
		cartMBC = MBC::MBC3;
		RAM = true;
		battery = true;
		return false;
	// MBC 5
	case 0x19:
		cartMBC = MBC::MBC5;
		return true;
	case 0x1A:
		cartMBC = MBC::MBC5;
		RAM = true;
		return true;
	case 0x1B:
		cartMBC = MBC::MBC5;
		RAM = true;
		battery = true;
		return true;
	case 0x1C:
		cartMBC = MBC::MBC5;
		rumble = true;
		return true;
	case 0x1D:
		cartMBC = MBC::MBC5;
		rumble = true;
		RAM = true;
		return true;
	case 0x1E:
		cartMBC = MBC::MBC5;
		rumble = true;
		RAM = true;
		battery = true;
		return true;
	// Error
	default:
		return false;
	}
}
bool gbMEM::setBanks(uint8_t code) {
	banks = 0x0002;
	banks <<= (code);
	return true;
}

bool gbMEM::setRam(uint8_t code) {
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
	ram = new char[(int)ramBanks * 0x2000];
	if (ramBanks && battery) {
		std::cout << "loading save - ";
		std::ifstream file("SAVES/" + (game) + ".SAV", std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			file.seekg(0, std::ios::beg);
			file.read(ram, (int)ramBanks * 0x2000);
			file.close();
			memcpy(MEM + 0xA000, ram + (ramBank*0x2000), 0x2000);
			std::cout << "save loaded" << std::endl;
		}
		else {
			std::cout << "Save not found" << std::endl;
		}
	}
	return true;
}
bool gbMEM::saveRam() {
	if (ramBanks > 0 && battery) {
		memcpy(ram + (ramBank * 0x2000), MEM + 0xA000, 0x2000);
		std::ofstream file("SAVES/" + (game) + ".SAV");
		file.open("SAVES/" + (game) + ".SAV", std::ios::out | std::ios::binary);
		if (file.is_open())
		{
			file.clear();
			file.write((char*)ram, (int)ramBanks * 0x2000);
			file.close();
			std::cout << "Saved game \n";
			return 1;
		}
		else {
			std::cout << "failed to save \n";
			return 0;
		}
	}
	return 1;
}

bool gbMEM::swapWramBank(uint8_t bank)
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
bool gbMEM::swapVramBank(uint8_t bank)
{
	// printf("Swap Vram bank - GBC\n");
	// Save old ram
	memcpy(Vram + (VramBank * 0x2000), MEM + 0x8000, 0x2000);
	// Load new data
	VramBank = (bank & 0x01);
	memcpy(MEM + 0x8000, Vram + (VramBank * 0x2000), 0x2000);
	return true;
}

bool gbMEM::saveVram()
{
	memcpy(Vram + (VramBank * 0x2000), MEM + 0x8000, 0x2000);
	return true;
}

bool gbMEM::vRamDMAFull(uint16_t size) {
	saveVram();
	uint16_t dest = ((uint16_t)MEM[0xFF51] << 8) + (uint16_t)MEM[0xFF52];
	dest &= 0x1FF0;
	dest |= 0x8000;
	uint16_t source = ((uint16_t)MEM[0xFF53] << 8) + (uint16_t)MEM[0xFF54];
	source &= 0xFFF0;

    memcpy(MEM + dest, MEM + source, size);
	return true;
}
