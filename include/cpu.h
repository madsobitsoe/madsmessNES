#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include "definitions.h"
/* #include "nes.h" */

void init_registers(registers *registers);
void print_regs(nes_state *state);
void cpu_step(nes_state *state);
void ppu_step(nes_state *state);
nes_state* init_state();
void set_pc(nes_state *state, unsigned short pc);
uint8_t read_mem_byte(nes_state *state, unsigned short memloc);
uint16_t read_mem_short(nes_state *state, unsigned short memloc);
uint16_t translate_memory_location(unsigned short memloc);

#endif
