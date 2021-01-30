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
  uint8_t status = state->cpu->registers->SR;
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
         state->cpu->registers->ACC,
         state->cpu->registers->X,
         state->cpu->registers->Y,
         state->cpu->registers->SP,
         state->cpu->registers->PC,
         state->cpu->registers->SR
         );
  print_status_reg(state);
  printf("\n");
}


void print_stack(nes_state *state) {
  // Stack is located at 0x100-0x1ff
  // stack pointer should be offset by 0x100
  // The stack grows downwards, so SP is initialized to 0xFF
  int offset = 0x100;
  int end = state->cpu->registers->SP+5 < 0xFF ? state->cpu->registers->SP+5 : 0xFF;
  char *space  = "        ";
  char *spline = "  SP--> ";
  /* int start = end - 10; */
  for (int i = 10; i >= 0; i--) {
    if ((uint8_t) end - i == state->cpu->registers->SP) {
      printf("%s%04X\t%02X\n", spline, end - i + offset, read_mem_byte(state, end - i + offset));
    }
    else {
      printf("%s%04X\t%02X\n", space, end - i + offset, read_mem_byte(state, end - i + offset));
    }
    /* printf("        %04X\t%02X\n", state->cpu->registers->SP-i + offset, read_mem_byte(state, state->cpu->registers->SP - i + offset)); */
  }
}



void set_pc(nes_state *state, unsigned short pc) {
  state->cpu->registers->PC = pc;
  state->cpu->current_opcode_PC = pc;
  state->cpu->current_opcode = read_mem_byte(state, pc);
}

void set_negative_flag(nes_state *state) {
  state->cpu->registers->SR |= 128;
}

void set_overflow_flag(nes_state *state) {
  state->cpu->registers->SR |= 64;
}


void set_break_flag(nes_state *state) {
  state->cpu->registers->SR |= 16;
}


void set_decimal_flag(nes_state *state) {
  state->cpu->registers->SR |= 8;
}

void set_interrupt_flag(nes_state *state) {
  state->cpu->registers->SR |= 4;
}

void set_zero_flag(nes_state *state) {
  state->cpu->registers->SR |= 2;
}

void set_carry_flag(nes_state *state) {
  state->cpu->registers->SR |= 1;
}

void clear_negative_flag(nes_state *state) {
  state->cpu->registers->SR &= 127;
}

void clear_overflow_flag(nes_state *state) {
  state->cpu->registers->SR &= (255-64);
}


void clear_break_flag(nes_state *state) {
  state->cpu->registers->SR &= (255-16);
}


void clear_decimal_flag(nes_state *state) {
  state->cpu->registers->SR &= (255-8);
}

void clear_interrupt_flag(nes_state *state) {
  state->cpu->registers->SR &= (255-4);
}

void clear_zero_flag(nes_state *state) {
  state->cpu->registers->SR &= (255-2);
}

void clear_carry_flag(nes_state *state) {
  state->cpu->registers->SR &= (255-1);
}


bool is_carry_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 1) == 1);
}
bool is_zero_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 2) == 2);
}
bool is_interrupt_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 4) == 4);
}
bool is_decimal_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 8) == 8);
}
bool is_break_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 16) == 16);
}
bool is_overflow_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 64) == 64);
}
bool is_negative_flag_set(nes_state *state) {
  return ((state->cpu->registers->SR & 128) == 128);
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
  state->memory[state->cpu->registers->SP + 0x100] = value;
  state->cpu->registers->SP--;;
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



/* uint8_t fetch_next_opcode(nes_state *state) { */
/*   printf("Fetching next opcode!"); */
/*   uint16_t translated = translate_memory_location(state->cpu->registers->PC); */

/*   uint8_t opcode = read_mem_byte(state, translated); */
/*   return opcode; */
/* } */

/* // Execute a JMP instruction */
/* void exec_JMP(nes_state *state) { */
/*   uint16_t new_pc; */
/*   switch (state->cpu->current_opcode) { */
/*     // JMP */
/*   case 0x4c: */
/*     /\*     Absolute addressing *\/ */

/*     /\*       JMP *\/ */

/*     /\* #  address R/W description *\/ */
/*     /\*       --- ------- --- ------------------------------------------------- *\/ */
/*     /\*       1    PC     R  fetch opcode, increment PC *\/ */
/*     /\*       2    PC     R  fetch low address byte, increment PC *\/ */
/*     /\*       3    PC     R  copy low address byte to PCL, fetch high address *\/ */
/*     /\*       byte to PCH *\/ */
/*     switch(state->cpu->stall_cycles) { */
/*     case 2: */
/*       // Should fetch low address byte and inc pc, but we "fake" it */
/*       state->cpu->registers->PC += 1; */
/*       break; */
/*     case 1: */
/*       new_pc = read_mem_short(state, state->cpu->registers->PC - 1); */
/*       set_pc(state, new_pc); */
/*       break; */
/*     } */
/*     break; */
/*     // JSR */
/*   case 0x20: */
/*     /\* #  address R/W description *\/ */
/*     /\*     --- ------- --- ------------------------------------------------- *\/ */
/*     /\*     1    PC     R  fetch opcode, increment PC *\/ */
/*     /\*       2    PC     R  fetch low address byte, increment PC *\/ */
/*     /\*       3  $0100,S  R  internal operation (predecrement S?) *\/ */
/*     /\*       4  $0100,S  W  push PCH on stack, decrement S *\/ */
/*     /\*       5  $0100,S  W  push PCL on stack, decrement S *\/ */
/*     /\*       6    PC     R  copy low address byte to PCL, fetch high address *\/ */
/*     /\*       byte to PCH *\/ */
/*     switch(state->cpu->stall_cycles) { */
/*     case 5: */
/*       state->cpu->registers->PC += 1; */
/*       break; */
/*     case 4: // Decrement SP here? */
/*       break; */
/*     case 3: */
/*       push(state, read_mem_byte(state, state->cpu->registers->PC)); */
/*       break; */
/*     case 2: */
/*       push(state, read_mem_byte(state, state->cpu->registers->PC+1)); */
/*       break; */
/*     case 1: */
/*       new_pc = read_mem_short(state, state->cpu->registers->PC - 1); */
/*       set_pc(state, new_pc); */
/*       break; */
/*     } */
/*     break; */

/*   } */


/*   //  case 0x6c: */
/*   /\*   Absolute indirect addressing (JMP) *\/ */

/*   /\* #   address  R/W description *\/ */
/*   /\*     --- --------- --- ------------------------------------------ *\/ */
/*   /\*     1     PC      R  fetch opcode, increment PC *\/ */
/*   /\*     2     PC      R  fetch pointer address low, increment PC *\/ */
/*   /\*     3     PC      R  fetch pointer address high, increment PC *\/ */
/*   /\*     4   pointer   R  fetch low address to latch *\/ */
/*   /\*     5  pointer+1* R  fetch PCH, copy latch to PCL *\/ */

/*   /\*     Note: * The PCH will always be fetched from the same page *\/ */
/*   /\*     than PCL, i.e. page boundary crossing is not handled. *\/ */


/* } */


/* void exec_LDX(nes_state *state) { */
/*   // ABSOLUTE ADDRESSING */
/*   /\*   Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT, *\/ */
/*   /\*                      LAX, NOP) *\/ */

/*   /\* #  address R/W description *\/ */
/*   /\*     --- ------- --- ------------------------------------------ *\/ */
/*   /\*     1    PC     R  fetch opcode, increment PC *\/ */
/*   /\*     2    PC     R  fetch low byte of address, increment PC *\/ */
/*   /\*     3    PC     R  fetch high byte of address, increment PC *\/ */
/*   /\*     4  address  R  read from effective address *\/ */
/*   uint8_t operand; */
/*   switch (state->cpu->current_opcode) { */
/*     // IMMEDIATE */
/*     /\*     Immediate addressing *\/ */

/*     /\* #  address R/W description *\/ */
/*     /\*       --- ------- --- ------------------------------------------ *\/ */
/*     /\*       1    PC     R  fetch opcode, increment PC *\/ */
/*     /\*       2    PC     R  fetch value, increment PC *\/ */
/*   case 0xa2: // LDX Immediate */
/*     switch(state->cpu->stall_cycles) { */
/*     case 1: */
/*       operand = read_mem_byte(state, state->cpu->registers->PC); */
/*       state->cpu->registers->X = operand; */
/*       state->cpu->registers->PC += 1; */
/*       if (operand == 0) { set_zero_flag(state); } */
/*       break; */
/*     } */
/*   } */
/* } */


/* void exec_STX(nes_state *state) { */
/*   uint8_t operand; */
/*   switch (state->cpu->current_opcode) { */
/*   case 0x86: // STX Zero page */

/*     /\*     Write instructions (STA, STX, STY, SAX) *\/ */

/*     /\* #  address R/W description *\/ */
/*     /\*       --- ------- --- ------------------------------------------ *\/ */
/*     /\*       1    PC     R  fetch opcode, increment PC *\/ */
/*     /\*       2    PC     R  fetch address, increment PC *\/ */
/*     /\*       3  address  W  write register to effective address *\/ */
/*     switch(state->cpu->stall_cycles) { */
/*     case 2: */
/*       state->cpu->registers->PC += 1; */
/*       break; */
/*     case 1: */
/*       operand = read_mem_byte(state, state->cpu->registers->PC-2); */
/*       state->memory[operand] = state->cpu->registers->X; */
/*       break; */
/*     } */
/*   } */
/* } */


/* void exec_FLAGS(nes_state *state) { */
/*   switch(state->cpu->current_opcode) { */
/*     // SEC - Set Carry Flag */
/*   case 0x38: */
/*     switch(state->cpu->stall_cycles) { */
/*     case 1: */
/*       set_carry_flag(state); */
/*       break; */
/*     } */
/*     break; */

/*   } */

/* } */

/* void exec_BRANCH(nes_state *state) { */
/*   uint8_t operand; */
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
/*   switch(state->cpu->current_opcode) { */
/*     // BCS - Branch on Carry Set */
/*   case 0xB0: */
/*     switch(state->cpu->stall_cycles) { */
/*       // Still not sure how to handle all of this branching stuff with extra cycles. */
/*     case 1: */
/*       operand = read_mem_byte(state, state->cpu->registers->PC); */
/*       state->cpu->registers->PC++; */
/*       break; */

/*     } */
/*   } */
/* } */


/* void exec_opcode(nes_state *state) { */
/*   // Terrible, but good enough for now. */
/*   switch(state->cpu->current_opcode) { */
/*   case 0x38: */
/*     exec_FLAGS(state); */
/*     break; */
/*     // JSR */
/*   case 0x20: */
/*     // JMP Absolute */
/*   case 0x4c: */
/*     exec_JMP(state); */
/*     break; */
/*   case 0xa2: */
/*     exec_LDX(state); */
/*     break; */
/*   case 0x86: */
/*     exec_STX(state); */
/*     break; */
/*   default: */
/*     printf("Opcode 0x%02x not implemented.\n", state->cpu->current_opcode); */

/*   } */
/* } */

/* int cycles_for_opcode(nes_state *state) { //uint8_t opcode) { */
/*   // In general an instruction takes 2 cycles. The first cycle is already handled, so default is 1. */
/*   uint8_t cycles = 1; */
/*   // TODO - Replace with table/array? */
/*   switch(state->cpu->current_opcode) { */
/*     // JSR */
/*   case 0x20: */
/*     cycles = 5; */
/*     break; */
/*     // SEC - Set carry flag */
/*   case 0x38: */
/*     cycles = 1; */
/*     break; */
/*   case 0xa2: */
  /*     cycles = 1; */
  /*     break; */
  /*   case 0x4c: */
  /*     cycles = 2; */
  /*     break; */
  /*   case 0x86: */
  /*     cycles = 2; */
  /*     break; */
  /*     // BCS - Branch if Carry Set */
  /*   case 0xB0: */
  /*     if (is_carry_flag_set(state)) { */
  /*       cycles++; */
  /*       // If a page boundary is crossed, add another cycle */
  /*       /\* if () {} *\/ */
  /*     } */

  /*     break; */

  /*   } */
  /*   return cycles; */
  /* } */




  // Instructions take 2-7 cycles.
  // Each cycle is either a READ or a WRITE cycle - never both
  // an instruction is a set of "actions" executed serially


  // Some example actions
  /*       1    PC     R  fetch opcode, increment PC */
  /*       2    PC     R  fetch low address byte, increment PC */
  /*       3    PC     R  copy low address byte to PCL, fetch high address */
  /*       byte to PCH */




void execute_next_action(nes_state *state) {
  switch (state->cpu->action_queue[state->cpu->next_action]) {
      // Dummy cycle, "do nothing"
  case 0:
    break;
      /* R fetch opcode, increment PC - first cycle in all instructions*/
  case 1:
    state->cpu->current_opcode = read_mem_byte(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
      /* R  fetch low address byte, increment PC */
  case 2:
    state->cpu->low_addr_byte = read_mem_byte(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
      /* R  copy low address byte to PCL, fetch high address byte to PCH */
  case 3:
    // Read the high address first to avoid overwriting PC (having to store it)
    state->cpu->high_addr_byte = read_mem_byte(state, state->cpu->registers->PC);
    state->cpu->registers->PC =  ((uint16_t) state->cpu->low_addr_byte | (state->cpu->high_addr_byte << 8));
    break;
    // fetch value, save to destination, increment PC, affect N and Z flags
  case 4:
    *(state->cpu->destination_reg) = read_mem_byte(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    if (*state->cpu->destination_reg == 0) { set_zero_flag(state); }
    else { clear_zero_flag(state);}
    if (*state->cpu->destination_reg >= 0x80) { set_negative_flag(state); }
    else { clear_negative_flag(state); }
    break;
    // W  write register to effective address - zeropage
  case 5:
    state->memory[state->cpu->low_addr_byte] = *(state->cpu->source_reg);
    break;
    // W  push PCH on stack, decrement S
  case 6:
    push(state, (uint8_t) (state->cpu->registers->PC >> 8));
    break;
    // W  push PCL on stack, decrement S
  case 7:
    push(state, (uint8_t) (state->cpu->registers->PC));
    break;
    // set carry flag
  case 8:
    set_carry_flag(state);
    break;
    // fetch operand, increment PC
  case 9:
    state->cpu->operand = read_mem_byte(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;

    /* add operand to PCL. */
    case 10:
      state->cpu->registers->PC += state->cpu->operand;
      break;
    /* increment PC. */
    case 11:
      state->cpu->registers->PC++;
      break;
      // clear carry flag
  case 12:
    clear_carry_flag(state);
    break;
    // BIT sets the Z flag as though the value in the address tested were ANDed with the accumulator.
    // The N and V flags are set to match bits 7 and 6 respectively in the value stored at the tested address.
  case 13:
    {
      uint8_t value = read_mem_byte(state, state->cpu->low_addr_byte);
      if ((value & state->cpu->registers->ACC) == 0) { set_zero_flag(state); }
    else { clear_zero_flag(state); }
    if ((value & 128) == 128) { set_negative_flag(state); } else {clear_negative_flag(state); }
    if ((value & 64) == 64) { set_overflow_flag(state); } else {clear_overflow_flag(state); }
    }
    break;
    // Increment S (stack pointer)
  case 14:
    state->cpu->registers->SP++;
    break;
    // pull PCL from stack, increment S
  case 15:
    state->cpu->registers->PC &= 0xFF00;
    state->cpu->registers->PC |= (uint8_t) read_mem_byte(state, state->cpu->registers->SP + 0x100);
    state->cpu->registers->SP++;
    break;
    // pull PCH from stack
  case 16:
    state->cpu->registers->PC &= 0x00FF;
    state->cpu->registers->PC |= ((uint8_t) read_mem_byte(state, state->cpu->registers->SP + 0x100) << 8);
    break;
    // Pull ACC from stack (In PLA) and affect flags
  case 17:
    state->cpu->registers->ACC = read_mem_byte(state, state->cpu->registers->SP + 0x100);
    if (state->cpu->registers->ACC == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
    if (state->cpu->registers->ACC > 0x7f) { set_negative_flag(state); } else { clear_negative_flag(state); }
    break;
    // push ACC on stack, decrement S
  case 18:
    state->memory[state->cpu->registers->SP + 0x100] = state->cpu->registers->ACC;
    state->cpu->registers->SP--;
    break;
    // Pull Status register from stack (In PLP)
  case 19:
    {
      // Two instructions (PLP and RTI) pull a byte from the stack and set all the flags. They ignore bits 5 and 4.
      uint8_t value = read_mem_byte(state, state->cpu->registers->SP + 0x100);
      // Ignore bits 4 and 5
      value &= 0xcf;
      uint8_t cur_flags = state->cpu->registers->SR;
      // Keey bits 4 and 5
      cur_flags &= 0x30;
      // Join!
      cur_flags |= value;
      state->cpu->registers->SR = cur_flags;
    }
    break;
    // Push Status register to stack, decrement S
  case 20:
    /* See this note about the B flag for explanation of the OR */
/* https://wiki.nesdev.com/w/index.php/Status_flags#The_B_flag */
    state->memory[state->cpu->registers->SP + 0x100] = 48 | state->cpu->registers->SR;
    state->cpu->registers->SP--;
    break;

    // And immediate, increment PC
  case 21:
    {
      uint8_t res = state->cpu->registers->ACC & (read_mem_byte(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
      break;
      // CMP immediate
  case 22:
        {
          uint8_t acc = state->cpu->registers->ACC;
          uint8_t value = read_mem_byte(state, state->cpu->registers->PC);
          uint8_t res = acc - value;
      /* http://www.6502.org/tutorials/6502opcodes.html#CMP */
      /* Compare sets flags as if a subtraction had been carried out. */
   /* If the value in the accumulator is equal or greater than the compared value, */
      /* the Carry will be set. */
      /* The equal (Z) and negative (N) flags will be set based on equality or lack */
      /* thereof and the sign (i.e. A>=$80) of the accumulator. */
      if (acc == value) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (acc >= value) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (res >= 0x80)  { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->PC++;
    }

    break;
    // ORA immediate, increment PC
  case 23:
    {
      uint8_t res = state->cpu->registers->ACC | (read_mem_byte(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
      break;
    // EOR immediate, increment PC
  case 24:
    {
      uint8_t res = state->cpu->registers->ACC ^ (read_mem_byte(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
      break;
    // ADC immediate, increment PC
  case 25:
    {
      uint8_t acc = state->cpu->registers->ACC;
      uint8_t value = read_mem_byte(state, state->cpu->registers->PC);
      uint16_t res = ((uint16_t) acc) + ((uint16_t) value);
      if (is_carry_flag_set(state)) { res++; }
      if ((uint8_t) res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((res & 128) == 128) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (res > 255) { set_carry_flag(state);} else { clear_carry_flag(state); }
      if ((acc ^ (uint8_t) res) & (value ^ (uint8_t) res) & 0x80)
        {    set_overflow_flag(state); }
      else { clear_overflow_flag(state);}
      state->cpu->registers->ACC = (uint8_t) res;
      state->cpu->registers->PC++;
    }
      break;
      // clear carry flag
  case 90:
    clear_carry_flag(state);
    break;
      // clear zero flag
  case 91:
    clear_zero_flag(state);
    break;
      // clear interrupt flag
  case 92:
    clear_interrupt_flag(state);
    break;
      // clear decimal flag
  case 93:
    clear_decimal_flag(state);
    break;
      // clear break flag
  case 94:
    clear_break_flag(state);
    break;
      // clear overflow flag
  case 96:
    clear_overflow_flag(state);
    break;
      // clear negative flag
  case 97:
    clear_negative_flag(state);
    break;
      // set carry flag
  case 100:
    set_carry_flag(state);
    break;
      // set zero flag
  case 101:
    set_zero_flag(state);
    break;
      // set interrupt flag
  case 102:
    set_interrupt_flag(state);
    break;
      // set decimal flag
  case 103:
    set_decimal_flag(state);
    break;
      // set break flag
  case 104:
    set_break_flag(state);
    break;
      // set overflow flag
  case 106:
    set_overflow_flag(state);
    break;
      // set negative flag
  case 107:
    set_negative_flag(state);
    break;

    }

  state->cpu->next_action++;
  if (state->cpu->next_action > 9) { state->cpu->next_action = 0; }
}

void add_action_to_queue(nes_state *state, uint8_t action) {
  state->cpu->action_queue[state->cpu->end_of_queue] = action;
  state->cpu->end_of_queue++;
  if (state->cpu->end_of_queue > 9) {   state->cpu->end_of_queue = 0; }
}

void add_instruction_to_queue(nes_state *state) {
  add_action_to_queue(state, 1);
  switch (read_mem_byte(state, state->cpu->registers->PC)) {
    // PHP - Push Processor Status on stack
  case 0x08:
      /* 2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, 0);
      /*   3  $0100,S  W  push register on stack, decrement S */
    add_action_to_queue(state, 20);
    break;
    // ORA immediate
  case 0x09:
    add_action_to_queue(state, 23);
    break;
    // BPL - Branch Result Plus
  case 0x10:
    add_action_to_queue(state, 9);
    if (!is_negative_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
  case 0x18:
    add_action_to_queue(state, 12);
    break;
    // JSR
  case 0x20:
    add_action_to_queue(state, 2);
    add_action_to_queue(state, 0);
    add_action_to_queue(state, 6);
    add_action_to_queue(state, 7);
    add_action_to_queue(state, 3);
    break;
    // BIT zero page
  case 0x24:
    // fetch address, increment PC
    add_action_to_queue(state, 2);
    // Read from effective address
    add_action_to_queue(state, 13);
    break;
    // PLP - Pull Process Register (flags) from stack
  case 0x28:
/*         2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, 0);
/*         3  $0100,S  R  increment S */
    add_action_to_queue(state, 14);
/*         4  $0100,S  R  pull register from stack */
    add_action_to_queue(state, 19);
    break;

    // AND immediate
  case 0x29:
    add_action_to_queue(state, 21);
    break;
    // JMP immediate
  case 0x4C:
    add_action_to_queue(state, 2);
    add_action_to_queue(state, 3);
    break;
    // BMI - Branch Result Minus
  case 0x30:
    add_action_to_queue(state, 9);
    if (!is_zero_flag_set(state) && is_negative_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // SEC - set carry flag
  case 0x38:
    add_action_to_queue(state, 100);
    break;
    // PHA - Push ACC to stack
  case 0x48:
 /* R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, 0);
 /* W  push register on stack, decrement S */
    add_action_to_queue(state, 18);
    break;
    // EOR immediate
  case 0x49:
    add_action_to_queue(state, 24);
    break;

    // BVC - Branch Overflow clear
  case 0x50:
    add_action_to_queue(state, 9);
    if (!is_overflow_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // RTS - Return from subroutine
  case 0x60:
       /*  #  address R/W description */
       /* --- ------- --- ----------------------------------------------- */
       /*  1    PC     R  fetch opcode, increment PC */
       /*  2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, 0); // doesn't read, just drones for one cycle
       /*  3  $0100,S  R  increment S */
    add_action_to_queue(state, 14);
       /*  4  $0100,S  R  pull PCL from stack, increment S */
    add_action_to_queue(state, 15);
    /*  5  $0100,S  R  pull PCH from stack */
    add_action_to_queue(state, 16);
    /*  6    PC     R  increment PC */
    add_action_to_queue(state, 11);
    break;

    // PLA - Pull Accumulator from stack
  case 0x68:
/*         2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, 0);
/*         3  $0100,S  R  increment S */
    add_action_to_queue(state, 14);
/*         4  $0100,S  R  pull register from stack */
    add_action_to_queue(state, 17);
    break;
    // ADC Immediate
  case 0x69:
    add_action_to_queue(state, 25);
    break;

    // BVS - Branch Overflow Set
  case 0x70:
    add_action_to_queue(state, 9);
    if (is_overflow_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // SEI - Set Interrupt Flag
  case 0x78:
    add_action_to_queue(state, 102);
    break;
    // STA Zeropage
  case 0x85:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 2);
    add_action_to_queue(state, 5);
    break;
    // STX Zeropage
  case 0x86:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 2);
    add_action_to_queue(state, 5);
    break;
    // BCC - Branch Carry Clear
  case 0x90:
    add_action_to_queue(state, 9);
    if (!is_carry_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // LDX Immediate
  case 0xA2:
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 4);
    break;
    // LDX Immediate
  case 0xA9:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 4);
    break;
    // NOP
  case 0xEA:
    add_action_to_queue(state, 0);
    break;
    // BCS
  case 0xB0:
    add_action_to_queue(state, 9);
    if (is_carry_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // CLV - Clear Overflow Flag
  case 0xB8:
    add_action_to_queue(state, 96);
    break;
    // CMP Acc immediate
  case 0xC9:
    add_action_to_queue(state, 22);
    break;
    // BNE
  case 0xD0:
    add_action_to_queue(state, 9);
    if (!is_zero_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // CLD - Clear Decimal Flag
  case 0xD8:
    add_action_to_queue(state, 93);
    break;
    // BEQ
  case 0xF0:
    add_action_to_queue(state, 9);
    if (is_zero_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;
    // SED - Set Decimal Flag
  case 0xF8:
    add_action_to_queue(state, 103);
    break;
  }

}

void cpu_step(nes_state *state) {
  if (state->cpu->next_action == state->cpu->end_of_queue) {
    add_instruction_to_queue(state);
  }
  execute_next_action(state);
  state->cpu->cpu_cycle++;

}


void ppu_step(nes_state *state) {
  for (int i = 0; i < 3; i++) {
    state->ppu_cycle++;
    if (state->ppu_cycle > 340) {
      state->ppu_cycle = 0;
      state->ppu_frame++;
    }
  }
}
