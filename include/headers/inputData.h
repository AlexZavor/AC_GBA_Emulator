#ifndef INPUTDATA_H
#define INPUTDATA_H

#include <stdbool.h>
#include <stdio.h>

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
} inputData;

void printInputData(inputData data);

void resetInputData(inputData *data);

#endif // INPUTDATA_H