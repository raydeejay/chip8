#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "line-processor.h"
#include "labels.h"

#include "assembler.h"

/*******************
 * ASSEMBLER
 ******************/

unsigned char *gMemory = NULL;

int getTarget(const char *arg, int addr, const char *filename, int linenum) {
    unsigned short target = 0xFFFF;

    if (arg[0] == '#') {
        target = strtol(arg+1, NULL, 16) & 0x0FFF;
        /* printf("Address #%03x\n", target); */
    }
    else if (arg[0] == '-') {
        target = addr - (strtol(arg+2, NULL, 16) & 0x0FFF);
        /* printf("Address -#%03x\n", target); */
    }
    else if (arg[0] == '+') {
        target = addr + (strtol(arg+2, NULL, 16) & 0x0FFF);
        /* printf("Address +#%03x\n", target); */
    }
    else {
        target = findLabel(arg);
        if (target != 0xFFFF) {
            /* printf("Address label %s #%03x\n", arg, target); */
        }
        else {
            /* printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg); */
            target = 0xFFFF;
        }
    }
    return target;
}

// mnemonics as per http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
int assemble(const char *filename, int linenum, int addr,
             char *instruction, char *arg1, char *arg2, char *arg3, const char *line) {
    int len = 0;

    if (instruction[0] == '.')
        len = 0;

    // produce code
    if (!strcmp(instruction, "CLS")) {
        // 00E0 - CLS
        // Clear the display.
        gMemory[addr]   = 0x00;
        gMemory[addr+1] = 0xE0;
        len = 2;
    }
    else if (!strcmp(instruction, "RET")) {

        // 00EE - RET
        // Return from a subroutine.
        gMemory[addr]   = 0x00;
        gMemory[addr+1] = 0xEE;
        len = 2;
    }
    else if (!strcmp(instruction, "JP")) {
        if (arg1[0] == 'V' && arg1[1] == '0') {
            // Bnnn - JP V0, addr
            unsigned short target = getTarget(arg2, addr, filename, linenum);
            if (arg2[0] == '#') {
                printf("JP V0, #%03x\n", target);
            }
            else if (arg2[0] == '-') {
                printf("JP V0, -#%3x\n", target);
            }
            else if (arg2[0] == '+') {
                printf("JP V0, +#%3x\n", target);
            }
            else {
                target = findLabel(arg2);
                if (target != 0xFFFF) {
                    printf("JP V0, label %s #%3x\n", arg2, target);
                }
                else {
                    printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg2);
                    registerRPLine(line, addr, linenum, filename);
                }
            }

            gMemory[addr]   = 0xB0 | target >> 8;
            gMemory[addr+1] = target & 0xFF;
        }
        else {
            // 1nnn - JP addr
            unsigned short target = getTarget(arg1, addr, filename, linenum);
            if (arg1[0] == '#') {
                printf("JP #%03x\n", target);
            }
            else if (arg1[0] == '-') {
                printf("JP -#%03x\n", target);
            }
            else if (arg1[0] == '+') {
                printf("JP +#%03x\n", target);
            }
            else {
                target = findLabel(arg1);
                if (target != 0xFFFF) {
                    printf("JP label %s #%03x\n", arg1, target);
                }
                else {
                    printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg1);
                }
            }

            gMemory[addr]   = 0x10 | target >> 8;
            gMemory[addr+1] = target & 0xFF;
        }
        len = 2;
    }
    else if (!strcmp(instruction, "CALL")) {
        unsigned short target = getTarget(arg1, addr, filename, linenum);
        if (arg1[0] == '#') {
            printf("CALL #%03x\n", target);
        }
        else if (arg1[0] == '-') {
            printf("CALL -#%3x\n", target);
        }
        else if (arg1[0] == '+') {
            printf("CALL +#%3x\n", target);
        }
        else {
            target = findLabel(arg1);
            printf("CALL label %s #%03x\n", arg1, target);
            if (target == 0xFFFF) {
                printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg1);
                registerRPLine(line, addr, linenum, filename);
            }
        }
        gMemory[addr]   = 0x20 | target >> 8;
        gMemory[addr+1] = target & 0xFF;
        len = 2;
    }
    else if (!strcmp(instruction, "SYS")) {
        unsigned short target = getTarget(arg1, addr, filename, linenum);
        if (arg1[0] == '#') {
            printf("CALL #%03x\n", target);
        }
        else {
            target = findLabel(arg1);
            printf("CALL label %s #%03x\n", arg1, target);
            if (target == 0xFFFF) {
                printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg1);
                registerRPLine(line, addr, linenum, filename);
            }
        }
        gMemory[addr]   = 0x00 | target >> 8;
        gMemory[addr+1] = target & 0xFF;
        len = 2;
    }
    else if (!strcmp(instruction, "LD")) {
        if (arg1[0] == 'I') {
            // Annn - LD I, addr
            unsigned short target = getTarget(arg2, addr, filename, linenum);
            if (arg2[0] == '#') {
                printf("LD I, #%03x\n", target);
            }
            else if (arg2[0] == '-') {
                printf("LD I, -#%3x\n", target);
            }
            else if (arg2[0] == '+') {
                printf("LD I, +#%3x\n", target);
            }
            else {
                target = findLabel(arg2);
                printf("LD I, label %s #%03x\n", arg2, target);
                if (target == 0xFFFF) {
                    printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg2);
                    registerRPLine(line, addr, linenum, filename);
                }
            }

            gMemory[addr]   = 0xA0 | target >> 8;
            gMemory[addr+1] = target & 0xFF;
            len = 2;
        }
        else if (arg1[0] == 'F') {
            // Fx29 - LD F, Vx
            unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("LD F, V%x\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x29;
            len = 2;
        }
        else if (arg1[0] == 'B') {
            // Fx33 - LD B, Vx
            unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("LD B, V%x\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x33;
            len = 2;
        }
        else if (arg1[0] == 'D' && arg1[1] == 'T') {
            // Fx15 - LD DT, Vx
            unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("LD DT, V%x\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x15;
            len = 2;
        }
        else if (arg1[0] == 'S' && arg1[1] == 'T') {
            // Fx18 - LD ST, Vx
            unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("LD ST, V%x\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x18;
            len = 2;
        }
        else if (arg1[0] == '[' && arg1[1] == 'I' && arg1[2] == ']') {
            // Fx55 - LD [I], Vx
            unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("LD ST, V%x\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x55;
            len = 2;
        }
        else if (arg1[0] == 'V' && arg2[0] == 'K') {
            // Fx0A - LD Vx, K
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            printf("LD V%x, K\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x0A;
            len = 2;
        }
        else if (arg1[0] == 'V' && arg2[0] == 'D' && arg2[1] == 'T') {
            // Fx07 - LD Vx, DT
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            printf("LD V%x, DT\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x07;
            len = 2;
        }
        else if (arg1[0] == 'V' && arg2[0] == '[' && arg2[1] == 'I' && arg2[1] == ']') {
            // Fx65 - LD Vx, [I]
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            printf("LD V%x, [I]\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x65;
            len = 2;
        }
        else if (arg1[0] == 'V' && arg2[0] == 'V') {
            // 8xy0 - LD Vx, Vy
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("LD V%x, V%x\n", x, y);
            gMemory[addr]   = 0x60 | x;
            gMemory[addr+1] = y << 4 & 0xF0;
            len = 2;
        }
        else if (arg1[0] == 'V') {
            // 6xkk - LD Vx, byte
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned short target = getTarget(arg2, addr, filename, linenum);
            if (arg2[0] == '#') {
                printf("LD V%x, #%03x\n", x, target);
            }
            else if (arg2[0] == '-') {
                printf("LD V%x, -#%3x\n", x, target);
            }
            else if (arg2[0] == '+') {
                printf("LD V%x, +#%3x\n", x, target);
            }
            else {
                target = findLabel(arg2);
                if (target != 0xFFFF) {
                    printf("LD V%x, label %s #%3x\n", x, arg2, target);
                }
                else {
                    printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg2);
                    registerRPLine(line, addr, linenum, filename);
                }
            }

            gMemory[addr]   = 0x60 | x;
            gMemory[addr+1] = target & 0xFF;
            len = 2;
        }

    }
    else if (!strcmp(instruction, "SE")) {
        if (arg1[0] == 'V' && arg2[0] == 'V') {
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("SE V%x, V%x\n", x, y);
            gMemory[addr]   = 0x50 | x;
            gMemory[addr+1] = y << 4 & 0xF0;
            len = 2;
        }
        else if (arg1[0] == 'V') {
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
            printf("SE V%x, #%x\n", x, kk);
            gMemory[addr]   = 0x30 | x;
            gMemory[addr+1] = kk;
            len = 2;
        }
    }
    else if (!strcmp(instruction, "SNE")) {
        if (arg1[0] == 'V' && arg2[0] == 'V') {
            // 9xy0 - SNE Vx, Vy
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("SNE V%x, V%x\n", x, y);
            gMemory[addr]   = 0x90 | x;
            gMemory[addr+1] = y << 4 & 0xF0;
            len = 2;
        }
        else if (arg1[0] == 'V') {
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
            printf("SNE V%x, #%x\n", x, kk);
            gMemory[addr]   = 0x40 | x;
            gMemory[addr+1] = kk;
            len = 2;
        }
    }
    else if (!strcmp(instruction, "DRW")) {
        // Dxyn - DRW Vx, Vy, nibble
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        unsigned char n = strtol(arg3+1, NULL, 16) & 0x0F;
        printf("DRW V%x, V%x, #%x\n", x, y, n);
        gMemory[addr]   = 0xD0 | x;
        gMemory[addr+1] = (y << 4) | n;
        len = 2;
    }
    else if (!strcmp(instruction, "RND")) {
        // Cxkk - RND Vx, byte
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
        printf("RND V%x, #%x\n", x, kk);
        gMemory[addr]   = 0xC0 | x;
        gMemory[addr+1] = kk;
        len = 2;
    }
    else if (!strcmp(instruction, "ADD")) {
        if (arg1[0] == 'V' && arg2[0] == 'V') {
            // 8xy4 - ADD Vx, Vy
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("ADD V%x, V%x\n", x, y);
            gMemory[addr]   = 0x80 | x;
            gMemory[addr+1] = y << 4 | 0x04;
            len = 2;
        }
        else if (arg1[0] == 'V') {
            // 7xkk - ADD Vx, kk
            unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
            unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
            printf("ADD V%x, #%x\n", x, kk);
            gMemory[addr]   = 0x70 | x;
            gMemory[addr+1] = kk;
            len = 2;
        }
        else if (arg1[0] == 'I') {
            // Fx1E - ADD I, Vx
            unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
            printf("ADD I, V%x\n", x);
            gMemory[addr]   = 0xF0 | x;
            gMemory[addr+1] = 0x1E;
            len = 2;
        }
    }
    else if (!strcmp(instruction, "OR")) {
        // 8xy1 - OR Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("OR V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x01;
        len = 2;
    }
    else if (!strcmp(instruction, "AND")) {
        // 8xy2 - AND Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("AND V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x02;
        len = 2;
    }
    else if (!strcmp(instruction, "XOR")) {
        // 8xy3 - XOR Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("XOR V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x03;
        len = 2;
    }
    else if (!strcmp(instruction, "SUB")) {
        // 8xy5 - SUB Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("SUB V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x05;
        len = 2;
    }
    else if (!strcmp(instruction, "SUBN")) {
        // 8xy7 - SUBN Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("SUBN V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x07;
        len = 2;
    }
    else if (!strcmp(instruction, "SHL")) {
        // 8xyE - SHL Vx {, Vy}
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("SHL V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x0E;
        len = 2;
    }
    else if (!strcmp(instruction, "SHR")) {
        // 8xy6 - SHR Vx {, Vy}
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("SHR V%x, V%x\n", x, y);
        gMemory[addr]   = 0x80 | x;
        gMemory[addr+1] = y << 4 | 0x06;
        len = 2;
    }
    else if (!strcmp(instruction, "SKP")) {
        // Ex9E - SKP Vx
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        printf("SKP V%xn", x);
        gMemory[addr]   = 0xE0 | x;
        gMemory[addr+1] = 0x9E;
        len = 2;
    }
    else if (!strcmp(instruction, "SKPN")) {
        // ExA1 - SKPN Vx
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        printf("SKPN V%xn", x);
        gMemory[addr]   = 0xE0 | x;
        gMemory[addr+1] = 0xA1;
        len = 2;
    }
    else if (!strcmp(instruction, "DB")) {
        unsigned char nn = strtol(arg1+1, NULL, 16) & 0xFF;
        printf("DB #%02x\n", nn);
        gMemory[addr]   = nn;
        len = 1;
    }
    else if (!strcmp(instruction, "DW")) {
        unsigned char hh = strtol(arg1+1, NULL, 16) >> 8;
        unsigned char ll = strtol(arg1+1, NULL, 16) & 0xFF;
        printf("DW #%02x%02x\n", hh, ll);
        gMemory[addr]   = hh;
        gMemory[addr+1] = ll;
        len = 2;
    }

    //printf("Assembling %u bytes (%x) %s %s %s\n", len, 0, instruction, arg1, arg2);
    return len;
}
