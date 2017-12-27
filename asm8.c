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
    int addr = 0x200;

    gMemory = malloc(MEMSIZE);

    // assemble the file straight away
    // the undefined references will be collected in gRPLines
    while ((read = getline(&line, &len, fin)) != -1) {
        addr += processLine(line, addr, linenum, argv[1], 1);
    }

    // now assemble the code that had undefined references again
    printf("Recompiling lines with undefined references\n");
    processRPLines(gRPLines);

    fwrite(gMemory + 0x200, addr - 0x200, 1, fout);

    free(gMemory);

    if (fin) fclose(fin);
    if (fout) fclose(fout);
    if (line) free(line);

    unregisterLabels();
    unregisterRPLines();

    exit(EXIT_SUCCESS);
}
