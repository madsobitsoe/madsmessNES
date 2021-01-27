#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu.h"
#include "rom.h"


// gdb-like step interface
void ndb(nes_state *state) {
  // Print current state, i.e. registers
  print_state(state);

  /* uint8_t ni; */
  uint16_t translated = translate_memory_location(state->current_opcode_PC);
  printf("Trans: 0x%04x\n", translated);
  // Print the next 20 bytes (instructions) after PC
  if (translated <= 0x7ff) {
    printf("reading from RAM\n");
    int counter = 0;
    while (counter < 20) {
      int incr_by = decode_instruction(state->memory[translated+counter - 0x8000], translated+counter);
      counter += incr_by;
    }
  }
  else
    {
      printf("reading from ROM\n");
      int counter = 0;
      uint16_t offset = translated - 0xc000;
      printf("Trying to read from 0x%04x.\n", offset);
      while (counter < 20) {
        int incr_by = decode_instruction(state->rom[offset+counter], translated+counter);
        counter += incr_by;
      }
    }
  printf("\n");



  // Get input from user
  printf("s: step, q: quit\n");
  char input;
  scanf(" %c", &input);
  switch(input) {
  case 'q':
    printf("Exiting..\n");
    state->running = false;
    break;
  case 's':
    printf("stepping.\n");
    step(state);
    break;
  default:
    printf("Invalid input\n");
  }
  if (state->running) { ndb(state); }
  else { return; }
}



int load_rom(char *filename, unsigned char **rombuf) {

  FILE *infile;
  struct stat romfilestat;
  int fstatus = stat(filename, &romfilestat);
  if (fstatus != 0) { perror("stat call failed.\n"); return EXIT_FAILURE; }

  printf("Filename: %s\nFilesize: %lld\n", filename, romfilestat.st_size);
  if ((infile = fopen(filename, "r")) == NULL) {
    perror("fopen() failed");
    return EXIT_FAILURE;
  }
  *rombuf = malloc(romfilestat.st_size);

  fread(*rombuf, 1, romfilestat.st_size, infile);
  fclose(infile);
  //  hexdump(*rombuf, romfilestat.st_size);
  return 0;

}

int main (int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s (<filename>|-)\n", argv[0]);
    return EXIT_FAILURE;
  }
  init_table();
  /*   char* filename = argv[1]; */
  unsigned char *rombuf;
  int load_rom_status = load_rom(argv[1], &rombuf);
  if (load_rom_status != 0) {
    return load_rom_status;
  };
  printf("Rom successfully loaded...\n");
  printf("Header:\n");
  print_header(rombuf);


  printf("Initializing state... ");
  nes_state *state = init_state();
  printf("Done!\n");
  printf("Attaching rom...");
  //+16 to skip nes header
  attach_rom(state, rombuf+16);
  printf(" Done!\n");

  printf("Trying to read the initial value for PC with read_mem..\n");
  uint16_t reset_vector_value = read_mem_short(state, 0xfffc);
  printf("Value: 0x%x\n", reset_vector_value);
  set_pc(state, reset_vector_value);
  set_pc(state, 0xc000);
  printf("Entering debug loop..\n");
  ndb(state);

  destroy_state(state);
  free(rombuf);
  return 0;
}
