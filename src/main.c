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
#include "rom_loader.h"


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
  uint8_t flags = rombuf[6];
  if (flags & 1) { printf("mirroring: vertical\n"); } else { printf("mirroring: horizontal\n"); }
  if (flags & 2) { printf("battery-backed RAM at $6000-$7FFF: yes\n"); } else { printf("battery-backed RAM at $6000-$7FFF: no\n"); }
  if (flags & 4) { printf("512-byte trainer at $7000-$71FF: yes\n"); } else { printf("512-byte trainer at $7000-$71FF: no\n"); }
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
  uint16_t offset = 16;
  if (flags & 4) { offset += 512; }
  return offset;
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
  uint16_t new_pc = 0xFFFD;
  bool overwrite_pc = false;
  opterr = 0;
  while ((opt = getopt(argc, argv, "l:c:s:")) != -1) {
    switch (opt) {
    case 'l':
      printf("Filename is: %s\n", optarg);
      logfile = optarg;
      break;
    case 'c':
      interactive = false;
      cycles_to_run = (uint32_t) strtol(optarg, NULL, 10);
      break;
    case 's':
      new_pc = (uint16_t) strtol(optarg, NULL, 16);
      overwrite_pc = true;
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
  /* int load_rom_status = load_rom(argv[optind], &rombuf); */
  /* if (load_rom_status != 0) { */
  /*   return load_rom_status; */
  /* }; */
  nes_rom *my_rom = malloc(sizeof(nes_rom));
  int romret = load_rom2(argv[optind], &rombuf, my_rom);
  printf("rom loaded: %d\n", romret);
  print_rom_info(my_rom);
  printf("Rom successfully loaded...\n");
  /* uint16_t rom_offset = print_header(rombuf); */
  printf("Initializing state... ");
  nes_state *state = init_state();
  printf("Done!\n");
  printf("Attaching rom...");
  //+offset to skip nes header and optional trainer
  /* attach_rom(state, rombuf+rom_offset); */
  attach_rom(state, my_rom);
  printf(" Done!\n");
  printf("Powering on console..");
  reset(state);
  printf("Done!\n");



  // The nestest.nes file requires PC to be set to 0xc000 after powerup.
  if (overwrite_pc) {
    set_pc(state, new_pc);
  }
  if (interactive) {
    printf("Entering debug loop..\n");
    ndb(state);
  }
  else {
    printf("Running for %d cycles.\n", cycles_to_run);
    run_for_n_cycles(state, cycles_to_run);
  }

  logger_stop_logger();
  /* free_rom(my_rom); */
  destroy_state(state);
  free(rombuf);
  return 0;
}
