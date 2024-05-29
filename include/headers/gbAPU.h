#ifndef GBAPU_H
#define GBAPU_H

#include "globals.h"

#define PLAYING_FREQ 440

void gbAudioCallback(Sint16* samples, int sample_count){
    static int count = (AUDIO_FREQ/PLAYING_FREQ)/2;
    static bool squareState = 0;
	
	for (int i = 0; i < sample_count; ++i)
	{
		samples[i] = VOLUME*squareState;
		count--;
		if(count == 0){
			count = (AUDIO_FREQ/PLAYING_FREQ)/2;
            squareState = !squareState;
		}
	}
}

#endif //GBAPU_H