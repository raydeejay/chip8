#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "labels.h"

/*******************
 * LABELS
 ******************/
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
        free(label);
        label = aux ? aux : NULL;
    }
}

int findLabel(const char *name) {
    label_t *label = gLabels;

    while (label) {
        if (!strcmp(label->name, name)) {
            /* printf("Found label %s with address 0x%04x\n", label->name, label->addr); */
            return label->addr;
        }

        label = label->next ? label->next : NULL;
    }

    return 0xFFFF;
}
