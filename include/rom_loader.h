#ifndef ROM_LOADER_H
#define ROM_LOADER_H
#include "definitions.h"

// Load the rom "filename" into rombuf, create the nes_rom
// returns 0 on success, 1 on error
int load_rom2(char *filename, uint8_t **rombuf, nes_rom *rom);
// Return a pointer to an empty nes_rom struct
/* static nes_rom* init_rom_struct(); */
// Free the allocations separate to ROM
void free_rom(nes_rom *rom);
void print_rom_info(nes_rom *rom);

#endif
