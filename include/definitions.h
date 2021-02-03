#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <stdbool.h>

// Define a structure for the CPU registers
typedef struct REGISTERS {
  uint8_t ACC; // Accumulator
  uint8_t X; // Index Register X
  uint8_t Y; // Index Register Y
  uint16_t PC; // Program counter - 16 bits
  uint8_t SP; // Stack Pointer
  uint8_t SR; // Status register
} registers;


typedef struct CPU_STATE {
  registers *registers;
  uint8_t current_opcode;
  uint8_t low_addr_byte;
  uint8_t high_addr_byte;
  uint8_t *destination_reg;
  uint8_t *source_reg;
  uint8_t operand;
  uint16_t current_opcode_PC;
  uint64_t cpu_cycle;
  uint16_t action_queue[10];
  uint8_t next_action;
  uint8_t end_of_queue;

} cpu_state;

// See: https://wiki.nesdev.com/w/index.php/PPU_registers
typedef struct PPU_REGISTERS {
  uint8_t ppu_ctrl; // $2000
  uint8_t ppu_mask; // $2001
  uint8_t ppu_status; // $2002
  uint8_t oam_addr; // $2003
  uint8_t oam_data; // $2004
  uint8_t ppu_scroll; // $2005
  uint8_t ppu_addr; // $2006
  uint8_t ppu_data; // $2007
  uint8_t oam_dma; // $4014
} ppu_registers;

// https://wiki.nesdev.com/w/index.php/PPU_memory_map
typedef struct PPU_STATE {
  uint8_t *ppu_vram; // 2kb vram in ppu
  uint8_t *palette_table; // 32 byte palette table
  uint8_t *chr_rom; // CHR_ROM in the rom (8kb only for now)
  uint8_t *oam_memory; // 256 bytes of Object Attribute Memory
  ppu_registers *registers;
  uint16_t ppu_cycle;
  uint16_t ppu_scanline;
  uint32_t ppu_frame;
  bool high_pointer; // Is the next write to $2005 the high or low byte?
  uint8_t address_latch; // "dynamic latch" aka. internal buffer in ppu used by $2005, $2006 and $2007
  uint16_t internal_addr_reg; // Use for the internal addr written through the ppu_addr $2006 register, updated by reads from $2007
} ppu_state;


// A struct representing a ROM in the iNES format
// Currently only mapper 0
typedef struct NES_ROM {
  uint8_t prg_rom_size; // size of PRG ROM in 16 kb units
  uint8_t chr_rom_size; // size of CHR ROM in 8 kb units (Value 0 means the board uses CHR RAM)
  uint8_t mapper; // byte indicating what mapper is used. low: byte 6, bits 4-7. high: byte 7, bits 4-7
  bool mirroring; // false = horizontal, true = vertical
  bool battery_backed; // true = battery-backed RAM at $6000-$7FFF
  bool trainer; // true = 0x10-0x210 has a "trainer" to be skipped (mapped to $7000-$71ff ?)
  bool four_screen_VRAM; // byte 6, bit 3
  bool vs_system_cartridge; // byte 7, bit 0
  /* Number of 8kB RAM banks. For compatibility with the previous */
  /*   versions of the .NES format, assume 1x8kB RAM page when this */
  /*   byte is zero. */
  uint8_t prg_ram_size; // (rarely used extension)
  bool ntsc; // true for ntsc, false for PAL
  uint8_t *prg_rom1; // Pointer to first part of PRG_ROM (for memory mapping) $8000-$BFFF
  uint8_t *prg_rom2; // Pointer to second part of PRG_ROM (for memory mapping) $C000-$FFFF
  uint8_t *chr_rom; // pointer to CHR ROM (for memory mapping)
} nes_rom;


// A struct representing the state of the console
// With pointers to cpu (registers), memory(stack+ram), ppu and apu
// Also contains information about the master clock
typedef struct NES_STATE {
  uint64_t master_clock;
    /* registers *registers; */
  cpu_state *cpu;
  uint8_t *memory; // Pointer to start of memory
  /* uint8_t *rom; // Pointer to memory containing the ROM */
  nes_rom *rom; // pointer to the rom struct
  bool running; // is the emulator still running?
  ppu_state *ppu;
  uint16_t ppu_cycle;
  uint16_t ppu_scanline;
  bool fatal_error;
} nes_state;

#endif
