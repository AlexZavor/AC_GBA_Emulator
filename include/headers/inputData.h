#ifndef INPUTDATA_H
#define INPUTDATA_H

#include <stdbool.h>
#include <stdio.h>

//Key definitions
#define KEY_UP		SDLK_UP
#define KEY_DOWN	SDLK_DOWN
#define KEY_LEFT	SDLK_LEFT
#define KEY_RIGHT	SDLK_RIGHT
#define KEY_A		SDLK_z
#define KEY_B		SDLK_x
#define KEY_SEL		SDLK_a
#define KEY_START	SDLK_s
#define KEY_RB		SDLK_w
#define KEY_LB		SDLK_q
#define KEY_MENU	SDLK_ESCAPE
#define KEY_MUTE    SDLK_m
#define QUIT        SDL_QUIT    

typedef struct
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool A;
    bool B;

    bool sel;
    bool start;

    bool RB;
    bool LB;

    bool quit;
} inputData;

// Prints out current input data in struct
void printInputData(inputData data);

// sets all input data to 0
void resetInputData(inputData *data);

#endif // INPUTDATA_H