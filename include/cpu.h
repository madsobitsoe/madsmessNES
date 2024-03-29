#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>
#include "definitions.h"
/* #include "nes.h" */


enum ACTION {
    STALL_CYCLE = 0,
    FETCH_OPCODE_INC_PC = 1,
    FETCH_LOW_ADDR_BYTE_INC_PC = 2,
    FETCH_HIGH_ADDR_BYTE_INC_PC = 3,
    COPY_LOW_ADDR_BYTE_TO_PCL_FETCH_HIGH_ADDR_BYTE_TO_PCH = 301,
    FETCH_VALUE_SAVE_TO_DEST = 4,
    WRITE_REG_TO_EFF_ADDR_ZEROPAGE = 5,
    PUSH_PCH_DEC_S = 6,
    PUSH_PCL_DEC_S = 7,
    FETCH_OPERAND_INC_PC = 8,
    ADD_OPERAND_TO_PCL = 9,
    INC_PC = 10,
    INC_SP = 11,
    INC_SOURCE_REG = 12,
    DEC_SOURCE_REG = 13,
    INC_MEMORY = 14,
    DEC_MEMORY = 15,
    BIT_READ_AFFECT_FLAGS = 308,
    PULL_PCL_FROM_STACK_INC_SP = 309,
    PULL_PCH_FROM_STACK = 16,
    PULL_ACC_FROM_STACK_AFFECT_FLAGS = 17,
    PUSH_ACC_DEC_SP = 18,
    PULL_STATUS_REG_FROM_STACK_PLP = 19,
    PUSH_STATUS_REG_DEC_SP = 20,
    PULL_STATUS_REG_FROM_STACK_RTI = 29,
    CLEAR_CARRY_FLAG = 90,
    CLEAR_ZERO_FLAG = 91,
    CLEAR_INTERRUPT_FLAG = 92,
    CLEAR_DECIMAL_FLAG = 93,
    CLEAR_BREAK_FLAG = 94,
    CLEAR_OVERFLOW_FLAG = 95,
    CLEAR_NEGATIVE_FLAG = 96,
    SET_CARRY_FLAG = 100,
    SET_ZERO_FLAG = 101,
    SET_INTERRUPT_FLAG = 102,
    SET_DECIMAL_FLAG = 103,
    SET_BREAK_FLAG = 104,
    SET_OVERFLOW_FLAG = 105,
    SET_NEGATIVE_FLAG = 106,
    COPY_SOURCE_REG_TO_DEST_REG_AFFECT_NZ_FLAGS = 202,
    COPY_SOURCE_REG_TO_DEST_REG_NO_FLAGS = 203,
    WRITE_REG_TO_EFF_ADDR_NON_ZEROPAGE = 302,
    READ_EFF_ADDR_STORE_IN_REG_AFFECT_NZ_FLAGS = 303,
    READ_ADDR_ADD_INDEX_STORE_IN_OPERAND = 304,
    FETCH_EFF_ADDR_LOW = 305,
    FETCH_EFF_ADDR_HIGH = 306,
    FETCH_EFF_ADDR_HIGH_ADD_INDEX = 3061,    
    FETCH_ZP_PTR_ADDR_INC_PC = 307,
    FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC = 310,
    READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 311,
    FETCH_HIGH_BYTE_ADDR_ADD_INDEX = 313,
    ORA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 314,
    AND_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 315,
    EOR_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 316,
    ADC_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 317,
    CMP_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 318,
    SBC_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 319,
    STA_STX_STY_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 320,
    STA_READ_FROM_EFF_ADDR_FIX_HIGH_BYTE = 321,
    FETCH_LOW_ADDR_TO_LATCH = 322,
    FETCH_PCH_COPY_LATCH_TO_PCL = 323,
    AND_IMM_INC_PC = 21,
    CMP_IMM_INC_PC = 22,
    ORA_IMM_INC_PC = 23,
    EOR_IMM_INC_PC = 24,
    ADC_IMM_INC_PC = 25,
    CPY_IMM_INC_PC = 26,
    CPX_IMM_INC_PC = 27,
    SBC_IMM_INC_PC = 28,
    LSR_SOURCE_REG = 404,
    ASL_SOURCE_REG = 405,
    ROR_SOURCE_REG = 406,
    ROL_SOURCE_REG = 407,
    LSR_MEMORY = 408,
    ASL_MEMORY = 409,
    ROR_MEMORY = 410,
    ROL_MEMORY = 411,
    ORA_MEMORY = 30,
    AND_MEMORY = 31,
    EOR_MEMORY = 32,
    ADC_MEMORY = 33,
    SBC_MEMORY = 35,
    CMP_MEMORY = 36,
    ZEROPAGE_ADD_INDEX = 500,
    FETCH_EFF_ADDR_HIGH_ADD_INDEX_INC_PC_NO_EXTRA_CYCLES = 700,
    NOP_ABSOLUTE_X_MAYBE_STALL = 800,
    LAX_READ_EFF_ADDR_STORE_IN_REGS_AFFECT_NZ_FLAGS = 900,
    SAX_PERFORM_AND_THEN_WRITE_EFF_ADDR_NO_AFFECT_FLAGS = 901,
    DCP_PERFORM_DEC_MEMORY_THEN_CMP_MEMORY = 902,
    FIX_HIGH_BYTE_NO_WRITE = 903,
    FETCH_HIGH_BYTE_ADDR_ADD_INDEX_NO_EXTRA_CYCLE = 904,
    ISB_PERFORM_INC_MEMORY_THEN_SBC_MEMORY = 905,    
    NMI_FETCH_PCL = 1000,
    NMI_FETCH_PCH = 1001,
    IRQ_FETCH_PCL = 1002,
    IRQ_FETCH_PCH = 1003,
    BRK_FETCH_PCL = 1004,
    BRK_FETCH_PCH = 1005,
    PUSH_STATUS_REG_DEC_S_CLEAR_B_FLAG = 1006,
    PUSH_STATUS_REG_DEC_S_SET_B_FLAG = 1007,
    SLO_DO_ASL_THEN_ORA = 1008,
    RLA_DO_ROL_THEN_AND = 1009,
    SRE_DO_LSR_THEN_EOR_ACC = 1010,
    RRA_DO_ROR_THEN_ADC    = 1011,
};




void init_registers(registers *registers);
void print_regs(nes_state *state);
void print_cpu_status(nes_state *state);
void print_stack(nes_state *state);
void cpu_step(nes_state *state);
nes_state* init_state();
void set_pc(nes_state *state, unsigned short pc);
uint8_t read_mem_byte(nes_state *state, unsigned short memloc);
uint16_t read_mem_short(nes_state *state, unsigned short memloc);
uint16_t translate_memory_location(unsigned short memloc);
void add_action_to_queue(nes_state *state, uint16_t action);

void trigger_interrupt(nes_state *state);

void set_negative_flag(nes_state *state);
void set_overflow_flag(nes_state *state);
void set_break_flag(nes_state *state);
void set_decimal_flag(nes_state *state);
void set_interrupt_flag(nes_state *state);
void set_zero_flag(nes_state *state);
void set_carry_flag(nes_state *state);

void clear_negative_flag(nes_state *state);
void clear_overflow_flag(nes_state *state);
void clear_break_flag(nes_state *state);
void clear_decimal_flag(nes_state *state);
void clear_interrupt_flag(nes_state *state);
void clear_zero_flag(nes_state *state);
void clear_carry_flag(nes_state *state);


bool is_carry_flag_set(nes_state *state);
bool is_zero_flag_set(nes_state *state);
bool is_interrupt_flag_set(nes_state *state);
bool is_decimal_flag_set(nes_state *state);
bool is_break_flag_set(nes_state *state);
bool is_overflow_flag_set(nes_state *state);
bool is_negative_flag_set(nes_state *state);



#endif
