#include "inputData.h"

void printInputData(inputData data){
    printf("%d %d %d %d | %d %d | %d %d \n", 
    data.up, data.down, data.left, data.right, data.sel, data.start, data.A, data.B);
}

void resetInputData(inputData *data)
{
    memset(data, 0, sizeof(inputData));
}

void readInput(inputData *data, SDL_Event* e){
        //Handle events on queue for menu
    while( SDL_PollEvent( e ) != 0 ) {
        //User requests quit (alt+f4 or X button)
        if( e->type == QUIT) {
            data->quit = true;
        }
        //User presses a key
        else if( e->type == SDL_KEYDOWN ) {
            switch( e->key.keysym.sym ) {
                case KEY_UP:
                data->up = 1;
                break;
                case KEY_DOWN:
                data->down = 1;
                break;
                case KEY_LEFT:
                data->left = 1;
                break;
                case KEY_RIGHT:
                data->right = 1;
                break;
                case KEY_A:
                data->A = 1;
                break;
                case KEY_B:
                data->B = 1;
                break;
                case KEY_SEL:
                data->sel = 1;
                break;
                case KEY_START:
                data->start = 1;
                break;
                case KEY_RB:
                data->RB = 1;
                break;
                case KEY_LB:
                data->LB = 1;
                break;
                case KEY_MENU:
                data->quit = 1;
                break;
                default:
                break;
            }
        }
        // User Releases key
        else if( e->type == SDL_KEYUP ) {
            switch( e->key.keysym.sym ) {
                case SDLK_UP:
                data->up = 0;
                break;
                case SDLK_DOWN:
                data->down = 0;
                break;
                case SDLK_LEFT:
                data->left = 0;
                break;
                case SDLK_RIGHT:
                data->right = 0;
                break;
                case KEY_A:
                data->A = 0;
                break;
                case KEY_B:
                data->B = 0;
                break;
                case KEY_SEL:
                data->sel = 0;
                break;
                case KEY_START:
                data->start = 0;
                break;
                case KEY_RB:
                data->RB = 0;
                break;
                case KEY_LB:
                data->LB = 0;
                break;
                case KEY_MENU:
                data->quit = 0;
                break;
                default:
                break;
            }
        }
    }
}