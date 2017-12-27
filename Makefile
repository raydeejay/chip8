BINARY=chip8
ASSEMBLER=asm8
CC=gcc
CFLAGS=-O3 -g -Wall -pedantic `sdl2-config --cflags`
LDFLAGS=-lm `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf

ASM_CFILES=asm8.o utils.c line-processor.c parser.c assembler.c labels.c
CFILES=main.c chip8.c fontset.c opcodes.c

.PHONY: clean

all: $(BINARY) $(ASSEMBLER)

$(BINARY): main.o chip8.o fontset.o opcodes.o
	${CC} ${CFLAGS} $^ ${LDFLAGS} -o ${BINARY}

$(ASSEMBLER): asm8.o utils.o line-processor.o parser.o assembler.o labels.o
	${CC} ${CFLAGS} $^ ${LDFLAGS} -o ${ASSEMBLER}

#.c.o: terminal.h buffer.h aria.h api.h
#	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -f *.o ${BINARY} ${ASSEMBLER} nul

# for flymake
check-syntax:
	gcc -Wall -pedantic -o nul -S ${CHK_SOURCES}

make.depend: main.c chip8.c chip8.h fontset.c opcodes.c opcodes.h asm8.c utils.c utils.h line-processor.c line-processor.h parser.c parser.h assembler.c assembler.h labels.c labels.h
	touch make.depend
	makedepend -I/usr/include/linux -I/usr/lib/gcc/x86_64-linux-gnu/5/include/ -fmake.depend $^

-include make.depend
