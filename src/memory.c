#include <stdio.h>
#include "memory.h"

uint8_t read_mem(nes_state *state, uint16_t memloc) {

  /*   8000-FFFF is the main area the cartridge ROM is mapped to in memory. Sometimes it can be bank switched, usually in 32k, 16k, or 8k sized banks. */
  if (memloc >= 0x8000) {
    /* return state->rom[memloc - 0xC000]; */
    uint16_t translated = memloc - 0x8000;
    /* printf("Memloc: %04X, translated: %04X\n", memloc, translated); */
    if (translated < 0x4000) {
      return state->rom->prg_rom1[translated];
    }
    else {
      translated -= 0x4000;
      return state->rom->prg_rom2[translated];
    }
  }
  /*   0000-07FF is RAM*/
  /* 0800-1FFF are mirrors of RAM (you AND the address with 07FF to get the effective address)    */
  if (memloc < 0x1fff) {
    return state->memory[memloc & 0x7FF];
  }
  /*   2000-2007 is how the CPU writes to the PPU, 2008-3FFF are mirrors of that address range. */
  if (memloc >= 0x2000 && memloc <= 0x3FFF) {
    printf("PPU Reg read! reg: %04X\n", memloc);
    uint16_t translated = memloc & 0x2007;
    switch (translated) {
      // write-only, return the latch
    case 0x2000:
    case 0x2001:
    case 0x2003:
    case 0x2005:
    case 0x2006:
      return state->ppu->address_latch;
      break;
    case 0x2002:
      return read_status_reg(state);
      break;
    case 0x2004:
      return read_oam_data_reg(state);
      break;
    case 0x2007:
      // When reading, the "address_latch"/internal buffer should be returned
      // Whether the internal buffer is updated, depends on where we read from
      // some parts of PPU memory, e.g. the palettes, are "directly on the bus",
      // while others require a "dummy read" to fill the buffer first
      // Reading 0x2007 also increments 0x2006 based on bit 2 of 0x2000
      // See https://wiki.nesdev.com/w/index.php/PPU_registers#Data_.28.242007.29_.3C.3E_read.2Fwrite
      return read_data_reg(state);
      break;
    }
  }
  /*   4000-401F is for IO ports and sound */
  if (memloc >= 0x4000 && memloc <= 0x401F) {
    // TODO - implement reading APU IO
    switch(memloc) {
    case 0x4014:
      return state->ppu->registers->oam_dma;
      break;
    }
  }

  /*   4020-4FFF is rarely used, but can be used by some cartridges */
  /*   5000-5FFF is rarely used, but can be used by some cartridges, often as bank switching registers, not actual memory, but some cartridges put RAM there */
  /*   6000-7FFF is often cartridge WRAM. Since emulators usually emulate this whether it actually exists in the cartridge or not, there's a little bit of controversy about NES headers not adequately representing a cartridge. */


  /*   The NES header takes up 16 bytes, after that is the PRG pages, then after that is the CHR pages. You look at the header to see how big the PRG and CHR of the cartridge are, see documentation for more details. The NES header does not exist outside of .NES files, you won't find it on any NES cartridges. */

  /*   So you load a Mapper 0 (NROM) cartridge into memory, and the first two PRG banks appear in NES memory at 8000-FFFF. If there is only one 16k bank, then it is mirrored at 8000-BFFF and C000-FFFF. */

  /*   When the CPU boots up, it reads the Reset vector, located at FFFC. That contains a 16-bit value which tells the CPU where to jump to. */
  /*   The first thing a game will do when it starts up is repeatedly read PPU register 2002 to wait for the NES to warm up, so you won't see a game doing anything until you throw in some rudimentary PPU emulation. */
  /*   Then the game clears the RAM, and waits for the NES to warm up some more. Then the system is ready, and the game will start running. */

  // If this point is reached we probably have an error
  state->fatal_error = true;
  state->running = false;

  return 0;
}


void write_mem(nes_state *state, uint16_t memloc, uint8_t value) {
  /*   8000-FFFF is the main area the cartridge ROM is mapped to in memory. Sometimes it can be bank switched, usually in 32k, 16k, or 8k sized banks. */

  // Writing to rom is not possible, signal fatal error
  if (memloc >= 0x8000) {
    state->fatal_error = true;
    state->running = false;
    return;
  }
  /*   0000-07FF is RAM*/
  /* 0800-1FFF are mirrors of RAM (you AND the address with 07FF to get the effective address)    */
  if (memloc < 0x1fff) {
    state->memory[memloc & 0x7FF] = value;
    return;
  }
  /*   2000-2007 is how the CPU writes to the PPU, 2008-3FFF are mirrors of that address range. */
  // Writing to any PPU IO port will fill the "latch" with that value
  if (memloc >= 0x2000 && memloc <= 0x3FFF) {
    printf("PPU Reg write! reg: %04X\n", memloc);
    // TODO - replace with writing to PPU IO regs
    if (memloc >= 0x2000 && memloc <= 0x3FFF) {
      uint16_t translated = memloc & 0x2007;
      switch (translated) {
      case 0x2000:
        state->ppu->registers->ppu_ctrl = value;
        break;
      case 0x2001:
        state->ppu->registers->ppu_mask = value;
        break;
      case 0x2002:
        // Read-only, just fill the latch
        break;
      case 0x2003:
        state->ppu->registers->oam_addr = value;
        break;
      case 0x2004:
        state->ppu->registers->oam_data = value;
        break;
      case 0x2005:
        state->ppu->registers->ppu_scroll = value;
        break;
      case 0x2006:
        if (state->ppu->high_pointer) {
          state->ppu->internal_addr_reg &= 0xff;
          state->ppu->internal_addr_reg |= ((uint16_t) value << 8);
        }
        else {
          state->ppu->internal_addr_reg &= 0xff00;
          state->ppu->internal_addr_reg |= (uint16_t) value;
        }
        state->ppu->high_pointer = !state->ppu->high_pointer;
        break;

      case 0x2007:
        state->ppu->registers->ppu_data = value;
        break;
      }
      // Writing to any PPU IO port will fill the latch/bus
      state->ppu->address_latch = value;

    }
    return;
  }
  /*   4000-401F is for IO ports and sound */
  if (memloc >= 0x4000 && memloc <= 0x401f) {
    // TODO - implement writing to APU IO
    switch(memloc) {
    case 0x4014:
      state->ppu->registers->oam_dma = value;
      state->ppu->address_latch = value;
      break;
    }
    return;
  }
  /*   4020-4FFF is rarely used, but can be used by some cartridges */
  /*   5000-5FFF is rarely used, but can be used by some cartridges, often as bank switching registers, not actual memory, but some cartridges put RAM there */
  /*   6000-7FFF is often cartridge WRAM. Since emulators usually emulate this whether it actually exists in the cartridge or not, there's a little bit of controversy about NES headers not adequately representing a cartridge. */



}


// Read from the memory mapped in the PPU
// https://wiki.nesdev.com/w/index.php/PPU_memory_map
uint8_t read_mem_ppu(nes_state *state, uint16_t memloc) {
  /* Address range	Size	Description */
  /*   $0000-$0FFF	$1000	Pattern table 0 */
  /*   $1000-$1FFF	$1000	Pattern table 1 */
  /*   $2000-$23FF	$0400	Nametable 0 */
  /*   $2400-$27FF	$0400	Nametable 1 */
  /*   $2800-$2BFF	$0400	Nametable 2 */
  /*   $2C00-$2FFF	$0400	Nametable 3 */
  /*   $3000-$3EFF	$0F00	Mirrors of $2000-$2EFF */
  /*   $3F00-$3F1F	$0020	Palette RAM indexes */
  /*   $3F20-$3FFF	$00E0	Mirrors of $3F00-$3F1F */

  /* $0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism. */
  if (memloc <= 0x1FFF) {
    return state->rom->chr_rom[memloc];
  }
  /* $2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to RAM on the cartridge, allowing up to 4 simultaneous nametables. */
  if (memloc <= 0x2FFF) {
    return state->ppu->ppu_vram[memloc - 0x2000];
  }
  /* $3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility. */
  // Mirrored VRAM
  if (memloc <= 0x3EFF) {
    return state->ppu->ppu_vram[memloc - 0x3000];
  }

  /* $3F00-3FFF is not configurable, always mapped to the internal palette control. */
  // Read from internal palette
  if (memloc <= 0x3FFF) {
    uint8_t translated = memloc & 0x1F;
    return state->ppu->palette_table[translated];
  }
  // Outside of memory, signal a fatal error
  state->fatal_error = true;
  state->running = false;
  return 0;
}
// The CPU can only write to PPU VRAM through the memory-mapped IO registers in the CPU memory map ($2000-$2007 + the dma port $4014
// This function does not care about "who" accesses, as it is used by both the ppu and the cpu
// Writes from cpu, will go through the write_mem function and that should be enough access control

/* void write_mem_ppu(nes_state *state, uint16_t memloc, uint8_t value) { */
/*   return; */
/* } */
