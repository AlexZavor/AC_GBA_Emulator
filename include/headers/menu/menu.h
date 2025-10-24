#ifndef MENU_H
#define MENU_H
#include "SDL.h"

typedef enum {
    ALERT_NONE,
    ALERT_INFO,
    ALERT_WARNING,
    ALERT_ERROR
} AlertLevel_t;

// Initialize menu and variables
void menu_init(SDL_Renderer* render, SDL_Event* event);

// Runs the menu. Call once. Calls entire GB system from inside,
// Once returned, close application.
void menu_run();

// Read input related to the Menu.
void menu_input();

// Draw Alert on top of screen, used for displaying errors and stuff.
void menu_alert(AlertLevel_t level, const char* text);


#endif // MENU_H