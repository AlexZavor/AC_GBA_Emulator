#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <stdbool.h>
#include <stdio.h>
#include "SDL.h"
#include "globals.h"
#include "gbAPU.h"
#include <time.h>
#include <math.h>

GB_sys audioSystem;
gbAPU* gbapu = new gbAPU();
bool isPaused = 0;

void emptyAudio(Sint16* samples, int sample_count){
	for (int i = 0; i < sample_count; ++i)
	{
		samples[i] = 0;
	}
}

void runAudio(void* userdata, Uint8* stream, int length)
{
	static Sint16* samples = (Sint16*) stream;
	int sample_count = length / sizeof(Sint16);
    switch (audioSystem) {
    case GB:
    case GBC:
        gbapu->gbAudioCallback(samples, sample_count);
        break;
    case GBA:
        emptyAudio(samples, sample_count);
        // TODO: add GBA audio here!
        break;
    default:
        #ifdef DEBUG
        printf("Well, That's not supposed to happen, audio handler bad system.\n");
        #endif
        break;
    }
}

SDL_AudioDeviceID initAudio(GB_sys audioSys){
    audioSystem = audioSys;
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec = {0};
    spec.freq = AUDIO_FREQ;
    spec.format = AUDIO_FORMAT;
    spec.channels = AUDIO_CHANNELS;
    spec.samples = AUDIO_SAMPLES;
    spec.callback = runAudio;
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    SDL_PauseAudioDevice(device_id, 0); // start audio
    isPaused = false;
    return device_id;
}

void deleteAudio(SDL_AudioDeviceID device_id){
    if(device_id > 1){
        SDL_CloseAudioDevice(device_id);
    }
}

void startAudio(SDL_AudioDeviceID device_id){
    SDL_PauseAudioDevice(device_id, 0); // start audio
    isPaused = false;
}

void stopAudio(SDL_AudioDeviceID device_id){
    SDL_PauseAudioDevice(device_id, 1); // pause audio
    isPaused = true;
}

void toggleAudio(SDL_AudioDeviceID device_id){
    if(isPaused){
        startAudio(device_id);
    }else{
        stopAudio(device_id);
    }
}

#endif // AUDIOHANDLER_H