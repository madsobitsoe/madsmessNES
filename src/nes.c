#include <stdlib.h>
#include "nes.h"
#include "memory.h"

void step(nes_state *state) {
  // Update the master clock by one
  state->master_clock += 1;
  // Only for logging right now
  ppu_step(state);
  // Log if needed
  // Step one cycle in CPU
  if (state->cpu->next_action == state->cpu->end_of_queue) {
    state->cpu->current_opcode_PC = state->cpu->registers->PC;
    state->cpu->current_opcode = read_mem(state, state->cpu->registers->PC);
    logger_log(state);
  }
  cpu_step(state);

}

nes_state* init_state() {
  // Create the state and allocate memory
  nes_state *state = malloc(sizeof(nes_state));
  state->master_clock = 0;
  cpu_state *cpu = malloc(sizeof(cpu_state));
  // Create the registers and allocate memory
  registers *regs = malloc(sizeof(registers));
  init_registers(regs);
  state->cpu = cpu;
  state->cpu->registers = regs;
  state->cpu->next_action = 0;
  state->cpu->end_of_queue = 0;
  // Set up memory (malloc)
  state->memory = malloc(2048); // 2kb ram (at least for now)
  state->running = true;
  state->ppu_cycle = 0;
  state->ppu_scanline = 0;
  state->fatal_error = false;
  return state;
}

// Free up the pointers used by a state
void destroy_state(nes_state *state) {
  free(state->cpu->registers);
  free(state->cpu);
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
  state->cpu->registers->PC = 0xFFFD;
  set_interrupt_flag(state);
  state->cpu->registers->SP = 0xFD;
  state->cpu->cpu_cycle = 7;
  state->ppu_cycle = 18;
}




void attach_rom(nes_state *state, unsigned char *rommem) {
  state->rom = rommem;
}
void print_state(nes_state *state) {
  printf("Cycle:  %lld\n", state->cpu->cpu_cycle);
  print_regs(state);
  print_cpu_status(state);
  print_stack(state);
  printf("Addr_dest: %p\tAddr_source: %p\tAddr_X: %p\n", (state->cpu->destination_reg), (state->cpu->source_reg), &(state->cpu->registers->X));
}


void ppu_step(nes_state *state) {
  for (int i = 0; i < 3; i++) {
    state->ppu_cycle++;
    if (state->ppu_cycle > 340) {
      state->ppu_cycle = 0;
      state->ppu_scanline++;
    }
  }
}
