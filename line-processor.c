#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "labels.h"
#include "parser.h"

#include "line-processor.h"

/*******************
 * LINE PROCESSOR
 ******************/
void registerRPLine(assembler_t *assembler) {
    /* printf("Registering rpline at addr 0x%04x   %s\n", addr, code); */

    // create the rpline
    rpline_t *line = malloc(sizeof(rpline_t));
    line->addr = assembler->addr;
    line->linenum = assembler->linenum;
    line->filename = strdup(assembler->filename);
    line->code = strndup(assembler->code, 63);

    if (assembler->RPLines == NULL)
        assembler->RPLines = line;
    else {
        // find a place to link it
        rpline_t *rpline = assembler->RPLines;

        while (rpline && rpline->next) {
            rpline = rpline->next;
        }

        // link it
        rpline->next = line;
    }
}

void unregisterRPLines(assembler_t *assembler) {
    rpline_t *rpline = assembler->RPLines;

    while (rpline) {
        rpline_t *aux = rpline->next ? rpline->next : NULL;
        free(rpline->code);
        free(rpline->filename);
        free(rpline);
        rpline = aux ? aux : NULL;
    }
}

int processRPLines(assembler_t *assembler) {
    const rpline_t *rpline = assembler->RPLines;

    while (rpline) {
        assembler->addr = rpline->addr;
        assembler->linenum = rpline->linenum;
        // assembler->code = rpline->code;
        
        processLine(assembler, rpline->code, 0);
        rpline = rpline->next ? rpline->next : NULL;
    }

    return 0xFFFF;
}

int processLine(assembler_t *assembler, const char *code, int preprocess) {
    char *s = (char *) code;
    char *token = NULL;
    int n = 0;
    int emittedBytes = 0;

    char *label = NULL;
    char *instruction = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL;
    char *arg3 = NULL;

    // process the first token, which can be one of these:
    // - a label
    // - an assembly instruction
    // - a metainstruction

    s = skipWhitespace(s + n);
    token = readToken(s);
    n = strlen(token);

    // if the line is empty, there's no point in continuing
    if (n == 0) { return 0; }

    // no line can begin with a single character, except comments
    // which have already been skipped
    if (n == 1) {
        // FIXME should probably yell at the user here
        return 0;
    }

    while (n) {
        if (arg2) {
            // read arg2
            arg3 = strdup(token);
        }
        else if (arg1) {
            // read arg2
            if (token[n-1] == ',')
                token[n-1] = '\0';
            arg2 = strdup(token);
        }
        else if (instruction) {
            // read arg1
            if (token[n-1] == ',')
                token[n-1] = '\0';
            arg1 = strdup(token);
        }
        else if (label) {
            // read instruction
            instruction = strdup(token);
        }
        else {
            // read either label or instruction
            if (token[n-1] == ':') {
                label = strdup(token);
                label[n-1] = '\0';
            }
            else {
                instruction = strdup(token);
            }
        }

        free(token);
        s = skipWhitespace(s + n);
        token = readToken(s);
        n = strlen(token);
    }

    assembler->code = strdup(code);
    
    if (preprocess) {
        if (label) {
            registerLabel(assembler, label, assembler->addr);
        }
    }

    // assemble straight away
    // undefined references will be resolved later
    if (instruction) {
        emittedBytes = assemble(assembler, instruction, arg1, arg2, arg3);
    }

    // free resources
    if (label) free(label);
    if (instruction) free(instruction);
    if (arg1) free(arg1);
    if (arg2) free(arg2);
    if (arg3) free(arg3);
    free(token);

    assembler->addr += emittedBytes;
    
    return emittedBytes;
}
