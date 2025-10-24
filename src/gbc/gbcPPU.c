#include "gbc/gbcPPU.h"
#include "gb/gbMEM.h"
#include "globals.h"

static SDL_Renderer* renderer;
static SDL_Texture* texture;

static uint8_t BGPriority[160];
static uint8_t line[160];
static uint16_t Vram[160][144];


static uint8_t lastDMA;

// ================= Helper Functions ==============

void drawBackground() {
    int addressBase;
	bool sign;
	if (gb_read(0xFF40) & 0b00010000) {
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
	if (gb_read(0xFF40) & 0b00001000) {
		//Tile map starts at $9C00
		map = 0x9C00 - 0x8000;
	}
	else {
		//Tile map starts at $9800
		map = 0x9800 - 0x8000;
	}


	for (int x = 0; x < 160; x++) {
		int y = gb_read(0xFF44);
		int X = (x + gb_read(0xFF43))%256;
		int Y = (y + gb_read(0xFF42))%256;
		int tx = X / 8;
		int ty = Y / 8;
		uint16_t pixel;
		uint8_t xBit = (7 - (X % 8));
		uint8_t yBit = ((Y % 8) * 2);
		// Attribute pulled from other vram bank
		uint8_t attr = gb_vramRead(map + (ty * 32) + tx + 0x2000);
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
			signed char tile = (signed)gb_vramRead(map + (ty * 32) + tx);
			pixel =
				((gb_vramRead(addressBase + (tile * 16) + yBit + (bank*0x2000)) & (0b00000001 << xBit)) >> xBit) +
				(((gb_vramRead(addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)) & (0b00000001 << xBit)) * 2) >> xBit);
		}
		else {
			unsigned char tile = (unsigned)gb_vramRead(map + (ty * 32) + tx);
			pixel =
				((gb_vramRead(addressBase + (tile * 16) + yBit + (bank*0x2000)) & (0b00000001 << xBit)) >> xBit) +
				(((gb_vramRead(addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)) & (0b00000001 << xBit)) * 2) >> xBit);
		}
		BGPriority[x] = (attr&0x80);
		line[x] = pixel;
		pixel = gb_getBackColor(pal, pixel);
		Vram[x][y] = pixel;
	}
}

void drawWindow() {
    int addressBase;
	bool sign;
	if (gb_read(0xFF40) & 0b00010000) {
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
	if (gb_read(0xFF40) & 0b01000000) {
		//Tile map starts at $9C00
		map = 0x9C00 - 0x8000;
	}
	else {
		//Tile map starts at $9800
		map = 0x9800 - 0x8000;
	}
	//draw Window on Vram
	for (int x = 0; x < 160; x++) {
		int y = gb_read(0xFF44);
		int X = x - (gb_read(0xFF4B)-7);
		int Y = y - gb_read(0xFF4A);
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
		// Attribute pulled from other vram bank
		uint8_t attr = gb_vramRead(map + (ty * 32) + tx + 0x2000);
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
			signed char tile = (signed)gb_vramRead(map + (ty * 32) + tx);
			pixel =
				((gb_vramRead(addressBase + (tile * 16) + yBit + (bank*0x2000)) & (0b00000001 << xBit)) >> xBit) +
				(((gb_vramRead(addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)) & (0b00000001 << xBit)) * 2) >> xBit);
		}
		else {
			unsigned char tile = (unsigned)gb_vramRead(map + (ty * 32) + tx);
			pixel =
				((gb_vramRead(addressBase + (tile * 16) + yBit + (bank*0x2000)) & (0b00000001 << xBit)) >> xBit) +
				(((gb_vramRead(addressBase + (tile * 16) + yBit + 1 + (bank*0x2000)) & (0b00000001 << xBit)) * 2) >> xBit);
		}
		if ((x - (gb_read(0xFF4B) - 7) < 160) && (x - (gb_read(0xFF4B) - 7) >= 0) && (y - gb_read(0xFF4A) < 144) && (y - gb_read(0xFF4A) >= 0)) {	
			line[x] = pixel;
			pixel = gb_getBackColor(pal, pixel);
			Vram[x][y] = pixel;
		}
	}
}

void drawSprites() {
	bool size = 0;
	if (gb_read(0xFF40) & 0b00000100) {
		size = 1; //set sprite size to 8x16
	}
	uint8_t y = gb_read(0xFF44);
	//draw Sprites on Vram
	int sprites_to_draw[40];
	int sprite_num = 0;
	for (int s = 0; s < 40; s++) {
		if(sprite_num == 10){break;}
		int ypos = gb_read(0xFE00 + (s * 4)) - 16;
		if (size) {
			if (y>=ypos && y < ypos+16) {
				sprites_to_draw[sprite_num] = s;
				sprite_num++;
			}
		}
		else {
			if (y>=ypos && y < ypos+8) {
				sprites_to_draw[sprite_num] = s;
				sprite_num++;
			}
		}
	}
	for (int s = 0; s < sprite_num; s++) {
		int ypos = gb_read(0xFE00 + (sprites_to_draw[s] * 4)) - 16;
		int xpos = gb_read((0xFE00 + (sprites_to_draw[s] * 4)) + 1) - 8;
		uint8_t tile = gb_read((0xFE00 + (sprites_to_draw[s] * 4)) + 2);
		if(size){tile &= 0xFE;}
		uint8_t flags = gb_read((0xFE00 + (sprites_to_draw[s] * 4)) + 3);
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
				((gb_vramRead(((tile) * 16) + (Y * 2) + (bank*0x2000)) & (0b00000001 << (7 - X))) >> (7 - X)) +
				(((gb_vramRead(((tile) * 16) + (Y * 2) + 1 + (bank*0x2000)) & (0b00000001 << (7 - X))) * 2) >> (7 - X));
			if (pixel != 0) { // ignore "transparent" pixels
				Y = ypos + r;
				X = xpos + bit;
				if (Y >= 0 && Y < 144 && X >= 0 && X < 160) {
					bool objPriority = ((gb_read(0xFF40)&0x01) && ((flags & 0x80) || (BGPriority[X])));
					if ((!objPriority) || (line[X] == 0)) {
						pixel = gb_getObjColor(pal, pixel);
						Vram[X][Y] = pixel;
					}
				}
			}
		}
	}
}

// ==================== Functions ==================

void gbcPPU_init(SDL_Renderer *rend) {
    renderer = rend;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	lastDMA = gb_read(0xff55);
}

void gbcPPU_deinit(){
	SDL_DestroyTexture(texture);
}

void gbcPPU_drawLine() {

	if (gb_read(0xFF44) == 144) {
		//turn on V-blank flag
		gb_orWrite(0xFF41, 0b00000001);
		gb_andWrite(0xFF41, 0b11111101);
		gb_orWrite(0xFF0F, 0b00000001);	//V-blank interrupt
		if (gb_read(0xFF41) & 0b00010000) {
			gb_orWrite(0xFF0F, 0b00000010);
			//vblank stat interrupt
		}
	}
	
	if (gb_read(0xFF44) < 144) {
		// Save Vram before drawing
		gbMEM_saveVram();
		drawBackground();
		if (gb_read(0xFF40) & 0b00100000) {
			//Window Enable
			drawWindow();
		}
        if (gb_read(0xFF40) & 0b00000010) {
            //Sprite Enable
            drawSprites();
        }
	}
}

uint32_t gbcPPU_updatePPU(int cycles) {
	uint32_t time = 0;

	// Cycles is how many cycles are left in the line 456-0
	if(gb_read(0xFF44) < 144) {
		if(cycles > (456-80)) {
			// Mode 2: OAM Scan
			gb_orWrite(0xFF41, 0b00000010);
			gb_andWrite(0xFF41, 0b11111110);

			if (gb_read(0xFF41) & 0b00100000) {
				gb_orWrite(0xFF0F, 0b00000010);
				// OAM Scan stat interrupt
			}
		}
		else if(cycles > (456-200)) {
			// Mode 3: Drawing
			gb_orWrite(0xFF41, 0b00000011);
		}
		else {
			// Mode 0: H-Blank
			gb_andWrite(0xFF41, 0b11111100);

			if (gb_read(0xFF41) & 0b00001000) {
				gb_orWrite(0xFF0F, 0b00000010);
				//Hb-lank stat interrupt
			}
		}
	}


	if (gb_read(0xFF44) == gb_read(0xFF45)) {
		//LYC == LY
		gb_orWrite(0xFF41, 0b00000100);
		if (gb_read(0xFF41) & 0b01000000) {
			gb_orWrite(0xFF0F, 0b00000010);
		}
	}
	else {
		//LYC != LY
		gb_andWrite(0xFF41, 0b11111011);
		if (gb_read(0xFF41) & 0b01000000) {
			gb_andWrite(0xFF0F, 0b11111101);
		}
	}

	return time;
}

void gbcPPU_renderFrame() {
	// Credit to DOOMReboot on Github for the pixel pusher system! https://github.com/DOOMReboot
	// The Back Buffer texture may be stored with an extra bit of width (pitch) on the video card in order to properly
    // align it in VRAM should the width not lie on the correct memory boundary (usually four bytes).
    int32_t pitch = 0;

    // This will hold a pointer to the memory position in VRAM where our Back Buffer texture lies
    uint32_t* pixelBuffer = NULL;

    // Lock the memory in order to write our Back Buffer image to it
    if (!SDL_LockTexture(texture, NULL, (void**)&pixelBuffer, &pitch))
    {
        // The pitch of the Back Buffer texture in VRAM must be divided by four bytes
        // as it will always be a multiple of four
        pitch /= sizeof(uint32_t);

        // Draw frame to texture
        for (uint32_t x = 0; x < (SCREEN_HEIGHT); x++){
			for(uint32_t y = 0; y < (SCREEN_WIDTH); y++){
				uint16_t data =  Vram[y][x];
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
	return;
}
