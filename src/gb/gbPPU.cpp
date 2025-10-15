#include "gb/gbPPU.h"

gbPPU::gbPPU(gbMEM* memory, SDL_Renderer* rend) {
    MEM = memory;
    dMEM = memory->MEM;
    renderer = rend;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void gbPPU::drawLine() {

	if (dMEM[0xFF44] == 144) {
		//turn on V-blank flag
		dMEM[0xFF41] |= 0b00000001;
		dMEM[0xFF41] &= 0b11111101;
		dMEM[0xFF0F] |= 0b00000001;	//V-blank interrupt
		if (dMEM[0xFF41] & 0b00010000) {
			dMEM[0xFF0F] |= 0b00000010;
			//vblank stat interrupt
		}
	}
	
	if (dMEM[0xFF44] < 144) {
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

}

void gbPPU::updatePPU(int cycles) {
	// Cycles is how many cycles are left in the line 456-0
	if(dMEM[0xFF44] < 144) {
		if(cycles > (456-80)) {
			// Mode 2: OAM Scan
			dMEM[0xFF41] |= 0b00000010;
			dMEM[0xFF41] &= 0b11111110;

			if (dMEM[0xFF41] & 0b00100000) {
				dMEM[0xFF0F] |= 0b00000010;
				// OAM Scan stat interrupt
			}
		}
		else if(cycles > (456-200)) {
			// Mode 3: Drawing
			dMEM[0xFF41] |= 0b00000011;
		}
		else {
			// Mode 0: H-Blank
			dMEM[0xFF41] &= 0b11111100;

			if (dMEM[0xFF41] & 0b00001000) {
				dMEM[0xFF0F] |= 0b00000010;
				// H-blank stat interrupt
			}
		}
	}


	if (dMEM[0xFF44] == dMEM[0xFF45]) {
		//LYC == LY
		dMEM[0xFF41] |= 0b00000100;
		if (dMEM[0xFF41] & 0b01000000) {
			dMEM[0xFF0F] |= 0b00000010;
		}
	}
	else {
		//LYC != LY
		dMEM[0xFF41] &= 0b11111011;
		if (dMEM[0xFF41] & 0b01000000) {
			dMEM[0xFF0F] &= 0b11111101;
		}
	}
}

void gbPPU::renderFrame() {
	// Credit to DOOMReboot on Github for the pixel pusher system! https://github.com/DOOMReboot
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

        // Draw frame to texture (Parallelizing didn't work, threads are too slow to spin up)
		uint32_t t;
        for (uint32_t x = 0; x < (SCREEN_HEIGHT); x++){
			for(uint32_t y = 0; y < (SCREEN_WIDTH); y++){
				switch (Vram[y][x])
				#ifdef GREEN_PALLET
				{
				case 0:
					t = 0xFF9bbc0f;
					break;
				case 1:
					t = 0xFF8bac0f;
					break;
				case 2:
					t = 0xFF306230;
					break;
				case 3:
					t = 0xFF0f380f;
					break;
				default:
            		t = 0xFFfc03e8;
					break;
				}
				#else
				{
				case 0:
            		f = 0xFFffffff;
					break;
				case 1:
            		f = 0xFFa9a9a9;
					break;
				case 2:
            		f = 0xFF545454;
					break;
				case 3:
            		f = 0xFF000000;
					break;
				default:
            		f = 0xFFfc03e8;
					break;
				}
				#endif
				pixelBuffer[((x)*(SCREEN_WIDTH)) + y] = t;
			}
		}
        
		// Unlock the texture in VRAM and send to renderer!
        SDL_UnlockTexture(texture);
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
		int X = (x + dMEM[0xFF43])%256;
		int Y = (y + dMEM[0xFF42])%256;
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
		int X = x - (dMEM[0xFF4B]-7);
		int Y = y - dMEM[0xFF4A];
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
	std::vector<int> sprites_to_draw;
	for (int s = 0; s < 40; s++) {
		int ypos = dMEM[0xFE00 + (s * 4)] - 16;
		if (size) {
			if (y>=ypos && y < ypos+16) {
				sprites_to_draw.push_back(s);
			}
		}
		else {
			if (y>=ypos && y < ypos+8) {
				sprites_to_draw.push_back(s);
			}
		}
	}
	for (auto s : sprites_to_draw) {
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
