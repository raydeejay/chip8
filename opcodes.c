#include <assert.h>

#include "chip8.h"
#include "opcodes.h"

void opcode0(chip8_t *machine) {
    switch (machine->opcode) {
    case 0x00E0:
        // clear the display
        memset(machine->VRAM, 0, VRAMSIZE);
        machine->redraw = 1;
        break;
    case 0x00EE:
        // return from a subroutine
        machine->SP--;
        machine->PC = machine->stack[machine->SP];
        return;
    default:
        // jump to machine code routine at nnn
        // may be left unimplemented
        break;
    }

    machine->PC += 2;
}

void opcode1(chip8_t *machine) {
    // Jump to location nnn
    machine->PC = machine->opcode & 0x0FFF;
}

void opcode2(chip8_t *machine) {
    // Call subroutine at nnn
    machine->SP++;
    if (machine->SP > STACKSIZE) {
        printf("Stack overflow\n");
        exit(1);
    }

    machine->stack[machine->SP - 1] = machine->PC + 2;
    // opcode1(machine);
    machine->PC = machine->opcode & 0x0FFF;
}

void opcode3(chip8_t *machine) {
    // skip next instruction if Vx = kk
    int x = (machine->opcode & 0x0F00) >> 8;
    int kk = (machine->opcode & 0x00FF);

    machine->PC += (machine->V[x] == kk) ? 4 : 2;
}

void opcode4(chip8_t *machine) {
    // skip next instruction if Vx = kk
    int x = (machine->opcode & 0x0F00) >> 8;
    int kk = (machine->opcode & 0x00FF);

    machine->PC += (machine->V[x] != kk) ? 4 : 2;
}

void opcode5(chip8_t *machine) {
    switch (machine->opcode & 0x000F) {
    case 0:
        // skip next instruction if Vx = Vy
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->PC += (machine->V[x] == machine->V[y]) ? 4 : 2;
        } while (0);
        break;
    default:
        printf("Unknown opcode 5x %4x\n", machine->opcode);
        break;
    }
}

void opcode6(chip8_t *machine) {
    // Sets Vx = kk
    int x = (machine->opcode & 0x0F00) >> 8;
    int kk = (machine->opcode & 0x00FF);

    machine->V[x] = kk;
    machine->PC += 2;
}

void opcode7(chip8_t *machine) {
    // Sets Vx = Vx + kk
    int x = (machine->opcode & 0x0F00) >> 8;
    int kk = (machine->opcode & 0x00FF);

    machine->V[x] += kk;
    machine->PC += 2;
}

void opcode8(chip8_t *machine) {
    // multiplexed
    switch (machine->opcode & 0x000F) {
    case 0x0000:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->V[x] = machine->V[y];
        } while (0);
        break;
    case 0x0001:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->V[x] |= machine->V[y];
        } while (0);
        break;
    case 0x0002:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->V[x] &= machine->V[y];
        } while (0);
        break;
    case 0x0003:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->V[x] ^= machine->V[y];
        } while (0);
        break;
    case 0x0004:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            int val = machine->V[x] + machine->V[y];
            machine->V[0xF] = val > 0xFF ? 1 : 0;
            machine->V[x] = val & 0xFF;
        } while (0);
        break;
    case 0x0005:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->V[0xF] = machine->V[x] > machine->V[y] ? 1 : 0;
            machine->V[x] -= machine->V[y];
        } while (0);
        break;
    case 0x0006:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            /* int y = (machine->opcode & 0x00F0) >> 4; */
            machine->V[0xF] = machine->V[x] & 0x01 ? 1 : 0;
            machine->V[x] = machine->V[x] >> 1;
        } while (0);
        break;
    case 0x0007:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            int y = (machine->opcode & 0x00F0) >> 4;
            machine->V[0xF] = machine->V[y] > machine->V[x] ? 1 : 0;
            machine->V[x] = machine->V[y] - machine->V[x];
        } while (0);
        break;
    case 0x000E:
        do {
            int x = (machine->opcode & 0x0F00) >> 8;
            /* int y = (machine->opcode & 0x00F0) >> 4; */
            machine->V[0xF] = (machine->V[x] & 0x80) >> 7 ? 1 : 0;
            machine->V[x] = machine->V[x] << 1;
        } while (0);
        break;
    default:
        printf("Unknown opcode 8x %4x\n", machine->opcode);
        break;
    }

    machine->PC += 2;
}

void opcode9(chip8_t *machine) {
    switch (machine->opcode & 0x000F) {
        // skip next instruction if Vx != Vy
        int x = (machine->opcode & 0x0F00) >> 8;
        int y = (machine->opcode & 0x00F0) >> 4;

        machine->PC += (machine->V[x] != machine->V[y]) ? 4 : 2;
        return;
    default:
        printf("Unknown opcode 9x %4x\n", machine->opcode);
        break;
    }
    machine->PC += 2;
}

void opcodeA(chip8_t *machine) {
    // Set I to nnn
    machine->I = machine->opcode & 0x0FFF;
    machine->PC += 2;
}

void opcodeB(chip8_t *machine) {
    // Jump to location nnn + V0
    machine->PC = (machine->opcode & 0x0FFF) + machine->V[0];
}

/* Returns an integer in the range [0, n).
 *
 * Uses rand(), and so is affected-by/affects the same seed.
 */
int randint(int n) {
    if ((n - 1) == RAND_MAX) {
        return rand();
    } else {
        // Chop off all of the values that would cause skew...
        long end = RAND_MAX / n; // truncate skew
        assert (end > 0L);
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end);

        return r % n;
    }
}

void opcodeC(chip8_t *machine) {
    // Set Vx = random byte AND kk
    int x = (machine->opcode & 0x0F00) >> 8;
    int kk = (machine->opcode & 0x00FF);

    machine->V[x] = (unsigned short) (randint(256) & kk);
    machine->PC += 2;
}

void opcodeD(chip8_t *machine) {
    // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
    int x = (machine->opcode & 0x0F00) >> 8;
    int y = (machine->opcode & 0x00F0) >> 4;
    int n = (machine->opcode & 0x000F);
    int Vx = machine->V[x];
    int Vy = machine->V[y];
    unsigned short erased = 0;

    machine->redraw = 1;

    for (int i = 0; i < n; ++i) {
        for (int b = 0; b < 8; b++) {
            int bit = (machine->RAM[machine->I + i] >> b) & 0x01;
            int Sx = (Vx + (7 - b)) % CHIP8_WIDTH;
            int Sy = (Vy + i) % CHIP8_HEIGHT;
            int pixel = machine->VRAM[Sy * CHIP8_WIDTH + Sx];
            if (bit && pixel) { erased = 1; }
            machine->VRAM[Sy * CHIP8_WIDTH + Sx] ^= bit;
        }
    }

    machine->V[0xF] = erased;
    machine->PC += 2;
}

void opcodeE(chip8_t *machine) {
    // multiplexed
    int x = (machine->opcode & 0x0F00) >> 8;
    int k = machine->V[x];

    switch (machine->opcode & 0x00FF) {
    case 0x009E:
        if (machine->keys[k])
            machine->PC += 2;
        break;
    case 0x00A1:
        if (!(machine->keys[k]))
            machine->PC += 2;
        break;
    default:
        printf("Unknown opcode Ex %4x\n", machine->opcode);
        break;
    }
    machine->PC += 2;
}

void opcodeF(chip8_t *machine) {
    // multiplexed
    int x = (machine->opcode & 0x0F00) >> 8;
    int pressed = -1;

    switch (machine->opcode & 0x00FF) {
    case 0x0007:
        machine->V[x] = machine->delay_timer;
        break;
    case 0x000A:
        for (int i = 0; i < 16; ++i)
            if (machine->keys[i]) {
                pressed = i;
                break;
            }
        if (pressed > -1)
            machine->V[x] = pressed;
        else
            return; // without advancing the PC
        break;
    case 0x0015:
        machine->delay_timer = machine->V[x];
        break;
    case 0x0018:
        machine->sound_timer = machine->V[x];
        break;
    case 0x001E:
        machine->I += machine->V[x];
        break;
    case 0x0029:
        machine->I = FONTBASEADDR + machine->V[x]* 5;
        break;
    case 0x0033:
        // store BCD
        machine->RAM[machine->I]     = (unsigned char) (machine->V[x] / 100);
        machine->RAM[machine->I + 1] = (unsigned char) ((machine->V[x] % 100) / 10);
        machine->RAM[machine->I + 2] = (unsigned char) (machine->V[x] % 10);
        break;
    case 0x0055:
        for (int i = 0; i <= x; ++i)
            machine->RAM[machine->I + i] = machine->V[i];
        break;
    case 0x0065:
        for (int i = 0; i <= x; ++i)
            machine->V[i] = machine->RAM[machine->I + i];
        break;
    default:
        printf("Unknown opcode Ex %4x\n", machine->opcode);
        break;
    }
    machine->PC += 2;
}

void (*opcodes[16])(chip8_t *machine) = {
    opcode0, opcode1, opcode2, opcode3,
    opcode4, opcode5, opcode6, opcode7,
    opcode8, opcode9, opcodeA, opcodeB,
    opcodeC, opcodeD, opcodeE, opcodeF
};
