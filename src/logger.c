#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "memory.h"
FILE *logfile;





void disass(nes_state *state, char *output) {
  switch(state->cpu->current_opcode) {
    // ORA indirect,X
  case 0x01:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     ORA ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    // *SLO indirect,X - Illegal instruction
  case 0x03:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *SLO ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    
    // *NOP Zeropage - "illegal instruction"
  case 0x04:
  case 0x44:
  case 0x64:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *NOP $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
      break;


    // *NOP Absolute, X - "illegal instruction"
  case 0x1C:
  case 0x3C:
  case 0x5C:
  case 0x7C:
  case 0xDC:
  case 0xFC:      
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *NOP $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

      
    // ORA Zeropage
  case 0x05:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     ORA $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    // ASL Zeropage
  case 0x06:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     ASL $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *SLO Zeropage - Illegal instruction
  case 0x07:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *SLO $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;


    
    // PHP
  case 0x08:
    sprintf(output, "%04X  %02X        PHP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // ORA immediate
  case 0x09:
    sprintf(output, "%04X  %02X %02X     ORA #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // ASL A
  case 0x0A:
    sprintf(output, "%04X  %02X        ASL A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

// *NOP Implied - illegal opcode
  case 0x1A:
  case 0x3A:
  case 0x5A:
  case 0x7A:
  case 0xDA:
  case 0xFA:      
    sprintf(output, "%04X  %02X       *NOP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
// *NOP Immediate - illegal opcode
  case 0x80:
    sprintf(output, "%04X  %02X %02X    *NOP #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;

    
    // *NOP Absolute - illegal opcode
  case 0x0C:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *NOP $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    // *NOP Zeropage, X - illegal opcode
  case 0x14:
  case 0x34:
  case 0x54:
  case 0x74:
  case 0xD4:
  case 0xF4:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *NOP $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    

    
    // ORA Absolute
  case 0x0D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  ORA $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // ASL Absolute
  case 0x0E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  ASL $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *SLO Absolute - Illegal instruction
  case 0x0F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *SLO $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    
    // BPL
  case 0x10:
    sprintf(output, "%04X  %02X %02X     BPL $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // ORA indirect-indexed,Y
  case 0x11:
    {
      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* uint16_t offset = (uint16_t) state->cpu->registers->Y; */
      /* if (((base & 0xFF) + offset) > 0xFF) { */
      /*   /\* effective_addr += 0x100; *\/ */
      /*   if (high_addr < 0xFF) { high_addr++; } */
      /* } */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */

      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     ORA ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;


    // *SLO indirect-indexed,Y - Illegal instruction
  case 0x13:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;


      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *SLO ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    
    // ORA Zeropage, X
  case 0x15:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     ORA $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // ASL Zeropage, X
  case 0x16:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     ASL $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *SLO Zeropage, X - Illegal instruction
  case 0x17:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *SLO $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    
    // CLC
  case 0x18:
    sprintf(output, "%04X  %02X        CLC",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
// ORA Absolute Y
  case 0x19:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  ORA $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
      
  }
  break;


// *SLO Absolute Y - Illegal instruction
  case 0x1B:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *SLO $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
      
  }
  break;

  
    // ORA Absolute X
  case 0x1D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  ORA $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // ASL Absolute X
  case 0x1E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  ASL $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;
    
    // *SLO Absolute X - Illegal instruction
  case 0x1F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *SLO $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;


    // JSR
  case 0x20:
    sprintf(output, "%04X  %02X %02X %02X  JSR $%02X%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+2),
            read_mem(state, state->cpu->current_opcode_PC+2),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;


    
    // AND indirect,X
  case 0x21:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     AND ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    // *RLA indirect,X - ROL followed by AND
  case 0x23:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *RLA ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


    
    // BIT Zeropage
  case 0x24:
    sprintf(output, "%04X  %02X %02X     BIT $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, read_mem(state, state->cpu->current_opcode_PC+1)));
    break;

    
    // AND Zeropage
  case 0x25:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     AND $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // ROL Zeropage
  case 0x26:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     ROL $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;


    // *RLA Zeropage - Illegal instruction
  case 0x27:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *RLA $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    // ROR A
  case 0x2A:
    sprintf(output, "%04X  %02X        ROL A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // BIT Absolute
  case 0x2C:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  BIT $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    // AND Absolute
  case 0x2D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  AND $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // ROL Absolute
  case 0x2E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  ROL $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *RLA Absolute - Illegal instruction
  case 0x2F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *RLA $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    
    // BMI
  case 0x30:
    sprintf(output, "%04X  %02X %02X     BMI $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // AND indirect-indexed,Y
  case 0x31:
    {

      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* uint16_t offset = (uint16_t) state->cpu->registers->Y; */
      /* if (((base & 0xFF) + offset) > 0xFF) { */
      /*   if (high_addr < 0xFF) { high_addr++; } */
      /* } */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;


      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     AND ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // *RLA indirect-indexed,Y - Illegal instruction
  case 0x33:
    {

      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *RLA ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    

    // AND Zeropage, X
  case 0x35:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     AND $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // ROL Zeropage, X
  case 0x36:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     ROL $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;
    
    // *RLA Zeropage, X
  case 0x37:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *RLA $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;
    
    // SEC
  case 0x38:
    sprintf(output, "%04X  %02X        SEC",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

// AND Absolute Y
  case 0x39:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  AND $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              /* read_mem(state, state->cpu->current_opcode_PC+2), */
              /* read_mem(state, state->cpu->current_opcode_PC+1), */
              read_mem(state, addr));
      
  }
  break;
  // *RLA Absolute Y - Illegal instruction
  case 0x3B:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *RLA $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;
  
    // AND Absolute X
  case 0x3D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  AND $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // ROL Absolute X
  case 0x3E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  ROL $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // *RLA Absolute X
  case 0x3F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *RLA $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    
    // RTI - Return from Interrupt
  case 0x40:
    sprintf(output, "%04X  %02X        RTI",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    // EOR indirect,X
  case 0x41:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     EOR ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    // *SRE indirect,X - Illegal instruction
  case 0x43:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *SRE ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


  // EOR Zeropage
  case 0x45:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     EOR $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // LSR Zeropage
  case 0x46:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     LSR $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *SRE Zeropage - Illegal instruction
  case 0x47:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *SRE $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+2),
            read_mem(state, state->cpu->current_opcode_PC+2),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    // EOR Absolute
  case 0x4D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  EOR $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;



    
    // LSR Absolute
  case 0x4E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  LSR $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *SRE Absolute - Illegal instruction
  case 0x4F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *SRE $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    
    // BVC
  case 0x50:
    sprintf(output, "%04X  %02X %02X     BVC $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // EOR indirect-indexed,Y
  case 0x51:
    {
      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* uint16_t offset = (uint16_t) state->cpu->registers->Y; */
      /* if (((base & 0xFF) + offset) > 0xFF) { */
      /*   if (high_addr < 0xFF) { high_addr++; } */
      /* } */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;


      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     EOR ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // *SRE indirect-indexed,Y
  case 0x53:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *SRE ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    
    // EOR Zeropage, X
  case 0x55:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     EOR $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // LSR Zeropage, X
  case 0x56:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     LSR $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *SRE Zeropage, X
  case 0x57:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *SRE $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    
    
// EOR Absolute Y
  case 0x59:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  EOR $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;

  // *SRE Absolute Y
  case 0x5B:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *SRE $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;


  
    // EOR Absolute X
  case 0x5D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  EOR $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // LSR Absolute X
  case 0x5E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  LSR $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

  // *SRE Absolute X
  case 0x5F:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *SRE $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;
  
    // RTS - Return from subroutine
  case 0x60:
    sprintf(output, "%04X  %02X        RTS",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    
    // ADC indirect,X
  case 0x61:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     ADC ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    // *RRA indirect,X - Illegal instruction
  case 0x63:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *RRA ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


    
    // ADC Zeropage
  case 0x65:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     ADC $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // ROR Zeropage
  case 0x66:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     ROR $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // *RRA Zeropage - Illegal instruction
  case 0x67:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *RRA $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // ROR A
  case 0x6A:
    sprintf(output, "%04X  %02X        ROR A",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

    // JMP indirect
  case 0x6C:
    {
      uint8_t operand1 = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t operand2 = read_mem(state, state->cpu->current_opcode_PC+2);
      uint16_t addr_addr1 = (uint16_t) operand1 | ((uint16_t) operand2) << 8;
      // Ensure page wrap is handled
      uint16_t addr_addr2 = addr_addr1 + 1;
      if (!((addr_addr1 & 0xff00) == (addr_addr2 & 0xff00))) {
        addr_addr2 = (addr_addr1 & 0xff00);
      }
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr1)) | (((uint16_t) read_mem(state, addr_addr2)) << 8);

      sprintf(output, "%04X  %02X %02X %02X  JMP ($%04X) = %04X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand1,
              operand2,
              addr_addr1,
              effective_addr);
    }
    break;


    // ADC Absolute
  case 0x6D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  ADC $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // ROR Absolute
  case 0x6E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  ROR $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *RRA Absolute
  case 0x6F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *RRA $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    
    // BVS
  case 0x70:
    sprintf(output, "%04X  %02X %02X     BVS $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // ADC indirect-indexed,Y
  case 0x71:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     ADC ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // *RRA indirect-indexed,Y
  case 0x73:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *RRA ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    
    // ADC Zeropage, X
  case 0x75:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     ADC $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // ROR Zeropage, X
  case 0x76:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     ROR $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *RRA Zeropage, X
  case 0x77:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *RRA $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;
    
    
    // SEI
  case 0x78:
    sprintf(output, "%04X  %02X        SEI",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    
// ADC Absolute Y
  case 0x79:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  ADC $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;

  // *RRA Absolute Y - Illegal instruction
  case 0x7B:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *RRA $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;
  
    // ADC Absolute X
  case 0x7D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  ADC $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // ROR Absolute X
  case 0x7E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  ROR $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

  // *RRA Absolute X - Illegal instruction
  case 0x7F:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *RRA $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));      
  }
  break;
    

    
    // STA indirect,X
  case 0x81:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     STA ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


    // *SAX indirect,X - Illegal instruction, ACC AND X -> Memory
  case 0x83:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *SAX ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;



    
    //STY Zero Page
  case 0x84:
    sprintf(output, "%04X  %02X %02X     STY $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, read_mem(state, state->cpu->current_opcode_PC+1)));
    break;
    //STA Zero Page
  case 0x85:
    sprintf(output, "%04X  %02X %02X     STA $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, read_mem(state, state->cpu->current_opcode_PC+1)));
    break;
    //STX Zero Page
  case 0x86:
    sprintf(output, "%04X  %02X %02X     STX $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, read_mem(state, state->cpu->current_opcode_PC+1)));
    break;

    // *SAX Zero Page - illegal instruction
  case 0x87:
    sprintf(output, "%04X  %02X %02X    *SAX $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, read_mem(state, state->cpu->current_opcode_PC+1)));
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
    //STY Absolute
  case 0x8C:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  STY $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    //STA Absolute
  case 0x8D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  STA $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    //STX Absolute
  case 0x8E:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
        addr |= read_mem(state, state->cpu->current_opcode_PC+1);

        sprintf(output, "%04X  %02X %02X %02X  STX $%02X%02X = %02X",
                state->cpu->current_opcode_PC,
                state->cpu->current_opcode,
                read_mem(state, state->cpu->current_opcode_PC+1),
                read_mem(state, state->cpu->current_opcode_PC+2),
                read_mem(state, state->cpu->current_opcode_PC+2),
                read_mem(state, state->cpu->current_opcode_PC+1),
                read_mem(state, addr));
      }
    break;

    //SAX Absolute - Illegal instruction
  case 0x8F:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *SAX $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;


    
    // BCC
  case 0x90:
    sprintf(output, "%04X  %02X %02X     BCC $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;


    // STA indirect-indexed,Y
  case 0x91:
    {

      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));
      low_addr += state->cpu->registers->Y;
      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     STA ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // STY Zeropage, X
  case 0x94:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     STY $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;
    
    // STA Zeropage, X
  case 0x95:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     STA $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // STX Zeropage, Y
  case 0x96:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->Y;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     STX $%02X,Y @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *SAX Zeropage, Y - Illegal instruction
  case 0x97:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->Y;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *SAX $%02X,Y @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;
    
    
    // TYA
  case 0x98:
    sprintf(output, "%04X  %02X        TYA",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;


  // STA Absolute Y
  case 0x99:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  STA $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              /* read_mem(state, state->cpu->current_opcode_PC+2), */
              /* read_mem(state, state->cpu->current_opcode_PC+1), */
              read_mem(state, addr));
      
  }
  break;

    
    // TXS
  case 0x9A:
    sprintf(output, "%04X  %02X        TXS",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;


    // STA Absolute X
  case 0x9D:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  STA $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    
    // LDY immediate
  case 0xA0:
    sprintf(output, "%04X  %02X %02X     LDY #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // LDA indirect,X
  case 0xA1:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     LDA ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;
    
    // LDX immediate
  case 0xA2:
    sprintf(output, "%04X  %02X %02X     LDX #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;

    // *LAX indirect,X - Illegal opcode, combines LDA and LDX
  case 0xA3:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *LAX ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


    
    // LDY Zeropage
  case 0xA4:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     LDY $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    // LDA Zeropage
  case 0xA5:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);


      sprintf(output, "%04X  %02X %02X     LDA $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              addr,
              addr,
              /* read_mem(state, state->cpu->current_opcode_PC+1), */
              /* read_mem(state, state->cpu->current_opcode_PC+1), */
              read_mem(state, addr));
    }
    break;
    // LDX Zeropage
  case 0xA6:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     LDX $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *LAX Zero Page - illegal instruction
  case 0xA7:
    sprintf(output, "%04X  %02X %02X    *LAX $%02X = %02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, read_mem(state, state->cpu->current_opcode_PC+1)));
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // TAX
  case 0xAA:
    sprintf(output, "%04X  %02X        TAX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    
    // LDY Absolute
  case 0xAC:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  LDY $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // LDA Absolute
  case 0xAD:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  LDA $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // LDX Absolute
  case 0xAE:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  LDX $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *LAX Absolute - Illegal opcode
  case 0xAF:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *LAX $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    
    // BCS
  case 0xB0:
    sprintf(output, "%04X  %02X %02X     BCS $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;
    
    // LDA indirect-indexed,Y
  case 0xB1:
    {
      /* LDY #$04 */
      /*   LDA ($02),Y */
      /*   In the above case, Y is loaded with four (4), and the vector is given as ($02) */
      /* If zero page memory $02-$03 contains 00 80, then the effective address from the vector ($02) plus the offset (Y) would be $8004. */
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      // check if page boundary was crossed and fix addresses
      uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      uint16_t offset = (uint16_t) state->cpu->registers->Y;
      if (((base & 0xFF) + offset) > 0xFF) {
        /* effective_addr += 0x100; */
        if (high_addr < 0xFF) { high_addr++; }
      }
      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     LDA ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // *LAX indirect-indexed,Y - Illegal opcode
  case 0xB3:
    {
      /* /\* LDY #$04 *\/ */
      /* /\*   LDA ($02),Y *\/ */
      /* /\*   In the above case, Y is loaded with four (4), and the vector is given as ($02) *\/ */
      /* /\* If zero page memory $02-$03 contains 00 80, then the effective address from the vector ($02) plus the offset (Y) would be $8004. *\/ */

      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* /\* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; *\/ */
      /* /\* uint16_t offset = (uint16_t) state->cpu->registers->Y; *\/ */
      /* /\* if (((base & 0xFF) + offset) > 0xFF) { *\/ */
      /* /\*   /\\* effective_addr += 0x100; *\\/ *\/ */
      /* /\*   if (high_addr < 0xFF) { high_addr++; } *\/ */
      /* /\* } *\/ */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */

      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *LAX ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;


    
    // LDY Zeropage, X
  case 0xB4:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     LDY $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // LDA Zeropage, X
  case 0xB5:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     LDA $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;
    
    // LDX Zeropage, Y
  case 0xB6:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->Y;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     LDX $%02X,Y @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *LAX Zeropage, Y - Illegal instruction
  case 0xB7:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->Y;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *LAX $%02X,Y @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    
    // CLV
  case 0xB8:
    sprintf(output, "%04X  %02X        CLV",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    
    // Load Accumuator Absolute Y
  case 0xB9:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  LDA $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;
    
    // TSX - Transfer SP to X
  case 0xBA:
    sprintf(output, "%04X  %02X        TSX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;


    // Load Y Absolute X
  case 0xBC:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  LDY $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // LDA Absolute X
  case 0xBD:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  LDA $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // LDX Absolute Y
  case 0xBE:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  LDX $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // *LAX Absolute X
  case 0xBF:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *LAX $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    
    // CPY immediate
  case 0xC0:
    sprintf(output, "%04X  %02X %02X     CPY #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // CMP indirect,X
  case 0xC1:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     CMP ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    // *DCP indirect,X - Illegal instruction
    // Equivalent to DEC value followed by CMP value
  case 0xC3:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *DCP ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


    
    // CPY Zeropage
  case 0xC4:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     CPY $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // CMP Zeropage
  case 0xC5:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     CMP $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // DEC Zeropage
  case 0xC6:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     DEC $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *DCP Zeropage - Illegal instruction
  case 0xC7:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *DCP $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    // DEX
  case 0xCA:
    sprintf(output, "%04X  %02X        DEX",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    
    // CPY Absolute
  case 0xCC:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  CPY $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // CMP Absolute
  case 0xCD:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  CMP $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // DEC Absolute
  case 0xCE:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X  DEC $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *DCP Absolute - Illegal Instruction
  case 0xCF:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X %02X *DCP $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // BNE
  case 0xD0:
      
    sprintf(output, "%04X  %02X %02X     BNE $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + ((int8_t) read_mem(state, state->cpu->current_opcode_PC+1)) + 2);
    break;

    // CMP indirect-indexed,Y
  case 0xD1:
    {
      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* uint16_t offset = (uint16_t) state->cpu->registers->Y; */
      /* if (((base & 0xFF) + offset) > 0xFF) { */
      /*   if (high_addr < 0xFF) { high_addr++; } */
      /* } */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     CMP ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // *DCP indirect-indexed,Y - Illegal instruction
  case 0xD3:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *DCP ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;
    
    // CMP Zeropage, X
  case 0xD5:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     CMP $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // DEC Zeropage, X
  case 0xD6:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     DEC $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *DCP Zeropage, X -- Illegal instruction
  case 0xD7:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *DCP $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    
  // CLD
  case 0xD8:
    sprintf(output, "%04X  %02X        CLD",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

  // CMP Absolute Y
  case 0xD9:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  CMP $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));    
  }
  break;

  // *DCP Absolute Y - Illegal instruction
  case 0xDB:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *DCP $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));    
  }
  break;

  
    // CMP Absolute X
  case 0xDD:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  CMP $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // DEC Absolute X
  case 0xDE:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  DEC $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

  // *DCP Absolute X - Illegal instruction
  case 0xDF:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *DCP $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));    
  }
  break;



    
    // CPX immediate
  case 0xE0:
    sprintf(output, "%04X  %02X %02X     CPX #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // SBC indirect,X
  case 0xE1:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     SBC ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;

    // *ISB indirect,X
  case 0xE3:
    {
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint16_t addr_addr = (uint16_t) state->cpu->registers->X + (uint16_t) operand;
      addr_addr &= 0xFF;
      uint16_t effective_addr = ((uint16_t) read_mem(state, addr_addr)) | (((uint16_t) read_mem(state, (addr_addr+1) & 0xFF)) << 8);
      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *ISB ($%02X,X) @ %02X = %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              addr_addr,
              effective_addr,
              value);
    }
    break;


    
    // CPX Zeropage
  case 0xE4:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      sprintf(output, "%04X  %02X %02X     CPX $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // SBC Zeropage
  case 0xE5:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     SBC $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // INC Zeropage
  case 0xE6:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X     INC $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *ISB Zeropage - Illegal instruction
  case 0xE7:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);

      sprintf(output, "%04X  %02X %02X    *ISB $%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
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
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // NOP
  case 0xEA:
    sprintf(output, "%04X  %02X        NOP",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;
    
    // SBC "Illegal" opcode
  case 0xEB:
    sprintf(output, "%04X  %02X %02X    *SBC #$%02X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            read_mem(state, state->cpu->current_opcode_PC+1));
    break;
    
    // CPX Absolute
  case 0xEC:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      sprintf(output, "%04X  %02X %02X %02X  CPX $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // SBC Absolute
  case 0xED:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      sprintf(output, "%04X  %02X %02X %02X  SBC $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;
    
    // INC Absolute
  case 0xEE:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      sprintf(output, "%04X  %02X %02X %02X  INC $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    // *ISB Absolute - Illegal instruction
  case 0xEF:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      sprintf(output, "%04X  %02X %02X %02X *ISB $%02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, addr));
    }
    break;

    
    // BEQ
  case 0xF0:
    sprintf(output, "%04X  %02X %02X     BEQ $%04X",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode,
            read_mem(state, state->cpu->current_opcode_PC+1),
            state->cpu->current_opcode_PC + read_mem(state, state->cpu->current_opcode_PC+1) + 2);
    break;

    // SBC indirect-indexed,Y
  case 0xF1:
    {
      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* uint16_t offset = (uint16_t) state->cpu->registers->Y; */
      /* if (((base & 0xFF) + offset) > 0xFF) { */
      /*   if (high_addr < 0xFF) { high_addr++; } */
      /* } */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X     SBC ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    // *ISB indirect-indexed,Y - Illegal instruction
  case 0xF3:
    {
      /* uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1); */
      /* uint8_t low_addr = read_mem(state, (uint16_t) operand); */
      /* uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1)); */

      /* // check if page boundary was crossed and fix addresses */
      /* uint16_t base = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* uint16_t offset = (uint16_t) state->cpu->registers->Y; */
      /* if (((base & 0xFF) + offset) > 0xFF) { */
      /*   if (high_addr < 0xFF) { high_addr++; } */
      /* } */
      /* uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8; */
      /* effective_addr += (uint16_t) state->cpu->registers->Y; */
      uint8_t operand = read_mem(state, state->cpu->current_opcode_PC+1);
      uint8_t low_addr = read_mem(state, (uint16_t) operand);
      uint8_t high_addr = read_mem(state, (uint16_t) (operand + 1));

      uint16_t effective_addr = (uint16_t) low_addr | ((uint16_t) high_addr) << 8;
      effective_addr += (uint16_t) state->cpu->registers->Y;

      uint8_t value = read_mem(state, effective_addr);
      sprintf(output, "%04X  %02X %02X    *ISB ($%02X),Y = %02X%02X @ %04X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              operand,
              operand,
              high_addr,
              low_addr,
              effective_addr,
              value);
    }
    break;

    
    // SBC Zeropage, X
  case 0xF5:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     SBC $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // INC Zeropage, X
  case 0xF6:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X     INC $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    // *ISB Zeropage, X - Illegal instruction
  case 0xF7:
    {
      uint16_t addr = (uint16_t) read_mem(state, state->cpu->current_opcode_PC+1);
      addr += state->cpu->registers->X;
      addr &= 0xFF;
      sprintf(output, "%04X  %02X %02X    *ISB $%02X,X @ %02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      (uint8_t) addr,
              read_mem(state, addr));
    }
    break;

    
    // SED
  case 0xF8:
    sprintf(output, "%04X  %02X        SED",
            state->cpu->current_opcode_PC,
            state->cpu->current_opcode);
    break;

  // SBC Absolute Y
  case 0xF9:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X  SBC $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              /* read_mem(state, state->cpu->current_opcode_PC+2), */
              /* read_mem(state, state->cpu->current_opcode_PC+1), */
              read_mem(state, addr));
      
  }
  break;


  // *ISB Absolute Y
  case 0xFB:
  {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->Y) > 0xFFFF) {
	  addr = state->cpu->registers->Y - 1;
      }
      else {
	  addr += state->cpu->registers->Y;
      }
      sprintf(output, "%04X  %02X %02X %02X *ISB $%02X%02X,Y @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              /* read_mem(state, state->cpu->current_opcode_PC+2), */
              /* read_mem(state, state->cpu->current_opcode_PC+1), */
              read_mem(state, addr));
      
  }
  break;

  
    // SBC Absolute X
  case 0xFD:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  SBC $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // INC Absolute X
  case 0xFE:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X  INC $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
    break;

    // *ISB Absolute X
  case 0xFF:
    {
      uint16_t addr = read_mem(state, state->cpu->current_opcode_PC+2) << 8;
      addr |= read_mem(state, state->cpu->current_opcode_PC+1);
      // Handle wrap-around
      if (((uint32_t) addr) + ((uint32_t) state->cpu->registers->X) > 0xFFFF) {
	  addr = state->cpu->registers->X - 1;
      }
      else {
	  addr += state->cpu->registers->X;
      }
      sprintf(output, "%04X  %02X %02X %02X *ISB $%02X%02X,X @ %02X%02X = %02X",
              state->cpu->current_opcode_PC,
              state->cpu->current_opcode,
              read_mem(state, state->cpu->current_opcode_PC+1),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+2),
              read_mem(state, state->cpu->current_opcode_PC+1),
	      addr >> 8,
	      addr & 0xFF,
              read_mem(state, addr));
    }
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
         state->ppu->ppu_scanline,
         state->ppu->ppu_cycle,
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
          state->ppu->ppu_scanline,
          state->ppu->ppu_cycle,
          (int32_t) state->cpu->cpu_cycle);

}
void logger_stop_logger()
{
  fclose(logfile);
}
