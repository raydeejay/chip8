BINARY=chip8
CC=gcc
CFLAGS=-O3 -g -Wall -pedantic `sdl2-config --cflags`
LDFLAGS=-lm `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf

CFILES=main.c chip8.c fontset.c opcodes.c

.PHONY: clean

all: $(BINARY)

$(BINARY): main.o chip8.o fontset.o opcodes.o
	${CC} ${CFLAGS} $^ ${LDFLAGS} -o ${BINARY}

#.c.o: terminal.h buffer.h aria.h api.h
#	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -f *.o ${BINARY} nul

# for flymake
check-syntax:
	gcc -o nul -S ${CHK_SOURCES}

make.depend: main.c chip8.c chip8.h fontset.c opcodes.c opcodes.h
	touch make.depend
	makedepend -I/usr/include/linux -I/usr/lib/gcc/x86_64-linux-gnu/5/include/ -fmake.depend $^

-include make.depend
