#ifndef PPU_H
#define PPU_H
#include "definitions.h"

void incr_addr_reg(nes_state *state);
uint8_t read_status_reg(nes_state *state);
uint8_t read_data_reg(nes_state *state);
uint8_t read_oam_data_reg(nes_state *state);
#endif
