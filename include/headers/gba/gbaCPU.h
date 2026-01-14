#ifndef GBACPU_H
#define GBACPU_H

#include "stdint.h"

void gbaCPU_init();
void gbaCPU_deinit();

// Returns number of cycles taken to execute
int8_t gbaCPU_instruction();

void gbaCPU_print_cycle();

// Debug functions for getting register data
uint32_t get_reg(int r);
uint32_t get_cpsr();

#endif /* GBACPU_H */