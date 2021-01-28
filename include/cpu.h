#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include "definitions.h"
/* #include "nes.h" */

void init_registers(registers *registers);
void print_regs(nes_state *state);
void print_stack(nes_state *state);
void cpu_step(nes_state *state);
void ppu_step(nes_state *state);
nes_state* init_state();
void set_pc(nes_state *state, unsigned short pc);
uint8_t read_mem_byte(nes_state *state, unsigned short memloc);
uint16_t read_mem_short(nes_state *state, unsigned short memloc);
uint16_t translate_memory_location(unsigned short memloc);

void set_negative_flag(nes_state *state);
void set_overflow_flag(nes_state *state);
void set_break_flag(nes_state *state);
void set_decimal_flag(nes_state *state);
void set_interrupt_flag(nes_state *state);
void set_zero_flag(nes_state *state);
void set_carry_flag(nes_state *state);
bool is_carry_flag_set(nes_state *state);
bool is_zero_flag_set(nes_state *state);
bool is_interrupt_flag_set(nes_state *state);
bool is_decimal_flag_set(nes_state *state);
bool is_break_flag_set(nes_state *state);
bool is_overflow_flag_set(nes_state *state);
bool is_negative_flag_set(nes_state *state);



#endif
