#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include "definitions.h"




uint8_t get_prg_rom_size(uint8_t *rombuf) {
  return rombuf[4];
}
uint8_t get_chr_rom_size(uint8_t *rombuf) {
  return rombuf[5];
}

bool get_mirroring(uint8_t *rombuf) {
  if (rombuf[6] & 1) { return true; } else { return false; }
}

bool get_battery(uint8_t *rombuf) {
  if (rombuf[6] & 2) { return true; } else { return false; }
}

bool get_trainer(uint8_t *rombuf) {
  if (rombuf[6] & 4) { return true; } else { return false; }
}

bool get_four_screen_vram(uint8_t *rombuf) {
  if (rombuf[6] & 8) { return true; } else { return false; }
}

uint8_t get_mapper(uint8_t *rombuf) {
  uint8_t low = rombuf[6] & 0xF0;
  uint8_t high = rombuf[7] & 0xF0;
  printf("low bytes: %02X, high: %02X\n", low, high);
  return (high | low >> 4);
}

bool get_vs_system_cartridge(uint8_t *rombuf) {
  return (rombuf[7] & 1);
}


int load_rom2(char *filename, uint8_t **rombuf, nes_rom *rom) {

  FILE *infile;
  struct stat romfilestat;
  int fstatus = stat(filename, &romfilestat);
  if (fstatus != 0) { perror("stat call failed.\n"); return EXIT_FAILURE; }

  printf("Filename: %s\nFilesize: %ld\n", filename, romfilestat.st_size);
  if ((infile = fopen(filename, "r")) == NULL) {
    perror("fopen() failed");
    return EXIT_FAILURE;
  }
  *rombuf = malloc(romfilestat.st_size);

  fread(*rombuf, 1, romfilestat.st_size, infile);
  fclose(infile);

  rom->prg_rom_size = get_prg_rom_size(*rombuf);
  rom->chr_rom_size = get_chr_rom_size(*rombuf);
  rom->mirroring = get_mirroring(*rombuf);
  rom->battery_backed = get_battery(*rombuf);
  rom->trainer = get_trainer(*rombuf);
  rom->four_screen_VRAM = get_four_screen_vram(*rombuf);
  rom->vs_system_cartridge = get_vs_system_cartridge(*rombuf);
  rom->mapper = get_mapper(*rombuf);

  rom->prg_rom1 = malloc(0x4000);
  rom->prg_rom2 = malloc(0x4000);
  uint32_t prg_start = 16;
  if (rom->trainer) { prg_start += 512; }
  // If only 16kb PRG_ROM, mirror it
  if (rom->prg_rom_size == 1) {
    memcpy(rom->prg_rom1, *rombuf+prg_start, 0x4000);
    memcpy(rom->prg_rom2, *rombuf+prg_start, 0x4000);
  }
  if (rom->prg_rom_size == 2) {
    memcpy(rom->prg_rom1, *rombuf+prg_start, 0x4000);
    memcpy(rom->prg_rom2, *rombuf+prg_start+0x4000, 0x4000);
  }

  // For now, we ONLY do mapper 0, which does not bankswitch
  // And has a maximum of 8 kb CHR rom available
  rom->chr_rom = malloc(0x2000);
  uint32_t chr_rom_offset = prg_start + (rom->prg_rom_size * 0x4000);
  memcpy(rom->chr_rom, *rombuf+chr_rom_offset, 0x2000);

  return 0;
}


void print_byte(uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    if (byte & (1 << i)) {
      printf("\x1b[1;31m1 \x1b[0m");
    }
    else {
      printf("0 ");
    }
  }
}

void pattern_table_dump(nes_rom *rom) {
  // 16 bytes
  // first 8 == low bytes
  // second 8 == high bytes
  // Read both bytes, OR them together
  // That becomes the 8x8 pixels
  // Draw the "lit ones" in red, others as 0's
  int index = 0;
  while (index < 0x2000) {
    /* for (int i = 0; i < 64; i++) { */
    uint8_t tile_byte = rom->chr_rom[index] | rom->chr_rom[index+8];
    print_byte(tile_byte);
    printf("\n");
    index++;
    if (index % 8 == 0) { index += 8; }
  }
}

void print_rom_info(nes_rom *rom) {
  printf("prg_rom_size: %u\n", rom->prg_rom_size);
  printf("chr_rom_size: %u\n", rom->chr_rom_size);
  if (rom->mirroring) { printf("mirroring: vertical\n"); } else { printf("mirroring: vertical\n"); }
  if (rom->battery_backed) { printf("battery: yes\n"); } else { printf("battery: false\n"); }
  printf("Trainer: ");
  if (rom->trainer) { printf("yes\n"); } else { printf("no\n"); }
  printf("Four screen VRAM: ");
  if (rom->four_screen_VRAM) { printf("yes\n"); } else { printf("no\n"); }
  printf("vs system cartridge: ");
  if (rom->vs_system_cartridge) { printf("yes\n"); } else { printf("no\n"); }

  printf("Mapper: %u\n", rom->mapper);


  /* printf("CHR_ROM dump:\n"); */
  /* for (int i = 0; i < 0x2000; i++) { */
  /*   if (i % 16 == 0) { printf("\n"); } */
  /*   printf("%02X ", rom->chr_rom[i]); */
  /* } */

  /* printf("here we go!\n"); */
  /* pattern_table_dump(rom); */

  /* for (int i = 0; i < 0xfff; i++) { */
  /*   printf("i: %d, test: %02X, %02X\n", i, rom->prg_rom1[i], rom->prg_rom2[i]); */
  /* } */
  /* printf("Reset vector: %02X%02X\n", rom->prg_rom2[0xfffd-0xc000], rom->prg_rom2[0xfffc-0xc000]); */

}


void free_rom(nes_rom *rom) {
  free(rom->prg_rom1);
  free(rom->prg_rom2);
  free(rom->chr_rom);
  free(rom);
}
