#ifndef LINE_PROCESSOR_H_
#define LINE_PROCESSOR_H_

typedef struct rpline {
    struct rpline *next;
    int addr;
    char *code;
    int linenum;
    char *filename;
} rpline_t;

extern rpline_t *gRPLines;

extern void registerRPLine(const char *code, unsigned int addr, int linenum, const char *filename);
extern void unregisterRPLines();
extern int processRPLines(const rpline_t *RPLines);
extern int processLine(const char *line, int addr, int linenum, const char *filename, int preprocess);

#endif
