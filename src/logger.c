#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

FILE *logfile;


/* static log_entry logger_state_to_log_entry(nes_state *state) { */
/*   log_entry entry = */
/*     { .PC = state->current_opcode_PC, */
/*       .opcode = state->current_opcode, */
/*       .operand1 = -1, */
/*       .operand2 = -1, */
/*       .ACC = state->registers->ACC, */
/*       .X = state->registers->X, */
/*       .Y = state->registers->Y, */
/*       .P = state->registers->SR, */
/*       .SP = state->registers->SP, */
/*       .CYC = state->master_clock */
/*     }; */
/*   return entry; */
/* } */



void disass(nes_state *state, char *output) {
  switch(state->cpu->current_opcode) {
    // ORA immediate
  case 0x09:
    sprintf(output, "%04X  %02X %02X     ORA #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // JSR
  case 0x20:
    sprintf(output, "%04X  %02X %02X %02X  JSR $%02X%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+2),
            read_mem_byte(state, state->cpu->current_opcode_PC+2),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // BIT Zeropage
  case 0x24:
    sprintf(output, "%04X  %02X %02X     BIT $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, read_mem_byte(state, state->cpu->current_opcode_PC+1)));
    break;
    // AND immediate
  case 0x29:
    sprintf(output, "%04X  %02X %02X     AND #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;

    // EOR immediate
  case 0x49:
    sprintf(output, "%04X  %02X %02X     EOR #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;


    // JMP
  case 0x4c:
    sprintf(output, "%04X  %02X %02X %02X  JMP $%02X%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+2),
            read_mem_byte(state, state->cpu->current_opcode_PC+2),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // LDX immediate
  case 0xa2:
    sprintf(output, "%04X  %02X %02X     LDX #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // LDA immediate
  case 0xa9:
    sprintf(output, "%04X  %02X %02X     LDA #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // CMP immediate
  case 0xC9:
    sprintf(output, "%04X  %02X %02X     CMP #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    //STA Zero Page
  case 0x85:
    sprintf(output, "%04X  %02X %02X     STA $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, read_mem_byte(state, state->cpu->current_opcode_PC+1)));
    break;
    //STX Zero Page
  case 0x86:
    sprintf(output, "%04X  %02X %02X     STX $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, read_mem_byte(state, state->cpu->current_opcode_PC+1)));
    break;
    // BPL
  case 0x10:
    sprintf(output, "%04X  %02X %02X     BPL $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // BMI
  case 0x30:
    sprintf(output, "%04X  %02X %02X     BMI $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // BVC
  case 0x50:
    sprintf(output, "%04X  %02X %02X     BVC $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // BVS
  case 0x70:
    sprintf(output, "%04X  %02X %02X     BVS $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // BCC
  case 0x90:
    sprintf(output, "%04X  %02X %02X     BCC $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // BCS
  case 0xB0:
    sprintf(output, "%04X  %02X %02X     BCS $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // BNE
  case 0xD0:
    sprintf(output, "%04X  %02X %02X     BNE $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // BEQ
  case 0xF0:
    sprintf(output, "%04X  %02X %02X     BEQ $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // NOP
  case 0xEA:
    sprintf(output, "%04X  %02X        NOP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // SEC
  case 0x38:
    sprintf(output, "%04X  %02X        SEC",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // SEI
  case 0x78:
    sprintf(output, "%04X  %02X        SEI",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // SED
  case 0xF8:
    sprintf(output, "%04X  %02X        SED",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // PHP
  case 0x08:
    sprintf(output, "%04X  %02X        PHP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // RTS - Return from subroutine
  case 0x60:
    sprintf(output, "%04X  %02X        RTS",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // PLA
  case 0x68:
    sprintf(output, "%04X  %02X        PLA",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // CLC
  case 0x18:
    sprintf(output, "%04X  %02X        CLC",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // CLD
  case 0xD8:
    sprintf(output, "%04X  %02X        CLD",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // PLP
  case 0x28:
    sprintf(output, "%04X  %02X        PLP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // PHA
  case 0x48:
    sprintf(output, "%04X  %02X        PHA",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // CLV
  case 0xB8:
    sprintf(output, "%04X  %02X        CLV",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
  default:
    sprintf(output, "%04X  %02X not implemented yet",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
  }
}




int logger_init_logger(char *filename) {
  if ((logfile = fopen(filename, "w")) == NULL) {
    perror("LOGGER: Failed to open %s for writing.\n");
    return EXIT_FAILURE;
  }
  return 0;
}


void print_log(nes_state *state) {
  char part1[48];
  part1[47] = '\0';
  disass(state, part1);
  // Fill out missing parts later
  printf("%-48sA:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u\n",
         part1,
         state->cpu->registers->ACC,
         state->cpu->registers->X,
         state->cpu->registers->Y,
         state->cpu->registers->SR,
         state->cpu->registers->SP,
         state->ppu_frame,
          state->ppu_cycle,
         (int32_t) state->cpu->cpu_cycle);

}

void logger_log(nes_state *state)
{
  char part1[48];
  part1[47] = '\0';
  disass(state, part1);
  // Fill out missing parts later
  fprintf(logfile, "%-48sA:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u\n",
          part1,
          state->cpu->registers->ACC,
          state->cpu->registers->X,
          state->cpu->registers->Y,
          state->cpu->registers->SR,
          state->cpu->registers->SP,
          state->ppu_frame,
          state->ppu_cycle,
          (int32_t) state->cpu->cpu_cycle);

}
void logger_stop_logger()
{
  fclose(logfile);
}
