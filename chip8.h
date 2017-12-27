#ifndef CHIP8_H_
#define CHIP8_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

extern SDL_Window* gWindow;

extern SDL_Renderer* gRenderer;

extern Mix_Chunk *gSfx[72];
extern int gMaxSfx;

#define RAMSIZE 4 * 1024
#define VRAMSIZE 64 * 32
#define STACKSIZE 16
#define NUM_REGISTERS 16
#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define FONTBASEADDR 0x050

typedef struct chip8 {
    unsigned short opcode;
    unsigned char RAM[RAMSIZE];
    unsigned char V[16];
    unsigned short I;
    unsigned short PC;
    unsigned char VRAM[VRAMSIZE];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short SP;
    unsigned char keys[16];
    unsigned char redraw;
} chip8_t;

extern chip8_t *chip8_new(void);
extern int chip8_init(chip8_t *machine);
extern int chip8_loadFile(chip8_t *machine, const char *filename);
extern int chip8_destroy(chip8_t *machine);
extern int chip8_draw(chip8_t *machine);
extern int chip8_cycle(chip8_t *machine);
extern int chip8_decrementTimers(chip8_t *machine);
extern int chip8_setKeys(chip8_t *machine, unsigned char state[16]);

extern unsigned char chip8_fontset[80];
#endif
