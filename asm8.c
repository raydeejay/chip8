#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "line-processor.h"
#include "labels.h"
#include "parser.h"
#include "assembler.h"
#include "utils.h"

/*******************
 * ENTRY POINT
 ******************/
int main(int argc, char*argv[]) {
    if (argc < 3) {
        printf("Usage: %s INPUTFILE OUTPUTFILE\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fin = NULL, *fout = NULL;

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fin = fopen(argv[1], "rb");
    if (fin == NULL) {
        printf("Can't open %s for reading.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    fout = fopen(argv[2], "wb");
    if (fout == NULL) {
        printf("Can't open %s for writing.\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    int linenum = 1;
    int base = 0x200;
    int end = base;
    int memsize = 4096 - 512;

    assembler_t *assembler = newAssembler(argv[1], base, memsize);

    assembler->addr = base;
    assembler->linenum = linenum;
    assembler->filename = argv[1];

    // assemble the file straight away
    // the undefined references will be collected in gRPLines
    while ((read = getline(&line, &len, fin)) != -1) {
        end += processLine(assembler, line, 1);
    }

    // now assemble the code that had undefined references again
    printf("Recompiling lines with undefined references\n");
    assembler->addr = base;
    processRPLines(assembler);

    fwrite(assembler->memory + base, end - base, 1, fout);

    if (fin) fclose(fin);
    if (fout) fclose(fout);
    if (line) free(line);

    destroyAssembler(assembler);

    exit(EXIT_SUCCESS);
}
