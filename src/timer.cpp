#include "timer.h"

#include "SDL.h"
#include "stdio.h"


#ifdef DEBUG
#define FPS_COUNT
#endif

#ifdef FPS_COUNT
    #define AVG_BUFFER_SIZE 60
	static float minTime = 999;
	static float maxTime = 0;
    static float avg_buffer[AVG_BUFFER_SIZE];
    static int buff_ptr = 0;
#endif

static Uint64 start;
static Uint64 end;

void timer_start(){
    start = SDL_GetPerformanceCounter();
}

void timer_end(){
    end = SDL_GetPerformanceCounter();
}

void timer_buff(){
    // 60FPS
	float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    if(16.666f - elapsedMS > 0){
        // Delay until about 16.6ms. not the most accurate but you know
        SDL_Delay((Uint32)(16.666f - elapsedMS));
    }
    #ifdef FPS_COUNT
        // if(elapsedMS > 20){
        //     printf("%f\n",(elapsedMS));
        // }
        if(elapsedMS > maxTime){maxTime = elapsedMS;}
        if(elapsedMS < minTime){minTime = elapsedMS;}
        avg_buffer[(buff_ptr++)%AVG_BUFFER_SIZE] = elapsedMS;
    #endif
}

void timer_print_data(){
    #ifdef FPS_COUNT
    float avgTime = 0;
    for(auto i : avg_buffer){avgTime += i;}
    avgTime /= AVG_BUFFER_SIZE;
    printf("___\n\tmin - %.3f\n\tmax - %.3f\n\tavg - %.3f\n~~~\n", minTime, maxTime, avgTime);
    minTime = 999;
    maxTime = 0;
    SDL_memset(avg_buffer, 0, AVG_BUFFER_SIZE*sizeof(float));
    #endif
}