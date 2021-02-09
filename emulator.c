#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

unsigned short opcode = 0;
unsigned char memory[4096]; 
unsigned char V[16];
unsigned short I = 0;
unsigned short pc = 0x200;
unsigned char gfx[64 * 32];
unsigned char delay_timer = 0;
unsigned char sound_timer = 0;
unsigned short stack[16];
unsigned short sp = 0;
unsigned char key[16];
unsigned char drawFlag = 0;
unsigned char fontset[80] = 
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize() {
    for (int i = 0; i < 64 * 32; i++)
        gfx[i] = 0;
    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        V[i] = 0;
    }
    for (int i = 0; i < 4096; i++) 
        memory[i] = 0;
    for (int i = 0; i < 80; i++)
        memory[i] = fontset[i];
}

void emulateCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xf000) {
        case 0xa000:
            I = opcode & 0x0fff;
            pc += 2;
        break;
        case 0x2000:
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0fff;
        break;
        case 0x0004:
            if (V[(opcode & 0x00f0) >> 4] > (0xff - V[(opcode & 0x0f00) >> 8]))
                V[0xf] = 1; // carry
            else
                V[0xf] = 0;
            V[(opcode & 0x0f00) >> 8] += V[(opcode & 0x00f0) >> 4];
            pc += 2;
        break;
        case 0x0033:
            memory[I] = V[(opcode & 0x0f00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0f00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0f0) >> 8] % 100) % 10;
            pc += 2;
        break;
        // ...
        case 0xd000: {
            unsigned short x = V[(opcode & 0x0f00) >> 8];
            unsigned short y = V[(opcode & 0x00f0) >> 4];
            unsigned short height = opcode & 0x000f;
            unsigned short pixel;
            V[0xf] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                            V[0xf] = 1;
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }
            drawFlag = 1;
            pc += 2;
        }
        break;
        case 0xe000:
            switch (opcode & 0x00ff) {
                case 0x009e: 
                    if (key[V[(opcode & 0x0f00) >> 8]] != 0)
                        pc += 4;
                    else
                        pc += 2;
                break;
            }
        break;
        default:
            printf("Unknow opcode 0x%X\n", opcode);
    }

    if (delay_timer > 0)
        --delay_timer;
    if (sound_timer > 0) {
        if (sound_timer == 1)
            printf("BEEP\n");
        --sound_timer;
    }
}

int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("error; Couldn't open %s\n", argv[1]);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);    
    int fsize = ftell(f);    
    fseek(f, 0L, SEEK_SET);    
    unsigned char *buffer = malloc(fsize);
    fread(buffer, fsize, 1, f);
    fclose(f);
    for (int i = 0; i < fsize; i++)
        printf("%04x\n", buffer[i]);

    return 0;
}
