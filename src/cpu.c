#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "logger.h"


void init_registers(registers *regs) {
  regs->ACC = 0;
  regs->X = 0;
  regs->Y = 0;
  regs->PC = 0;
  regs->SP = 0;
  // init the SR register by setting it to the value specified by the nestest log
  // This is not "correct". See here: https://wiki.nesdev.com/w/index.php/CPU_power_up_state
  // and here: https://wiki.nesdev.com/w/index.php/Status_flags
  regs->SR = 0x24;
}

void print_status_reg(nes_state *state) {
  // nv1bdizc
  uint8_t status = state->registers->SR;
  if ((status & 128) == 128) { printf("N"); } else { printf("n"); }
  if ((status & 64) == 64) { printf("V"); } else { printf("v"); }
  if ((status & 32) == 32) { printf("1"); } else { printf("ERROR in always 1"); }
  if ((status & 16) == 16) { printf("B"); } else { printf("b"); }
  if ((status & 8) == 8) { printf("D"); } else { printf("d"); }
  if ((status & 4) == 4) { printf("I"); } else { printf("i"); }
  if ((status & 2) == 2) { printf("Z"); } else { printf("z"); }
  if ((status & 1) == 1) { printf("C"); } else { printf("c"); }
}

void print_regs(nes_state *state) {
  printf("\x1b[1;31mACC: \x1b[0m0x%02x\n\x1b[1;32mX: \x1b[0m  0x%02x\n\x1b[1;33mY: \x1b[0m  0x%02x\n\x1b[1;34mSP: \x1b[0m 0x%04x\n\x1b[1;35mPC:\x1b[0m  0x%04x\n\x1b[1;36mFlags: \x1b[0m0x%02x - ",
         state->registers->ACC,
         state->registers->X,
         state->registers->Y,
         state->registers->SP,
         state->registers->PC,
         state->registers->SR
         );
  print_status_reg(state);
  printf("\n");
}


void print_stack(nes_state *state) {
  // Stack is located at 0x100-0x1ff
  // stack pointer should be offset by 0x100
  // The stack grows downwards, so SP is initialized to 0xFF
  int offset = 0x100;
  for (int i = 10; i >= 0; i--) {
    printf("\t%04X\t%02X\n", state->registers->SP-i + offset, read_mem_byte(state, state->registers->SP - i + offset));
  }
}



void set_pc(nes_state *state, unsigned short pc) {
  state->registers->PC = pc;
  state->current_opcode_PC = pc;
  state->current_opcode = read_mem_byte(state, pc);
}

void set_negative_flag(nes_state *state) {
  state->registers->SR |= 128;
}

void set_overflow_flag(nes_state *state) {
  state->registers->SR |= 64;
}


void set_break_flag(nes_state *state) {
  state->registers->SR |= 16;
}


void set_decimal_flag(nes_state *state) {
  state->registers->SR |= 8;
}

void set_interrupt_flag(nes_state *state) {
  state->registers->SR |= 4;
}

void set_zero_flag(nes_state *state) {
  state->registers->SR |= 2;
}

void set_carry_flag(nes_state *state) {
  state->registers->SR |= 1;
}

bool is_carry_flag_set(nes_state *state) {
  return ((state->registers->SR & 1) == 1);
}
bool is_zero_flag_set(nes_state *state) {
  return ((state->registers->SR & 2) == 2);
}
bool is_interrupt_flag_set(nes_state *state) {
  return ((state->registers->SR & 4) == 4);
}
bool is_decimal_flag_set(nes_state *state) {
  return ((state->registers->SR & 8) == 8);
}
bool is_break_flag_set(nes_state *state) {
  return ((state->registers->SR & 16) == 16);
}
bool is_overflow_flag_set(nes_state *state) {
  return ((state->registers->SR & 64) == 64);
}
bool is_negative_flag_set(nes_state *state) {
  return ((state->registers->SR & 128) == 128);
}



// Translate a memory location to the actual memory location in the nes
  uint16_t translate_memory_location(unsigned short memloc) {
  /*   0000-07FF is RAM*/
  if (memloc <= 0x7ff) { return memloc; }
  /* 0800-1FFF are mirrors of RAM (you AND the address with 07FF to get the effective address)    */
  if (memloc <= 0x1fff) { return memloc & 0x7ff; }
  /*   2000-2007 is how the CPU writes to the PPU, 2008-3FFF are mirrors of that address range. */
  if (memloc <= 0x3fff) { return 0x2000 + (memloc & 0x7); }

  /*   4000-401F is for IO ports and sound */
  /*   4020-4FFF is rarely used, but can be used by some cartridges */
  /*   5000-5FFF is rarely used, but can be used by some cartridges, often as bank switching registers, not actual memory, but some cartridges put RAM there */
  /*   6000-7FFF is often cartridge WRAM. Since emulators usually emulate this whether it actually exists in the cartridge or not, there's a little bit of controversy about NES headers not adequately representing a cartridge. */
  /*   8000-FFFF is the main area the cartridge ROM is mapped to in memory. Sometimes it can be bank switched, usually in 32k, 16k, or 8k sized banks. */
  /*   The NES header takes up 16 bytes, after that is the PRG pages, then after that is the CHR pages. You look at the header to see how big the PRG and CHR of the cartridge are, see documentation for more details. The NES header does not exist outside of .NES files, you won't find it on any NES cartridges. */

  /*   So you load a Mapper 0 (NROM) cartridge into memory, and the first two PRG banks appear in NES memory at 8000-FFFF. If there is only one 16k bank, then it is mirrored at 8000-BFFF and C000-FFFF. */

  /*   When the CPU boots up, it reads the Reset vector, located at FFFC. That contains a 16-bit value which tells the CPU where to jump to. */
  /*   The first thing a game will do when it starts up is repeatedly read PPU register 2002 to wait for the NES to warm up, so you won't see a game doing anything until you throw in some rudimentary PPU emulation. */
  /*   Then the game clears the RAM, and waits for the NES to warm up some more. Then the system is ready, and the game will start running. */

  /* if (memloc > 0xffff) { */
  /*   // Somehow signal an invalid memloc */
  /* } */
  return memloc; // TODO Fix this
}


// Push a value to the stack
void push(nes_state *state, uint8_t value) {
  state->memory[state->registers->SP + 0x100] = value;
  state->registers->SP--;;
}


uint16_t read_mem_short(nes_state *state, uint16_t memloc) {
  /* printf("Translating memory location 0x%x.\n", memloc); */
  uint16_t translated = translate_memory_location(memloc);
  /* printf("Translated memory location 0x%x.\n", translated); */

  uint16_t value = 0;
  if (translated >= 0x0 && translated <= 0x7ff) {
    value = state->memory[translated] | state->memory[translated+1] << 8;
  }
  else if (translated >= 0x8000 && translated <= 0xffff) {
    unsigned short loc = translated - 0xc000;
    value = (uint16_t) state->rom[loc] | (((uint16_t) state->rom[loc+1]) << 8);
  }
  else {
    printf("location outside of valid memory! %04X\n", memloc);
    print_state(state);
  }
  return value;
}
uint8_t read_mem_byte(nes_state *state, uint16_t memloc) {
  uint16_t first_read = read_mem_short(state, memloc);
  return (uint8_t) (first_read & 0xff);
}



uint8_t fetch_next_opcode(nes_state *state) {
  printf("Fetching next opcode!");
  uint16_t translated = translate_memory_location(state->registers->PC);

  uint8_t opcode = read_mem_byte(state, translated);
  return opcode;
}

// Execute a JMP instruction
void exec_JMP(nes_state *state) {
  uint16_t new_pc;
  switch (state->current_opcode) {
    // JMP
  case 0x4c:
    /*     Absolute addressing */

    /*       JMP */

    /* #  address R/W description */
    /*       --- ------- --- ------------------------------------------------- */
    /*       1    PC     R  fetch opcode, increment PC */
    /*       2    PC     R  fetch low address byte, increment PC */
    /*       3    PC     R  copy low address byte to PCL, fetch high address */
    /*       byte to PCH */
    switch(state->stall_cycles) {
    case 2:
      // Should fetch low address byte and inc pc, but we "fake" it
      state->registers->PC += 1;
      break;
    case 1:
      new_pc = read_mem_short(state, state->registers->PC - 1);
      set_pc(state, new_pc);
      break;
    }
    break;
    // JSR
  case 0x20:
    /* #  address R/W description */
    /*     --- ------- --- ------------------------------------------------- */
    /*     1    PC     R  fetch opcode, increment PC */
    /*       2    PC     R  fetch low address byte, increment PC */
    /*       3  $0100,S  R  internal operation (predecrement S?) */
    /*       4  $0100,S  W  push PCH on stack, decrement S */
    /*       5  $0100,S  W  push PCL on stack, decrement S */
    /*       6    PC     R  copy low address byte to PCL, fetch high address */
    /*       byte to PCH */
    switch(state->stall_cycles) {
    case 5:
      state->registers->PC += 1;
      break;
    case 4: // Decrement SP here?
      break;
    case 3:
      push(state, read_mem_byte(state, state->registers->PC));
      break;
    case 2:
      push(state, read_mem_byte(state, state->registers->PC+1));
      break;
    case 1:
      new_pc = read_mem_short(state, state->registers->PC - 1);
      set_pc(state, new_pc);
      break;
    }
    break;

  }


  //  case 0x6c:
  /*   Absolute indirect addressing (JMP) */

  /* #   address  R/W description */
  /*     --- --------- --- ------------------------------------------ */
  /*     1     PC      R  fetch opcode, increment PC */
  /*     2     PC      R  fetch pointer address low, increment PC */
  /*     3     PC      R  fetch pointer address high, increment PC */
  /*     4   pointer   R  fetch low address to latch */
  /*     5  pointer+1* R  fetch PCH, copy latch to PCL */

  /*     Note: * The PCH will always be fetched from the same page */
  /*     than PCL, i.e. page boundary crossing is not handled. */


}


void exec_LDX(nes_state *state) {
  // ABSOLUTE ADDRESSING
  /*   Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT, */
  /*                      LAX, NOP) */

  /* #  address R/W description */
  /*     --- ------- --- ------------------------------------------ */
  /*     1    PC     R  fetch opcode, increment PC */
  /*     2    PC     R  fetch low byte of address, increment PC */
  /*     3    PC     R  fetch high byte of address, increment PC */
  /*     4  address  R  read from effective address */
  uint8_t operand;
  switch (state->current_opcode) {
    // IMMEDIATE
    /*     Immediate addressing */

    /* #  address R/W description */
    /*       --- ------- --- ------------------------------------------ */
    /*       1    PC     R  fetch opcode, increment PC */
    /*       2    PC     R  fetch value, increment PC */
  case 0xa2: // LDX Immediate
    switch(state->stall_cycles) {
    case 1:
      operand = read_mem_byte(state, state->registers->PC);
      state->registers->X = operand;
      state->registers->PC += 1;
      if (operand == 0) { set_zero_flag(state); }
      break;
    }
  }
}


void exec_STX(nes_state *state) {
  uint8_t operand;
  switch (state->current_opcode) {
  case 0x86: // STX Zero page

    /*     Write instructions (STA, STX, STY, SAX) */

    /* #  address R/W description */
    /*       --- ------- --- ------------------------------------------ */
    /*       1    PC     R  fetch opcode, increment PC */
    /*       2    PC     R  fetch address, increment PC */
    /*       3  address  W  write register to effective address */
    switch(state->stall_cycles) {
    case 2:
      state->registers->PC += 1;
      break;
    case 1:
      operand = read_mem_byte(state, state->registers->PC-2);
      state->memory[operand] = state->registers->X;
      break;
    }
  }
}


void exec_FLAGS(nes_state *state) {
  switch(state->current_opcode) {
    // SEC - Set Carry Flag
  case 0x38:
    switch(state->stall_cycles) {
    case 1:
      set_carry_flag(state);
      break;
    }
    break;

  }

}

void exec_BRANCH(nes_state *state) {
  uint8_t operand;
  // See this page: http://www.6502.org/tutorials/6502opcodes.html#BCS
  /* MNEMONIC                       HEX */
  /*   BPL (Branch on PLus)           $10 */
  /*   BMI (Branch on MInus)          $30 */
  /*   BVC (Branch on oVerflow Clear) $50 */
  /*   BVS (Branch on oVerflow Set)   $70 */
  /*   BCC (Branch on Carry Clear)    $90 */
  /*   BCS (Branch on Carry Set)      $B0 */
  /*   BNE (Branch on Not Equal)      $D0 */
  /*   BEQ (Branch on EQual)          $F0 */
  /*   Relative addressing (BCC, BCS, BNE, BEQ, BPL, BMI, BVC, BVS) */

  /* #   address  R/W description */
  /*     --- --------- --- --------------------------------------------- */
  /*     1     PC      R  fetch opcode, increment PC */
  /*     2     PC      R  fetch operand, increment PC */
  /*     3     PC      R  Fetch opcode of next instruction, */
  /*     If branch is taken, add operand to PCL. */
  /*     Otherwise increment PC. */
  /*     4+    PC*     R  Fetch opcode of next instruction. */
  /*     Fix PCH. If it did not change, increment PC. */
  /*     5!    PC      R  Fetch opcode of next instruction, */
  /*     increment PC. */

  /*     Notes: The opcode fetch of the next instruction is included to */
  /*     this diagram for illustration purposes. When determining */
  /*     real execution times, remember to subtract the last */
  /*     cycle. */
  /*          * The high byte of Program Counter (PCH) may be invalid */
  /*             at this time, i.e. it may be smaller or bigger by $100. */
  /*          + If branch is taken, this cycle will be executed. */
  /*          ! If branch occurs to different page, this cycle will be */
  /*            executed. */
  switch(state->current_opcode) {
    // BCS - Branch on Carry Set
  case 0xB0:
    switch(state->stall_cycles) {
      // Still not sure how to handle all of this branching stuff with extra cycles.
    case 1:
      operand = read_mem_byte(state, state->registers->PC);
      state->registers->PC++;
      break;

    }
  }
}


void exec_opcode(nes_state *state) {
  // Terrible, but good enough for now.
  switch(state->current_opcode) {
  case 0x38:
    exec_FLAGS(state);
    break;
    // JSR
  case 0x20:
    // JMP Absolute
  case 0x4c:
    exec_JMP(state);
    break;
  case 0xa2:
    exec_LDX(state);
    break;
  case 0x86:
    exec_STX(state);
    break;
  default:
    printf("Opcode 0x%02x not implemented.\n", state->current_opcode);

  }
}

int cycles_for_opcode(nes_state *state) { //uint8_t opcode) {
  // In general an instruction takes 2 cycles. The first cycle is already handled, so default is 1.
  uint8_t cycles = 1;
  // TODO - Replace with table/array?
  switch(state->current_opcode) {
    // JSR
  case 0x20:
    cycles = 5;
    break;
    // SEC - Set carry flag
  case 0x38:
    cycles = 1;
    break;
  case 0xa2:
    cycles = 1;
    break;
  case 0x4c:
    cycles = 2;
    break;
  case 0x86:
    cycles = 2;
    break;
    // BCS - Branch if Carry Set
  case 0xB0:
    if (is_carry_flag_set(state)) {
      cycles++;
      // If a page boundary is crossed, add another cycle
      /* if () {} */
    }

    break;

  }
  return cycles;
}

void cpu_step(nes_state *state) {
  if (state->stall_cycles <= 0) {
    // Store the program counter pointing at the current opcode
    state->current_opcode_PC = state->registers->PC;
    // Fetch the instruction at $PC
    state->current_opcode = fetch_next_opcode(state);
    // Get the amount of cycles for the instruction
    state->stall_cycles = cycles_for_opcode(state);
    // Increment PC
    state->registers->PC += 1;
  }
  else {
    // Execute the instruction
    //    printf("Executing opcode: 0x%02x\n", state->current_opcode);
    exec_opcode(state);
    state->stall_cycles -= 1;
  }
}


void ppu_step(nes_state *state) {
  for (int i = 0; i < 3; i++) {
    state->ppu_cycle++;
    if (state->ppu_cycle == 333) {
      state->ppu_cycle = 0;
      state->ppu_frame++;
    }
  }
}
