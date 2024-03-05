#include "gbPPU.h"

/*
PPU Tests
Basic old graphics		max - 57, min - 0
No graphics 			max - 3,  min - 0
No SDL_Renderer calls	max - 5,  min - 0
Write to Array			max - 6,  min - 0
Render array once 		max - 59, min - 11
Pixel pusher 			max - 17, min - 2 (but adverage was like 3-4!)
Final design			max - 10.7, min - 4.1 (better when plugged in)
*/
// #define GREEN_PALLET

gbPPU::gbPPU(gbMEM* memory, SDL_Renderer* rend, SDL_Texture* textu) {
    MEM = memory;
    dMEM = memory->MEM;
    renderer = rend;
	texture = textu;
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

void gbPPU::renderFrame(){
	// The Back Buffer texture may be stored with an extra bit of width (pitch) on the video card in order to properly
    // align it in VRAM should the width not lie on the correct memory boundary (usually four bytes).
    int32_t pitch = 0;

    // This will hold a pointer to the memory position in VRAM where our Back Buffer texture lies
    uint32_t* pixelBuffer = nullptr;

    // Lock the memory in order to write our Back Buffer image to it
    if (!SDL_LockTexture(texture, NULL, (void**)&pixelBuffer, &pitch))
    {
        // The pitch of the Back Buffer texture in VRAM must be divided by four bytes
        // as it will always be a multiple of four
        pitch /= sizeof(uint32_t);

        // Fill texture with randomly colored pixels
        for (uint32_t x = 0; x < (SCREEN_HEIGHT); x++){
			for(uint32_t y = 0; y < (SCREEN_WIDTH); y++){
				#ifdef GREEN_PALLET
				switch (Vram[y/SCALE][x/SCALE])
				{
				case 0:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0x9bbc0f;
					break;
				case 1:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0x8bac0f;
					break;
				case 2:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0x306230;
					break;
				case 3:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0x0f380f;
					break;
				default:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0xfc03e8;
					break;
				}
				#else
				switch (Vram[y/SCALE][x/SCALE])
				{
				case 0:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0xffffff;
					break;
				case 1:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0xa9a9a9;
					break;
				case 2:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0x545454;
					break;
				case 3:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0x000000;
					break;
				default:
            		pixelBuffer[(x*(SCREEN_WIDTH)) + y] = (0xff << 24) | 0xfc03e8;
					break;
				}
				#endif
			}
		}

        // Unlock the texture in VRAM to let the GPU know we are done writing to it
        SDL_UnlockTexture(texture);

        // Copy our texture in VRAM to the display framebuffer in VRAM
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }
	return;
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
		Vram[x][y] = pixel;
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
		int X = x + (dMEM[0xFF4B]-7);
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
		if ((x - (dMEM[0xFF4B] - 7) < 160) && (x - (dMEM[0xFF4B] - 7) >= 0) && (y - dMEM[0xFF4A] < 144) && (y - dMEM[0xFF4A] >= 0)) {
			Vram[x][y] = pixel;
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
					if ((!(flags & 0b10000000)) || Vram[X][Y] == 0) {
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
						Vram[X][Y] = pixel;
					}
				}
			}
		}
	}
}
