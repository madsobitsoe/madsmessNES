IDIR=include
CC=gcc -Wall -Wextra
CFLAGS=-I$(IDIR)
SRCDIR=src
ODIR=obj

LIBS=

_DEPS = cpu.h rom.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = cpu.o rom.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

emu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f emu
