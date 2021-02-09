#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct chip8 {
    unsigned short opcode;
    unsigned char memory[4096]; 
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned char gfx[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short sp;
    unsigned char key[16];
    // fontset
} chip8;

void initialize() {
    chip8.pc = 0x200; 
    chip8.opcode = 0;
    chip8.I = 0;
    chip8.sp = 0;
    for (int i = 0; i < 64 * 32; i++)
        chip8.gfx[i] = 0;
    for (int i = 0; i < 16; i++) {
        chip8.stack[i] = 0;
        chip8.V[i] = 0;
    }
    for (int i = 0; i < 4096; i++) 
        chip8.memory[i] = 0;
    // for (int i = 0; i < 80; i++)
    //     chip9.memory[i] = chip8.fontset[i];
    chip8.sound_timer = 0;
    chip8.delay_timer = 0;
}

void emulateCycle() {
    chip8.opcode = chip8.memory[chip8.pc] << 8 | chip8.memory[chip8.pc + 1];

    switch (chip8.opcode & 0xf000) {
        case 0xa000:
            chip8.I = chip8.opcode & 0x0fff;
            chip8.pc += 2;
        break;
        // ...
        default:
            printf("Unknow opcode 0x%X\n", chip8.opcode);
    }

    if (chip8.delay_timer > 0)
        --chip8.delay_timer;
    if (chip8.sound_timer > 0) {
        if (chip8.sound_timer == 1)
            printf("BEEP\n");
        --chip8.sound_timer;
    }
}

int main() {
    return 0;
}
