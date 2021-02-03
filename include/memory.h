#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include "definitions.h"
#include "cpu.h"
#include "ppu.h"

// Functions for memory read/write in the nes
uint8_t read_mem(nes_state *state, uint16_t memloc);
void write_mem(nes_state *state, uint16_t memloc, uint8_t value);
uint8_t read_mem_ppu(nes_state *state, uint16_t memloc);
void write_mem_ppu(nes_state *state, uint16_t memloc, uint8_t value);

#endif
