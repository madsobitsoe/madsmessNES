#ifndef NES_H
#define NES_H

#include <stdio.h>

#include "definitions.h"
#include "cpu.h"
#include "logger.h"

nes_state* init_state();
void destroy_state(nes_state *state);
void power_on(nes_state *state);
void reset(nes_state *state);
void step(nes_state *state);
void ppu_step(nes_state *state);
void print_state(nes_state *state);
void attach_rom(nes_state *state, unsigned char *rommem);

#endif
