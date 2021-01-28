#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include "cpu.h"
#include "nes.h"
/* #include "rom.h" */
#include "logger.h"

// gdb-like step interface
void ndb(nes_state *state) {
  // Print current state, i.e. registers
  print_state(state);

  /* uint8_t ni; */
  /* uint16_t translated = translate_memory_location(state->current_opcode_PC); */
  /* printf("Trans: 0x%04x\n", translated); */
  // Print the next 20 bytes (instructions) after PC
  /* if (translated <= 0x7ff) { */
  /*   printf("reading from RAM\n"); */
  /*   int counter = 0; */
  /*   while (counter < 20) { */
  /*     int incr_by = decode_instruction(state->memory[translated+counter - 0x8000], translated+counter); */
  /*     counter += incr_by; */
  /*   } */
  /* } */
  /* else */
  /*   { */
  /*     printf("reading from ROM\n"); */
  /*     int counter = 0; */
  /*     uint16_t offset = translated - 0xc000; */
  /*     printf("Trying to read from 0x%04x.\n", offset); */
  /*     while (counter < 20) { */
  /*       int incr_by = decode_instruction(state->rom[offset+counter], translated+counter); */
  /*       counter += incr_by; */
  /*     } */
  /*   } */
  /* printf("\n"); */

  print_log(state);

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


void run_for_n_cycles(nes_state *state, uint32_t cycles) {
  for (uint32_t i = 0; i < cycles; i++)
    {
      step(state);
    }
}

int main (int argc, char **argv) {
  int opt;
  bool interactive = true;
  char *logfile = "testlog.log";
  uint32_t cycles_to_run = 0;
  opterr = 0;
  while ((opt = getopt(argc, argv, "l:c:")) != -1) {
    switch (opt) {
    case 'l':
      printf("Filename is: %s\n", optarg);
      logfile = optarg;
      break;
    case 'c':
      interactive = false;
      cycles_to_run = (uint32_t) strtol(optarg, NULL, 10);
      break;
    case '?':
      if (optopt == 'c')
        fprintf (stderr, "Option -%c requires cycles as an argument.\n", optopt);
      else if (optopt == 'l')
        fprintf (stderr, "Option -%c requires a filename as an argument.\n", optopt);
      else if (isprint (optopt))
        fprintf (stderr, "Unknown option `-%c'.\n", optopt);
      else {
        fprintf (stderr,
                 "Unknown option character `\\x%x'.\n",
                 optopt);
        return 1;
      }
    default:
      fprintf(stderr, "Usage: %s [-cl] [file...]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // Now optind (declared extern int by <unistd.h>) is the index of the first non-option argument.
  // If it is >= argc, there were no non-option arguments.

  /* printf ("interactive = %d, logfile = %s, cycles = %u\n", */
  /*         interactive, logfile, cycles_to_run); */

  /* for (int index = optind; index < argc; index++) */
  /*   printf ("Non-option argument %s\n", argv[index]); */

  if (optind >= argc) {
    fprintf(stderr, "Usage: %s (<filename>|-)\n", argv[0]);
    return EXIT_FAILURE;
  }


  // Initialize the logger
  logger_init_logger(logfile);


  unsigned char *rombuf;
  int load_rom_status = load_rom(argv[optind], &rombuf);
  if (load_rom_status != 0) {
    return load_rom_status;
  };
  printf("Rom successfully loaded...\n");

  printf("Initializing state... ");
  nes_state *state = init_state();
  printf("Done!\n");
  printf("Attaching rom...");
  //+16 to skip nes header
  attach_rom(state, rombuf+16);
  printf(" Done!\n");
  printf("Powering on console..");
  reset(state);
  printf("Done!\n");

  /* printf("Trying to read the initial value for PC with read_mem..\n"); */
  /* uint16_t reset_vector_value = read_mem_short(state, 0xfffc); */
  /* printf("Value: 0x%x\n", reset_vector_value); */
  /* set_pc(state, reset_vector_value); */
  // The nestest.nes file requires PC to be set to this address after powerup.
  set_pc(state, 0xc000);
  if (interactive) {
    printf("Entering debug loop..\n");
    ndb(state);
  }
  else {
    printf("Running for %d cycles.\n", cycles_to_run);
    run_for_n_cycles(state, cycles_to_run);
  }

  logger_stop_logger();

  destroy_state(state);
  free(rombuf);
  return 0;
}
