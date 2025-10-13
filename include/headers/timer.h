#ifndef TIMER_H
#define TIMER_H

// Starts Timer for FPS and debugging
void timer_start();
// Ends timer for FPS and debugging
void timer_end();
// Pads time from timer to a clean 60 FPS
void timer_buff();
// Prints data (and clears data) for debugging
void timer_print_data();

#endif //TIMER_H