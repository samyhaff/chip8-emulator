/* 
TODO get key presses
TODO set speed to 60Hz
TODO add all opcodes
TODO BEEP
TODO TUI
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

unsigned short opcode;
unsigned char memory[4096]; 
unsigned char V[16];
unsigned short I = 0;
unsigned short pc = 0x200; // program starts at location 0x200
unsigned char gfx[64 * 32];
unsigned char delay_timer = 0;
unsigned char sound_timer = 0;
unsigned short stack[16];
unsigned char sp = 0;
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

void emulateCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xf000) {
        case 0x0000:
            switch (opcode & 0x000f) {
                case 0x0000:
                    // clear the display
                    pc += 2;
                    break;
                case 0x000e:
                    sp--;
                    pc = stack[sp];
                    break;
            }
            break;
        case 0x1000:
            pc = opcode & 0x0fff;
            break;
        case 0x2000:
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0fff;
            break;
        case 0x3000:
           if (V[(opcode & 0x0f00) >> 8]  == opcode & 0x00ff)
               pc += 2;
            pc += 2;
            break;
        case 0x4000:
           if (V[(opcode & 0x0f00) >> 8]  != opcode & 0x00ff)
               pc += 2;
            pc += 2;
            break;
        case 0x5000:
            if (V[(opcode & 0x0f00) >> 8] == V[(opcode & 0x00f0) > 4]) 
                pc += 2;
            pc += 2;
            break;
        case 0x6000:
            V[(opcode & 0x0f00) > 8] = opcode & 0x00ff;
            pc += 2;
            break;
        case 0x7000:
            V[(opcode & 0x0f00) > 8] += opcode & 0x00ff;
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000f) {
                case 0x0000:
                    V[(opcode & 0x0f00) >> 8] = V[(opcode & 0x00f0) >> 4];
                    pc += 2;
                    break;
                case 0x0001:
                    V[(opcode & 0x0f00) >> 8] |= V[(opcode & 0x00f0) >> 4];
                    pc += 2;
                    break;
                case 0x0002:
                    V[(opcode & 0x0f00) >> 8] &= V[(opcode & 0x00f0) >> 4];
                    pc += 2;
                    break;
                case 0x0003:
                    V[(opcode & 0x0f00) >> 8] ^= V[(opcode & 0x00f0) >> 4];
                    pc += 2;
                    break;
                case 0x0004: {
                    unsigned short result = V[(opcode & 0x0f00) >> 8] + V[(opcode & 0x00f0) >> 4];
                    if (result > 255)
                        V[0xf] = 1;
                    else 
                        V[0xf] = 0;
                    V[(opcode & 0x0f00) >> 8] = result && 0x00ff;
                    pc += 2;
                }
                break;
                case 0x0005:
                    if (V[(opcode & 0x0f00) >> 8] > V[(opcode & 0x00f0) >> 4])
                        V[0xf] = 1;
                    else
                        V[0xf] = 0;
                    V[(opcode & 0x0f00) >> 8] -= V[(opcode & 0x00f0) >> 4];
                    pc += 2;
                    break;
                case 0x0006:
                    if (V[(opcode & 0x0f00) >> 8] & 0x0001 == 1) 
                        V[0xf] = 1;
                    else
                        V[0xf] = 0;
                    V[(opcode & 0x0f00) >> 8] /= 2;
                    pc += 2;
                    break;
                case 0x0007:
                    if (V[(opcode & 0x0f00) >> 8] < V[(opcode & 0x00f0) >> 4])
                        V[0xf] = 1;
                    else
                        V[0xf] = 0;
                    V[(opcode & 0x0f00) >> 8] = V[(opcode & 0x00f0) >> 4] - V[(opcode & 0x0f00) >> 8];
                    pc += 2;
                    break;
                case 0x000e:
                    if (V[(opcode & 0x8000) >> 15] == 1)
                        V[0xf] = 1;
                    else
                        V[0xf] = 0;
                    V[(opcode & 0x0f00) >> 8] *= 2;
                    pc += 2;
                    break;
            }
            break;
        case 0x9000:
            if (!(V[(opcode & 0x0f00) >> 8] == V[(opcode & 0x00f0) >> 4]))
                pc += 2;
            pc += 2;
            break;
        case 0xa000:
            I = opcode & 0x0fff;
            pc += 2;
            break;
        case 0xb000:
            pc = V[0] + (opcode & 0x0fff);
        case 0xc000: {
            unsigned char r = rand() % 256;            
            V[(opcode & 0x0f00) >> 8] = (opcode & 0x00ff) & r;
            pc += 2;
        }
        break;
        case 0xd000:
            // affichage
            pc += 2;
            break; 
        case 0xe000:
            switch (opcode & 0x000f) {
                case 0x000e:
                    // key
                    pc += 2;
                    break;
                case 0x0001:
                    // key
                    pc += 2;
                    break;
            }
            break;
        case 0xf000:
            switch (opcode & 0x000f) {
                case 0x0007:
                    V[(opcode & 0x0f00) >> 8] = delay_timer;
                    pc += 2;
                    break;
                case 0x000a:
                    // key
                    pc += 2;
                    break;
                case 0x0005:
                    switch (opcode & 0x00f0) {
                        case 0x0010:
                            delay_timer = V[(opcode & 0x0f00) >> 8];
                            pc += 2;
                            break;
                        case 0x0050:
                            for (int i = 0; i <= (opcode & 0x0f00) >> 8; i++) 
                                memory[I + i] = V[i];
                            pc += 2;
                            break;
                        case 0x0060:
                            for (int i = 0; i <= (opcode & 0x0f00) >> 8; i++)
                                V[i] = memory[I + i];
                            pc += 2;
                            break;
                    }
                    break;
                case 0x0008:
                    sound_timer = V[(opcode & 0x0f00) >> 8];
                    pc += 2;
                    break;
                case 0x000e:
                    I += V[(opcode & 0x0f00) >> 8];
                    pc += 2;
                    break;
                case 0x0009:
                    // graphics
                    pc += 2;
                    break;
                case 0x0003:
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
            }
            break;
    }
}


int main(int argc, char **argv) {
    // load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = fontset[i];

    // load program
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
        memory[i + 512] = buffer[i];

    for (;;) {
        emulateCycle();
        // if (drawFlag)
            // drax graphics
        if (delay_timer > 0)
            --delay_timer;
        if (sound_timer > 0) {
            // BEEP
            --sound_timer;
        }
        // store keys state
    }

    return 0;
}
