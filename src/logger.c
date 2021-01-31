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
    // PHP
  case 0x08:
    sprintf(output, "%04X  %02X        PHP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // ASL A
  case 0x0A:
    sprintf(output, "%04X  %02X        ASL A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // CLC
  case 0x18:
    sprintf(output, "%04X  %02X        CLC",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // SEC
  case 0x38:
    sprintf(output, "%04X  %02X        SEC",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // ORA immediate
  case 0x09:
    sprintf(output, "%04X  %02X %02X     ORA #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // BPL
  case 0x10:
    sprintf(output, "%04X  %02X %02X     BPL $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
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
    // PLP
  case 0x28:
    sprintf(output, "%04X  %02X        PLP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // AND immediate
  case 0x29:
    sprintf(output, "%04X  %02X %02X     AND #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // ROR A
  case 0x2A:
    sprintf(output, "%04X  %02X        ROL A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // BMI
  case 0x30:
    sprintf(output, "%04X  %02X %02X     BMI $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // RTI - Return from Interrupt
  case 0x40:
    sprintf(output, "%04X  %02X        RTI",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // PHA
  case 0x48:
    sprintf(output, "%04X  %02X        PHA",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // EOR immediate
  case 0x49:
    sprintf(output, "%04X  %02X %02X     EOR #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // LSR A
  case 0x4A:
    sprintf(output, "%04X  %02X        LSR A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // JMP
  case 0x4C:
    sprintf(output, "%04X  %02X %02X %02X  JMP $%02X%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+2),
            read_mem_byte(state, state->cpu->current_opcode_PC+2),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // BVC
  case 0x50:
    sprintf(output, "%04X  %02X %02X     BVC $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
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
    // ADC immediate
  case 0x69:
    sprintf(output, "%04X  %02X %02X     ADC #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // ROR A
  case 0x6A:
    sprintf(output, "%04X  %02X        ROR A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // BVS
  case 0x70:
    sprintf(output, "%04X  %02X %02X     BVS $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // SEI
  case 0x78:
    sprintf(output, "%04X  %02X        SEI",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
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
    // DEY
  case 0x88:
    sprintf(output, "%04X  %02X        DEY",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // TXA
  case 0x8A:
    sprintf(output, "%04X  %02X        TXA",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    //STX Absolute
  case 0x8E:
    {
      uint16_t addr = read_mem_byte(state, state->cpu->current_opcode_PC+2) << 8;
        addr |= read_mem_byte(state, state->cpu->current_opcode_PC+1);

        sprintf(output, "%04X  %02X %02X %02X  STX $%02X%02X = %02X",
                state->cpu->current_opcode_PC,
                state->cpu->current_opcode,
                read_mem_byte(state, state->cpu->current_opcode_PC+1),
                read_mem_byte(state, state->cpu->current_opcode_PC+2),
                read_mem_byte(state, state->cpu->current_opcode_PC+2),
                read_mem_byte(state, state->cpu->current_opcode_PC+1),
                read_mem_byte(state, addr));
      }
    break;
    // BCC
  case 0x90:
    sprintf(output, "%04X  %02X %02X     BCC $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // TYA
  case 0x98:
    sprintf(output, "%04X  %02X        TYA",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // TXS
  case 0x9A:
    sprintf(output, "%04X  %02X        TXS",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // LDY immediate
  case 0xA0:
    sprintf(output, "%04X  %02X %02X     LDY #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // LDX immediate
  case 0xA2:
    sprintf(output, "%04X  %02X %02X     LDX #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // TAY
  case 0xA8:
    sprintf(output, "%04X  %02X        TAY",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // LDA immediate
  case 0xA9:
    sprintf(output, "%04X  %02X %02X     LDA #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // TAX
  case 0xAA:
    sprintf(output, "%04X  %02X        TAX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // LDA Absolute
  case 0xAD:
    {
      uint16_t addr = read_mem_byte(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem_byte(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  LDA $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem_byte(state, state->cpu->current_opcode_PC+1),
              read_mem_byte(state, state->cpu->current_opcode_PC+2),
              read_mem_byte(state, state->cpu->current_opcode_PC+2),
              read_mem_byte(state, state->cpu->current_opcode_PC+1),
              read_mem_byte(state, addr));
    }
    break;
    // LDX Absolute
  case 0xAE:
    {
      uint16_t addr = read_mem_byte(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem_byte(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  LDX $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem_byte(state, state->cpu->current_opcode_PC+1),
              read_mem_byte(state, state->cpu->current_opcode_PC+2),
              read_mem_byte(state, state->cpu->current_opcode_PC+2),
              read_mem_byte(state, state->cpu->current_opcode_PC+1),
              read_mem_byte(state, addr));
    }
                    break;
    // BCS
  case 0xB0:
    sprintf(output, "%04X  %02X %02X     BCS $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // CLV
  case 0xB8:
    sprintf(output, "%04X  %02X        CLV",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // TSX - Transfer SP to X
  case 0xBA:
    sprintf(output, "%04X  %02X        TSX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // CPY immediate
  case 0xC0:
    sprintf(output, "%04X  %02X %02X     CPY #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // INY
  case 0xC8:
    sprintf(output, "%04X  %02X        INY",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // CMP immediate
  case 0xC9:
    sprintf(output, "%04X  %02X %02X     CMP #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // DEX
  case 0xCA:
    sprintf(output, "%04X  %02X        DEX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // BNE
  case 0xD0:
    sprintf(output, "%04X  %02X %02X     BNE $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // CLD
  case 0xD8:
    sprintf(output, "%04X  %02X        CLD",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // CPX immediate
  case 0xE0:
    sprintf(output, "%04X  %02X %02X     CPX #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // INX
  case 0xE8:
    sprintf(output, "%04X  %02X        INX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // SBC immediate
  case 0xE9:
    sprintf(output, "%04X  %02X %02X     SBC #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // NOP
  case 0xEA:
    sprintf(output, "%04X  %02X        NOP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // SBC "Illegal" opcode
  case 0xEB:
    sprintf(output, "%04X  %02X %02X     SBC #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            read_mem_byte(state, state->cpu->current_opcode_PC+1));
    break;
    // BEQ
  case 0xF0:
    sprintf(output, "%04X  %02X %02X     BEQ $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem_byte(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem_byte(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    // SED
  case 0xF8:
    sprintf(output, "%04X  %02X        SED",
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
