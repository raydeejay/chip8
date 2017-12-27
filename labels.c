#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "labels.h"

/*******************
 * LABELS
 ******************/
void registerLabel(assembler_t *assembler, const char *name, unsigned int addr) {
    printf("Registering label %s with address 0x%04x\n", name, addr);

    // create the label
    label_t *lbl = malloc(sizeof(label_t));
    lbl->addr = addr;
    strncpy(lbl->name, name, 63);
    lbl->name[63] = '\0';     /* make sure it's null-terminated */

    if (assembler->labels == NULL)
        assembler->labels = lbl;
    else {
        // find a place to link it
        label_t *label = assembler->labels;

        while (label && label->next) {
            label = label->next;
        }

        // link it
        label->next = lbl;
    }
}

void unregisterLabels(assembler_t *assembler) {
    label_t *label = assembler->labels;

    while (label != NULL) {
        label_t *aux = label->next ? label->next : NULL;
        free(label);
        label = aux ? aux : NULL;
    }
}

int findLabel(assembler_t *assembler, const char *name) {
    label_t *label = assembler->labels;

    while (label != NULL) {
        if (!strcmp(label->name, name)) {
            /* printf("Found label %s with address 0x%04x\n", label->name, label->addr); */
            return label->addr;
        }

        label = label->next ? label->next : NULL;
    }

    return 0xFFFF;
}
