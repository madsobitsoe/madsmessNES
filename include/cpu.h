#include <stdbool.h>
#include <stdint.h>
// Define a structure for internal state of the cpu
// Including all registers and their current contents
// and "RAM"
typedef struct REGISTERS {
  uint8_t ACC; // Accumulator
  uint8_t X; // Index Register X
  uint8_t Y; // Index Register Y
  uint16_t PC; // Program counter - 16 bits
  uint8_t SP; // Stack Pointer
  uint8_t SR; // Status register
} registers;

// A struct representing the state of the console
// With pointers to cpu (registers), memory(stack+ram), ppu and apu
// Also contains information about the master clock
typedef struct NES_STATE {
  uint64_t master_clock;
  registers *registers;
  uint8_t *memory; // Pointer to start of memory
  uint8_t *rom; // Pointer to memory containing the ROM
  bool running; // is the emulator still running?
  uint8_t stall_cycles;
  int16_t current_function_index;
  uint8_t current_opcode;
  uint16_t current_opcode_PC;
} nes_state;


void destroy_state(nes_state *state);
void cpu_step(nes_state *state);
void step(nes_state *state);
nes_state* init_state();
void print_state(nes_state *state);
void attach_rom(nes_state *state, unsigned char *rommem);
void set_pc(nes_state *state, unsigned short pc);
uint8_t read_mem_byte(nes_state *state, unsigned short memloc);
uint16_t read_mem_short(nes_state *state, unsigned short memloc);
uint16_t translate_memory_location(unsigned short memloc);
