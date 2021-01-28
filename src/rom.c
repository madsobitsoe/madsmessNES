#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#include "rom.h"


const char* addressing_mode_strings[] = {
  "Immediate",
  "Zero Page",
  "Absolute",
  "Implied",
  "Accumulator",
  "Indexed",
  "Zero-Page Indexed",
  "Indirect",
  "Pre-indexed Indirect",
  "Post-indexed Indirect",
  "Relative"
};



// Dummy unknown instruction
// Should be removed when table is fully implemented
const instruction_entry unknown_instruction = { .instruction = "UNKNOWN", .length = 1, .cycles = 1, .addressing_mode = IMMEDIATE };


const int table_size = 0x100;

instruction_entry table[table_size];


void hexdump(unsigned char *rombuf, int length){
  // Loop through the read bytes, printing them 8 at a time
  for (int i = 0; i < length; i++) {
    if (i % 16 == 0) { printf("\n%08x\t", i); }
    printf("%02x ", rombuf[i]);
  }
  printf("\n");
}


void print_instruction_entry(instruction_entry *entry, int addr, uint8_t opcode) {
  printf("%04x\t%-16s  Length: %2d  cycles: %2d  AddrMode: %s opcode: 0x%02x\n",
         addr,
         entry->instruction,
         entry->length,
         entry->cycles,
         addressing_mode_strings[entry->addressing_mode],
         opcode);
}



// Parses a header of the iNES format (https://wiki.nesdev.com/w/index.php/INES#iNES_file_format)
// returns the offset where the "actual rom" begins.
int print_header(unsigned char* rombuf) {
  /* // Header is 16 bytes */
  /* The format of the header is as follows: */
  /*   0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file) */
  /*   4: Size of PRG ROM in 16 KB units */
  /*   5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM) */
  /*   6: Flags 6 - Mapper, mirroring, battery, trainer */
  /*   7: Flags 7 - Mapper, VS/Playchoice, NES 2.0 */
  /*   8: Flags 8 - PRG-RAM size (rarely used extension) */
  /*   9: Flags 9 - TV system (rarely used extension) */
  /*   10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension) */
  /*   11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15) */
  /*   Flags 6 */
  // First four bytes
  printf("First four bytes: ");
  for (int i = 0; i < 4; i++) {
    printf("%c", rombuf[i]);
  }
  printf("\n");
  printf("Size of PRG ROM in 16 KB units: %d\n", rombuf[4]);
  printf("Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM) : %d\n", rombuf[5]);
  printf("\n");


  /*   6: Flags 6 - Mapper, mirroring, battery, trainer */
  /* 6        */
  /* bit 0     1 for vertical mirroring, 0 for horizontal mirroring. */
  /* bit 1     1 for battery-backed RAM at $6000-$7FFF. */
  /* bit 2     1 for a 512-byte trainer at $7000-$71FF. */
  /* bit 3     1 for a four-screen VRAM layout.  */
  /* bit 4-7   Four lower bits of ROM Mapper Type. */

  /* 7: Flags 7 - Mapper, VS/Playchoice, NES 2.0 */
  /* bit 0     1 for VS-System cartridges. */
  /* bit 1-3   Reserved, must be zeroes! */
  /* bit 4-7   Four higher bits of ROM Mapper Type. */
  /* 8: Flags 8 - PRG-RAM size (rarely used extension) */
  /* Number of 8kB RAM banks. For compatibility with the previous */
  /*   versions of the .NES format, assume 1x8kB RAM page when this */
  /*   byte is zero. */
  /* 9: Flags 9 - TV system (rarely used extension) */
  /* bit 0     1 for PAL cartridges, otherwise assume NTSC. */
  /* bit 1-7   Reserved, must be zeroes! */
  /* 10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension) */
  /*   11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15) */

  printf("Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15): ");
  for (int i = 11; i < 16; i++) {
    printf("%c", rombuf[i]);
  }
 printf("\n");
  return 16;
}

// prints instruction,
// returns length of instruction
int decode_instruction(uint8_t opcode, int addr) {
  instruction_entry entry = table[opcode];
  print_instruction_entry(&entry, addr, opcode);
  return entry.length;
}

void decode_rom(unsigned char *rombuf, int length){
  int counter = 0;
  while (counter < length) {
    int incr_by = decode_instruction(rombuf[counter], counter);
    counter += incr_by;
  }
}


void init_table() {
  // Fill table with unknown instruction
  for (int i = 0; i < table_size; i++) {
    table[i] = unknown_instruction;
  };


  // Initialize actual table
  table[0].instruction = "BRK", table[0].length = 2, table[0].cycles = 7, table[0].addressing_mode = IMMEDIATE;
  /*   (Indirect, X)	ORA (Operand, X)	01	2	6 */
  table[1].instruction = "ORA (Operand, X)", table[1].length = 2, table[1].cycles = 6, table[1].addressing_mode = PRE_INDEXED_INDIRECT;
  /*   Zero Page	ORA Operand	05	2	3 */
  table[5].instruction = "ORA Operand", table[5].length = 2, table[5].cycles = 3, table[5].addressing_mode = ZEROPAGE;
  /* Immediate	ORA #Operand	09	2	2 */
  table[9].instruction = "ORA #Operand", table[9].length = 2, table[9].cycles = 2, table[9].addressing_mode = IMMEDIATE;
  /*   Zero Page, X	ORA Operand, X	15	2	4 */
  table[0x15].instruction = "ORA Operand, X", table[0x15].length = 2, table[0x15].cycles = 4, table[0x15].addressing_mode = ZEROPAGE_INDEXED;
  /*   Absolute	ORA Operand	0D	3	4 */
  table[0x0d].instruction = "ORA Operand", table[0x0d].length = 3, table[0x0d].cycles = 4, table[0x0d].addressing_mode = ABSOLUTE;
  /*   (Indirect), Y	ORA (Operand), Y	11	2	5* */
  table[0x11].instruction = "ORA (Operand), Y", table[0x11].length = 2, table[0x11].cycles = 5, table[0x11].addressing_mode = POST_INDEXED_INDIRECT;
  /*   Absolute, Y	ORA Operand, Y	19	3	4* */
  table[0x19].instruction = "ORA Operand, Y", table[0x19].length = 3, table[0x19].cycles = 4, table[0x19].addressing_mode = ABSOLUTE;
  /*   Absolute, X	ORA Operand, X	1D	3	4* */
  table[0x1d].instruction = "ORA Operand, X", table[0x1d].length = 3, table[0x1d].cycles = 4, table[0x1d].addressing_mode = ABSOLUTE;
  /* Implied	NOP	EA	1	2 */
  table[0xea].instruction = "NOP", table[0xea].length = 1, table[0xea].cycles = 2, table[0xea].addressing_mode = IMPLIED;

  table[0x78].instruction = "SEI", table[0x78].length = 1, table[0x78].cycles = 2, table[0x78].addressing_mode = IMPLIED;
  table[0xd8].instruction = "CLD", table[0xd8].length = 1, table[0xd8].cycles = 2, table[0xd8].addressing_mode = IMPLIED;
  table[0x8d].instruction = "STA", table[0x8d].length = 3, table[0x8d].cycles = 4, table[0x8d].addressing_mode = ABSOLUTE;
  table[0xa2].instruction = "LDX", table[0xa2].length = 2, table[0xa2].cycles = 2, table[0xa2].addressing_mode = IMMEDIATE;
  table[0x9a].instruction = "TXS", table[0x9a].length = 1, table[0x9a].cycles = 2, table[0x9a].addressing_mode = IMPLIED;
  table[0x10].instruction = "BPL", table[0x10].length = 1, table[0x10].cycles = 1, table[0x10].addressing_mode = IMPLIED;
  // LDA
  /* Immediate	LDA #Operand	A9	2	2 */
  table[0xa9].instruction = "LDA #Operand", table[0xa9].length = 2, table[0xa9].cycles = 2, table[0xa9].addressing_mode = IMMEDIATE;
  /*   Zero Page	LDA Operand	A5	2	3 */
  table[0xa5].instruction = "LDA Operand", table[0xa5].length = 2, table[0xa5].cycles = 3, table[0xa5].addressing_mode = ZEROPAGE;
  /*   Zero Page, X	LDA Operand, X	B5	2	4 */
  table[0xb5].instruction = "LDA Operand, X", table[0xb5].length = 2, table[0xb5].cycles = 4, table[0xb5].addressing_mode = ZEROPAGE_INDEXED;
  /*   Absolute	LDA Operand	AD	3	4 */
  table[0xad].instruction = "LDA Operand", table[0xad].length = 3, table[0xad].cycles = 4, table[0xad].addressing_mode = ABSOLUTE;
  table[0x4c].instruction = "JMP", table[0x4c].length = 3, table[0x4c].cycles = 3, table[0x4c].addressing_mode = ABSOLUTE;
  table[0x6c].instruction = "JMP", table[0x6c].length = 3, table[0x6c].cycles = 5, table[0x6c].addressing_mode = INDIRECT;
  table[0x86].instruction = "STX", table[0x86].length = 2, table[0x86].cycles = 3, table[0x86].addressing_mode = ZEROPAGE;
  /*   Absolute, X	LDA Operand, X	BD	3	4* */
  /*   Absolute, Y	LDA Operand, Y	B9	3	4* */
  /*   (Indirect, X)	LDA (Operand, X)	A1	2	6 */
  /*   (Indirect), Y	LDA (Operand), Y	B1	2	5* */

}


/* int main (int argc, char **argv) { */
/*   printf("Awesome!\n"); */

/*   init_table(); */


/*   struct stat romfilestat; */

/*   FILE *infile; */
/*   // Buffer to read into */
/*   unsigned char *p; */

/*   if (argc < 2) { */
/*     fprintf(stderr, "Usage: %s (<filename>|-)\n", argv[0]); */
/*     return EXIT_FAILURE; */
/*   } */

/*   char* filename = argv[1]; */
/*   int fstatus = stat(filename, &romfilestat); */
/*   if (fstatus != 0) { perror("stat call failed.\n"); return EXIT_FAILURE; } */

/*   printf("Filename: %s\nFilesize: %lld\n", filename, romfilestat.st_size); */
/*   if ((infile = fopen(filename, "r")) == NULL) { */
/*     perror("fopen() failed"); */
/*     return EXIT_FAILURE; */
/*   } */
/*   p = malloc(romfilestat.st_size); */

/*   fread(p, 1, romfilestat.st_size, infile); */
/*   fclose(infile); */

/*   /\* hexdump(p, romfilestat.st_size); *\/ */
/*   int offset = print_header(p); */
/*   decode_rom(p+offset, romfilestat.st_size - offset); */
/*   /\* for (int i = 0; i < 5; i++) { *\/ */
/*   /\*   print_instruction_entry(&table[i]); *\/ */
/*   /\* } *\/ */

/*   free(p); */

/*   return 0; */
/* } */
