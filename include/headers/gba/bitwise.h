#ifndef BITWISE_H
#define BITWISE_H
#include <stdint.h>
#include <stdbool.h>

// Swaps 2 item (32 bit)
void swp(void* d1, void* d2);

// Get/Set bit function
void set_bit(uint32_t* data, uint8_t bit, bool set);
bool get_bit(uint32_t data, uint8_t bit);

// Sign Extend data 
int32_t sign_extend(uint32_t data, uint8_t original_bit_width);
int64_t sign_extend64(uint64_t data, uint8_t original_bit_width);

// returns the number of 1's in the number
int high_count(uint32_t data);

#endif