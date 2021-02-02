#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cpu.h"
#include "nes.h"
#include "logger.h"
#include "memory.h"

void run_for_n_cycles(nes_state *state, uint32_t cycles) {
  uint32_t count = 0;
  while (count < cycles && !state->fatal_error) {
    /* for (uint32_t i = 0; i < cycles; i++) */
    /*        { */
    step(state);
    count++;
  }
  if (state->fatal_error) {
    printf("Fatal error at cycle: %llu\n", state->cpu->cpu_cycle);
    print_state(state);
    print_log(state);
  }
}

int parse_cmd(char *line) {
  // Quit on 0
  if (strncmp(line, "q", 1) == 0) { return 0; }
  // Step N on return N
  if (strncmp(line, "s", 1) == 0) {
    int steps = 1; // Default step
    if (strlen(line) == 1) { return steps; }
    // Check if there is a base 10 number supplied
    steps = strtol(line+1, NULL, 10);
    // return if conversion succeeded
    if (steps > 0) { return steps; }
    // If not, check if the input was in hex
    steps = strtol(line+1, NULL, 16);
    if (steps > 0) { return steps; }
  }
  else if (strncmp(line, "p", 1) == 0) {
    int loc = 0;
    if (strlen(line) == 1) { return -1; }
    loc = strtol(line+1, NULL, 16);
    if (loc >= 0) { return (-2 - loc); }
  }
  // Invalid input on -1
  return -1;
}

void print_mem(nes_state *state, uint16_t loc) {
  printf("Printing memory starting at: %04X\n", loc);
  uint8_t count = 0;
  while (count < 64) {
    printf("%02X ", read_mem(state, loc+count));
    count++;
  }
  printf("\n");

}

// gdb-like step interface
void ndb(nes_state *state) {

  // Get input from user
  char* linebuf;
  int cmd;
  while (state->running) {
    // Print current state, i.e. registers
    print_state(state);

    print_log(state);

    linebuf = readline("s [N]: step, p N: print memory, q: quit >");
    if (strlen(linebuf) > 0) {
      add_history(linebuf);
      cmd = parse_cmd(linebuf);
    }
    switch(cmd) {
    case 0:
      printf("Exiting..\n");
      state->running = false;
      break;
    case (-1):
      printf("Invalid input\n");
      break;
    case 1:
      /* printf("stepping.\n"); */
      step(state);
      break;
    default:
      if (cmd > 0) {
        run_for_n_cycles(state, cmd);
               break;
      }
      else {
        print_mem(state, (uint16_t) (-2 - cmd));
      }
    }
    // readline malloc's a new linebuffer every time. Free it.
    free(linebuf);

  }
  return;
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
  return 0;

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
