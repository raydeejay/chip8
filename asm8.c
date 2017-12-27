#include <stdio.h>
#include <stdlib.h>

// trimming from https://rosettacode.org/wiki/Strip_whitespace_from_a_string/Top_and_tail#C
#include <string.h>
#include <ctype.h>

char *rtrim(const char *s)
{
    while( isspace(*s) || !isprint(*s) ) ++s;
    return strdup(s);
}

char *ltrim(const char *s)
{
    char *r = strdup(s);
    if (r != NULL)
    {
        char *fr = r + strlen(s) - 1;
        while( (isspace(*fr) || !isprint(*fr) || *fr == 0) && fr >= r) --fr;
        *++fr = 0;
    }
    return r;
}

char *trim(const char *s)
{
    char *r = rtrim(s);
    char *f = ltrim(r);
    free(r);
    return f;
}

/*******************
 * LABELS
 ******************/
typedef struct label {
    struct label *next;
    int addr;
    char name[64];
} label_t;

label_t *gLabels = NULL;

void registerLabel(const char *name, unsigned int addr) {
    printf("Registering label %s with address 0x%04x\n", name, addr);

    // create the label
    label_t *lbl = malloc(sizeof(label_t));
    lbl->addr = addr;
    strncpy(lbl->name, name, 63);
    lbl->name[63] = '\0';     /* make sure it's null-terminated */

    if (gLabels == NULL)
        gLabels = lbl;
    else {
        // find a place to link it
        label_t *label = gLabels;

        while (label && label->next) {
            label = label->next;
        }

        // link it
        label->next = lbl;
    }
}

void unregisterLabels() {
    label_t *label = gLabels;

    while (label) {
        label_t *aux = label->next ? label->next : NULL;
        if (aux) free(label);
        label = aux;
    }
}

int findLabel(const char *name) {
    label_t *label = gLabels;

    while (label) {
        if (!strcmp(label->name, name)) {
            printf("Found label %s with address 0x%04x\n", label->name, label->addr);
            return label->addr;
        }
            
        label = label->next ? label->next : NULL;
    }

    return 0xFFFF;
}

/*******************
 * ASSEMBLER
 ******************/

#define MEMSIZE 4096
#define ROMSIZE 512
#define RAMSIZE (MEMSIZE - ROMSIZE)
#define RAMBASE ROMSIZE

unsigned char *gMemory = NULL;

int getTarget(const char *arg, int addr, const char *filename, int linenum) {
    unsigned short target = 0xFFFF;
    
    if (arg[0] == '#') {
        target = strtol(arg+1, NULL, 16) & 0x0FFF;
        printf("Address #%03x\n", target);
    }
    else if (arg[0] == '-') {
        target = addr - (strtol(arg+1, NULL, 16) & 0x0FFF);
        printf("Address -#%03x\n", target);
    }
    else if (arg[0] == '+') {
        target = addr + (strtol(arg+1, NULL, 16) & 0x0FFF);
        printf("Address +#%03x\n", target);
    }
    else {
        target = findLabel(arg);
        if (target != 0xFFFF) {
            printf("Address label %s #%03x\n", arg, target);
        }
        else {
            printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg);
        }
    }
    return target;
}

// mnemonics as per http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
int assemble(const char *filename, int linenum, int addr,
             char *instruction, char *arg1, char *arg2, char *arg3) {
    int len = 0;

    if (instruction[0] == '.')
        len = 0;

    // produce code
    if (!strcmp(instruction, "CLS")) {
        gMemory[addr]   = 0x00;
        gMemory[addr+1] = 0xE0;
        len = 2;
    }
    else if (!strcmp(instruction, "RET")) {
        gMemory[addr]   = 0x00;
        gMemory[addr+1] = 0xEE;
        len = 2;
    }
    else if (!strcmp(instruction, "JP")) {
        unsigned short target = getTarget(arg1, addr, filename, linenum);

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
            target = findLabel(arg1);
            if (target != 0xFFFF) {
                printf("JP label %s #%03x\n", arg1, target);
            }
            else {
                printf("%s:%u:1: warning: undefined label %s\n", filename, linenum, arg1);
            }
        }
        
        gMemory[addr]   = 0x10 | target >> 8;
        gMemory[addr+1] = target & 0xFF;
        len = 2;
    }
    else if (!strcmp(instruction, "CALL")) {
        unsigned short target = getTarget(arg1, addr, filename, linenum);
        gMemory[addr]   = 0x20 | target >> 8;
        gMemory[addr+1] = target & 0xFF;
        len = 2;
    }
    else if (!strcmp(instruction, "LD")) {
        if (arg1[0] == 'I') {
            unsigned short target = getTarget(arg2, addr, filename, linenum);
            printf("LD I, #%03x\n", target);
            gMemory[addr]   = 0xA0 | target >> 8;
            gMemory[addr+1] = target & 0xFF;
            len = 2;
        }
    }
    else if (!strcmp(instruction, "SE")) {
        unsigned char x = atoi(arg1+1) & 0x0F;
        unsigned char kk = atoi(arg2) & 0xFF;
        printf("SE V%u, %u\n", x, kk);
        gMemory[addr]   = 0x30 | x;
        gMemory[addr+1] = kk;
        len = 2;
    }
    else if (!strcmp(instruction, "SNE")) {
        unsigned char x = atoi(arg1+1) & 0x0F;
        unsigned char kk = atoi(arg2) & 0xFF;
        printf("SNE V%u, %u\n", x, kk);
        gMemory[addr]   = 0x40 | x;
        gMemory[addr+1] = kk;
        len = 2;
    }
    else if (!strcmp(instruction, "DRW")) {
        unsigned char x = atoi(arg1+1) & 0x0F;
        unsigned char y = atoi(arg2+1) & 0x0F;
        unsigned char n = atoi(arg3) & 0x0F;
        printf("DRW V%u, V%u, %u\n", x, y, n);
        gMemory[addr]   = 0xD0 | x;
        gMemory[addr+1] = y << 4 | n;
        len = 2;
    }
    else if (!strcmp(instruction, "DB")) {
        unsigned char nn = strtol(arg1+1, NULL, 16) & 0xFF;
        printf("DB #%02x\n", nn);
        gMemory[addr]   = nn;
        len = 1;
    }
    else if (!strcmp(instruction, "DW")) {
        unsigned char hh = atoi(arg1) >> 8;
        unsigned char ll = atoi(arg1) & 0xFF;
        printf("DW #%02x#%02x\n", hh, ll);
        gMemory[addr]   = hh;
        gMemory[addr+1] = ll;
        len = 2;
    }

    //printf("Assembling %u bytes (%x) %s %s %s\n", len, 0, instruction, arg1, arg2);
    return len;
}

/*******************
 * PARSER
 ******************/
char *skipWhitespace(const char *line) {
    char *s = (char *) line;

    // skip until the next non-whitespace
    while(*s && (*s == ' ' || *s == '\t' || *s == '\n')) s++;

    // if there's a comment, skip until the end of the line
    if (*s == ';') while(*s) s++;

    return s;
}

char *readToken(const char *line) {
    char *s = (char *) line;
    while(*s && *s != ' ' && *s != '\t' && *s != ';' && *s != '\n')
        s++;
    return strndup(line, s - line);
}

/*******************
 * LINE PROCESSOR
 ******************/
int processLine(const char *line, int addr, int linenum, const char *filename, int preprocess) {
    char *s = (char *) line;
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
        if (arg1) {
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

    if (preprocess) {
        if (label) {
            registerLabel(label, addr);
        }
    }

    // assemble anyway because we need to know the addresses, and for
    // that we need to know how long the code will be
    if (instruction) {
        emittedBytes = assemble(filename, linenum, addr, instruction, arg1, arg2, arg3);
    }

    // free resources
    if (label) free(label);
    if (instruction) free(instruction);
    if (arg1) free(arg1);
    if (arg2) free(arg2);
    if (arg3) free(arg3);
    free(token);

    return emittedBytes;
}

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

    // preprocess the file, to register labels and such
    while ((read = getline(&line, &len, fin)) != -1) {
        addr += processLine(line, addr, linenum, argv[1], 1);
    }

    fseek(fin, 0, SEEK_SET);
    addr = 0x200;

    // now assemble the code again
    while ((read = getline(&line, &len, fin)) != -1) {
        addr += processLine(line, addr, linenum, argv[1], 0);
    }

    fwrite(gMemory + 0x200, addr - 0x200, 1, fout);

    free(gMemory);

    if (fin) fclose(fin);
    if (fout) fclose(fout);
    if (line) free(line);

    unregisterLabels();

    exit(EXIT_SUCCESS);
}
