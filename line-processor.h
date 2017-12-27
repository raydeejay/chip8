#ifndef LINE_PROCESSOR_H_
#define LINE_PROCESSOR_H_

#include "assembler.h"

extern void registerRPLine(assembler_t *assembler);
extern void unregisterRPLines(assembler_t *assembler);
extern int processRPLines(assembler_t *assembler);
extern int processLine(assembler_t *assembler, const char *code, int preprocess);

#endif
