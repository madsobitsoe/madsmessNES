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
  switch (state->cpu->action_queue[state->cpu->next_action]) {
    // Dummy cycle, "do nothing"
  case STALL_CYCLE:
    break;
    /* R fetch opcode, increment PC - first cycle in all instructions*/
  case FETCH_OPCODE_INC_PC:
    state->cpu->current_opcode = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
    /* R  fetch low address byte, increment PC */
  case FETCH_LOW_ADDR_BYTE_INC_PC:
    state->cpu->low_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
    /* R  copy low address byte to PCL, fetch high address byte to PCH */
  case COPY_LOW_ADDR_BYTE_TO_PCL_FETCH_HIGH_ADDR_BYTE_TO_PCH:
    // Read the high address first to avoid overwriting PC (having to store it)
    state->cpu->high_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC =  ((uint16_t) state->cpu->low_addr_byte | (state->cpu->high_addr_byte << 8));
    break;
    // fetch value, save to destination, increment PC, affect N and Z flags
  case FETCH_VALUE_SAVE_TO_DEST:
    *(state->cpu->destination_reg) = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    if (*state->cpu->destination_reg == 0) { set_zero_flag(state); }
    else { clear_zero_flag(state);}
    if (*state->cpu->destination_reg >= 0x80) { set_negative_flag(state); }
    else { clear_negative_flag(state); }
    break;
    // W  write register to effective address - zeropage
  case WRITE_REG_TO_EFF_ADDR_ZEROPAGE:
    state->memory[state->cpu->low_addr_byte] = *(state->cpu->source_reg);
    break;
    // W  push PCH on stack, decrement S
  case PUSH_PCH_DEC_S:
    push(state, (uint8_t) (state->cpu->registers->PC >> 8));
    break;
    // W  push PCL on stack, decrement S
  case PUSH_PCL_DEC_S:
    push(state, (uint8_t) (state->cpu->registers->PC));
    break;
    // fetch operand, increment PC
  case FETCH_OPERAND_INC_PC:
    state->cpu->operand = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;

    /* add operand to PCL. */
  case ADD_OPERAND_TO_PCL:
    state->cpu->registers->PC += state->cpu->operand;
    break;
    /* increment PC. */
  case INC_PC:
    state->cpu->registers->PC++;
    break;
    // BIT sets the Z flag as though the value in the address tested were ANDed with the accumulator.
    // The N and V flags are set to match bits 7 and 6 respectively in the value stored at the tested address.
  case BIT_READ_AFFECT_FLAGS:
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
  case INC_SP:
    state->cpu->registers->SP++;
    break;
    // pull PCL from stack, increment S
  case PULL_PCL_FROM_STACK_INC_SP:
    state->cpu->registers->PC &= 0xFF00;
    state->cpu->registers->PC |= (uint8_t) read_mem(state, state->cpu->registers->SP + 0x100);
    state->cpu->registers->SP++;
    break;
    // pull PCH from stack
  case PULL_PCH_FROM_STACK:
    state->cpu->registers->PC &= 0x00FF;
    state->cpu->registers->PC |= ((uint8_t) read_mem(state, state->cpu->registers->SP + 0x100) << 8);
    break;
    // Pull ACC from stack (In PLA) and affect flags
  case PULL_ACC_FROM_STACK_AFFECT_FLAGS:
    state->cpu->registers->ACC = read_mem(state, state->cpu->registers->SP + 0x100);
    if (state->cpu->registers->ACC == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
    if (state->cpu->registers->ACC > 0x7f) { set_negative_flag(state); } else { clear_negative_flag(state); }
    break;
    // push ACC on stack, decrement S
  case PUSH_ACC_DEC_SP:
    state->memory[state->cpu->registers->SP + 0x100] = state->cpu->registers->ACC;
    state->cpu->registers->SP--;
    break;
    // Pull Status register from stack (In PLP and RTI)
  case PULL_STATUS_REG_FROM_STACK_PLP:
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
  case PUSH_STATUS_REG_DEC_SP:
    /* See this note about the B flag for explanation of the OR */
    /* https://wiki.nesdev.com/w/index.php/Status_flags#The_B_flag */
    state->memory[state->cpu->registers->SP + 0x100] = 48 | state->cpu->registers->SR;
    state->cpu->registers->SP--;
    break;

    // And immediate, increment PC
  case AND_IMM_INC_PC:
    {
      uint8_t res = state->cpu->registers->ACC & (read_mem(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
    break;
    // CMP immediate
  case CMP_IMM_INC_PC:
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
  case ORA_IMM_INC_PC:
    {
      uint8_t res = state->cpu->registers->ACC | (read_mem(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
    break;
    // EOR immediate, increment PC
  case EOR_IMM_INC_PC:
    {
      uint8_t res = state->cpu->registers->ACC ^ (read_mem(state, state->cpu->registers->PC));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
      state->cpu->registers->PC++;
    }
    break;
    // ADC immediate, increment PC
  case ADC_IMM_INC_PC:
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
  case CPY_IMM_INC_PC:
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
  case CPX_IMM_INC_PC:
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
  case SBC_IMM_INC_PC:
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
  case PULL_STATUS_REG_FROM_STACK_RTI:
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
  case ORA_MEMORY:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte << 8) | ((uint16_t) state->cpu->low_addr_byte);
      uint8_t res = state->cpu->registers->ACC | (read_mem(state, addr));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
    }
    break;
    // AND memory
  case AND_MEMORY:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte << 8) | ((uint16_t) state->cpu->low_addr_byte);
      uint8_t res = state->cpu->registers->ACC & (read_mem(state, addr));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
    }
    break;
    // EOR memory
  case EOR_MEMORY:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte << 8) | ((uint16_t) state->cpu->low_addr_byte);
      uint8_t res = state->cpu->registers->ACC ^ (read_mem(state, addr));
      if (res == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (res > 0x7F) { set_negative_flag(state); } else { clear_negative_flag(state); }
      state->cpu->registers->ACC = res;
    }
    break;

    // ADC memory
  case ADC_MEMORY:
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
  case CMP_CPX_CPY_MEMORY:
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
    // CMP/CPX/CPY memory
  case CMP_MEMORY:
    {
      uint8_t reg = state->cpu->registers->ACC;
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
  case SBC_MEMORY:
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
  case CLEAR_CARRY_FLAG:
    clear_carry_flag(state);
    break;
    // clear zero flag
  case CLEAR_ZERO_FLAG:
    clear_zero_flag(state);
    break;
    // clear interrupt flag
  case CLEAR_INTERRUPT_FLAG:
    clear_interrupt_flag(state);
    break;
    // clear decimal flag
  case CLEAR_DECIMAL_FLAG:
    clear_decimal_flag(state);
    break;
    // clear break flag
  case CLEAR_BREAK_FLAG:
    clear_break_flag(state);
    break;
    // clear overflow flag
  case CLEAR_OVERFLOW_FLAG:
    clear_overflow_flag(state);
    break;
    // clear negative flag
  case CLEAR_NEGATIVE_FLAG:
    clear_negative_flag(state);
    break;
    // set carry flag
  case SET_CARRY_FLAG:
    set_carry_flag(state);
    break;
    // set zero flag
  case SET_ZERO_FLAG:
    set_zero_flag(state);
    break;
    // set interrupt flag
  case SET_INTERRUPT_FLAG:
    set_interrupt_flag(state);
    break;
    // set decimal flag
  case SET_DECIMAL_FLAG:
    set_decimal_flag(state);
    break;
    // set break flag
  case SET_BREAK_FLAG:
    set_break_flag(state);
    break;
    // set overflow flag
  case SET_OVERFLOW_FLAG:
    set_overflow_flag(state);
    break;
    // set negative flag
  case SET_NEGATIVE_FLAG:
    set_negative_flag(state);
    break;

    // Increment source register
  case INC_SOURCE_REG:
    (*state->cpu->source_reg)++;
    if (*state->cpu->source_reg == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
    if (*state->cpu->source_reg & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    break;
    // Decrement source register
  case DEC_SOURCE_REG:
    (*state->cpu->source_reg)--;
    if (*state->cpu->source_reg == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
    if (*state->cpu->source_reg & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    break;
    // Copy source_reg to destination_reg, affect N,Z flags
  case COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS:
    (*state->cpu->destination_reg) = (*state->cpu->source_reg);
    if (*state->cpu->destination_reg == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
     if (*state->cpu->destination_reg & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
     break;
     // Copy source_reg to destination_reg, affect no flags
  case COPY_SOURCE_REG_TO_DEST_REG_NO_FLAGS:
     (*state->cpu->destination_reg) = (*state->cpu->source_reg);
     break;

    
    // Fetch high byte of address, increment PC
  case FETCH_HIGH_ADDR_BYTE_INC_PC:
    state->cpu->high_addr_byte = read_mem(state, state->cpu->registers->PC);
    state->cpu->registers->PC++;
    break;
    // Write register to effective address - non-zero page
  case WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE:
    {
      uint16_t addr = state->cpu->high_addr_byte << 8 | state->cpu->low_addr_byte;

      state->memory[addr] = (*state->cpu->source_reg);
    }
    break;
    // Read from effective address, store in register, affect N,Z flags
  case READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS:
    {
      uint16_t addr = state->cpu->high_addr_byte << 8 | state->cpu->low_addr_byte;
      uint8_t value = read_mem(state, addr);
      (*state->cpu->destination_reg) = value;
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;
    // Read from address, add X-register to result, store in "operand"
  case READ_ADDR_ADD_INDEX_STORE_IN_OPERAND:
    state->cpu->operand = read_mem(state, state->cpu->registers->PC - 1);
    state->cpu->operand += *state->cpu->index_reg;
    break;
    
    // Fetch effective address low
  case FETCH_EFF_ADDR_LOW:
    state->cpu->low_addr_byte = read_mem(state, (uint16_t) state->cpu->operand);
    break;
    // Fetch effective address high
  case FETCH_EFF_ADDR_HIGH:
    /* printf("fetching high addr_byte from %04X\n", (uint16_t) state->cpu->operand+1); */
    state->cpu->high_addr_byte = read_mem(state, (uint16_t) ((uint16_t) state->cpu->operand+1) & 0xff);

    break;

    // Fetch zeropage pointer address, store pointer in "operand", increment PC
  case FETCH_ZP_PTR_ADDR_INC_PC:
    {
      state->cpu->operand = read_mem(state, state->cpu->registers->PC);
      state->cpu->registers->PC++;
    }
    break;
    
    // Increment memory
  case INC_MEMORY:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte;
      (state->memory[addr])++;
      if ((state->memory[addr]) == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((state->memory[addr]) & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;
    
    // Decrement memory
  case DEC_MEMORY:
    {
      uint16_t addr = ((uint16_t) state->cpu->high_addr_byte) << 8 | (uint16_t) state->cpu->low_addr_byte;
      (state->memory[addr])--;
      if ((state->memory[addr]) == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if ((state->memory[addr]) & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
        break;

  // Fetch effective address high from PC+1, add index to low byte of effective address, inc pc
  case FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC:
    state->cpu->high_addr_byte = read_mem(state, (uint16_t) ((uint16_t) state->cpu->registers->PC));
    // Fix high address, one cycle early
    // Add a stall cycle if page boundary is crossed
    if (((uint16_t)state->cpu->low_addr_byte + (uint16_t)*state->cpu->index_reg) > 0xFF) {
	state->cpu->high_addr_byte++;
	add_action_to_queue(state, STALL_CYCLE);
    }
    state->cpu->low_addr_byte += *state->cpu->index_reg;
    state->cpu->registers->PC++;
    break;

    // read from effective address, "fix high byte" (write to destination_reg)
  case READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
    {

	uint16_t eff_addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
	uint8_t value = read_mem(state, eff_addr);
	*state->cpu->destination_reg = value;
	// Set flags for LDA
	if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
	if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;

    // Fetch low byte of address from operand (ZP-pointer)
  case FETCH_LOW_BYTE_ADDR_ZP:
    state->cpu->low_addr_byte = read_mem(state, state->cpu->operand);
    break;
    
    // Fetch high byte of address from operand+1, add index to low_addr
  case FETCH_HIGH_BYTE_ADDR_ADD_INDEX:
    {
      state->cpu->high_addr_byte = read_mem(state, state->cpu->operand+1);
      // Add a stall cycle if page boundary crossed
      /* This penalty applies to calculated 16bit addresses that are of the type base16 + offset, where the final memory location (base16 + offset) is in a different page than base. base16 can either be the direct or indirect version, but it'll be 16bits either way (and offset will be the contents of either x or y) */
      uint16_t base = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      uint16_t offset = *state->cpu->index_reg;
      if (((base & 0xFF) + offset) > 0xFF) {
        add_action_to_queue(state, STALL_CYCLE); // add a stall cycle
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
  case ORA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
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
  case AND_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
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
  case EOR_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
    {
    uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

    uint8_t value = (*state->cpu->destination_reg) ^ read_mem(state, addr);
    *state->cpu->destination_reg = value;
    // Set flags for EOR
      if (value == 0) { set_zero_flag(state); } else { clear_zero_flag(state); }
      if (value & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); }
    }
    break;

    // ADC read from effective address, "fix high byte" (write to destination_reg)
  case ADC_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
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
  case CMP_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
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
      
    // SBC read from effective address, "fix high byte" (write to destination_reg)
  case SBC_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
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

    // STA read from effective address, "fix high byte" (write to destination_reg)
  case STA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
    {

      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

      state->memory[addr] = *state->cpu->destination_reg;
    }
    break;

    
    // STA/STX/STY read from effective address, "fix high byte" (write to destination_reg)
  case STA_STX_STY_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE:
    {

      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);

      state->memory[addr] = *state->cpu->source_reg;
    }
    break;

    // R  fetch low address to "latch"
  case FETCH_LOW_ADDR_TO_LATCH:
    // use "operand" as latch
    {
      uint16_t addr = ((uint16_t) state->cpu->low_addr_byte) | (((uint16_t) state->cpu->high_addr_byte) << 8);
      state->cpu->operand = read_mem(state,addr);
    }
    break;
    // R  fetch PCH, copy "latch" to PCL
  case FETCH_PCH_COPY_LATCH_TO_PCL:
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
  case LSR_ACC:
    if (state->cpu->registers->ACC & 0x1) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    state->cpu->registers->ACC = state->cpu->registers->ACC >> 1;
    clear_negative_flag(state);
    if (state->cpu->registers->ACC != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    
    // ASL A
  case ASL_ACC:
    if (state->cpu->registers->ACC & 0x80) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    state->cpu->registers->ACC = state->cpu->registers->ACC << 1;
    if (state->cpu->registers->ACC & 0x80) { set_negative_flag(state); } else {
      clear_negative_flag(state);
    }
    if (state->cpu->registers->ACC != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    
  /*   // ROR A */
  /* case ROR_ACC: */
  /*   { */
  /*     uint8_t newacc = state->cpu->registers->ACC; */
  /*     bool carry = is_carry_flag_set(state); */
  /*     uint8_t lsb = newacc & 1; */
  /*     newacc = newacc >> 1; */
  /*     if (carry) { newacc |= 0x80; set_negative_flag(state); } else { clear_negative_flag(state); } */
  /*     if (lsb) { set_carry_flag(state); } else { clear_carry_flag(state); } */
  /*     if (newacc == 0) { set_zero_flag(state); } else { clear_zero_flag(state); } */
  /*     state->cpu->registers->ACC = newacc; */
  /*   } */
  /*   break; */
    // ROL A
  /* case ROL_ACC: */
  /*   { */
  /*     uint8_t newacc = state->cpu->registers->ACC; */
  /*     bool carry = is_carry_flag_set(state); */
  /*     uint8_t msb = newacc & 0x80; */
  /*     newacc = newacc << 1; */
  /*     if (carry) { newacc |= 0x1; } */
  /*     if (newacc & 0x80) { set_negative_flag(state); } else { clear_negative_flag(state); } */
  /*     if (msb) { set_carry_flag(state); } else { clear_carry_flag(state); } */
  /*     if (newacc == 0) { set_zero_flag(state); } else { clear_zero_flag(state); } */
  /*     state->cpu->registers->ACC = newacc; */
  /*   } */
  /*   break; */

    
    // LSR (reg and zero-page Memory)
  case LSR_SOURCE_REG:
    if (*state->cpu->source_reg & 0x1) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    *state->cpu->source_reg = *state->cpu->source_reg >> 1;
    clear_negative_flag(state);
    if (*state->cpu->source_reg != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    // ASL source-reg
  case ASL_SOURCE_REG:
    if (*state->cpu->source_reg & 0x80) { set_carry_flag(state); }
    else { clear_carry_flag(state); }
    *state->cpu->source_reg = *state->cpu->source_reg << 1;
    if (*state->cpu->source_reg & 0x80) { set_negative_flag(state); } else {
      clear_negative_flag(state);
    }
    if (*state->cpu->source_reg != 0) { clear_zero_flag(state); } else { set_zero_flag(state); }
    break;
    
    // ROR source-reg (zeropage)
  case ROR_SOURCE_REG_ZP:
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
  case ROL_SOURCE_REG:
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
  case LSR_MEMORY:
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
  case ASL_MEMORY:
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
  case ROR_MEMORY:
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
  case ROL_MEMORY:
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
    
  case ZEROPAGE_ADD_INDEX:
      state->cpu->low_addr_byte += *state->cpu->index_reg;
      state->cpu->high_addr_byte = 0;
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
    add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, ORA_MEMORY);
    break;
    // ORA Zero page
  case 0x05:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, ORA_MEMORY);
    break;
    // ASL Zeropage
  case 0x06:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, STALL_CYCLE); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, ASL_SOURCE_REG);
    break;
    // PHP - Push Processor Status on stack
  case 0x08:
    /* 2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, STALL_CYCLE);
    /*   3  $0100,S  W  push register on stack, decrement S */
    add_action_to_queue(state, PUSH_STATUS_REG_DEC_SP);
    break;
    // ORA immediate
  case 0x09:
    add_action_to_queue(state, ORA_IMM_INC_PC);
    break;
    // ASL A
  case 0x0A:
    add_action_to_queue(state, ASL_ACC);
    break;
    // ORA Absolute
  case 0x0D:
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, ORA_MEMORY);
    break;
    // ASL Absolute
  case 0x0E:
    /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall
    /*        5  address  W  write the value back to effective address, */
    /*                       and do the operation on it */
    add_action_to_queue(state, STALL_CYCLE); // Stall
    /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, ASL_MEMORY);
    break;
    // BPL - Branch Result Plus
  case 0x10:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (!is_negative_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // ORA indirect-indexed, Y
  case 0x11:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
    /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
    /*       4   pointer+1   R  fetch effective address high, */
    /*                          add Y to low byte of effective address */
    add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
    /*       5   address+Y*  R  read from effective address, */
    /*                          fix high byte of effective address */
    add_action_to_queue(state, ORA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
    /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;
// ORA zeropage, X
  case 0x15:
        /* 2     PC      R  fetch address, increment PC */
    state->cpu->high_addr_byte = 0x0;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
        /* 3   address   R  read from address, add index register to it */
    add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+I* R  read from effective address */
    add_action_to_queue(state, ORA_MEMORY);
      break;

// ASL zero page, X
  case 0x16:
      state->cpu->high_addr_byte = 0;
        /* 2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
        /* 3   address   R  read from address, add index register X to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+X* R  read from effective address */
      add_action_to_queue(state, STALL_CYCLE);
        /* 5  address+X* W  write the value back to effective address, */
        /*                  and do the operation on it */
      add_action_to_queue(state, STALL_CYCLE);      
        /* 6  address+X* W  write the new value to effective address */
      add_action_to_queue(state, ASL_MEMORY);

      break;


      
    // CLC
  case 0x18:
    add_action_to_queue(state, CLEAR_CARRY_FLAG);
    break;
    // ORA absolute, Y
  case 0x19:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->Y;
    
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
    add_action_to_queue(state, ORA_MEMORY);
      
      break;


    // ORA absolute, X
  case 0x1D:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->X;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);      
      add_action_to_queue(state, ORA_MEMORY);      
      break;
      
    // JSR
  case 0x20:
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, STALL_CYCLE);
    add_action_to_queue(state, PUSH_PCH_DEC_S);
    add_action_to_queue(state, PUSH_PCL_DEC_S);
    add_action_to_queue(state, COPY_LOW_ADDR_BYTE_TO_PCL_FETCH_HIGH_ADDR_BYTE_TO_PCH);
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
    add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, AND_MEMORY);
    break;
    // BIT zero page
  case 0x24:
    // fetch address, increment PC
    state->cpu->high_addr_byte = 0x0;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    // Read from effective address
    add_action_to_queue(state, BIT_READ_AFFECT_FLAGS);
    break;
    // AND zeropage
  case 0x25:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, AND_MEMORY);
    break;
    // ROL Zeropage
  case 0x26:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, STALL_CYCLE); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, ROL_SOURCE_REG);
    break;
    // PLP - Pull Process Register (flags) from stack
  case 0x28:
/*         2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, STALL_CYCLE);
/*         3  $0100,S  R  increment S */
    add_action_to_queue(state, INC_SP);
/*         4  $0100,S  R  pull register from stack */
    add_action_to_queue(state, PULL_STATUS_REG_FROM_STACK_PLP);
    break;

    // AND immediate
  case 0x29:
    add_action_to_queue(state, AND_IMM_INC_PC);
    break;
    // ROL A
  case 0x2A:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, ROL_SOURCE_REG);
    break;
    // BIT Absolute
  case 0x2C:
    /* state->cpu->destination_reg = &state->cpu->registers->Y; */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, BIT_READ_AFFECT_FLAGS);
    break;

    // AND Absolute
  case 0x2D:
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, AND_MEMORY);
    break;
    // ROL Absolute
  case 0x2E:
 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
 /*        4  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall
 /*        5  address  W  write the value back to effective address, */
    add_action_to_queue(state, STALL_CYCLE); // Stall
    /*                       and do the operation on it */
 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, ROL_MEMORY);
    break;
    // BMI - Branch Result Minus
  case 0x30:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (!is_zero_flag_set(state) && is_negative_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
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
    add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, AND_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
  /*      Notes: The effective address is always fetched from zero page, */
  /*             i.e. the zero page boundary crossing is not handled. */

  /*             * The high byte of the effective address may be invalid */
  /*               at this time, i.e. it may be smaller by $100. */

  /*             + This cycle will be executed only if the effective address */
  /*               was invalid during cycle #5, i.e. page boundary was crossed. */
    break;

  // AND zeropage, X
  case 0x35:
      /* 2     PC      R  fetch address, increment PC */
      state->cpu->high_addr_byte = 0x0;
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
      /* 3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
      /* 4  address+I* R  read from effective address */
      add_action_to_queue(state, AND_MEMORY);
      break;

// ROL zero page, X
  case 0x36:
      state->cpu->high_addr_byte = 0;
        /* 2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
        /* 3   address   R  read from address, add index register X to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+X* R  read from effective address */
      add_action_to_queue(state, STALL_CYCLE);
        /* 5  address+X* W  write the value back to effective address, */
        /*                  and do the operation on it */
      add_action_to_queue(state, STALL_CYCLE);      
        /* 6  address+X* W  write the new value to effective address */
      add_action_to_queue(state, ROL_MEMORY);

      break;



      
    // SEC - set carry flag
  case 0x38:
    add_action_to_queue(state, SET_CARRY_FLAG);
    break;

    // AND absolute, Y
  case 0x39:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->Y;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, AND_MEMORY);
      
      break;

    // AND absolute, X
  case 0x3D:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->X;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, AND_MEMORY);
      break;

      
    
    // RTI - Return from interrupt
  case 0x40:
    /* 2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, STALL_CYCLE); // doesn't read, just drones for one cycle
    /*   3  $0100,S  R  increment S */
    add_action_to_queue(state, INC_SP);
    /*   4  $0100,S  R  pull P from stack, increment S */
    add_action_to_queue(state, PULL_STATUS_REG_FROM_STACK_RTI);

    /*   5  $0100,S  R  pull PCL from stack, increment S */
    add_action_to_queue(state, PULL_PCL_FROM_STACK_INC_SP);
    /*   6  $0100,S  R  pull PCH from stack */
    add_action_to_queue(state, PULL_PCH_FROM_STACK);
    break;
    
    // EOR indexed indirect
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
    add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, EOR_MEMORY);
    break;
    // EOR zeropage
  case 0x45:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, 32);
    break;

    // LSR Zeropage
  case 0x46:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, STALL_CYCLE); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, LSR_SOURCE_REG);
    break;

    // PHA - Push ACC to stack
  case 0x48:
    /* R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, STALL_CYCLE);
    /* W  push register on stack, decrement S */
    add_action_to_queue(state, PUSH_ACC_DEC_SP);
    break;
    // EOR immediate
  case 0x49:
    add_action_to_queue(state, EOR_IMM_INC_PC);
    break;

    // LSR A - Logical Shift Right accumulator
  case 0x4A:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, LSR_SOURCE_REG);
    break;

    // JMP immediate
  case 0x4C:
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, COPY_LOW_ADDR_BYTE_TO_PCL_FETCH_HIGH_ADDR_BYTE_TO_PCH);
    break;
    // EOR Absolute
  case 0x4D:
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, 32);
    break;
    // LSR Absolute
  case 0x4E:


 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
 /*        4  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall
 /*        5  address  W  write the value back to effective address, */
    add_action_to_queue(state, STALL_CYCLE); // Stall
    /*                       and do the operation on it */

 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, LSR_MEMORY);
    break;

    // BVC - Branch Overflow clear
  case 0x50:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (!is_overflow_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
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
    add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);

  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, EOR_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;
  // EOR zeropage, X
  case 0x55:
      /* 2     PC      R  fetch address, increment PC */
      state->cpu->high_addr_byte = 0x0;
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
      /* 3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
      /* 4  address+I* R  read from effective address */
      add_action_to_queue(state, EOR_MEMORY);
      break;


// LSR zero page, X
  case 0x56:
      state->cpu->high_addr_byte = 0;
        /* 2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
        /* 3   address   R  read from address, add index register X to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+X* R  read from effective address */
      add_action_to_queue(state, STALL_CYCLE);
        /* 5  address+X* W  write the value back to effective address, */
        /*                  and do the operation on it */
      add_action_to_queue(state, STALL_CYCLE);      
        /* 6  address+X* W  write the new value to effective address */
      add_action_to_queue(state, LSR_MEMORY);

      break;



      
    // EOR absolute, Y
  case 0x59:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->Y;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, EOR_MEMORY);
      
      break;

    // EOR absolute, X
  case 0x5D:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->X;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, EOR_MEMORY);
      break;

      
    // RTS - Return from subroutine
  case 0x60:
    /*  #  address R/W description */
    /* --- ------- --- ----------------------------------------------- */
    /*  1    PC     R  fetch opcode, increment PC */
    /*  2    PC     R  read next instruction byte (and throw it away) */
    add_action_to_queue(state, STALL_CYCLE); // doesn't read, just drones for one cycle
    /*  3  $0100,S  R  increment S */
    add_action_to_queue(state, INC_SP);
    /*  4  $0100,S  R  pull PCL from stack, increment S */
add_action_to_queue(state, PULL_PCL_FROM_STACK_INC_SP);
                        /*  5  $0100,S  R  pull PCH from stack */
                        add_action_to_queue(state, PULL_PCH_FROM_STACK);
                        /*  6    PC     R  increment PC */
                        add_action_to_queue(state, INC_PC);
                        break;

                        // PLA - Pull Accumulator from stack
                        case 0x68:
                        /*         2    PC     R  read next instruction byte (and throw it away) */
                        add_action_to_queue(state, STALL_CYCLE);
/*         3  $0100,S  R  increment S */
    add_action_to_queue(state, INC_SP);
/*         4  $0100,S  R  pull register from stack */
    add_action_to_queue(state, PULL_ACC_FROM_STACK_AFFECT_FLAGS);
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
    add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, ADC_MEMORY);
    break;
    // ADC zeropage
  case 0x65:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, ADC_MEMORY);
    break;
    // ROR Zeropage
  case 0x66:
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &(state->memory[state->cpu->low_addr_byte]);
    /*   2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*   3  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall for one cycle, no need to read
    /*   4  address  W  write the value back to effective address, */
    /*   and do the operation on it */
    add_action_to_queue(state, STALL_CYCLE); // // Stall for one cycle, no need to write
    /*   5  address  W  write the new value to effective address */
    add_action_to_queue(state, ROR_SOURCE_REG_ZP);
    break;

    // ADC Immediate
  case 0x69:
    add_action_to_queue(state, ADC_IMM_INC_PC);
    break;
    // ROR A
  case 0x6A:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, ROR_SOURCE_REG_ZP);

    break;


    // JMP Absolute indirect
  case 0x6C:
    /* #   address  R/W description */
    /*     --- --------- --- ------------------------------------------ */
    /*     1     PC      R  fetch opcode, increment PC */
    /*       2     PC      R  fetch pointer address low, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3     PC      R  fetch pointer address high, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*       4   pointer   R  fetch low address to latch */
    add_action_to_queue(state, FETCH_LOW_ADDR_TO_LATCH);
    /*       5  pointer+1* R  fetch PCH, copy latch to PCL */
    add_action_to_queue(state, FETCH_PCH_COPY_LATCH_TO_PCL);
    /* Note: * The PCH will always be fetched from the same page */
    /*     than PCL, i.e. page boundary crossing is not handled. */


    break;

    // ADC Absolute
  case 0x6D:
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, ADC_MEMORY);
    break;
    // ROR Absolute
  case 0x6E:
    /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall
    /*        5  address  W  write the value back to effective address, */
    add_action_to_queue(state, STALL_CYCLE); // Stall
    /*                       and do the operation on it */

    /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, ROR_MEMORY);
    break;

    // BVS - Branch Overflow Set
  case 0x70:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (is_overflow_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
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
    add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, ADC_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;

  // ADC zeropage, X
  case 0x75:
      /* 2     PC      R  fetch address, increment PC */
      state->cpu->high_addr_byte = 0x0;
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
      /* 3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
      /* 4  address+I* R  read from effective address */
      add_action_to_queue(state, ADC_MEMORY);
      break;

// ROR zero page, X
  case 0x76:
      state->cpu->high_addr_byte = 0;
        /* 2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
        /* 3   address   R  read from address, add index register X to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+X* R  read from effective address */
      add_action_to_queue(state, STALL_CYCLE);
        /* 5  address+X* W  write the value back to effective address, */
        /*                  and do the operation on it */
      add_action_to_queue(state, STALL_CYCLE);      
        /* 6  address+X* W  write the new value to effective address */
      add_action_to_queue(state, ROR_MEMORY);

      break;



      
    // SEI - Set Interrupt Flag
  case 0x78:
    add_action_to_queue(state, SET_INTERRUPT_FLAG);
    break;


    // ADC absolute, Y
  case 0x79:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->Y;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, ADC_MEMORY);
      
      break;

    // ADC absolute, X
  case 0x7D:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->X;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, ADC_MEMORY);
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
    add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
    /*   6    address    W  write ACC to effective address */
    add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE);

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
    add_action_to_queue(state, DEC_SOURCE_REG);
    break;
    // STY Absolute
  case 0x8C:
    state->cpu->source_reg = &state->cpu->registers->Y;
    /*       2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*       4  address  W  write register to effective address */
    add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE);
    break;

    // STA Absolute
  case 0x8D:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    /*       2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*       4  address  W  write register to effective address */
    add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE);
    break;
    // STX Absolute
  case 0x8E:
    state->cpu->source_reg = &state->cpu->registers->X;
    /*       2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    /*       3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*       4  address  W  write register to effective address */
    add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE);
    break;
    // TXA
  case 0x8A:
    state->cpu->source_reg = &state->cpu->registers->X;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS);
    break;
    // BCC - Branch Carry Clear
  case 0x90:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (!is_carry_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
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
    add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
   /*      3    pointer    R  fetch effective address low */
    add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
    /*      4   pointer+1   R  fetch effective address high, */
   /*                         add Y to low byte of effective address */

    add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
    add_action_to_queue(state, STALL_CYCLE);
    /*      5   address+Y*  R  read from effective address, */
   /*                         fix high byte of effective address */

   /*      6   address+Y   W  write to effective address */
    add_action_to_queue(state, STA_STX_STY_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
   /*     Notes: The effective address is always fetched from zero page, */
   /*            i.e. the zero page boundary crossing is not handled. */

   /*            * The high byte of the effective address may be invalid */
   /*              at this time, i.e. it may be smaller by $100. */
    /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */

  /*       3    pointer    R  fetch effective address low */

  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */

  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */

  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;


// STY zero page, X
  case 0x94:
        /* 2     PC      R  fetch address, increment PC */
        /* 3   address   R  read from address, add index register to it */
        /* 4  address+I* W  write to effective address */

      state->cpu->source_reg = &state->cpu->registers->Y;
       /*  2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
       /*  3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
       /*  4  address+I* R  Write to effective address */
      add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_ZEROPAGE);
       /* Notes: I denotes either index register (X or Y). */

       /*        * The high byte of the effective address is always zero, */
       /*          i.e. page boundary crossings are not handled. */

      break;


// STA zero page, X
  case 0x95:
        /* 2     PC      R  fetch address, increment PC */
        /* 3   address   R  read from address, add index register to it */
        /* 4  address+I* W  write to effective address */

      state->cpu->source_reg = &state->cpu->registers->ACC;
       /*  2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
       /*  3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
       /*  4  address+I* R  Write to effective address */
      add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_ZEROPAGE);
       /* Notes: I denotes either index register (X or Y). */

       /*        * The high byte of the effective address is always zero, */
       /*          i.e. page boundary crossings are not handled. */

      break;
// STX zero page, Y
  case 0x96:
        /* 2     PC      R  fetch address, increment PC */
        /* 3   address   R  read from address, add index register to it */
        /* 4  address+I* W  write to effective address */

      state->cpu->source_reg = &state->cpu->registers->X;
       /*  2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
       /*  3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
       /*  4  address+I* R  Write to effective address */
      add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_ZEROPAGE);
       /* Notes: I denotes either index register (X or Y). */

       /*        * The high byte of the effective address is always zero, */
       /*          i.e. page boundary crossings are not handled. */

      break;
      
    
    // TYA
  case 0x98:
    state->cpu->source_reg = &state->cpu->registers->Y;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS);
    break;
    // STA absolute, Y
  case 0x99:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->source_reg = &state->cpu->registers->Y;
    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      // Add a stall-cycle as we don't do the read before the write.
      add_action_to_queue(state, STALL_CYCLE);
      add_action_to_queue(state, STA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
      
      break;
    

    
    // TXS - Transfer X to Stack Pointer, affect no flags
  case 0x9A:
    state->cpu->source_reg = &state->cpu->registers->X;
    state->cpu->destination_reg = &state->cpu->registers->SP;
    add_action_to_queue(state, COPY_SOURCE_REG_TO_DEST_REG_NO_FLAGS);
    break;

    // STA Absolute, X
  case 0x9D:
       /*  2     PC      R  fetch low byte of address, increment PC */
       /*  3     PC      R  fetch high byte of address, add index register to low address byte, increment PC */
       /*  4  address+I* R  read from effective address, fix the high byte of effective address */
       /*  5  address+I  W  write to effective address */
       /* Notes: I denotes either index register (X or Y). */
      
      state->cpu->source_reg = &state->cpu->registers->ACC;
      state->cpu->index_reg = &state->cpu->registers->X;
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, STA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
      add_action_to_queue(state, WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE);
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
    state->cpu->index_reg = &state->cpu->registers->X;
    /* 2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
    /*   3    pointer    R  read from the address, add X to it */
    add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
    /*   4   pointer+X   R  fetch effective address low */
    add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
    /*   5  pointer+X+1  R  fetch effective address high */
    add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
    /*   6    address    R  read from effective address */
    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
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

    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
    break;
    
    // LDA Zero page
  case 0xA5:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */

    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
    break;
    
    // LDX Zero page
  case 0xA6:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->X;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */

    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
    break;

    // TAY
  case 0xA8:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    state->cpu->destination_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS);
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
    add_action_to_queue(state, COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS);
    break;
    
    // LDY Absolute
  case 0xAC:
    state->cpu->destination_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
    break;
    
    // LDA Absolute
  case 0xAD:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
    break;
    
    // LDX Absolute
  case 0xAE:
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
    break;
    
    // TSX - Transfer SP to X
  case 0xBA:
    state->cpu->source_reg = &state->cpu->registers->SP;
    state->cpu->destination_reg = &state->cpu->registers->X;
    add_action_to_queue(state, COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS);
    break;
    
    // CPY Immediate
  case 0xC0:
    add_action_to_queue(state, CPY_IMM_INC_PC);
    break;
    
    // BCS
  case 0xB0:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (is_carry_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // LDA indirect-indexed, Y
  case 0xB1:
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    state->cpu->index_reg = &state->cpu->registers->Y;
  /* #    address   R/W description */
  /*      --- ----------- --- ------------------------------------------ */
  /*       1      PC       R  fetch opcode, increment PC */
  /*       2      PC       R  fetch pointer address, increment PC */
    add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
  /*       3    pointer    R  fetch effective address low */
    add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
  /*       4   pointer+1   R  fetch effective address high, */
  /*                          add Y to low byte of effective address */
    add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
  /*       5   address+Y*  R  read from effective address, */
  /*                          fix high byte of effective address */
    add_action_to_queue(state, READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
  /*      Notes: The effective address is always fetched from zero page, */
  /*             i.e. the zero page boundary crossing is not handled. */

  /*             * The high byte of the effective address may be invalid */
  /*               at this time, i.e. it may be smaller by $100. */

  /*             + This cycle will be executed only if the effective address */
  /*               was invalid during cycle #5, i.e. page boundary was crossed. */
    break;
    
// LDY zero page, X
  case 0xB4:
      state->cpu->index_reg = &state->cpu->registers->X;
      /* state->cpu->source_reg = &state->cpu->registers->X; */
      state->cpu->destination_reg = &state->cpu->registers->Y;
       /*  2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
       /*  3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
       /*  4  address+I* R  read from effective address */
      add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
       /* Notes: I denotes either index register (X or Y). */

       /*        * The high byte of the effective address is always zero, */
       /*          i.e. page boundary crossings are not handled. */

      break;

// LDA zero page, X
  case 0xB5:
      state->cpu->index_reg = &state->cpu->registers->X;
      
      /* state->cpu->source_reg = &state->cpu->registers->X; */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
       /*  2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
       /*  3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
       /*  4  address+I* R  read from effective address */
      add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
       /* Notes: I denotes either index register (X or Y). */
       /*        * The high byte of the effective address is always zero, */
       /*          i.e. page boundary crossings are not handled. */
      break;


// LDX zero page, Y
  case 0xB6:
      state->cpu->index_reg = &state->cpu->registers->Y;
      
      /* state->cpu->source_reg = &state->cpu->registers->Y; */
      state->cpu->destination_reg = &state->cpu->registers->X;
       /*  2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
       /*  3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
       /*  4  address+I* R  read from effective address */
      add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
       /* Notes: I denotes either index register (X or Y). */

       /*        * The high byte of the effective address is always zero, */
       /*          i.e. page boundary crossings are not handled. */

      break;

      

// CLV - Clear Overflow Flag
  case 0xB8:
    add_action_to_queue(state, CLEAR_OVERFLOW_FLAG);
    break;
    
// LDA Indexed Absolute Y
  case 0xB9:
      state->cpu->index_reg = &state->cpu->registers->Y;
      
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* state->cpu->source_reg = &state->cpu->registers->Y; */
 /* 2     PC      R  fetch low byte of address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
 /*        3     PC      R  fetch high byte of address, add index register to low address byte, increment PC */
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
 /*        4  address+Y* R  read from effective address,fix the high byte of effective address */

      add_action_to_queue(state, READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
      /* add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);       */
 /*        5+ address+Y  R  re-read from effective address */
 /*              * The high byte of the effective address may be invalid */
 /*                at this time, i.e. it may be smaller by $100. */
 /*              + This cycle will be executed only if the effective address */
 /*                was invalid during cycle #4, i.e. page boundary was crossed. */      

      break;

// LDY Indexed Absolute X
  case 0xBC:
      state->cpu->index_reg = &state->cpu->registers->X;      
      state->cpu->destination_reg = &state->cpu->registers->Y;
      /* state->cpu->source_reg = &state->cpu->registers->X; */
 /* 2     PC      R  fetch low byte of address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
 /*        3     PC      R  fetch high byte of address, add index register to low address byte, increment PC */
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
 /*        4  address+Y* R  read from effective address,fix the high byte of effective address */

      add_action_to_queue(state, READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
      /* add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);       */
 /*        5+ address+Y  R  re-read from effective address */
 /*              * The high byte of the effective address may be invalid */
 /*                at this time, i.e. it may be smaller by $100. */
 /*              + This cycle will be executed only if the effective address */
 /*                was invalid during cycle #4, i.e. page boundary was crossed. */      
      break;

    // LDA absolute, X
  case 0xBD:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* state->cpu->source_reg = &state->cpu->registers->X; */
      state->cpu->index_reg = &state->cpu->registers->X;
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS);
      break;

      
    // CMP indexed indirect
  case 0xC1:
    /*   2      PC       R  fetch pointer address, increment PC */
    /*   3    pointer    R  read from the address, add X to it */
    /*   4   pointer+X   R  fetch effective address low */
    /*   5  pointer+X+1  R  fetch effective address high */
    /*   6    address    R  read from effective address */

      state->cpu->index_reg = &state->cpu->registers->X;
      state->cpu->high_addr_byte = 0x0;
      state->cpu->low_addr_byte = 0x0;
      //    state->cpu->source_reg = &state->cpu->registers->ACC;
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* 2      PC       R  fetch pointer address, increment PC */
      add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
      /*   3    pointer    R  read from the address, add X to it */
      add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
      /*   4   pointer+X   R  fetch effective address low */
      add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
      /*   5  pointer+X+1  R  fetch effective address high */
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
      /*   6    address    W  write ACC to effective address */
      add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
    break;
    
    // CPY zeropage
  case 0xC4:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->Y;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
    break;
    // CMP zeropage
  case 0xC5:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
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
    add_action_to_queue(state, DEC_MEMORY);
    break;

    // INY - Increment Y register
  case 0xC8:
    state->cpu->source_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, INC_SOURCE_REG);
    break;
    // CMP Acc immediate
  case 0xC9:
    add_action_to_queue(state, CMP_IMM_INC_PC);
    break;
    // DEX - Decrement X register
  case 0xCA:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, DEC_SOURCE_REG);
    break;
    // CPY Absolute
  case 0xCC:
    state->cpu->source_reg = &state->cpu->registers->Y;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
    break;
    // CMP Absolute
  case 0xCD:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
    break;
    
    // DEC Absolute
  case 0xCE:
 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, 0); // Stall, no need to read
 /*        5  address  W  write the value back to effective address, */
 /*                       and do the operation on it */
    add_action_to_queue(state, 0); // Stall, no need to read
 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, DEC_MEMORY);
    break;
    // BNE
  case 0xD0:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (!is_zero_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // CMP indirect-indexed, Y
  case 0xD1:
      state->cpu->index_reg = &state->cpu->registers->Y;
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* #    address   R/W description */
      /*      --- ----------- --- ------------------------------------------ */
      /*       1      PC       R  fetch opcode, increment PC */
      /*       2      PC       R  fetch pointer address, increment PC */
      add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
      /*       3    pointer    R  fetch effective address low */
      add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
      /*       4   pointer+1   R  fetch effective address high, */
      /*                          add Y to low byte of effective address */
      add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
      /*       5   address+Y*  R  read from effective address, */
      /*                          fix high byte of effective address */
      add_action_to_queue(state, CMP_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
  /*       6+  address+Y   R  read from effective address */
    // ^Will be added in 313 if necessary
    break;

  // CMP zeropage, X
  case 0xD5:
      /* 2     PC      R  fetch address, increment PC */
      state->cpu->index_reg = &state->cpu->registers->X;      
      state->cpu->high_addr_byte = 0x0;
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
      /* 3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
      /* 4  address+I* R  read from effective address */
      add_action_to_queue(state, CMP_MEMORY);
      break;

// DEC zero page, X
  case 0xD6:
      state->cpu->index_reg = &state->cpu->registers->X;      
      state->cpu->high_addr_byte = 0;
        /* 2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
        /* 3   address   R  read from address, add index register X to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+X* R  read from effective address */
      add_action_to_queue(state, STALL_CYCLE);
        /* 5  address+X* W  write the value back to effective address, */
        /*                  and do the operation on it */
      add_action_to_queue(state, STALL_CYCLE);      
        /* 6  address+X* W  write the new value to effective address */
      add_action_to_queue(state, DEC_MEMORY);

      break;


      
    // CLD - Clear Decimal Flag
  case 0xD8:
    add_action_to_queue(state, CLEAR_DECIMAL_FLAG);
    break;

    // CMP absolute, Y
  case 0xD9:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* state->cpu->source_reg = &state->cpu->registers->Y; */
      state->cpu->index_reg = &state->cpu->registers->Y;          
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      
      add_action_to_queue(state, CMP_MEMORY);
      break;
    
    // CMP absolute, X
  case 0xDD:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* state->cpu->source_reg = &state->cpu->registers->X; */
      state->cpu->index_reg = &state->cpu->registers->X;
      
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, CMP_MEMORY);
      break;
    
    
    // CPX Immediate
  case 0xE0:
    add_action_to_queue(state, CPX_IMM_INC_PC);
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
      state->cpu->index_reg = &state->cpu->registers->X;    
      /* 2      PC       R  fetch pointer address, increment PC */
      add_action_to_queue(state, INC_PC); // increment PC, nowhere to store pointer
      /*   3    pointer    R  read from the address, add X to it */
      add_action_to_queue(state, READ_ADDR_ADD_INDEX_STORE_IN_OPERAND);
      /*   4   pointer+X   R  fetch effective address low */
      add_action_to_queue(state, FETCH_EFF_ADDR_LOW);
      /*   5  pointer+X+1  R  fetch effective address high */
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH);
      /*   6    address    W  write ACC to effective address */
      add_action_to_queue(state, SBC_MEMORY);
    break;
    
    // CPX zeropage
  case 0xE4:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->source_reg = &state->cpu->registers->X;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
    break;
    
    // SBC zeropage
  case 0xE5:
    // Clear out high addr byte, to ensure zero-page read
    state->cpu->high_addr_byte = 0x0;
    state->cpu->destination_reg = &state->cpu->registers->ACC;
    /* 2    PC     R  fetch address, increment PC */
    add_action_to_queue(state, 2);
    /*       3  address  R  read from effective address */
    add_action_to_queue(state, SBC_MEMORY);
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
    add_action_to_queue(state, INC_MEMORY);
    break;

    // INX - Increment X register
  case 0xE8:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, INC_SOURCE_REG);
    break;
    
    // SBC Immediate
  case 0xE9:
    add_action_to_queue(state, SBC_IMM_INC_PC);
    break;
    
    // NOP
  case 0xEA:
    add_action_to_queue(state, STALL_CYCLE);
    break;
    
// SBC Immediate (Illegal opcode)
  case 0xEB:
    add_action_to_queue(state, SBC_IMM_INC_PC);
    break;
    
    // CPX Absolute
  case 0xEC:
    state->cpu->source_reg = &state->cpu->registers->X;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, CMP_CPX_CPY_MEMORY);
    break;
    
    // SBC Absolute
  case 0xED:
    state->cpu->source_reg = &state->cpu->registers->ACC;
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    // Read from effective address, copy to register
    add_action_to_queue(state, SBC_MEMORY);
    break;
    
    // INC Absolute
  case 0xEE:
 /* 2    PC     R  fetch low byte of address, increment PC */
    add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
 /*        3    PC     R  fetch high byte of address, increment PC */
    add_action_to_queue(state, FETCH_HIGH_ADDR_BYTE_INC_PC);
    /*        4  address  R  read from effective address */
    add_action_to_queue(state, STALL_CYCLE); // Stall, no need to read
 /*        5  address  W  write the value back to effective address, */
 /*                       and do the operation on it */
    add_action_to_queue(state, STALL_CYCLE); // Stall, no need to read
 /*        6  address  W  write the new value to effective address */
    add_action_to_queue(state, INC_MEMORY);
    break;
    
    // BEQ
  case 0xF0:
    add_action_to_queue(state, FETCH_OPERAND_INC_PC);
    if (is_zero_flag_set(state)) {
      add_action_to_queue(state, ADD_OPERAND_TO_PCL);
    }
    // TODO : Add extra action for crossing page boundary
    break;

    // SBC indirect-indexed, Y
  case 0xF1:
      state->cpu->index_reg = &state->cpu->registers->Y;      
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* #    address   R/W description */
      /*      --- ----------- --- ------------------------------------------ */
      /*       1      PC       R  fetch opcode, increment PC */
      /*       2      PC       R  fetch pointer address, increment PC */
      add_action_to_queue(state, FETCH_ZP_PTR_ADDR_INC_PC);
      /*       3    pointer    R  fetch effective address low */
      add_action_to_queue(state, FETCH_LOW_BYTE_ADDR_ZP);
      /*       4   pointer+1   R  fetch effective address high, */
      /*                          add Y to low byte of effective address */
      add_action_to_queue(state, FETCH_HIGH_BYTE_ADDR_ADD_INDEX);
      /*       5   address+Y*  R  read from effective address, */
      /*                          fix high byte of effective address */
      add_action_to_queue(state, SBC_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE);
      /*       6+  address+Y   R  read from effective address */
      // ^Will be added in 313 if necessary
    break;

  // SBC zeropage, X
  case 0xF5:
      /* 2     PC      R  fetch address, increment PC */
      state->cpu->high_addr_byte = 0x0;
      state->cpu->index_reg = &state->cpu->registers->X;      
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);      
      /* 3   address   R  read from address, add index register to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
      /* 4  address+I* R  read from effective address */
      add_action_to_queue(state, SBC_MEMORY);
      break;

// INC zero page, X
  case 0xF6:
      state->cpu->high_addr_byte = 0;
      state->cpu->index_reg = &state->cpu->registers->X;      
        /* 2     PC      R  fetch address, increment PC */
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
        /* 3   address   R  read from address, add index register X to it */
      add_action_to_queue(state, ZEROPAGE_ADD_INDEX);
        /* 4  address+X* R  read from effective address */
      add_action_to_queue(state, STALL_CYCLE);
        /* 5  address+X* W  write the value back to effective address, */
        /*                  and do the operation on it */
      add_action_to_queue(state, STALL_CYCLE);      
        /* 6  address+X* W  write the new value to effective address */
      add_action_to_queue(state, INC_MEMORY);
      break;

      
    // SED - Set Decimal Flag
  case 0xF8:
    add_action_to_queue(state, SET_DECIMAL_FLAG);
    break;

    // SBC absolute, Y
  case 0xF9:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      /* state->cpu->source_reg = &state->cpu->registers->Y; */
      state->cpu->index_reg = &state->cpu->registers->Y;
      
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, SBC_MEMORY);
      
      break;
    
    // SBC absolute, X
  case 0xFD:
        /* 2     PC      R  fetch low byte of address, increment PC */
        /* 3     PC      R  fetch high byte of address, */
        /*                  add index register to low address byte, */
        /*                  increment PC */
        /* 4  address+I* R  read from effective address, */
        /*                  fix the high byte of effective address */
        /* 5+ address+I  R  re-read from effective address */
      state->cpu->destination_reg = &state->cpu->registers->ACC;
      state->cpu->index_reg = &state->cpu->registers->X;    
      add_action_to_queue(state, FETCH_LOW_ADDR_BYTE_INC_PC);
      add_action_to_queue(state, FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC);
      add_action_to_queue(state, SBC_MEMORY);
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
