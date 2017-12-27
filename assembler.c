#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "line-processor.h"
#include "labels.h"

#include "assembler.h"

/*******************
 * ASSEMBLER
 ******************/

assembler_t *newAssembler(const char *filename, int addr, int memsize) {
    assembler_t *assembler = malloc(sizeof(assembler_t));

    assembler->memory = malloc(memsize);
    assembler->filename = strdup(filename);
    assembler->base = addr;
    assembler->addr = addr;
    assembler->labels = NULL;
    assembler->RPLines = NULL;

    return assembler;
}

int destroyAssembler(assembler_t *assembler) {
    unregisterLabels(assembler);
    unregisterRPLines(assembler);

    free((char *) assembler->memory);
    //free((char *) assembler->filename);
    free((char *) assembler->code);

    free(assembler);

    return 1;
}

int getTarget(assembler_t *assembler, const char *arg, int addr) {
    unsigned short target = 0xFFFF;

    if (arg[0] == '#') {
        target = strtol(arg+1, NULL, 16) & 0x0FFF;
    }
    else if (arg[0] == '-') {
        target = addr - (strtol(arg+2, NULL, 16) & 0x0FFF);
    }
    else if (arg[0] == '+') {
        target = addr + (strtol(arg+2, NULL, 16) & 0x0FFF);
    }
    else {
        target = findLabel(assembler, arg);
    }
    return target;
}

typedef int bytecount_t;

bytecount_t assembleCLS(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 00E0 - CLS
    a->memory[a->addr]   = 0x00;
    a->memory[a->addr+1] = 0xE0;
    return 2;
}

bytecount_t assembleRET(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 00EE - RET
    a->memory[a->addr]   = 0x00;
    a->memory[a->addr+1] = 0xEE;
    return 2;
}

bytecount_t assembleJP(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    if (arg1[0] == 'V' && arg1[1] == '0') {
        // Bnnn - JP V0, addr
        unsigned short target = getTarget(a, arg2, a->addr);
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
            target = findLabel(a, arg2);
            if (target != 0xFFFF) {
                printf("JP V0, label %s #%3x\n", arg2, target);
            }
            else {
                printf("%s:%u:1: warning: undefined label %s\n",
                       a->filename, a->linenum, arg2);
                registerRPLine(a);
            }
        }

        a->memory[a->addr]   = 0xB0 | target >> 8;
        a->memory[a->addr+1] = target & 0xFF;
    }
    else {
        // 1nnn - JP addr
        unsigned short target = getTarget(a, arg1, a->addr);
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
            target = findLabel(a, arg1);
            if (target != 0xFFFF) {
                printf("JP label %s #%03x\n", arg1, target);
            }
            else {
                printf("%s:%u:1: warning: undefined label %s\n",
                       a->filename, a->linenum, arg2);

            }
        }

        a->memory[a->addr]   = 0x10 | target >> 8;
        a->memory[a->addr+1] = target & 0xFF;
    }
    return 2;
}

bytecount_t assembleSE(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    if (arg1[0] == 'V' && arg2[0] == 'V') {
        // 5xy0 - SE Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("SE V%x, V%x\n", x, y);
        a->memory[a->addr]   = 0x50 | x;
        a->memory[a->addr+1] = y << 4 & 0xF0;
    }
    else if (arg1[0] == 'V') {
        // 3xkk - SE Vx, kk
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
        printf("SE V%x, #%x\n", x, kk);
        a->memory[a->addr]   = 0x30 | x;
        a->memory[a->addr+1] = kk;
    }
    return 2;
}

bytecount_t assembleSNE(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    if (arg1[0] == 'V' && arg2[0] == 'V') {
        // 9xy0 - SNE Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("SNE V%x, V%x\n", x, y);
        a->memory[a->addr]   = 0x90 | x;
        a->memory[a->addr+1] = y << 4 & 0xF0;
    }
    else if (arg1[0] == 'V') {
        // 4xkk - SE Vx, kk
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
        printf("SNE V%x, #%x\n", x, kk);
        a->memory[a->addr]   = 0x40 | x;
        a->memory[a->addr+1] = kk;
        }
    return 2;
}

bytecount_t assembleADD(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    if (arg1[0] == 'V' && arg2[0] == 'V') {
        // 8xy4 - ADD Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("ADD V%x, V%x\n", x, y);
        a->memory[a->addr]   = 0x80 | x;
        a->memory[a->addr+1] = y << 4 | 0x04;
    }
    else if (arg1[0] == 'V') {
        // 7xkk - ADD Vx, kk
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
        printf("ADD V%x, #%x\n", x, kk);
        a->memory[a->addr]   = 0x70 | x;
        a->memory[a->addr+1] = kk;
    }
    else if (arg1[0] == 'I') {
        // Fx1E - ADD I, Vx
        unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("ADD I, V%x\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x1E;
    }
    return 2;
}

bytecount_t assembleCALL(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 2nnn - CALL addr
    unsigned short target = getTarget(a, arg1, a->addr);
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
        target = findLabel(a, arg1);
        printf("CALL label %s #%03x\n", arg1, target);
        if (target == 0xFFFF) {
            printf("%s:%u:1: warning: undefined label %s\n",
                   a->filename, a->linenum, arg2);

            registerRPLine(a);
        }
    }
    a->memory[a->addr]   = 0x20 | target >> 8;
    a->memory[a->addr+1] = target & 0xFF;
    return 2;
}

bytecount_t assembleSYS(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 0nnn - SYS addr
    unsigned short target = getTarget(a, arg1, a->addr);
    if (arg1[0] == '#') {
        printf("SYS #%03x\n", target);
    }
    else {
        target = findLabel(a, arg1);
        printf("SYS label %s #%03x\n", arg1, target);
        if (target == 0xFFFF) {
            printf("%s:%u:1: warning: undefined label %s\n",
                   a->filename, a->linenum, arg2);

            registerRPLine(a);
        }
    }
    a->memory[a->addr]   = 0x00 | target >> 8;
    a->memory[a->addr+1] = target & 0xFF;
    return 2;
}

bytecount_t assembleLD(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    if (arg1[0] == 'I') {
        // Annn - LD I, addr
        unsigned short target = getTarget(a, arg2, a->addr);
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
            target = findLabel(a, arg2);
            printf("LD I, label %s #%03x\n", arg2, target);
            if (target == 0xFFFF) {
                printf("%s:%u:1: warning: undefined label %s\n",
                       a->filename, a->linenum, arg2);
                registerRPLine(a);
            }
        }

        a->memory[a->addr]   = 0xA0 | target >> 8;
        a->memory[a->addr+1] = target & 0xFF;
    }
    else if (arg1[0] == 'F') {
        // Fx29 - LD F, Vx
        unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("LD F, V%x\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x29;
    }
    else if (arg1[0] == 'B') {
        // Fx33 - LD B, Vx
        unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("LD B, V%x\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x33;
    }
    else if (arg1[0] == 'D' && arg1[1] == 'T') {
        // Fx15 - LD DT, Vx
        unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("LD DT, V%x\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x15;
    }
    else if (arg1[0] == 'S' && arg1[1] == 'T') {
        // Fx18 - LD ST, Vx
        unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("LD ST, V%x\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x18;
    }
    else if (arg1[0] == '[' && arg1[1] == 'I' && arg1[2] == ']') {
        // Fx55 - LD [I], Vx
        unsigned char x = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("LD ST, V%x\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x55;
    }
    else if (arg1[0] == 'V' && arg2[0] == 'K') {
        // Fx0A - LD Vx, K
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        printf("LD V%x, K\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x0A;
    }
    else if (arg1[0] == 'V' && arg2[0] == 'D' && arg2[1] == 'T') {
        // Fx07 - LD Vx, DT
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        printf("LD V%x, DT\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x07;
    }
    else if (arg1[0] == 'V' && arg2[0] == '[' && arg2[1] == 'I' && arg2[1] == ']') {
        // Fx65 - LD Vx, [I]
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        printf("LD V%x, [I]\n", x);
        a->memory[a->addr]   = 0xF0 | x;
        a->memory[a->addr+1] = 0x65;
    }
    else if (arg1[0] == 'V' && arg2[0] == 'V') {
        // 8xy0 - LD Vx, Vy
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
        printf("LD V%x, V%x\n", x, y);
        a->memory[a->addr]   = 0x60 | x;
        a->memory[a->addr+1] = y << 4 & 0xF0;
    }
    else if (arg1[0] == 'V') {
        // 6xkk - LD Vx, byte
        unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
        unsigned short target = getTarget(a, arg2, a->addr);
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
            target = findLabel(a, arg2);
            if (target != 0xFFFF) {
                printf("LD V%x, label %s #%3x\n", x, arg2, target);
            }
            else {
                printf("%s:%u:1: warning: undefined label %s\n",
                       a->filename, a->linenum, arg2);
                registerRPLine(a);
            }
        }

        a->memory[a->addr]   = 0x60 | x;
        a->memory[a->addr+1] = target & 0xFF;
    }

    return 2;
}

bytecount_t assembleDRW(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // Dxyn - DRW Vx, Vy, nibble
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    unsigned char n = strtol(arg3+1, NULL, 16) & 0x0F;
    printf("DRW V%x, V%x, #%x\n", x, y, n);
    a->memory[a->addr]   = 0xD0 | x;
    a->memory[a->addr+1] = (y << 4) | n;
    return 2;
}

bytecount_t assembleRND(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // Cxkk - RND Vx, byte
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char kk = strtol(arg2+1, NULL, 16) & 0xFF;
    printf("RND V%x, #%x\n", x, kk);
    a->memory[a->addr]   = 0xC0 | x;
    a->memory[a->addr+1] = kk;
    return 2;
}

bytecount_t assembleAND(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xy2 - AND Vx, Vy
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("AND V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x02;
    return 2;
}

bytecount_t assembleOR(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xy1 - OR Vx, Vy
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("OR V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x01;
    return 2;
}

bytecount_t assembleXOR(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xy3 - XOR Vx, Vy
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("XOR V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x03;
    return 2;
}

bytecount_t assembleSUB(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xy5 - SUB Vx, Vy
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("SUB V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x05;
    return 2;
}

bytecount_t assembleSUBN(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xy7 - SUBN Vx, Vy
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("SUBN V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x07;
    return 2;
}

bytecount_t assembleSHL(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xyE - SHL Vx {, Vy}
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("SHL V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x0E;
    return 2;
}

bytecount_t assembleSHR(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // 8xy6 - SHR Vx {, Vy}
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    unsigned char y = strtol(arg2+1, NULL, 16) & 0x0F;
    printf("SHR V%x, V%x\n", x, y);
    a->memory[a->addr]   = 0x80 | x;
    a->memory[a->addr+1] = y << 4 | 0x06;
    return 2;
}

bytecount_t assembleSKP(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // Ex9E - SKP Vx
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    printf("SKP V%xn", x);
    a->memory[a->addr]   = 0xE0 | x;
    a->memory[a->addr+1] = 0x9E;
    return 2;
}

bytecount_t assembleSKPN(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // ExA1 - SKPN Vx
    unsigned char x = strtol(arg1+1, NULL, 16) & 0x0F;
    printf("SKPN V%xn", x);
    a->memory[a->addr]   = 0xE0 | x;
    a->memory[a->addr+1] = 0xA1;
    return 2;
}

bytecount_t assembleDB(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // nn - DB nn
    unsigned char nn = strtol(arg1+1, NULL, 16) & 0xFF;
    printf("DB #%02x\n", nn);
    a->memory[a->addr]   = nn;
    return 1;
}

bytecount_t assembleDW(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    // hhll - DW hhll
    unsigned char hh = strtol(arg1+1, NULL, 16) >> 8;
    unsigned char ll = strtol(arg1+1, NULL, 16) & 0xFF;
    printf("DW #%02x%02x\n", hh, ll);
    a->memory[a->addr]   = hh;
    a->memory[a->addr+1] = ll;
    return 2;
}

bytecount_t assembleMETA(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    return 0;
}

typedef bytecount_t (*asmFn)(assembler_t *a, char *arg1, char *arg2, char *arg3);

struct { const char *name; asmFn fn; } instructions[] = {
    { "CLS",  assembleCLS  },
    { "RET",  assembleRET  },
    { "JP",   assembleJP   },
    { "CALL", assembleCALL },
    { "SYS",  assembleSYS  },
    { "LD",   assembleLD   },
    { "SE",   assembleSE   },
    { "SNE",  assembleSNE  },
    { "DRW",  assembleDRW  },
    { "RND",  assembleRND  },
    { "ADD",  assembleADD  },
    { "OR",   assembleOR   },
    { "AND",  assembleAND  },
    { "XOR",  assembleXOR  },
    { "SUB",  assembleSUB  },
    { "SUBN", assembleSUBN },
    { "SHL",  assembleSHL  },
    { "SHR",  assembleSHR  },
    { "SKP",  assembleSKP  },
    { "SKPN", assembleSKPN },
    { "DB",   assembleDB   },
    { "DW",   assembleDW   },
    { NULL, NULL }
};

bytecount_t metaBASE(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    return 0;
}

bytecount_t metaADDR(assembler_t *a, char *arg1, char *arg2, char *arg3) {
    a->addr = strtol(arg1+1, NULL, 16);
    return 0;
}

struct { const char *name; asmFn fn; } metainstructions[] = {
    { ".BASE",   metaBASE },
    { ".ADDR",   metaADDR },
    { NULL, NULL }
};

// mnemonics as per http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
int assemble(assembler_t *assembler, char *instruction, char *arg1, char *arg2, char *arg3) {
    // printf("Assembling %s %s %s %s\n", instruction, arg1, arg2, arg3);
    for (int i = 0; metainstructions[i].name; i++) {
        if (!strcmp(instruction, metainstructions[i].name)) {
            return metainstructions[i].fn(assembler, arg1, arg2, arg3);
        }
    }

    for (int i = 0; instructions[i].name; i++) {
        if (!strcmp(instruction, instructions[i].name)) {
            return instructions[i].fn(assembler, arg1, arg2, arg3);
        }
    }

    printf("%s:%u:1: error: unknown instruction %s [%s]\n",
           assembler->filename, assembler->linenum, instruction, assembler->code);

    exit(1);
}
