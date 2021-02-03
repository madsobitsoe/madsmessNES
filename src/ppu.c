#include <stdlib.h>

#include "ppu.h"
#include "memory.h"

void incr_addr_reg(nes_state *state) {
  // If bit 2 is set, add 32
  // if not set, add 1
  if (state->ppu->registers->ppu_ctrl & 0x4) {
    state->ppu->internal_addr_reg += 32;
  }
  else {
    state->ppu->internal_addr_reg += 1;
  }
}
// See: https://wiki.nesdev.com/w/index.php/PPU_registers#Data_.28.242007.29_.3C.3E_read.2Fwrite
uint8_t read_status_reg(nes_state *state) {
  uint8_t return_val = state->ppu->registers->ppu_status;
  // The 4 LSB of status-reg are unused. A read will return the 4 LSB of the latch.
  return_val &= 0xf0;
  return_val |= (state->ppu->address_latch & 0xf);;
  // Clear bit 7 of status reg
  state->ppu->registers->ppu_status &= 0x7f;
  // And clear the "internal buffer"
  state->ppu->address_latch = 0;
  return return_val;
}

uint8_t read_oam_data_reg(nes_state *state) {
  // Not at all sure this is correct.

  uint8_t return_val = state->ppu->oam_memory[state->ppu->registers->oam_addr];
  state->ppu->address_latch = return_val;
  return return_val;

}
uint8_t read_data_reg(nes_state *state) {
  // Save the "old latch" if needed
  uint16_t addr = state->ppu->internal_addr_reg;
  uint8_t return_val;
  // "normal" access to vram
  if (0x0 <= addr && addr < 0x3f00) {
    // Return the "old" value
    return_val = state->ppu->address_latch;
    // Update the internal buffer
    state->ppu->address_latch = read_mem_ppu(state, addr);
  }
  // Palette data, immediate access. (mirroring handled in read_mem_ppu)
  else if (0x3f00 <= addr && addr <= 0x3fff)
    {
      state->ppu->address_latch = read_mem_ppu(state, addr);
      return_val = state->ppu->address_latch;

    }
  // Invalid read
  else {
    return_val = 0;
    state->fatal_error = true;
    state->running = false;
  }
  incr_addr_reg(state);
  return return_val;
}
