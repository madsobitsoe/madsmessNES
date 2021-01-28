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
    //STX Zero Page
  case 0x86:
    sprintf(output, "%04X  %02X %02X     STX $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, read_mem_byte(state, state->cpu->current_opcode_PC+1)));
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
  default:
    sprintf(output, "not implemented yet");
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
