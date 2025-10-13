#include "inputData.h"

void printInputData(inputData data){
    printf("%d %d %d %d | %d %d | %d %d \n", 
    data.up, data.down, data.left, data.right, data.sel, data.start, data.A, data.B);
}

void resetInputData(inputData *data)
{
    *data = {0,0,0,0,0,0, 0,0, 0,0, 0};
}