#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#define MEMSIZE 4096
#define ROMSIZE 512
#define RAMSIZE (MEMSIZE - ROMSIZE)
#define RAMBASE ROMSIZE

extern unsigned char *gMemory;

extern int getTarget(const char *arg, int addr, const char *filename, int linenum);
extern int assemble(const char *filename, int linenum, int addr,
             char *instruction, char *arg1, char *arg2, char *arg3, const char *line);

#endif
