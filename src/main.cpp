#include <stdio.h>
#include "globals.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "menu/menu.h"

/*
Menu

GB
	PPU
	MEM
		lots of crashing
	CPU
GBC
	PPU
		Priority
	Audio!
GBA
*/

// TODO: Reorganize system from C++ to C based (init and update func.)
// TODO: GB sub-systems refactor (cpu macros, better mem unit)
// TODO: input base functions for doing some reading easier than big copy paste



bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer){
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}
	//Create window
	*window = SDL_CreateWindow( "--AC-GBA--", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE, SDL_WINDOW_SHOWN );
	if( *window == NULL ) {
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}
	//Create v-synced renderer for window
	*renderer = SDL_CreateRenderer( *window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/ );
	if( *renderer == NULL ) {
		printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
	}

	SDL_RenderSetScale(*renderer, SCALE, SCALE);

	TTF_Init();

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
	TTF_Quit();
	SDL_Quit();
	return true;
}

int main(int argc, char* argv[]) {
	// The window we'll be rendering to
	SDL_Window* window = NULL;
	// The window renderer
	SDL_Renderer* renderer = NULL;
	// Event handler
	SDL_Event e;

	if(initializeSDL(&window, &renderer)){
		// Menu system
		menu_init(renderer, &e);
		menu_run();
	}

	closeSDL(window, renderer);

	return 0;
}
