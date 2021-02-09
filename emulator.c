#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct chip8 {
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
} chip8;

int main() {
    return 0;
}
