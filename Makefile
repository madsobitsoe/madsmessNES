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

emu: src/cpu.c src/nes.c src/logger.c src/memory.c src/main.c
	gcc -Wall -Wextra -o emu src/memory.c src/cpu.c src/nes.c src/logger.c src/main.c -Iinclude -lreadline

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f emu
