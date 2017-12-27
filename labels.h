#ifndef LABELS_H_
#define LABELS_H_

typedef struct label {
    struct label *next;
    int addr;
    char name[64];
} label_t;

extern label_t *gLabels;

extern void registerLabel(const char *name, unsigned int addr);
extern void unregisterLabels();
extern int findLabel(const char *name);

#endif
