#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#define MEMSIZE 4096
#define ROMSIZE 512
#define RAMSIZE (MEMSIZE - ROMSIZE)
#define RAMBASE ROMSIZE

typedef struct label {
    struct label *next;
    int addr;
    char name[64];
} label_t;

typedef struct rpline {
    struct rpline *next;
    int addr;
    char *code;
    int linenum;
    char *filename;
} rpline_t;

typedef struct assembler {
    unsigned char *memory;
    const char *filename;
    const char *code;
    int base;
    int addr;
    int linenum;
    rpline_t *RPLines;
    label_t *labels;
} assembler_t;

extern assembler_t *newAssembler(const char *filename, int addr, int memsize);
extern int destroyAssembler(assembler_t *assembler);

extern int getTarget(assembler_t *assembler, const char *arg, int addr);
extern int assemble(assembler_t *assembler, char *instruction, char *arg1, char *arg2, char *arg3);

#endif
