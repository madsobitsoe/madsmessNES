#include <stdlib.h>
#include "nes.h"

void step(nes_state *state) {
  // Update the master clock by one
  state->master_clock += 1;
  // Only for logging right now
  ppu_step(state);
  // Log if needed
  // Step one cycle in CPU
  if (state->stall_cycles == 0) {
    state->current_opcode_PC = state->registers->PC;
    state->current_opcode = read_mem_byte(state, state->registers->PC);
    logger_log(state);
  }
  cpu_step(state);

}

nes_state* init_state() {
  // Create the state and allocate memory
  nes_state *state = malloc(sizeof(nes_state));
  state->master_clock = 0;
  // Create the registers and allocate memory
  registers *regs = malloc(sizeof(registers));
  init_registers(regs);
  state->registers = regs;
  // Set up memory (malloc)
  state->memory = malloc(2048); // 2kb ram (at least for now)
  state->running = true;
  state->stall_cycles = 0;
  state->ppu_cycle = 0;
  state->ppu_frame = 0;
  return state;
}

// Free up the pointers used by a state
void destroy_state(nes_state *state) {
  free(state->registers);
  free(state->memory);
  free(state);
}


/* On power on, the RST pin is asserted. RESET consists of the following CPU cycles: */

/*   1. [READ] read (PC) */
/*   2. [READ] read (PC) */
/*   3. [READ] read (S), decrement S */
/*   4. [READ] read (S), decrement S */
/*   5. [READ] read (S), decrement S */
/*   6. [WRITE] PC_low = ($FFFC), set interrupt flag */
/*   7. [WRITE] PC_high = ($FFFD) */

/*   S is actually initialized to $00. But, those 3 decrements cause it to end up at $FD. */


void power_on(nes_state *state) {
  // For now, do nothing except for calling reset
  reset(state);
}

void reset(nes_state *state) {
  // Just fake it for now.
  state->registers->PC = 0xFFFD;
  set_interrupt_flag(state);
  state->registers->SP = 0xFD;
  state->master_clock = 6;
  /* state->current_opcode_PC = 0xFFFD; */
  /* state->current_opcode = read_mem_byte(state, state->registers->PC); */
  state->ppu_cycle = 18;
}




void attach_rom(nes_state *state, unsigned char *rommem) {
  state->rom = rommem;
}
void print_state(nes_state *state) {
  printf("Cycle:  %lld\n", state->master_clock);
  printf("Stall cycles: %d\n", state->stall_cycles);
  print_regs(state);
  print_stack(state);
}
