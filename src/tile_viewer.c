#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "rom_loader.h"



/* Sets constants */
#define WIDTH 1024
#define HEIGHT 1024
#define DELAY 20000


void set_color(uint8_t color, SDL_Renderer *renderer) {
  switch (color) {
  case 0:
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    break;
  case 1:
    SDL_SetRenderDrawColor(renderer, 150, 50, 0, 255);
    break;
  case 2:
    SDL_SetRenderDrawColor(renderer, 240, 210, 00, 255);
    break;
  case 3:
    SDL_SetRenderDrawColor(renderer, 10, 30, 180, 255);
    break;
  }
}

// Draw a tile starting at x,y
void render_tile(uint8_t *buffer, uint8_t start_x, uint8_t start_y, SDL_Renderer *renderer) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      set_color(buffer[y * 8 + x], renderer);
      SDL_RenderDrawPoint(renderer, start_x+x, start_y+y);
    }
  }
}


void grab_next_tile(nes_rom *rom, uint8_t *tilebuffer, int index) {
  for (int i = 0; i < 8; i++) {
    uint8_t pixel_low = rom->chr_rom[index+i];
    uint8_t pixel_high = rom->chr_rom[index+i+8];

    for (int bit = 7; bit >= 0; bit--) {
      uint8_t bit_low = pixel_low & (1 << bit);
      uint8_t bit_high = pixel_high & (1 << bit);
      bit_low = bit_low >> bit;
      bit_high = bit_high >> bit;
      uint8_t pixel = bit_low | (bit_high << 1);
      /* uint8_t pixel = (pixel_low & ( 1 << bit)) | ((pixel_high & (1 << bit)) << 1); */
      tilebuffer[i*8+(7-bit)] = pixel;
    }
  }
}

int main (int argc, char **argv)
{

  // 1) Load the rom file
  // 2) Parse the header
  // 3) Grab the chr rom
  // 4) Convert them to "pixels"
  // 5) Open an SDL window
  // 6) Draw the pixels
  // 7) exit

  if (argc < 2) {
    printf("usage: %s ROMFILE\n", argv[0]);
    return 0;
  }

  unsigned char *rombuf;
  nes_rom *my_rom = malloc(sizeof(nes_rom));
  int romret = load_rom2(argv[1], &rombuf, my_rom);
  if (romret != 0) { printf("Something went wrong when loading rom..\n"); return 1; }
  printf("rom loaded: %d\n", romret);
  print_rom_info(my_rom);
  printf("Rom successfully loaded...\n");


  /* Initialises data */
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  /*
   * Initialises the SDL video subsystem (as well as the events subsystem).
   * Returns 0 on success or a negative error code on failure using SDL_GetError().
   */
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
    return 1;
  }

  SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);
  /* Checks if window has been created; if not, exits program */
  if (window == NULL) {
    fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
    return 1;
  }
  /* Checks if renderer has been created; if not, exits program */
  if (renderer == NULL) {
    fprintf(stderr, "SDL renderer failed to initialise: %s\n", SDL_GetError());
    return 1;
  }

  // Upscale by 2x, window is 512,512
  SDL_RenderSetLogicalSize(renderer, 256, 256);
  // Set up renderer (to draw with red)
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  uint8_t *tilebuf = malloc(64);
  int tile = 0;

  int x = 0;
  int y = 0;
  // With mapper 0, CHR_ROM can have a max size of 8kb
  // 8kb (0x2000) bytes of tiles
  // Each tile takes up 16 bytes
  // A max of 0x200 tiles
  while (tile < 0x200) {
    grab_next_tile(my_rom, tilebuf, tile*16);
    render_tile(tilebuf, x, y, renderer);
    x += 8;
    if (x >= 255) {
      x = 0;
      y += 8;
    }
    tile++;
  }


  // Show the window
  SDL_RenderPresent(renderer);

  // Just quit on all keypresses
  SDL_Event e;
  bool quit = false;
  while (!quit){
    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){
        quit = true;
      }
      if (e.type == SDL_KEYDOWN){
        quit = true;
      }
      if (e.type == SDL_MOUSEBUTTONDOWN){
        quit = true;
      }
    }
  }

  /* Frees memory */
  SDL_DestroyWindow(window);

  /* Shuts down all SDL subsystems */
  SDL_Quit();
  free_rom(my_rom);
  return 0;
           }
