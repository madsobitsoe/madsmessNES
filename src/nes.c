#include <stdlib.h>
#include "nes.h"
#include "memory.h"
#include "rom_loader.h"
#include "ppu.h"

void step(nes_state *state) {
  // Update the master clock by one
  state->master_clock += 1;
  // ppu has 3 cycles for every cpu cycle
  for (int i = 0; i < 3; i++) {
    ppu_step(state);
  }
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
  state->fatal_error = false;
  // PPU init
  ppu_state *ppu = malloc(sizeof(ppu_state));
  ppu_registers *ppu_regs = malloc(sizeof(ppu_registers));
  ppu->registers = ppu_regs;
  ppu->ppu_vram = malloc(0x800); // 2kb vram
  ppu->chr_rom = malloc(0x2000); //8kb for chr_rom
  ppu->oam_memory = malloc(0x100); // 256 bytes OAM memory
  state->ppu = ppu;
  state->ppu->ppu_cycle = 0;
  state->ppu->ppu_scanline = 0;
  state->ppu->ppu_frame = 0;
  state->ppu->address_latch = 0;
  state->ppu->high_pointer = true;
  state->ppu->internal_addr_reg = 0;
  return state;
}

// Free up the pointers used by a state
void destroy_state(nes_state *state) {
  free(state->cpu->registers);
  free(state->cpu);
  free(state->memory);
  free(state->ppu->registers);
  free(state->ppu->ppu_vram);
  /* free(state->ppu->chr_rom); */
  free(state->ppu->oam_memory);
  free(state->ppu);
  free_rom(state->rom);
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
  uint8_t pc_low = read_mem(state, 0xFFFC);
  uint8_t pc_high = read_mem(state, 0xFFFD);
  uint16_t pc_addr = ((uint16_t) pc_low) | (((uint16_t) pc_high) << 8);
  // Just fake it for now.
  state->cpu->registers->PC = pc_addr;
  set_interrupt_flag(state);
  state->cpu->registers->SP = 0xFD;
  state->cpu->cpu_cycle = 7;
  state->ppu->ppu_cycle = 18;
  state->ppu->ppu_scanline = 0;
  state->cpu->current_opcode_PC = pc_addr;
  state->cpu->current_opcode = read_mem(state, pc_addr);
}



void attach_rom(nes_state *state, nes_rom *rom) {
  state->rom = rom;
}

void print_state(nes_state *state) {
  printf("Cycle:  %ld\n", state->cpu->cpu_cycle);
  print_regs(state);
  print_cpu_status(state);
  print_stack(state);
  printf("Addr_dest: %p\tAddr_source: %p\tAddr_X: %p\n", (state->cpu->destination_reg), (state->cpu->source_reg), &(state->cpu->registers->X));
}
