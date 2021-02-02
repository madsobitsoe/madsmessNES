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

// A struct representing the state of the console
// With pointers to cpu (registers), memory(stack+ram), ppu and apu
// Also contains information about the master clock
typedef struct NES_STATE {
  uint64_t master_clock;
    /* registers *registers; */
    cpu_state *cpu;
    uint8_t *memory; // Pointer to start of memory
    uint8_t *rom; // Pointer to memory containing the ROM
    bool running; // is the emulator still running?
    uint16_t ppu_cycle;
    uint16_t ppu_scanline;
  bool fatal_error;
} nes_state;

#endif
