#include "gbPPU.h"

gbPPU::gbPPU(gbMEM* memory, SDL_Renderer* rend)
{
    MEM = memory;
    dMEM = memory->MEM;
    renderer = rend;
}

void gbPPU::drawLine(uint8_t line)
{
    dMEM[0xFF44] = line;

	if (line == 144) {
		//turn on V-blank flag
		dMEM[0xFF41] |= 0b00000001;
		dMEM[0xFF41] &= 0b11111101;
		dMEM[0xFF0F] |= 0b00000001;	//V-blank interrupt
		if (dMEM[0xFF41] & 0b00010000) {
			dMEM[0xFF0F] |= 0b00000010;
			//vblank stat intterrupt
		}
	} 
	else if (line < 144) {
        
		//Reading drawing
		dMEM[0xFF41] &= 0b11111100;
		dMEM[0xFF0F] &= 0b11111110;	//V-blank disable

        if (dMEM[0xFF40] & 0b00000001) {
            //Background and Window Enable
            drawBackground();
            if (dMEM[0xFF40] & 0b00100000) {
                //Window Enable
                drawWindow();
            }
        }
        if (dMEM[0xFF40] & 0b00000010) {
            //Sprite Enable
            drawSprites();
        }
	}

	if (line == dMEM[0xFF45]) {
		//LYC == LY
		dMEM[0xFF41] |= 0b00000100;
		if (dMEM[0xFF41] & 0b01000000) {
			dMEM[0xFF0F] |= 0b00000010;
		}
	}
	else {
		//LYC != LY
		dMEM[0xFF41] &= 0b11111011;
		dMEM[0xFF0F] &= 0b11111101;
	}
}

void gbPPU::drawBackground() {
    int addressBase;
	bool sign;
	if (dMEM[0xFF40] & 0b00010000) {
		//unsigned data starting at $8000
		addressBase = 0x8000;
		sign = 0;
	}
	else {
		//signed data starting at $9000
		addressBase = 0x9000;
		sign = 1;
	}
	int map;
	if (dMEM[0xFF40] & 0b00001000) {
		//Tile map starts at $9C00
		map = 0x9C00;
	}
	else {
		//Tile map starts at $9800
		map = 0x9800;
	}


	for (int x = 0; x < 160; x++) {
		int y = dMEM[0xFF44];
		int X = x + dMEM[0xFF43];
		int Y = y + dMEM[0xFF42];
		if (Y >= 256) {
			Y -= 256;
		}
		if (X >= 256) {
			X -= 256;
		}
		int tx = X / 8;
		int ty = Y / 8;
		uint8_t pixel;
		if (sign) {
			signed char tile = (signed)dMEM[map + (ty * 32) + tx];
			pixel =
				((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2)] & (0b00000001 << (7 - (X % 8)))) >> (7 - (X % 8))) +
				(((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2) + 1] & (0b00000001 << (7 - (X % 8)))) * 2) >> (7 - (X % 8)));
		}
		else {
			unsigned char tile = (unsigned)dMEM[map + (ty * 32) + tx];
			pixel =
				((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2)] & (0b00000001 << (7 - (X % 8)))) >> (7 - (X % 8))) +
				(((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2) + 1] & (0b00000001 << (7 - (X % 8)))) * 2) >> (7 - (X % 8)));
		}
		switch (pixel)
		{
		case 0:
			pixel = (dMEM[0xFF47] & 0b00000011);
			break;
		case 1:
			pixel = (dMEM[0xFF47] & 0b00001100) >> 2;
			break;
		case 2:
			pixel = (dMEM[0xFF47] & 0b00110000) >> 4;
			break;
		case 3:
			pixel = (dMEM[0xFF47] & 0b11000000) >> 6;
			break;
		default:
			break;
		}
		// Vram[x][y] = pixel;
        SDL_SetRenderDrawColor(renderer, ~pixel<<6, ~pixel<<6, ~pixel<<6, 0xFF);
        SDL_RenderDrawPoint(renderer, x, y);
		currentLine[x] = pixel;
        // printf("%s\n",SDL_GetError());
	}
}

void gbPPU::drawWindow() {
    int addressBase;
	bool sign;
	if (dMEM[0xFF40] & 0b00010000) {
		//unsigned data starting at $8000
		addressBase = 0x8000;
		sign = 0;
	}
	else {
		//signed data starting at $9000
		addressBase = 0x9000;
		sign = 1;
	}
	int map;
	if (dMEM[0xFF40] & 0b01000000) {
		//Tile map starts at $9C00
		map = 0x9C00;
	}
	else {
		//Tile map starts at $9800
		map = 0x9800;
	}
	//draw Window on Vram
	for (int x = 0; x < 160; x++) {
		int y = dMEM[0xFF44];
		int X = x + dMEM[0xFF4B]-7;
		int Y = y + dMEM[0xFF4A];
		if (Y >= 256) {
			Y -= 256;
		}
		if (X >= 256) {
			X -= 256;
		}
		else if (X < 0) {
			X += 256;
		}
		int tx = X / 8;
		int ty = Y / 8;
		uint8_t pixel;
		if (sign) {
			signed char tile = (signed)dMEM[map + (ty * 32) + tx];
			pixel =
				((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2)] & (0b00000001 << (7 - (X % 8)))) >> (7 - (X % 8))) +
				(((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2) + 1] & (0b00000001 << (7 - (X % 8)))) * 2) >> (7 - (X % 8)));
		}
		else {
			unsigned char tile = (unsigned)dMEM[map + (ty * 32) + tx];
			pixel =
				((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2)] & (0b00000001 << (7 - (X % 8)))) >> (7 - (X % 8))) +
				(((dMEM[addressBase + (tile * 16) + ((Y % 8) * 2) + 1] & (0b00000001 << (7 - (X % 8)))) * 2) >> (7 - (X % 8)));
		}
		if ((x - (dMEM[0xFF4B] - 7) <= 160) && (x - (dMEM[0xFF4B] - 7) > 0) && (y - dMEM[0xFF4A] < 144) && (y - dMEM[0xFF4A] >= 0)) {
			// Vram[x][y] = pixel;
            SDL_SetRenderDrawColor(renderer, ~pixel<<6, ~pixel<<6, ~pixel<<6, 0xFF);
            SDL_RenderDrawPoint(renderer, x, y);
			currentLine[x] = pixel;
		}
	}
}

void gbPPU::drawSprites() {
	bool size = 0;
	if (dMEM[0xFF40] & 0b00000100) {
		size = 1; //set sprite size to 8x16
	}
	uint8_t y = dMEM[0xFF44];
	//draw Sprites on Vram
	std::vector<int> Spritestodraw;
	for (int s = 0; s < 40; s++) {
		int ypos = dMEM[0xFE00 + (s * 4)] - 16;
		if (size) {
			if (y>=ypos && y < ypos+16) {
				Spritestodraw.push_back(s);
			}
		}
		else {
			if (y>=ypos && y < ypos+8) {
				Spritestodraw.push_back(s);
			}
		}
	}
	for (auto s : Spritestodraw) {
		int ypos = dMEM[0xFE00 + (s * 4)] - 16;
		int xpos = dMEM[(0xFE00 + (s * 4)) + 1] - 8;
		uint8_t tile = dMEM[(0xFE00 + (s * 4)) + 2];
		uint8_t flags = dMEM[(0xFE00 + (s * 4)) + 3];
		int r = y - ypos;
		for (int bit = 0; bit < 8; bit++) {
			uint8_t Y = r;
			uint8_t X = bit;
			if (flags & 0b00100000) {
				//X flip
				X = 7 - bit;
			}
			if (flags & 0b01000000) {
				//Y flip
				if (size) {
					Y = 15 - (r);
				}
				else {
					Y = 7 - (r);
				}
			}
			uint8_t pixel =
				((dMEM[0x8000 + ((tile) * 16) + (Y * 2)] & (0b00000001 << (7 - X))) >> (7 - X)) +
				(((dMEM[0x8000 + ((tile) * 16) + (Y * 2) + 1] & (0b00000001 << (7 - X))) * 2) >> (7 - X));
			if (pixel != 0) {
				uint8_t Y = ypos + r;
				uint8_t X = xpos + bit;
				if (Y >= 0 && Y < 144 && X >= 0 && X < 160) {
					if ((!(flags & 0b10000000)) || currentLine[X] == 0) {
						switch (pixel)
						{
						case 1:
							pixel = (dMEM[0xFF48 + ((flags & 0b00010000) >> 4)] & 0b00001100) >> 2;
							break;
						case 2:
							pixel = (dMEM[0xFF48 + ((flags & 0b00010000) >> 4)] & 0b00110000) >> 4;
							break;
						case 3:
							pixel = (dMEM[0xFF48 + ((flags & 0b00010000) >> 4)] & 0b11000000) >> 6;
							break;
						default:
							break;
						}
						// Vram[X][Y] = pixel;
                        SDL_SetRenderDrawColor(renderer, ~pixel<<6, ~pixel<<6, ~pixel<<6, 0xFF);
                        SDL_RenderDrawPoint(renderer, X, Y);
						currentLine[X] = pixel;
					}
				}
			}
		}
	}
}
