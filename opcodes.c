#include "chip8.h"
#include "opcodes.h"

void opcode0(chip8_t *machine) {
    switch (machine->opcode) {
    case 0x00E0:
        // clear the display
        memset(machine->VRAM, 0, VRAMSIZE);
        break;
    case 0x00F0:
        // return from a subroutine
        machine->PC = machine->stack[machine->sp];
        machine->sp--;
        return;
    default:
        // jump to machine code routine at nnn
        // may be left unimplemented
        break;
    }

    machine->PC += 2;
}

void opcode1(chip8_t *machine) {
}

void opcode2(chip8_t *machine) {
    
}

void opcode3(chip8_t *machine) {
    
}

void opcode4(chip8_t *machine) {
    
}

void opcode5(chip8_t *machine) {
    
}

void opcode6(chip8_t *machine) {
    
}

void opcode7(chip8_t *machine) {
    
}

void opcode8(chip8_t *machine) {
    // multiplexed
}

void opcode9(chip8_t *machine) {
    
}

void opcodeA(chip8_t *machine) {
    
}

void opcodeB(chip8_t *machine) {
    
}

void opcodeC(chip8_t *machine) {
    
}

void opcodeD(chip8_t *machine) {
    
}

void opcodeE(chip8_t *machine) {
    // multiplexed
}

void opcodeF(chip8_t *machine) {
    // multiplexed
}

void (*opcodes[16])(chip8_t *machine);
