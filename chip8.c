#include "chip8.h"
#include "opcodes.h"

chip8_t *chip8_new(void) {
    chip8_t *machine = malloc(sizeof(chip8_t));
    return machine;
}

int chip8_init(chip8_t *machine) {
    // should init everything but I feel lazy now
    memset(machine->RAM, 0, RAMSIZE); /* 4k RAM */
    memset(machine->VRAM, 0, VRAMSIZE); /* 2k VRAM */
    memset(machine->V, 0, NUM_REGISTERS); /* 16 registers */
    memset(machine->stack, 0, STACKSIZE * 2); /* 16 shorts */
    machine->PC = 0;
    
    return 1;
}

int chip8_loadFile(chip8_t *machine, const char *filename) {
    int success = 0;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Could not open file %s\n", filename);
        goto cleanup;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);

    if (size > RAMSIZE) {
        printf("The file is %d bytes long, the maximum size is 4096 bytes.\n", size);
        goto cleanup;
    }
        
    fseek(fp, 0, SEEK_SET);
    int r = fread(machine->RAM, 1, size, fp);

    if (r != size) {
        printf("Could not read file %s\n", filename);
        goto cleanup;
    }

    success = 1;
    
cleanup:
    fclose(fp);
    return success;
}

int chip8_destroy(chip8_t *machine) {
    // nothing to do, really...
    return 1;
}

int chip8_draw(chip8_t *machine) {
    // use the rendering callback here
    return 1;
}

int chip8_cycle(chip8_t *machine) {
    // fetch opcode
    machine->opcode = machine->RAM[machine->PC] << 8 | machine->RAM[machine->PC + 1];

    // decode opcode
    int index = machine->opcode & 0xF000;
    opcodes[index](machine);

    // execute opcode

    // update timers
}

int chip8_setKeys(chip8_t *machine, unsigned short state) {
    machine->keys = state;
    return 1;
}

