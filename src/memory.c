#include "memory.h"

uint8_t read_mem(nes_state *state, uint16_t memloc) {
  /*   8000-FFFF is the main area the cartridge ROM is mapped to in memory. Sometimes it can be bank switched, usually in 32k, 16k, or 8k sized banks. */
  if (memloc >= 0x8000 && memloc <= 0xFFFF) {
    return state->rom[memloc];
  }
  /*   0000-07FF is RAM*/
  /* 0800-1FFF are mirrors of RAM (you AND the address with 07FF to get the effective address)    */
  if (memloc < 0x1fff) {
    return state->memory[memloc & 0x7FF];
  }
  /*   2000-2007 is how the CPU writes to the PPU, 2008-3FFF are mirrors of that address range. */
  if (memloc >= 0x2000 && memloc <= 0x3FFF) {
    // TODO - replace with reading PPU IO
    return 0;
  }
  /*   4000-401F is for IO ports and sound */
  if (memloc >= 0x4000 && memloc <= 0x401F) {
    // TODO - Replace with reading APU IO
    return 0;
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
  if (memloc >= 0x8000 && memloc <= 0xFFFF) {
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
  if (memloc >= 0x2000 && memloc <= 0x3FFF) {
    // TODO - replace with writing to PPU IO regs
    return;
  }
  /*   4000-401F is for IO ports and sound */
  if (memloc >= 0x4000 && memloc <= 0x401f) {
    // TODO - replace with writing to APU IO
    return;
  }
  /*   4020-4FFF is rarely used, but can be used by some cartridges */
  /*   5000-5FFF is rarely used, but can be used by some cartridges, often as bank switching registers, not actual memory, but some cartridges put RAM there */
  /*   6000-7FFF is often cartridge WRAM. Since emulators usually emulate this whether it actually exists in the cartridge or not, there's a little bit of controversy about NES headers not adequately representing a cartridge. */



}
