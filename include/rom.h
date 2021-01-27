// Addressing modes for the nes
/* Immediate Addressing */
/* Zero-Page Addressing */
/* Absolute Addressing */
/* Implied Addressing */
/* Accumulator Addressing */
/* Indexed Addressing */
/* Zero-Page Indexed Addressing */
/* Indirect Addressing */
/* Pre-Indexed Indirect Addressing */
/* Post-Indexed Indirect Addressing */
/* Relative Addressing */
// http://www.romdetectives.com/Wiki/index.php?title=Addressing_Modes

#define IMMEDIATE             0
#define ZEROPAGE              1
#define ABSOLUTE              2
#define IMPLIED               3
#define ACCUMULATOR           4
#define INDEXED               5
#define ZEROPAGE_INDEXED      6
#define INDIRECT              7
#define PRE_INDEXED_INDIRECT  8
#define POST_INDEXED_INDIRECT 9
#define RELATIVE              10


typedef struct INSTRUCTION_ENTRY {
  char *instruction;
  int length;
  int cycles;
  int addressing_mode;
} instruction_entry;



void hexdump(unsigned char *rombuf, int length);
void print_instruction_entry(instruction_entry *entry, int addr, uint8_t opcode);
int print_header(unsigned char* rombuf);
int decode_instruction(unsigned char opcode, int addr);
void decode_rom(unsigned char *rombuf, int length);
void init_table();
