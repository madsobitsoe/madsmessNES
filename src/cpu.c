#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "nes.h"
#include "logger.h"
#include "memory.h"

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
// Print the "extra info" stored in the cpu-state
void print_cpu_status(nes_state *state) {
  printf("low_addr_byte: %02X\nhigh_addr_byte: %02X\noperand: %02X\n",
         state->cpu->low_addr_byte, state->cpu->high_addr_byte,
         state->cpu->operand);
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
      printf("%s%04X\t%02X\n", spline, end - i + offset, read_mem(state, end - i + offset));
    }
    else {
      printf("%s%04X\t%02X\n", space, end - i + offset, read_mem(state, end - i + offset));
    }
  }
}



void set_pc(nes_state *state, unsigned short pc) {
  state->cpu->registers->PC = pc;
  state->cpu->current_opcode_PC = pc;
  state->cpu->current_opcode = read_mem(state, pc);
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



// Push a value to the stack
void push(nes_state *state, uint8_t value) {
  write_mem(state, state->cpu->registers->SP + 0x100, value);
  state->cpu->registers->SP--;;
}

// Instructions take 2-7 cycles.
// Each cycle is either a READ or a WRITE cycle - never both
// an instruction is a set of "actions" executed serially


// Some example actions
/*       1    PC     R  fetch opcode, increment PC */
/*       2    PC     R  fetch low address byte, increment PC */
/*       3    PC     R  copy low address byte to PCL, fetch high address */
/*       byte to PCH */
void execute_next_action(nes_state *state) {
  /*   // Add a stall cycle if page boundary crossed */
  /* if (state->cpu->page_boundary_crossed) { add_action_to_queue(state, 0); } */
  switch (state->cpu->action_queue[state->cpu->next_action]) {
    // Dummy cycle, "do nothing"
  case 0:
    break;
    /* R fetch opcode, increment PC - first cycle in all instructions*/
  case 1:
    state->cpu->current_opcode = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
    /* R  fetch low address byte, increment PC */
  case 2:
    state->cpu->low_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
    /* R  copy low address byte to PCL, fetch high address byte to PCH */
  case 3:
    // Read the high address first to avoid overwriting PC (having to store it)
    state->cpu->high_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC =  ((uint16_t) state->cpu->low_addr_byte | (state->cpu->high_addr_byte << 8));
    break;
    // fetch value, save to destination, increment PC, affect N and Z flags
  case 4:
    *(state->cpu->destination_reg) = read_mem(state, state->cpu->registers->PC);
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
    state->cpu->operand = read_mem(state, state->cpu->registers->PC);
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
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
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
    state->cpu->registers->PC |= (uint8_t) read_mem(state, state->cpu->registers->SP + 0x100);
    state->cpu->registers->SP++;
    break;
    // pull PCH from stack
  case 16:
    state->cpu->registers->PC &= 0x00FF;
    state->cpu->registers->PC |= ((uint8_t) read_mem(state, state->cpu->registers->SP + 0x100) << 8);
    break;
    // Pull ACC from stack (In PLA) and affect flags
  case 17:
    state->cpu->registers->ACC = read_mem(state, state->cpu->registers->SP + 0x100);
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
      uint8_t value = read_mem(state, state->cpu->registers->SP + 0x100);
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
      uint8_t res = state->cpu->registers->ACC & (read_mem(state, state->cpu->registers->PC));
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
      uint8_t value = read_mem(state, state->cpu->registers->PC);
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
      uint8_t res = state->cpu->registers->ACC | (read_mem(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
    break;
    // EOR immediate, increment PC
  case 24:
    {
      uint8_t res = state->cpu->registers->ACC ^ (read_mem(state, state->cpu->registers->PC));
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
      uint8_t value = read_mem(state, state->cpu->registers->PC);
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

    // CPY Immediate
  case 26:
    {
      uint8_t y = state->cpu->registers->Y;
      uint8_t value = read_mem(state, state->cpu->registers->PC);
      uint8_t res = y - value;
      /* http://www.6502.org/tutorials/6502opcodes.html#CMP */
      /* Compare sets flags as if a subtraction had been carried out. */
      /* If the value in the accumulator is equal or greater than the compared value, */
      /* the Carry will be set. */
      /* The equal (Z) and negative (N) flags will be set based on equality or lack */
      /* thereof and the sign (i.e. A>=$80) of the accumulator. */
      if (y == value) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (y >= value) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (res >= 0x80)  { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->PC++;
    }

    break;

    // CPX Immediate
  case 27:
    {
      uint8_t x = state->cpu->registers->X;
      uint8_t value = read_mem(state, state->cpu->registers->PC);
      uint8_t res = x - value;
      /* http://www.6502.org/tutorials/6502opcodes.html#CMP */
      /* Compare sets flags as if a subtraction had been carried out. */
      /* If the value in the accumulator is equal or greater than the compared value, */
      /* the Carry will be set. */
      /* The equal (Z) and negative (N) flags will be set based on equality or lack */
      /* thereof and the sign (i.e. A>=$80) of the accumulator. */
      if (x == value) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (x >= value) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (res >= 0x80)  { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->PC++;
    }

    break;

    // SBC immediate, increment PC
  case 28:
    {
      uint8_t acc = state->cpu->registers->ACC;
      // The only difference between ADC and SBC should be that SBC "complements" (negates) it's argument
      uint8_t value = ~read_mem(state, state->cpu->registers->PC);
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
    // Pull Status register from stack and increment SP (In RTI)
  case 29:
    {
      // Two instructions (PLP and RTI) pull a byte from the stack and set all the flags. They ignore bits 5 and 4.
      uint8_t value = read_mem(state, state->cpu->registers->SP + 0x100);
      // Ignore bits 4 and 5
      value &= 0xcf;
      uint8_t cur_flags = state->cpu->registers->SR;
      // Keep bits 4 and 5
      cur_flags &= 0x30;
      // Join!
      cur_flags |= value;
      state->cpu->registers->SR = cur_flags;
      state->cpu->registers->SP++;
    }
    break;
    // ORA memory
  case 30:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte << 8) | ((uint16_t) state->cpu->low_addr_byte);
      uint8_t res = state->cpu->registers->ACC | (read_mem(state, addr));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
    }
    break;
    // AND memory
  case 31:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte << 8) | ((uint16_t) state->cpu->low_addr_byte);
      uint8_t res = state->cpu->registers->ACC & (read_mem(state, addr));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
    }
    break;
    // EOR memory
  case 32:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte << 8) | ((uint16_t) state->cpu->low_addr_byte);
      uint8_t res = state->cpu->registers->ACC ^ (read_mem(state, addr));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
    }
    break;

    // ADC memory
  case 33:
    {
      uint8_t acc = state->cpu->registers->ACC;
      uint8_t value = read_mem(state, ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte);
      uint16_t res = ((uint16_t) acc) + ((uint16_t) value);
      if (is_carry_flag_set(state)) { res++; }
      if ((uint8_t) res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((res & 128) == 128) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (res > 255) { set_carry_flag(state);} else { clear_carry_flag(state); }
      if ((acc ^ (uint8_t) res) & (value ^ (uint8_t) res) & 0x80)
        {    set_overflow_flag(state); }
      else { clear_overflow_flag(state);}
      state->cpu->registers->ACC = (uint8_t) res;
    }
    break;

    // CMP/CPX/CPY memory
  case 34:
    {
      uint8_t reg = *state->cpu->source_reg;
      uint8_t value = read_mem(state, ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte);
      uint8_t res = reg - value;
      /* http://www.6502.org/tutorials/6502opcodes.html#CMP */
      /* Compare sets flags as if a subtraction had been carried out. */
      /* If the value in the accumulator is equal or greater than the compared value, */
      /* the Carry will be set. */
      /* The equal (Z) and negative (N) flags will be set based on equality or lack */
      /* thereof and the sign (i.e. A>=$80) of the accumulator. */
      if (reg == value) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (reg >= value) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (res >= 0x80)  { set_negative_flag(state); } else { clear_negative_flag(state); }
    }

    break;
    // SBC memory
  case 35:
    {
      uint8_t acc = state->cpu->registers->ACC;
      // The only difference between ADC and SBC should be that SBC "complements" (negates) it's argument
      uint8_t value = ~read_mem(state, ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte);
      uint16_t res = ((uint16_t) acc) + ((uint16_t) value);
      if (is_carry_flag_set(state)) { res++; }
      if ((uint8_t) res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((res & 128) == 128) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (res > 255) { set_carry_flag(state);} else { clear_carry_flag(state); }
      if ((acc ^ (uint8_t) res) & (value ^ (uint8_t) res) & 0x80)
        {    set_overflow_flag(state); }
      else { clear_overflow_flag(state);}
      state->cpu->registers->ACC = (uint8_t) res;
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

    // Increment source register
  case 200:
    (*state->cpu->source_reg)++;
    if (*state->cpu->source_reg == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
    if (*state->cpu->source_reg & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    break;
    // Decrement source register
  case 201:
    (*state->cpu->source_reg)--;
    if (*state->cpu->source_reg == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
    if (*state->cpu->source_reg & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    break;
    // Copy source_reg to destination_reg, affect N,Z flags
  case 202:
    (*state->cpu->destination_reg) = (*state->cpu->source_reg);
    if (*state->cpu->destination_reg == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
     if (*state->cpu->destination_reg & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
     break;
     // Copy source_reg to destination_reg, affect no flags
     case 203:
     (*state->cpu->destination_reg) = (*state->cpu->source_reg);
     break;


     // Fetch low byte of address, increment PC
  case 300:
    state->cpu->low_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    /* printf("Fetched low byte: %02X\n", state->cpu->low_addr_byte); */
    break;
    // Fetch high byte of address, increment PC
  case 301:
    state->cpu->high_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
    // Write register to effective address
  case 302:
    {
      uint16_t addr = state->cpu->high_addr_byte << 8 | state->cpu->low_addr_byte;
      /* printf("Writing %02X to %04X\n", *state->cpu->source_reg, addr); */
      state->memory[addr] = (*state->cpu->source_reg);
    }
    break;
    // Read from effective address, store in register, affect N,Z flags
  case 303:
    {
      uint16_t addr = state->cpu->high_addr_byte << 8 | state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
      (*state->cpu->destination_reg) = value;
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;
    // Read from address, add X-register to result, store in "operand"
  case 304:
    state->cpu->operand = read_mem(state, state->cpu->registers->PC - 1);
    state->cpu->operand += state->cpu->registers->X;
    break;
    // Fetch effective address low
  case 305:
    /* printf("fetching low addr_byte from %04X\n", (uint16_t) state->cpu->operand); */
    state->cpu->low_addr_byte = read_mem(state, (uint16_t) state->cpu->operand);
    break;
    // Fetch effective address high
  case 306:
    /* printf("fetching high addr_byte from %04X\n", (uint16_t) state->cpu->operand+1); */
    state->cpu->high_addr_byte = read_mem(state, (uint16_t) ((uint16_t) state->cpu->operand+1) & 0xff);

    break;

    // Fetch zeropage pointer address, store pointer in "operand", increment PC
  case 307:
    {
      state->cpu->operand = read_mem(state, state->cpu->registers->PC);
      /* printf("operand: %02X, PC: %04X\n",       state->cpu->operand, state->cpu->registers->PC); */
      state->cpu->registers->PC++;
    }
    break;
    // Increment memory
  case 308:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte;
      (state->memory[addr])++;
      if ((state->memory[addr]) == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((state->memory[addr]) & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;
    // Decrement memory
  case 309:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte;
      (state->memory[addr])--;
      if ((state->memory[addr]) == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((state->memory[addr]) & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
        break;

        // Fetch effective address high from PC+1, add Y to low byte of effective address
  case 310:
    state->cpu->high_addr_byte = read_mem(state, (uint16_t) ((uint16_t) state->cpu->registers->PC+1) & 0xff);
    state->cpu->low_addr_byte += state->cpu->registers->Y;
    break;

    // read from effective address, "fix high byte" (write to destination_reg)
  case 311:
    {
    uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
    uint8_t value = read_mem(state, addr);
      *state->cpu->destination_reg = value;
    // Set flags for LDA
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;

    // Fetch low byte of address from operand (ZP-pointer)
  case 312:
    state->cpu->low_addr_byte = read_mem(state, state->cpu->operand);
    break;
    // Fetch high byte of address from operand+1, add Y to low_addr
  case 313:
    {
      state->cpu->high_addr_byte = read_mem(state, state->cpu->operand+1);
      // Add a stall cycle if page boundary crossed
      /* This penalty applies to calculated 16bit addresses that are of the type base16 + offset, where the final memory location (base16 + offset) is in a different page than base. base16 can either be the direct or indirect version, but it'll be 16bits either way (and offset will be the contents of either x or y) */
      uint16_t base = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      uint16_t offset = state->cpu->registers->Y;
      if (((base & 0xFF) + offset) > 0xFF) {
        add_action_to_queue(state, 0); // add a stall cycle
        // fix high_addr (one cycle early, but hell)
        if (state->cpu->high_addr_byte < 0xFF) {
          state->cpu->high_addr_byte += 1;
        }
      }
      uint16_t eff_addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      eff_addr += offset;
      state->cpu->high_addr_byte = eff_addr >> 8;
      state->cpu->low_addr_byte = eff_addr & 0xFF;
    }
    break;

    // ORA read from effective address, "fix high byte" (write to destination_reg)
  case 314:
    {
    uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

    uint8_t value = (*state->cpu->destination_reg) | read_mem(state, addr);
    *state->cpu->destination_reg = value;
    // Set flags for ORA
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;

        // AND read from effective address, "fix high byte" (write to destination_reg)
  case 315:
    {
    uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

    uint8_t value = (*state->cpu->destination_reg) & read_mem(state, addr);
      *state->cpu->destination_reg = value;
    // Set flags for AND
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;
    // EOR read from effective address, "fix high byte" (write to destination_reg)
  case 316:
    {
    uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

    uint8_t value = (*state->cpu->destination_reg) ^ read_mem(state, addr);
    *state->cpu->destination_reg = value;
    // Set flags for ORA
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;

    // ADC read from effective address, "fix high byte" (write to destination_reg)
  case 317:
    {
      uint8_t acc = *state->cpu->destination_reg;
      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      uint8_t value = read_mem(state, addr);
      uint16_t res = ((uint16_t) acc) + ((uint16_t) value);
      // Set flags for ADC
      if (is_carry_flag_set(state)) { res++; }
      if ((uint8_t) res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((res & 128) == 128) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (res > 255) { set_carry_flag(state);} else { clear_carry_flag(state); }
      if ((acc ^ (uint8_t) res) & (value ^ (uint8_t) res) & 0x80)
        {    set_overflow_flag(state); }
      else { clear_overflow_flag(state);}
      *state->cpu->destination_reg = (uint8_t) res;
    }
    break;

    // CMP read from effective address, "fix high byte" (write to destination_reg)
  case 318:
    {
      uint8_t reg = *state->cpu->destination_reg;
      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      uint8_t value = read_mem(state, addr);
      uint8_t res = reg - value;
      /* http://www.6502.org/tutorials/6502opcodes.html#CMP */
      /* Compare sets flags as if a subtraction had been carried out. */
      /* If the value in the accumulator is equal or greater than the compared value, */
      /* the Carry will be set. */
      /* The equal (Z) and negative (N) flags will be set based on equality or lack */
      /* thereof and the sign (i.e. A>=$80) of the accumulator. */
      if (reg == value) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (reg >= value) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (res >= 0x80)  { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
      break;
    // SDC read from effective address, "fix high byte" (write to destination_reg)
  case 319:
    {
      uint8_t acc = *state->cpu->destination_reg;
      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      uint8_t value = ~(read_mem(state, addr));
      uint16_t res = ((uint16_t) acc) + ((uint16_t) value);
      // Set flags for ADC
      if (is_carry_flag_set(state)) { res++; }
      if ((uint8_t) res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((res & 128) == 128) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (res > 255) { set_carry_flag(state);} else { clear_carry_flag(state); }
      if ((acc ^ (uint8_t) res) & (value ^ (uint8_t) res) & 0x80)
        {    set_overflow_flag(state); }
      else { clear_overflow_flag(state);}
      *state->cpu->destination_reg = (uint8_t) res;
    }
    break;
    // STA/STX/STY read from effective address, "fix high byte" (write to destination_reg)
  case 320:
    {

      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

      state->memory[addr] = *state->cpu->source_reg;
    }
    break;
    // STA/SHA - Fetch high byte of address from operand+1, add Y to low_addr
    // The "extra boundary cycle" is always added
  case 321:
    {
      state->cpu->high_addr_byte = read_mem(state, state->cpu->operand+1);

      /* This penalty applies to calculated 16bit addresses that are of the type base16 + offset, where the final memory location (base16 + offset) is in a different page than base. base16 can either be the direct or indirect version, but it'll be 16bits either way (and offset will be the contents of either x or y) */
      uint16_t base = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      uint16_t offset = state->cpu->registers->Y;
      if (((base & 0xFF) + offset) > 0xFF) {
        // fix high_addr (one cycle early, but hell)
        if (state->cpu->high_addr_byte < 0xFF) {
        state->cpu->high_addr_byte += 1;
        }
      }
        uint16_t eff_addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
        eff_addr += offset;
        state->cpu->high_addr_byte = eff_addr >> 8;
        state->cpu->low_addr_byte = eff_addr & 0xFF;
    }
    break;

    // R  fetch low address to "latch"
  case 322:
    // use "operand" as latch
    {
      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      state->cpu->operand = read_mem(state,addr);
    }
    break;
    // R  fetch PCH, copy "latch" to PCL
  case 323:
    // use "operand" as latch
    {
      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      // Ensure we fetch from the same page as PCL!
      uint16_t addr_pch = addr + 1;
      if ((addr & 0xff00) == (addr_pch & 0xff00)) {
        // No page change, just increment
          addr++;
        }
      // else, wraparound
      else { addr &= 0xff00; }
      uint16_t pch = (uint16_t) read_mem(state,addr);
      state->cpu->registers->PC = ((uint16_t) state->cpu->operand) | (pch << 8);
    }
    break;

    // LSR A
  case 400:
    if (state->cpu->registers->ACC & 0x1) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    state->cpu->registers->ACC = state->cpu->registers->ACC >> 1;
    clear_negative_flag(state);
    if (state->cpu->registers->ACC != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    // ASL A
  case 401:
    if (state->cpu->registers->ACC & 0x80) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    state->cpu->registers->ACC = state->cpu->registers->ACC << 1;
    if (state->cpu->registers->ACC & 0x80) { set_negative_flag(state); } else {
      clear_negative_flag(state);
    }
    if (state->cpu->registers->ACC != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    // ROR A
  case 402:
    {
      uint8_t newacc = state->cpu->registers->ACC;
      bool carry = is_carry_flag_set(state);
      uint8_t lsb = newacc & 1;
      newacc = newacc >> 1;
      if (carry) { newacc |= 0x80; set_negative_flag(state); } else { clear_negative_flag(state); }
      if (lsb) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (newacc == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      state->cpu->registers->ACC = newacc;
    }
    break;
    // ROL A
  case 403:
    {
      uint8_t newacc = state->cpu->registers->ACC;
      bool carry = is_carry_flag_set(state);
      uint8_t msb = newacc & 0x80;
      newacc = newacc << 1;
      if (carry) { newacc |= 0x1; }
      if (newacc & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (msb) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (newacc == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      state->cpu->registers->ACC = newacc;
    }
    break;
    // LSR (reg and zero-page Memory)
  case 404:
    if (*state->cpu->source_reg & 0x1) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    *state->cpu->source_reg = *state->cpu->source_reg >> 1;
    clear_negative_flag(state);
    if (*state->cpu->source_reg != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    // ASL source-reg
  case 405:
    if (*state->cpu->source_reg & 0x80) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    *state->cpu->source_reg = *state->cpu->source_reg << 1;
    if (*state->cpu->source_reg & 0x80) { set_negative_flag(state); } else {
      clear_negative_flag(state);
    }
    if (*state->cpu->source_reg != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    // ROR source-reg (zeropage)
  case 406:
    {
      uint8_t newval = *state->cpu->source_reg;
      bool carry = is_carry_flag_set(state);
      uint8_t lsb = newval & 1;
      newval = newval >> 1;
      if (carry) { newval |= 0x80; set_negative_flag(state); } else { clear_negative_flag(state); }
      if (lsb) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (newval == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      *state->cpu->source_reg = newval;
    }
    break;
    // ROL source_reg
  case 407:
    {
      uint8_t newacc = *state->cpu->source_reg;
      bool carry = is_carry_flag_set(state);
      uint8_t msb = newacc & 0x80;
      newacc = newacc << 1;
      if (carry) { newacc |= 0x1; }
      if (newacc & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (msb) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (newacc == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      *state->cpu->source_reg = newacc;
    }
    break;
    // LSR (memory)
  case 408:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t)state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
      if (value & 0x1) { set_carry_flag(state); }
      else { clear_carry_flag(state); }
      value = value >> 1;
      clear_negative_flag(state);
      if (value != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
      state->memory[addr] = value;
    }
    break;
    // ASL memory
  case 409:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t)state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
      if (value & 0x80) { set_carry_flag(state); }
      else { clear_carry_flag(state); }
      value = value << 1;
      if (value & 0x80) { set_negative_flag(state); } else {
        clear_negative_flag(state);
      }
      if (value != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
      state->memory[addr] = value;
    }
    break;
    // ROR Memory
  case 410:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t)state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
      bool carry = is_carry_flag_set(state);
      uint8_t lsb = value & 1;
      value = value >> 1;
      if (carry) { value |= 0x80; set_negative_flag(state); } else { clear_negative_flag(state); }
      if (lsb) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      state->memory[addr] = value;
    }
    break;
    // ROL Memory
  case 411:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t)state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
      bool carry = is_carry_flag_set(state);
      uint8_t msb = value & 0x80;
      value = value << 1;
      if (carry) { value |= 0x1; }
      if (value & 0x80 ) { set_negative_flag(state); } else { clear_negative_flag(state); }
      if (msb) { set_carry_flag(state); } else { clear_carry_flag(state); }
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      write_mem(state, addr, value);
    }
    break;
  }

  state->cpu->next_action++;
  if (state->cpu->next_action > 9) { state->cpu->next_action = 0; }
}

void add_action_to_queue(nes_state *state, uint16_t action) {
  state->cpu->action_queue[state->cpu->end_of_queue] = action;
  state->cpu->end_of_queue++;
  if (state->cpu->end_of_queue > 9) {   state->cpu->end_of_queue = 0; }
}

void add_instruction_to_queue(nes_state *state) {
  add_action_to_queue(state, 1);
  switch (read_mem(state, state->cpu->registers->PC)) {
    // ORA indexed indirect
  case 0x01:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 30);
    break;
    // ORA Zero page
  case 0x05:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 30);
    break;
    // ASL Zeropage
  case 0x06:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, 0); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, 405);
    break;
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
    // ASL A
  case 0x0A:
    add_action_to_queue(state, 401);
    break;
    // ORA Absolute
  case 0x0D:
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 30);
    break;
    // ASL Absolute
  case 0x0E:
    /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
    /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall
    /*        5  address  W  write the value back to effective address, */
    /*                       and do the operation on it */
    add_action_to_queue(state, 0); // Stall
    /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, 409);
    break;
    // BPL - Branch Result Plus
  case 0x10:
    add_action_to_queue(state, 9);
    if (!is_negative_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // ORA indirect-indexed, Y
  case 0x11:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
    /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
    /*       4   pointer+1   R  fetch effective address high, */
    /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
    /*       5   address+Y*  R  read from effective address, */
    /*                          fix high byte of effective address */
    add_action_to_queue(state, 314);
    /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;
    // CLC
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
    // AND indexed indirect
  case 0x21:
    /*   2      PC       R  fetch pointer address, increment PC */
    /*   3    pointer    R  read from the address, add X to it */
    /*   4   pointer+X   R  fetch effective address low */
    /*   5  pointer+X+1  R  fetch effective address high */
    /*   6    address    R  read from effective address */

    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    //    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 31);
    break;
    // BIT zero page
  case 0x24:
    // fetch address, increment PC
    state->cpu->high_addr_byte = 0x0;
    add_action_to_queue(state, 2);
    // Read from effective address
    add_action_to_queue(state, 13);
    break;
    // AND zeropage
  case 0x25:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 31);
    break;
    // ROL Zeropage
  case 0x26:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, 0); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, 407);
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
    // ROL A
  case 0x2A:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 407);
    break;
    // BIT Absolute
  case 0x2C:
    /* state->cpu->destination_reg = &state->cpu->registers->Y; */
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 13);
    break;

    // AND Absolute
  case 0x2D:
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 31);
    break;
    // ROL Absolute
  case 0x2E:
 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
 /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall
 /*        5  address  W  write the value back to effective address, */
    add_action_to_queue(state, 0); // Stall
    /*                       and do the operation on it */
 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, 411);
    break;
    // BMI - Branch Result Minus
  case 0x30:
    add_action_to_queue(state, 9);
    if (!is_zero_flag_set(state) && is_negative_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // AND indirect-indexed, Y
  case 0x31:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, 315);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
  /*      Notes: The effective address is always fetched from zero page, */
  /*             i.e. the zero page boundary crossing is not handled. */

  /*             * The high byte of the effective address may be invalid */
  /*               at this time, i.e. it may be smaller by $100. */

  /*             + This cycle will be executed only if the effective address */
  /*               was invalid during cycle #5, i.e. page boundary was crossed. */
    break;


    // SEC - set carry flag
  case 0x38:
    add_action_to_queue(state, 100);
    break;
    // RTI - Return from interrupt
  case 0x40:
    /* 2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, 0); // doesn't read, just drones for one cycle
    /*   3  $0100,S  R  increment S */
    add_action_to_queue(state, 14);
    /*   4  $0100,S  R  pull P from stack, increment S */
    add_action_to_queue(state, 29);

    /*   5  $0100,S  R  pull PCL from stack, increment S */
    add_action_to_queue(state, 15);
    /*   6  $0100,S  R  pull PCH from stack */
    add_action_to_queue(state, 16);

    break;
    // AND indexed indirect
  case 0x41:
    /*   2      PC       R  fetch pointer address, increment PC */
    /*   3    pointer    R  read from the address, add X to it */
    /*   4   pointer+X   R  fetch effective address low */
    /*   5  pointer+X+1  R  fetch effective address high */
    /*   6    address    R  read from effective address */

    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    //    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 32);
    break;
    // EOR zeropage
  case 0x45:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 32);
    break;

    // LSR Zeropage
  case 0x46:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, 0); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, 404);
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

    // LSR A - Logical Shift Right accumulator
  case 0x4A:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 404);
    break;

    // JMP immediate
  case 0x4C:
    add_action_to_queue(state, 2);
    add_action_to_queue(state, 3);
    break;
    // EOR Absolute
  case 0x4D:
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 32);
    break;
    // LSR Absolute
  case 0x4E:


 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
 /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall
 /*        5  address  W  write the value back to effective address, */
    add_action_to_queue(state, 0); // Stall
    /*                       and do the operation on it */

 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, 408);
    break;

    // BVC - Branch Overflow clear
  case 0x50:
    add_action_to_queue(state, 9);
    if (!is_overflow_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // EOR indirect-indexed, Y
  case 0x51:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, 316);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
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
    // ADC indexed indirect
  case 0x61:
    /*   2      PC       R  fetch pointer address, increment PC */
    /*   3    pointer    R  read from the address, add X to it */
    /*   4   pointer+X   R  fetch effective address low */
    /*   5  pointer+X+1  R  fetch effective address high */
    /*   6    address    R  read from effective address */

    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    //    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 33);
    break;
    // ADC zeropage
  case 0x65:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 33);
    break;
    // ROR Zeropage
  case 0x66:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, 0); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, 406);
    break;

    // ADC Immediate
  case 0x69:
    add_action_to_queue(state, 25);
    break;
    // ROR A
  case 0x6A:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 406);
    /* add_action_to_queue(state, 402); */
    break;


    // JMP Absolute indirect
  case 0x6C:
    /* #   address  R/W description */
    /*     --- --------- --- ------------------------------------------ */
    /*     1     PC      R  fetch opcode, increment PC */
    /*       2     PC      R  fetch pointer address low, increment PC */
    add_action_to_queue(state, 300);
    /*       3     PC      R  fetch pointer address high, increment PC */
    add_action_to_queue(state, 301);
    /*       4   pointer   R  fetch low address to latch */
    add_action_to_queue(state, 322);
    /*       5  pointer+1* R  fetch PCH, copy latch to PCL */
    add_action_to_queue(state, 323);
    /* Note: * The PCH will always be fetched from the same page */
    /*     than PCL, i.e. page boundary crossing is not handled. */


    break;

    // ADC Absolute
  case 0x6D:
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 33);
    break;
    // ROR Absolute
  case 0x6E:
    /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
    /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall
    /*        5  address  W  write the value back to effective address, */
    add_action_to_queue(state, 0); // Stall
    /*                       and do the operation on it */

    /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, 410);
    break;

    // BVS - Branch Overflow Set
  case 0x70:
    add_action_to_queue(state, 9);
    if (is_overflow_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // ADC indirect-indexed, Y
  case 0x71:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, 317);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;


    // SEI - Set Interrupt Flag
  case 0x78:
    add_action_to_queue(state, 102);
    break;

    // STA indirect, X (indexed indirect)
  case 0x81:
    /*        2      PC       R  fetch pointer address, increment PC */
    /*        3    pointer    R  read from the address, add X to it */
    /*        4   pointer+X   R  fetch effective address low */
    /*        5  pointer+X+1  R  fetch effective address high */
    /*        6    address    W  write to effective address */

    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 302);

    break;
    // STY Zeropage
  case 0x84:
    state->cpu->source_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 2);
    add_action_to_queue(state, 5);
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
    // DEC - Decrement Y register
  case 0x88:
    state->cpu->source_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 201);
    break;
    // STY Absolute
  case 0x8C:
    state->cpu->source_reg = &state->cpu->registers->Y;
    /*       2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
    /*       3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*       4  address  W  write register to effective address */
    add_action_to_queue(state, 302);
    break;
    // STA Absolute
  case 0x8D:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    /*       2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
    /*       3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*       4  address  W  write register to effective address */
    add_action_to_queue(state, 302);
    break;
    // STX Absolute
  case 0x8E:
    state->cpu->source_reg = &state->cpu->registers->X;
    /*       2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
    /*       3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*       4  address  W  write register to effective address */
    add_action_to_queue(state, 302);
    break;
    // TXA
  case 0x8A:
    state->cpu->source_reg = &state->cpu->registers->X;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 202);
    break;
    // BCC - Branch Carry Clear
  case 0x90:
    add_action_to_queue(state, 9);
    if (!is_carry_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // STA indirect-indexed, Y
  case 0x91:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    state->cpu->source_reg = &state->cpu->registers->ACC;
   /* #    address   R/W description */
   /*     --- ----------- --- ------------------------------------------ */
   /*      1      PC       R  fetch opcode, increment PC */
   /*      2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
   /*      3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
    /*      4   pointer+1   R  fetch effective address high, */
   /*                         add Y to low byte of effective address */
    add_action_to_queue(state, 321);
    add_action_to_queue(state, 0);
    /*      5   address+Y*  R  read from effective address, */
   /*                         fix high byte of effective address */

   /*      6   address+Y   W  write to effective address */
    add_action_to_queue(state, 320);
   /*     Notes: The effective address is always fetched from zero page, */
   /*            i.e. the zero page boundary crossing is not handled. */

   /*            * The high byte of the effective address may be invalid */
   /*              at this time, i.e. it may be smaller by $100. */
    /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */

    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */

  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */

  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */

  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;


    // TYA
  case 0x98:
    state->cpu->source_reg = &state->cpu->registers->Y;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 202);
    break;
    // TXS - Transfer X to Stack Pointer, affect no flags
  case 0x9A:
    state->cpu->source_reg = &state->cpu->registers->X;
    state->cpu->destination_reg = &state->cpu->registers->SP;
    add_action_to_queue(state, 203);
    break;
    // LDY Immediate
  case 0xA0:
    state->cpu->destination_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 4);
    break;
    // LDA indirect,x
  case 0xA1:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /* add_action_to_queue(state, 307); */
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    R  read from effective address */
    add_action_to_queue(state, 303);
    break;

    // LDX Immediate
  case 0xA2:
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 4);
    break;
    // LDY Zero page
  case 0xA4:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->Y;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */

    add_action_to_queue(state, 303);
    break;
    // LDA Zero page
  case 0xA5:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */

    add_action_to_queue(state, 303);
    break;
    // LDX Zero page
  case 0xA6:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->X;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */

    add_action_to_queue(state, 303);
    break;

    // TAY
  case 0xA8:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 202);
    break;
    // LDA Immediate
  case 0xA9:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 4);
    break;
    // TAX
  case 0xAA:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 202);
    break;
    // LDY Absolute
  case 0xAC:
    state->cpu->destination_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 303);
    break;
    // LDA Absolute
  case 0xAD:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 303);
    break;
    // LDX Absolute
  case 0xAE:
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 303);
    break;
    // TSX - Transfer SP to X
  case 0xBA:
    state->cpu->source_reg = &state->cpu->registers->SP;
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 202);
    break;
    // CPY Immediate
  case 0xC0:
    add_action_to_queue(state, 26);
    break;
    // BCS
  case 0xB0:
    add_action_to_queue(state, 9);
    if (is_carry_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // LDA indirect-indexed, Y
  case 0xB1:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, 311);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
  /*      Notes: The effective address is always fetched from zero page, */
  /*             i.e. the zero page boundary crossing is not handled. */

  /*             * The high byte of the effective address may be invalid */
  /*               at this time, i.e. it may be smaller by $100. */

  /*             + This cycle will be executed only if the effective address */
  /*               was invalid during cycle #5, i.e. page boundary was crossed. */
    break;
    // CLV - Clear Overflow Flag
  case 0xB8:
    add_action_to_queue(state, 96);
    break;
    // CMP indexed indirect
  case 0xC1:
    /*   2      PC       R  fetch pointer address, increment PC */
    /*   3    pointer    R  read from the address, add X to it */
    /*   4   pointer+X   R  fetch effective address low */
    /*   5  pointer+X+1  R  fetch effective address high */
    /*   6    address    R  read from effective address */

    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    //    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 34);
    break;
    // CPY zeropage
  case 0xC4:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->Y;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 34);
    break;
    // CMP zeropage
  case 0xC5:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 34);
    break;
    // DEC Zeropage
  case 0xC6:
    state->cpu->high_addr_byte = 0x0;
    // This should probably happen after the actions :(
    /* state->cpu->source_reg = &state->memory[ state->cpu->low_addr_byte]; */
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, 0); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, 309);
    break;

    // INY - Increment Y register
  case 0xC8:
    state->cpu->source_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 200);
    break;
    // CMP Acc immediate
  case 0xC9:
    add_action_to_queue(state, 22);
    break;
    // DEX - Decrement X register
  case 0xCA:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 201);
    break;
    // CPY Absolute
  case 0xCC:
    state->cpu->source_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 34);
    break;
    // CMP Absolute
  case 0xCD:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 34);
    break;
    // DEC Absolute
  case 0xCE:
 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall, no need to read
 /*        5  address  W  write the value back to effective address, */
 /*                       and do the operation on it */
    add_action_to_queue(state, 0); // Stall, no need to read
 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, 309);
    break;
    // BNE
  case 0xD0:
    add_action_to_queue(state, 9);
    if (!is_zero_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // CMP indirect-indexed, Y
  case 0xD1:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, 318);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;

    // CLD - Clear Decimal Flag
  case 0xD8:
    add_action_to_queue(state, 93);
    break;
    // CPX Immediate
  case 0xE0:
    add_action_to_queue(state, 27);
    break;
    // SBC indexed indirect
  case 0xE1:
    /*   2      PC       R  fetch pointer address, increment PC */
    /*   3    pointer    R  read from the address, add X to it */
    /*   4   pointer+X   R  fetch effective address low */
    /*   5  pointer+X+1  R  fetch effective address high */
    /*   6    address    R  read from effective address */

    state->cpu->high_addr_byte = 0x0;
    state->cpu->low_addr_byte = 0x0;
    //    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 11); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, 304);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, 305);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, 306);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, 35);
    break;
    // CPX zeropage
  case 0xE4:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->X;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 34);
    break;
    // SBC zeropage
  case 0xE5:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 35);
    break;
    // INC Zeropage
  case 0xE6:
    state->cpu->high_addr_byte = 0x0;
    // This should probably happen after the actions :(
    /* state->cpu->source_reg = &state->memory[ state->cpu->low_addr_byte]; */
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, 0); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, 308);
    break;

    // INX - Increment X register
  case 0xE8:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 200);
    break;
    // SBC Immediate
  case 0xE9:
    add_action_to_queue(state, 28);
    break;
    // NOP
  case 0xEA:
    add_action_to_queue(state, 0);
    break;
// SBC Immediate (Illegal opcode)
  case 0xEB:
    add_action_to_queue(state, 28);
    break;
    // CPX Absolute
  case 0xEC:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 34);
    break;
    // SBC Absolute
  case 0xED:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, 300);
    add_action_to_queue(state, 301);
    // Read from effective address, copy to register
    add_action_to_queue(state, 35);
    break;
    // INC Absolute
  case 0xEE:
 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, 300);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, 301);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall, no need to read
 /*        5  address  W  write the value back to effective address, */
 /*                       and do the operation on it */
    add_action_to_queue(state, 0); // Stall, no need to read
 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, 308);
    break;
    // BEQ
  case 0xF0:
    add_action_to_queue(state, 9);
    if (is_zero_flag_set(state)) {
      add_action_to_queue(state, 10);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // SBC indirect-indexed, Y
  case 0xF1:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, 307);
    /* add_action_to_queue(state, 11); // increment PC, nowhere to store pointer */
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, 312);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, 313);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, 319);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;


    // SED - Set Decimal Flag
  case 0xF8:
    add_action_to_queue(state, 103);
    break;
    // unimplemented instruction is a fatal error
    // Flag it so emulation can be stopped
  default:
    state->fatal_error = true;
    state->running = false;
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
