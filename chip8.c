#include "chip8.h"
#include "opcodes.h"

chip8_t *chip8_new(void) {
    chip8_t *machine = malloc(sizeof(chip8_t));
    return machine;
}

int chip8_init(chip8_t *machine) {
    // init everything
    machine->opcode = 0;
    machine->I = 0;
    machine->PC = 0x0200;

    // Clear memory
    memset(machine->RAM, 0, RAMSIZE); /* 4k RAM */
    // Clear display
    memset(machine->VRAM, 0, VRAMSIZE); /* 2k VRAM */
    // Clear registers V0-VF
    memset(machine->V, 0, NUM_REGISTERS); /* 16 registers */
    // Clear stack
    memset(machine->stack, 0, STACKSIZE * 2); /* 16 shorts */
    machine->SP = 0;

    // 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    // 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    // 0x200-0xFFF - Program ROM and work RAM

    // Load fontset
    for(int i = 0; i < 80; ++i)
        machine->RAM[FONTBASEADDR + i] = chip8_fontset[i];

    // reset timers
    machine->delay_timer = 0;
    machine->sound_timer = 0;

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

    if (size > RAMSIZE - 0x0200) {
        printf("The file is %d bytes long, the maximum size is 3584 bytes.\n", size);
        goto cleanup;
    }

    fseek(fp, 0, SEEK_SET);
    int r = fread(machine->RAM + 0x0200, 1, size, fp);

    if (r != size) {
        printf("Could not read file %s\n", filename);
        goto cleanup;
    }

    // debug code
    /* machine->RAM[0x0200] = 0xa2; */
    /* machine->RAM[0x0201] = 0x10; */
    /* machine->RAM[0x0202] = 0xd0; */
    /* machine->RAM[0x0203] = 0x01; */
    /* machine->RAM[0x0204] = 0x12; */
    /* machine->RAM[0x0205] = 0x00; */
    
    /* machine->RAM[0x0206] = 0x00; */
    /* machine->RAM[0x0207] = 0x00; */
    /* machine->RAM[0x0208] = 0x00; */
    /* machine->RAM[0x0209] = 0x00; */

    /* machine->RAM[0x0210] = 0x80; */

    printf("Loaded %u bytes\n", size);
    
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
    machine->opcode = (machine->RAM[machine->PC] << 8) | (machine->RAM[machine->PC + 1]);

    //printf("Executing opcode 0x%04x address 0x%04x\n", machine->opcode, machine->PC);

    // decode opcode
    int index = (machine->opcode & 0xF000) >> 12;

    // execute opcode
    opcodes[index](machine);

    // update timers
    if (machine->sound_timer > 0) { machine->sound_timer--; }
    if (machine->delay_timer > 0) { machine->delay_timer--; }

    return 1;
}

int chip8_setKeys(chip8_t *machine, unsigned char state[16]) {
    for (int i = 0; i < 16; ++i)
        machine->keys[i] = state[i];
    return 1;
}
