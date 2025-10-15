#ifndef GLOBALS
#define GLOBALS

typedef enum{
    GB,
    GBC,
    GBA
} GB_sys;

// Graphics
#define SCALE 4
//Screen dimension constants
#define SCREEN_WIDTH (160)
#define SCREEN_HEIGHT (144)
#define GREEN_PALLET

// Directorys
#define GAME_DIR "./ROMS/"
#define SAVE_DIR "./SAVES/"

// Audio
#define VOLUME 7000

#define AUDIO_FREQ 44100
#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLES 1024

#endif /*GLOBALS*/