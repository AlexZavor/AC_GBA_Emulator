#ifdef DEBUG
	// #define FPS_COUNT
#endif

#include <stdio.h>
#include <vector>
#include "tinydir.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "inputData.h"
#include "gbEmulator.h"

/*
GB
	PPU
		work on update function
		figure out flickering
		yl=ylc?
	MEM
		clover requests rom bank 135??
		lots of crashing
	CPU
GBC
GBA
*/

#define GAME_DIR "ROMS/"
#define SAVE_DIR "SAVES/"

#ifdef FPS_COUNT
	float minTime = 200;
	float maxTime = 0;
#endif

bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture){
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}
	//Create window
	*window = SDL_CreateWindow( "--AC-GBA--", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	if( *window == NULL ) {
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}
	//Create vsynced renderer for window
	*renderer = SDL_CreateRenderer( *window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/ );
	if( *renderer == NULL ) {
		printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
	}
	//Create texture to render pixels to.
	*texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if( *texture == NULL ) {
		printf( "Texture could not be created! SDL Error: %s\n", SDL_GetError() );
	}

	SDL_RenderSetScale(*renderer, SCALE, SCALE);

	TTF_Init();

	return true;
}

bool closeSDL(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture){
	//Destroy window
	if( texture != NULL ) {
		SDL_DestroyTexture( texture );
	}
	//Destroy renderer
	if( renderer != NULL ) {
		SDL_DestroyRenderer(renderer);
	}
	//Destroy window
	if( window != NULL ) {
		SDL_DestroyWindow( window );
	}
	//Quit SDL subsystems
	TTF_Quit();
	SDL_Quit();
	return true;
}

void loadGames(std::vector<std::string>* Roms){
	tinydir_dir dir;
    tinydir_open(&dir, "./ROMS/");

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if((file.name)[0] != '.'){
			uint8_t i = 0;
			while (file.name[i] != '\0')
			{
				i++;
			}
			if(file.name[i-1] == 'b'){
				Roms->push_back(file.name);
			}
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);

}

int main(int argc, char* argv[]) {
	// The window we'll be rendering to
	SDL_Window* window = NULL;
	// The window renderer
	SDL_Renderer* renderer = NULL;
	// The texture to render to
    SDL_Texture* texture = NULL;
	// Event handler
	SDL_Event e;
	// Input from the player
	inputData input;
	resetInputData(&input);
	// List of games
	std::vector<std::string> Roms;
	loadGames(&Roms);

	if(initializeSDL(&window, &renderer, &texture)){
		// Menu system
		std::string Game;
		unsigned int page = 0;
		unsigned int selection = 0;
		bool menu = true;
		
		// Emulator
		gbEmulator* Emulator;

		// Main loop logic
		bool quit = false;

		while(!quit){
			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 ) {
				//User requests quit
				if( e.type == SDL_QUIT ) {
					quit = true;
				}
				//User presses a key
				else if( e.type == SDL_KEYDOWN ) {
					//Select surfaces based on key press
					switch( e.key.keysym.sym ) {
						case KEY_UP:
						input.up = 1;
						if (selection > 0 && menu)	{ 
							selection--; 
							if ((selection % 13) == 12 && menu) { page--; }
						}
						break;
						case KEY_DOWN:
						input.down = 1;
						if (selection < Roms.size()-1 && menu) { 
							selection++; 
							if ((selection % 13) == 0 && menu) { page++; }
						}
						break;
						case KEY_LEFT:
						input.left = 1;
						break;
						case KEY_RIGHT:
						input.right = 1;
						break;
						case KEY_A:
						input.A = 1;
						break;
						case KEY_B:
						input.B = 1;
						break;
						case KEY_SEL:
						input.sel = 1;
						break;
						case KEY_START:
						input.start = 1;
						break;
						case KEY_MENU:
						menu = true;
						#ifdef FPS_COUNT
							printf("max - %f, min - %f\n", maxTime, minTime);
						#endif
						if(Emulator != nullptr){
							delete Emulator;
						}
						break;
					}
				}
				else if( e.type == SDL_KEYUP ) {
					//Select surfaces based on key press
					switch( e.key.keysym.sym ) {
						case SDLK_UP:
						input.up = 0;
						break;
						case SDLK_DOWN:
						input.down = 0;
						break;
						case SDLK_LEFT:
						input.left = 0;
						break;
						case SDLK_RIGHT:
						input.right = 0;
						break;
						case KEY_A:
						input.A = 0;
						break;
						case KEY_B:
						input.B = 0;
						break;
						case KEY_SEL:
						input.sel = 0;
						break;
						case KEY_START:
						input.start = 0;
						break;
					}
				}
			}
			
			if(menu){
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				static TTF_Font* Sans = TTF_OpenFont("../fonts/Minecraft.ttf", 16);
				unsigned int y = 5;
				for (unsigned int i = 0; i < 13;i++) {
					if (i + (page * 13) < Roms.size()) {
						SDL_Color White = {255, 255, 255};

						// as TTF_RenderText_Solid could only be used on
						// SDL_Surface then you have to create the surface first
						SDL_Surface* surfaceMessage =
							TTF_RenderText_Solid(Sans, Roms[i + (page * 13)].c_str(), White); 

						// now you can convert it into a texture
						SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

						SDL_Rect Message_rect; //create a rect
						Message_rect.x = 10;  //controls the rect's x coordinate 
						Message_rect.y = y+2; // controls the rect's y coordinte
						Message_rect.w = Roms[i + (page * 13)].length()*5; // controls the width of the rect
						Message_rect.h = 8; // controls the height of the rect
						SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

						// Don't forget to free your surface and texture
						SDL_FreeSurface(surfaceMessage);
						SDL_DestroyTexture(Message);

					}
					y += 10;
				}

				SDL_RenderDrawLine(renderer, 2, ((selection%13) * 10) + 8, 2, ((selection % 13) * 10) + 11);
				SDL_RenderDrawPoint(renderer, 4, ((selection % 13) * 10) + 10);
				// FillTri(2, ((selection%13) * 10) + 7, 2, ((selection % 13) * 10) + 11, 4, ((selection % 13) * 10) + 9);

				if (input.A || input.sel || input.start) {
					Game = GAME_DIR + Roms[selection];
					//Create Emulator
					Emulator = new gbEmulator(renderer, texture);
					//Insert cartrage, if succsess, leave menu
					if(Emulator->insertCart(Game)){
						menu = false;
					}
				}
				SDL_RenderPresent( renderer );
			}else{

				Uint64 start = SDL_GetPerformanceCounter();

				//Run Emulator for one frame
				Emulator->runFrame(input);
				//Update screen
				SDL_RenderPresent( renderer );

				Uint64 end = SDL_GetPerformanceCounter();
				float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
				// Cap to 60 FPS
				if(16.666f - elapsedMS > 0){
					SDL_Delay((Uint32)(16.666f - elapsedMS));
				}

				#ifdef FPS_COUNT
					if(elapsedMS > 20){
						printf("%f\n",(elapsedMS));
					}
					if(elapsedMS > maxTime){maxTime = elapsedMS;}
					if(elapsedMS < minTime){minTime = elapsedMS;}
				#endif
			}
		}

		delete Emulator;
	}

	closeSDL(window, renderer, texture);

	return 0;
}