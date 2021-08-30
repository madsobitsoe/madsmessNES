#include <stdlib.h>
#include <stdio.h>
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
  if (addr < 0x3f00) {
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

// Do one step of the ppu
void ppu_step(nes_state *state) {
  // Scanlines 0-239 are all "similar"
  // They differ a tiny amount, but we'll handle that later
  // VISIBLE FRAME SCANLINES
    uint16_t scanline = state->ppu->ppu_scanline;
    uint16_t cycle = state->ppu->ppu_cycle;
  if (scanline <= 239) {
/* This is an idle cycle. The value on the PPU address bus during this cycle appears to be the same CHR address that is later used to fetch the low background tile byte starting at dot 5 (possibly calculated during the two unused NT fetches at the end of the previous scanline).  */      
    if (cycle == 0) {
	  
    }
/*     The data for each tile is fetched during this phase. Each memory access takes 2 PPU cycles to complete, and 4 must be performed per tile: */

/*     Nametable byte */
/*     Attribute table byte */
/*     Pattern table tile low */
/*     Pattern table tile high (+8 bytes from pattern table tile low) */

/* The data fetched from these accesses is placed into internal latches, and then fed to the appropriate shift registers when it's time to do so (every 8 cycles). Because the PPU can only fetch an attribute byte every 8 cycles, each sequential string of 8 pixels is forced to have the same palette attribute. */

/* Sprite zero hits act as if the image starts at cycle 2 (which is the same cycle that the shifters shift for the first time), so the sprite zero flag will be raised at this point at the earliest. Actual pixel output is delayed further due to internal render pipelining, and the first pixel is output during cycle 4. */

/* The shifters are reloaded during ticks 9, 17, 25, ..., 257. */

/* Note: At the beginning of each scanline, the data for the first two tiles is already loaded into the shift registers (and ready to be rendered), so the first tile that gets fetched is Tile 3.  */
/* While all of this is going on, sprite evaluation for the next scanline is taking place as a seperate process, independent to what's happening here. */
    else if (cycle <= 256) {

    }
/* The tile data for the sprites on the next scanline are fetched here. Again, each memory access takes 2 PPU cycles to complete, and 4 are performed for each of the 8 sprites: */
/*     Garbage nametable byte */
/*     Garbage nametable byte */
/*     Pattern table tile low */
/*     Pattern table tile high (+8 bytes from pattern table tile low) */
/* The garbage fetches occur so that the same circuitry that performs the BG tile fetches could be reused for the sprite tile fetches. */
/* If there are less than 8 sprites on the next scanline, then dummy fetches to tile $FF occur for the left-over sprites, because of the dummy sprite data in the secondary OAM (see sprite evaluation). This data is then discarded, and the sprites are loaded with a transparent set of values instead. */
/* In addition to this, the X positions and attributes for each sprite are loaded from the secondary OAM into their respective counters/latches. This happens during the second garbage nametable fetch, with the attribute byte loaded during the first tick and the X coordinate during the second.  */
    else if (cycle <= 320) {
    }
    /* This is where the first two tiles for the next scanline are fetched, and loaded into the shift registers. Again, each memory access takes 2 PPU cycles to complete, and 4 are performed for the two tiles: */

    /* Nametable byte */
    /* Attribute table byte */
    /* Pattern table tile low */
    /* Pattern table tile high (+8 bytes from pattern table tile low) */
    else if (cycle <= 336) {
	
    }
/* Two bytes are fetched, but the purpose for this is unknown. These fetches are 2 PPU cycles each. */
/*     Nametable byte */
/*     Nametable byte */
/* Both of the bytes fetched here are the same nametable byte that will be fetched at the beginning of the next scanline (tile 3, in other words). At least one mapper -- MMC5 -- is known to use this string of three consecutive nametable fetches to clock a scanline counter.  */    
    else if (cycle <= 340) {
	
    }

  }
  else {
    switch (scanline) {
/* Post-render scanline (240) */
/* The PPU just idles during this scanline. Even though accessing PPU memory from the program would be safe here, the VBlank flag isn't set until after this scanline.  */	
    case 240:
	break;
      // In cycle 1 (0-indexed), VBLank flag is set
/* Vertical blanking lines (241-260) */
/* The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241, where the VBlank NMI also occurs. The PPU makes no memory accesses during these scanlines, so PPU memory can be freely accessed by the program.  */	
    case 241:
      // Set Vblank flag
	printf("Setting vblank at scanline 241\n");
	state->ppu->registers->ppu_status |= 128;
      break;
      // Last scanline! Lots of stuff happens
      // cycle 1 (0-indexed) - clear VBlank, sprite 0, Overflow
      // More stuff happens, see wiki when we get to that
    case 261:
	if (cycle == 1) {
	    // Clear VBLANK, sprite 0 and overflow - three highest bits
	    state->ppu->registers->ppu_status &= 0x1f;

	}
      break ;
      // Nothing happens
    default: break;
    }
  }
  // Update cycle and scanline count
  state->ppu->ppu_cycle++;
  if (state->ppu->ppu_cycle > 340) {
    state->ppu->ppu_cycle = 0;
    state->ppu->ppu_scanline++;
  }
  if (state->ppu->ppu_scanline > 261) {
    state->ppu->ppu_scanline = 0;

  }
}
