#include "gbcPPU.h"

gbcPPU::gbcPPU(gbMEM *memory, SDL_Renderer *rend, SDL_Texture *textu) {
    MEM = memory;
    dMEM = memory->MEM;
    renderer = rend;
	texture = textu;
	lastDMA = dMEM[0xff55];
}

void gbcPPU::drawLine() {

	if (dMEM[0xFF44] == 144) {
		//turn on V-blank flag
		dMEM[0xFF41] |= 0b00000001;
		dMEM[0xFF41] &= 0b11111101;
		dMEM[0xFF0F] |= 0b00000001;	//V-blank interrupt
		if (dMEM[0xFF41] & 0b00010000) {
			dMEM[0xFF0F] |= 0b00000010;
			//vblank stat intterrupt
		}
	}
	
	if (dMEM[0xFF44] < 144) {
		// Save Vram before drawing
		MEM->saveVram();
		drawBackground();
		if (dMEM[0xFF40] & 0b00100000) {
			//Window Enable
			drawWindow();
		}
        if (dMEM[0xFF40] & 0b00000010) {
            //Sprite Enable
            drawSprites();
        }
	}
}

uint32_t gbcPPU::updatePPU(int cycles) {
	uint32_t time = 0;

	// Cycles is how many cycles are left in the line 456-0
	if(dMEM[0xFF44] < 144) {
		if(cycles > (456-80)) {
			// Mode 2: OAM Scan
			dMEM[0xFF41] |= 0b00000010;
			dMEM[0xFF41] &= 0b11111110;

			if (dMEM[0xFF41] & 0b00100000) {
				dMEM[0xFF0F] |= 0b00000010;
				//OAM Scan stat intterrupt
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
				//Hblank stat intterrupt
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

	return time;
}

void gbcPPU::renderFrame() {
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

        // Draw frame to texture
        for (uint32_t x = 0; x < (SCREEN_HEIGHT); x++){
			for(uint32_t y = 0; y < (SCREEN_WIDTH); y++){
				uint16_t data =  Vram[y/SCALE][x/SCALE];
				uint8_t red = data&0x001F ;
				uint8_t green = (data&0x03E0)>>5;
				uint8_t blue = (data&0x7C00)>>10;
				uint32_t pixel = (red << 19) | (green << 11) | (blue << 3);
                pixelBuffer[(x*(SCREEN_WIDTH)) + y] = pixel;
			}
		}

        // Unlock the texture in VRAM and send to renderer!
        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }
	memset(Vram,0,(160*144));
	return;
}

void gbcPPU::drawBackground() {
    int addressBase;
	bool sign;
	if (dMEM[0xFF40] & 0b00010000) {
		//unsigned data starting at $8000
		// addressBase = 0x8000;
		addressBase = 0x0000;
		sign = 0;
	}
	else {
		//signed data starting at $9000
		// addressBase = 0x9000;
		addressBase = 0x1000;
		sign = 1;
	}
	int map;
	if (dMEM[0xFF40] & 0b00001000) {
		//Tile map starts at $9C00
		map = 0x9C00 - 0x8000;
	}
	else {
		//Tile map starts at $9800
		map = 0x9800 - 0x8000;
	}


	for (int x = 0; x < 160; x++) {
		int y = dMEM[0xFF44];
		int X = (x + dMEM[0xFF43])%256;
		int Y = (y + dMEM[0xFF42])%256;
		int tx = X / 8;
		int ty = Y / 8;
		uint16_t pixel;
		uint8_t xBit = (7 - (X % 8));
		uint8_t yBit = ((Y % 8) * 2);
		// Atribute pulled from other vram bank
		uint8_t attr = MEM->Vram[map + (ty * 32) + tx + 0x2000];
		bool bank = (attr&0b00001000) ? 1 : 0;
		if (attr & 0b00100000) {
			//X flip
			xBit = (X % 8);
		}
		if (attr & 0b01000000) {
			//Y flip
			yBit = ((7 - (Y % 8)) * 2);
		}
		uint8_t pal = attr&0x07;

		if (sign) {
			signed char tile = (signed)MEM->Vram[map + (ty * 32) + tx];
			pixel =
				((MEM->Vram[addressBase + (tile * 16) + yBit + (bank*0x2000)] & (0b00000001 << xBit)) >> xBit) +
				(((MEM->Vram[addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)] & (0b00000001 << xBit)) * 2) >> xBit);
		}
		else {
			unsigned char tile = (unsigned)MEM->Vram[map + (ty * 32) + tx];
			pixel =
				((MEM->Vram[addressBase + (tile * 16) + yBit + (bank*0x2000)] & (0b00000001 << xBit)) >> xBit) +
				(((MEM->Vram[addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)] & (0b00000001 << xBit)) * 2) >> xBit);
		}
		BGPriority[x] = (attr&0x80);
		line[x] = pixel;
		switch (pixel)
		{
		case 0:
			pixel = MEM->BGColorPallet[pal].color0;
			break;
		case 1:
			pixel = MEM->BGColorPallet[pal].color1;
			break;
		case 2:
			pixel = MEM->BGColorPallet[pal].color2;
			break;
		case 3:
			pixel = MEM->BGColorPallet[pal].color3;
			break;
		default:
			break;
		}
		Vram[x][y] = pixel;
	}
}

void gbcPPU::drawWindow() {
    int addressBase;
	bool sign;
	if (dMEM[0xFF40] & 0b00010000) {
		//unsigned data starting at $8000
		// addressBase = 0x8000;
		addressBase = 0x0000;
		sign = 0;
	}
	else {
		//signed data starting at $9000
		// addressBase = 0x9000;
		addressBase = 0x1000;
		sign = 1;
	}
	int map;
	if (dMEM[0xFF40] & 0b01000000) {
		//Tile map starts at $9C00
		map = 0x9C00 - 0x8000;
	}
	else {
		//Tile map starts at $9800
		map = 0x9800 - 0x8000;
	}
	//draw Window on Vram
	for (int x = 0; x < 160; x++) {
		static int y = dMEM[0xFF44];
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
		uint16_t pixel;
		uint8_t xBit = (7 - (X % 8));
		uint8_t yBit = ((Y % 8) * 2);
		// Atribute pulled from other vram bank
		uint8_t attr = MEM->Vram[map + (ty * 32) + tx + 0x2000];
		bool bank = (attr&0b00001000) ? 1 : 0;
		if (attr & 0b00100000) {
			//X flip
			xBit = (X % 8);
		}
		if (attr & 0b01000000) {
			//Y flip
			yBit = ((7 - (Y % 8)) * 2);
		}
		uint8_t pal = attr&0x07;
		if (sign) {
			signed char tile = (signed)MEM->Vram[map + (ty * 32) + tx];
			pixel =
				((MEM->Vram[addressBase + (tile * 16) + yBit + (bank*0x2000)] & (0b00000001 << xBit)) >> xBit) +
				(((MEM->Vram[addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)] & (0b00000001 << xBit)) * 2) >> xBit);
		}
		else {
			unsigned char tile = (unsigned)MEM->Vram[map + (ty * 32) + tx];
			pixel =
				((MEM->Vram[addressBase + (tile * 16) + yBit + (bank*0x2000)] & (0b00000001 << xBit)) >> xBit) +
				(((MEM->Vram[addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)] & (0b00000001 << xBit)) * 2) >> xBit);
		}
		if ((x - (dMEM[0xFF4B] - 7) < 160) && (x - (dMEM[0xFF4B] - 7) >= 0) && (y - dMEM[0xFF4A] < 144) && (y - dMEM[0xFF4A] >= 0)) {	
			line[x] = pixel;
			switch (pixel)
			{
			case 0:
				pixel = MEM->BGColorPallet[pal].color0;
				break;
			case 1:
				pixel = MEM->BGColorPallet[pal].color1;
				break;
			case 2:
				pixel = MEM->BGColorPallet[pal].color2;
				break;
			case 3:
				pixel = MEM->BGColorPallet[pal].color3;
				break;
			default:
				break;
			}
			Vram[x][y] = pixel;
		}
	}
}

void gbcPPU::drawSprites() {
	bool size = 0;
	if (dMEM[0xFF40] & 0b00000100) {
		size = 1; //set sprite size to 8x16
	}
	uint8_t y = dMEM[0xFF44];
	//draw Sprites on Vram
	std::vector<int> Spritestodraw;
	for (int s = 0; s < 40; s++) {
		if(Spritestodraw.size() == 10){break;}
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
		if(size){tile &= 0xFE;}
		uint8_t flags = dMEM[(0xFE00 + (s * 4)) + 3];
		int r = y - ypos;
		uint8_t pal = flags&0x07;
		bool bank = (flags&0b00001000) ? 1 : 0;
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
			uint16_t pixel =
				((MEM->Vram[((tile) * 16) + (Y * 2) + (bank*0x2000)] & (0b00000001 << (7 - X))) >> (7 - X)) +
				(((MEM->Vram[((tile) * 16) + (Y * 2) + 1 + (bank*0x2000)] & (0b00000001 << (7 - X))) * 2) >> (7 - X));
			if (pixel != 0) { // ignore "transparent" pixels
				Y = ypos + r;
				X = xpos + bit;
				if (Y >= 0 && Y < 144 && X >= 0 && X < 160) {
					bool objPriority = ((dMEM[0xFF40]&0x01) && ((flags & 0x80) || (BGPriority[X])));
					if ((!objPriority) || (line[X] == 0)) {
						switch (pixel)
						{
						case 1:
							pixel = MEM->OBJColorPallet[pal].color1;
							break;
						case 2:
							pixel = MEM->OBJColorPallet[pal].color2;
							break;
						case 3:
							pixel = MEM->OBJColorPallet[pal].color3;
							break;
						}
						Vram[X][Y] = pixel;
					}
				}
			}
		}
	}
}
