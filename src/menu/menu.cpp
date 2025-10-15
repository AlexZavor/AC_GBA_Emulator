#include "menu/menu.h"

#include <vector>
#include "SDL_ttf.h"
#include "inputData.h"
#include "game.h"
#include "string.h"
#include "timer.h"

#include "gb/gbEmulator.h"

#define BKG_COLOR 12, 12, 32, 255
#define RED     {255,   0,   0}
#define GREEN   {  0, 255,   0}
#define BLUE    {  0,   0, 255}

static SDL_Renderer* renderer;
bool quit = false;
std::vector<game> Roms;
static inputData input;
static SDL_Event* e;
static TTF_Font* font;

unsigned int page = 0;
unsigned int selection = 0;

// Initialize menu and variables
void menu_init(SDL_Renderer* render, SDL_Event* event){
    renderer = render;
    quit = false;
	game_loadGames(&Roms);
	resetInputData(&input);
    e = event;
    font = TTF_OpenFont("../fonts/Minecraft.ttf", 16);

    page = 0;
    selection = 0;
}

// Runs the menu. Call once. Calls entire GB system from inside,
// Once returned, close application.
void menu_run(){
    while(true){ // loop forever until we quit (return)
    // --------------Input handleing--------------
    // Update input
    menu_input();
    // Quit Condition
    if(input.quit) return;
    // Move in Menu
    if (input.up && selection > 0){ 
        selection--; 
        if ((selection % 13) == 12) { page--; }
        input.up = false; // turn of so only reads once
    }
    if (input.down && selection < Roms.size()-1){ 
        selection++; 
        if ((selection % 13) == 0) { page++; }
        input.down = false; // turn of so only reads once
    }
    // Start up Consoles
    if (input.A || input.sel || input.start) {
        resetInputData(&input);
        game* Game = &Roms[selection];
        switch (Game->system) {
            // TODO: Start emulators from menu
        case GB:
            //Create Emulator
            // printf("Started GB Game\n");
            // menu_alert(ALERT_INFO,"Game boy game started!");
            gbEmulator_init(renderer, e);
            gbEmulator_insertCart(Game);
            gbEmulator_run();
            gbEmulator_deinit();
            timer_print_data();
            break;
        case GBC:
            //Create Emulator
            printf("Started GBC Game\n");
            menu_alert(ALERT_WARNING,"Game boy Color game started!");
            resetInputData(&input);
            break;
        case GBA:
            //Create Emulator
            printf("Started GBA Game\n");
            menu_alert(ALERT_ERROR,"Game boy Advance game started!");
            resetInputData(&input);
            break;
        }
        // #ifdef FPS_COUNT
        //     printf("max - %f, min - %f\n", maxTime, minTime);
        // #endif
    }
    // Delete save file
    if (input.B){
        resetInputData(&input);
        game* Game = &Roms[selection];
        if(Game->has_save){
            menu_alert(ALERT_WARNING, "Delete Save? A-yes B-no");
            if(input.A){
                game_removeSave(Game);
            }
            resetInputData(&input);
        }

    }


    // -------------Render Menu-----------------
    // Clear Screen to Background color
    SDL_SetRenderDrawColor(renderer, BKG_COLOR);
    SDL_RenderClear(renderer);

    // Draw current menu screen
    unsigned int y = 5;
    for (unsigned int i = 0; i < 13;i++) { // all menu options
        if (i + (page * 13) < Roms.size()) { // Load in roms on the page
            SDL_Color color;
            switch (Roms[i + (page * 13)].system) {
            case GB:
                color = RED;
                break;
            case GBC:
                color = GREEN;
                break;
            case GBA:
                color = BLUE;
                break;
            }

            // as TTF_RenderText_Solid could only be used on
            // SDL_Surface then you have to create the surface first
            SDL_Surface* surfaceMessage =
                TTF_RenderText_Solid(font, Roms[i + (page * 13)].name.c_str(), color); 

            // now you can convert it into a texture
            SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

            SDL_Rect Message_rect; //create a rect
            Message_rect.x = 10;  //controls the rect's x coordinate 
            Message_rect.y = y+2; // controls the rect's y coordinate
            Message_rect.w = Roms[i + (page * 13)].name.length()*5; // controls the width of the rect
            Message_rect.h = 8; // controls the height of the rect
            SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

            // Don't forget to free your surface and texture
            SDL_FreeSurface(surfaceMessage);
            SDL_DestroyTexture(Message);

            if(Roms[i + (page * 13)].has_save){
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_Point save_lines[10];
                save_lines[0] = {142,(int)(y)};
                save_lines[1] = {148,(int)(y)};
                save_lines[2] = {150,(int)(y+2)};
                save_lines[3] = {150,(int)(y+8)};
                save_lines[4] = {144,(int)(y+8)};
                save_lines[5] = {144,(int)(y+5)};
                save_lines[6] = {148,(int)(y+5)};
                save_lines[7] = {148,(int)(y+8)};
                save_lines[8] = {142,(int)(y+8)};
                save_lines[9] = {142,(int)(y)};
                SDL_RenderDrawLines(renderer, save_lines, 10);
            }

        }
        y += 10;
    }

    // Draw Cursor
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, 2, ((selection%13) * 10) + 8, 2, ((selection%13) * 10) + 11);
    SDL_RenderDrawPoint(renderer, 4, ((selection%13) * 10) + 10);

    // Other Menu draw items here
    //----------------------------

    // Draw to screen
    SDL_RenderPresent( renderer );
    SDL_Delay(15); // wait 15ms each time. just relax, it's not that serious
    }
}

// Read input related to the Menu.
void menu_input(){
    readInput(&input, e);
}

// Draw Alert on top of screen, used for displaying errors and stuff.
void menu_alert(AlertLevel level, const char* text){
    // Draw Box for text
    SDL_Rect r = {0,(SCREEN_HEIGHT)-24,SCREEN_WIDTH,24};
    SDL_SetRenderDrawColor(renderer, 0xAA, 0x88, 0xAA, 0xFF);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 0x66, 0x44, 0x66, 0xFF);
    SDL_RenderDrawRect(renderer, &r);

    // Set Color from Warning
    SDL_Color color;
    switch (level){
    case ALERT_INFO:
        color = {0xFF, 0xFF, 0xFF};
    break;
    case ALERT_WARNING:
        color = {0x88, 0x44, 0x00};
    break;
    case ALERT_ERROR:
        color = {0x88, 0x00, 0x00};
    break;
    default:
        
        break;
    }

    // Draw Text
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text, color); 
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    r = {10,128,(int)strlen(text)*5,8};
    SDL_RenderCopy(renderer, Message, NULL, &r);

    // Clean up
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);

    SDL_RenderPresent(renderer);

    // Wait for input
    resetInputData(&input);
    while(!(input.A || input.B || input.sel || input.start)){
        SDL_Delay(10);
        menu_input();
        if(input.quit){
            return;
        }
    }
    // resetInputData(&input);

}


