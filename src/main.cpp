#define SDL_MAIN_HANDLED
#include <stdio.h>
#include "SDL.h"
#include "inputData.h"
#include "gbEmulator.h"

#define SCALE 4
//Screen dimension constants
#define SCREEN_WIDTH (160 * SCALE)
#define SCREEN_HEIGHT (144 * SCALE)

#define GAME "ROMS/Tetris.gb"
// #define GAME "ROMS/cpu_instrs.gb"
// #define GAME "ROMS/07-jr,jp,call,ret,rst.gb"

bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer){
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
	*renderer = SDL_CreateRenderer( *window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if( *renderer == NULL ) {
		printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
	}
	SDL_RenderSetScale(*renderer, SCALE, SCALE);

	return true;
}

bool closeSDL(SDL_Window* window, SDL_Renderer* renderer){
	//Destroy renderer
	if( renderer != NULL ) {
		SDL_DestroyRenderer(renderer);
	}
	//Destroy window
	if( window != NULL ) {
		SDL_DestroyWindow( window );
	}
	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	//The window we'll be rendering to
	SDL_Window* window = NULL;
	//The window renderer
	SDL_Renderer* renderer = NULL;
	//Event handler
	SDL_Event e;
	// Input from the player
	inputData input;
	resetInputData(&input);

	if(initializeSDL(&window, &renderer)){
		//main loop logic
		bool quit = false;

		//Create Emulator
		gbEmulator* Emulator = new gbEmulator(renderer);

		if(Emulator->insertCart(GAME)){

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
							case SDLK_UP:
							input.up = 1;
							break;
							case SDLK_DOWN:
							input.down = 1;
							break;
							case SDLK_LEFT:
							input.left = 1;
							break;
							case SDLK_RIGHT:
							input.right = 1;
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
						}
					}
				}
				
				Emulator->runFrame(input);

				//Update screen
				// SDL_SetRenderDrawColor(renderer, 255,0,0,255);
				// SDL_RenderDrawLine(renderer, 0, 0, rand()%100, rand()%100);
				SDL_RenderPresent( renderer );
			}
		}

		delete Emulator;
	}

	closeSDL(window, renderer);

	return 0;
}