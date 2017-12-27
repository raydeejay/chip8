#ifndef LABELS_H_
#define LABELS_H_

#include "assembler.h"

extern void registerLabel(assembler_t *assembler, const char *name, unsigned int addr);
extern void unregisterLabels(assembler_t *assembler);
extern int findLabel(assembler_t *assembler, const char *name);

#endif
