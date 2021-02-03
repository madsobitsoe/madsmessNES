IDIR=include
CC=gcc -Wall -Wextra
CFLAGS=-I$(IDIR)
SRCDIR=src
ODIR=obj

all: emu

# _DEPS = definitions.h nes.h cpu.h logger.h
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# _OBJ = nes.o logger.o main.o
# OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


# $(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)


# $(ODIR)/nes.o: $(SRCDIR)/cpu.c $(SRCDIR)/nes.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

# $(ODIR)/logger.o: $(SRCDIR)/logger.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

# $(ODIR)/main.o: $(SRCDIR)/main.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

# emu: $(OBJ)
# 	$(CC) -o $@ $^ $(CFLAGS)

emu: src/cpu.c src/rom_loader.c src/nes.c src/ppu.c src/logger.c src/memory.c src/main.c src/rom_loader.c include/rom_loader.h include/nes.h include/cpu.h include/definitions.h include/ppu.h
	gcc -Wall -Wextra -o emu src/memory.c src/cpu.c src/ppu.c src/rom_loader.c src/nes.c src/logger.c src/main.c -Iinclude -lreadline

# terrible, but good enough for now
tileviewer: src/tile_viewer.c src/rom_loader.c include/rom_loader.h
	gcc -Wall -Wextra -o tile_viewer src/tile_viewer.c src/rom_loader.c `sdl2-config --cflags` -g `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm -Iinclude


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f emu
	rm -f tile_viewer
