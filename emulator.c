#define FREQUENCY 60
#define HEIGHT 32
#define WIDTH 64

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>

const double delay = 1000 / FREQUENCY;

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
                    for (int i = 0; i < 64 * 32; i++)
                        gfx[i] = 0;
                    pc += 2;
                    break;
                case 0x000e:
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
            }
            break;
        case 0x1000:
            pc = opcode & 0x0fff;
            /* printf("%03x\n", pc); */
            break;
        case 0x2000:
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0fff;
            break;
        case 0x3000:
            if (V[(opcode & 0x0f00) >> 8]  == (opcode & 0x00ff)) {
                /* printf("skip\n"); */
                pc += 2;
            }
            pc += 2;
            break;
        case 0x4000:
           if (V[(opcode & 0x0f00) >> 8]  != (opcode & 0x00ff))
               pc += 2;
            pc += 2;
            break;
        case 0x5000:
            if (V[(opcode & 0x0f00) >> 8] == V[(opcode & 0x00f0) >> 4]) 
                pc += 2;
            pc += 2;
            break;
        case 0x6000:
            V[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
            /* printf("%02x\n", V[(opcode & 0x0f00) >> 8]); */
            pc += 2;
            break;
        case 0x7000:
            V[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
            /* printf("%02x\n", V[(opcode & 0x0f00) >> 8]); */
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
                case 0x0004:
					if (V[(opcode & 0x00f0) >> 4] > (0xff - V[(opcode & 0x0f00) >> 8])) 
						V[0xf] = 1;
					else 
						V[0xf] = 0;					
					V[(opcode & 0x0f00) >> 8] += V[(opcode & 0x00f0) >> 4];
					pc += 2;					
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
                    if ((V[(opcode & 0x0f00) >> 8] & 0x0001) == 1) 
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
                    if ((V[(opcode & 0x0f00) >> 8] >> 7) == 1)
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
        case 0xd000: {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;
            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++) {
                    if((pixel & (0x80 >> xline)) != 0) {
                        if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                            V[0xF] = 1;                                 
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }
          pc += 2;
        }
        break;
        case 0xe000:
            switch (opcode & 0x000f) {
                case 0x000e:
                    if (key[V[(opcode & 0x0f00) >> 8]] == 1)
                        pc += 2;
                    pc += 2;
                    break;
                case 0x0001:
                    if (!(key[V[(opcode & 0x0f00) >> 8]] == 1))
                        pc += 2;
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
                case 0x000a: {
                    while (getch() == ERR) 
                        ;
                    char pressed = getch();
                    switch (pressed) {
                        case 1:
                            V[(opcode & 0x0f00) >> 8] = 1;
                            break;
                        case 2:
                            V[(opcode & 0x0f00) >> 8] = 2;
                            break;
                        case 3:
                            V[(opcode & 0x0f00) >> 8] = 3;
                            break;
                        case 4:
                            V[(opcode & 0x0f00) >> 8] = 0xc;
                            break;
                        case 'a':
                            V[(opcode & 0x0f00) >> 8] = 4;
                            break;
                        case 'z':
                            V[(opcode & 0x0f00) >> 8] = 5;
                            break;
                        case 'e':
                            V[(opcode & 0x0f00) >> 8] = 6;
                            break;
                        case 'r':
                            V[(opcode & 0x0f00) >> 8] = 0xd;
                            break;
                        case 'q':
                            V[(opcode & 0x0f00) >> 8] = 7;
                            break;
                        case 's':
                            V[(opcode & 0x0f00) >> 8] = 8;
                            break;
                        case 'd':
                            V[(opcode & 0x0f00) >> 8] = 9;
                            break;
                        case 'f':
                            V[(opcode & 0x0f00) >> 8] = 0xe;
                            break;
                        case 'w':
                            V[(opcode & 0x0f00) >> 8] = 0xa;
                            break;
                        case 'x':
                            V[(opcode & 0x0f00) >> 8] = 0;
                            break;
                        case 'c':
                            V[(opcode & 0x0f00) >> 8] = 0xb;
                            break;
                        case 'v':
                            V[(opcode & 0x0f00) >> 8] = 0xf;
                            break;
                    }
                    pc += 2;
                }
                break;
                case 0x0005:
                    switch (opcode & 0x00f0) {
                        case 0x0010:
                            delay_timer = V[(opcode & 0x0f00) >> 8];
                            pc += 2;
                            break;
                        case 0x0050:
                            for (int i = 0; i <= ((opcode & 0x0f00) >> 8); i++) 
                                memory[I + i] = V[i];
                            I += ((opcode & 0x0f00) >> 8) + 1; // hmmmm
                            pc += 2;
                            break;
                        case 0x0060:
                            for (int i = 0; i <= ((opcode & 0x0f00) >> 8); i++)
                                V[i] = memory[I + i];
                            I += ((opcode & 0x0f00) >> 8) + 1; // hmmmm
                            pc += 2;
                            break;
                    }
                    break;
                case 0x0008:
                    sound_timer = V[(opcode & 0x0f00) >> 8];
                    pc += 2;
                    break;
                case 0x000e:
                    if (I + V[(opcode & 0x0f00) >> 8] > 0x0fff)
                        V[0xf] = 1;
                    else
                        V[0xf] = 0;
                    I += V[(opcode & 0x0f00) >> 8];
                    pc += 2;
                    break;
                case 0x0009:
                    I = V[(opcode & 0x0f00) >> 8] * 5;
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

void draw(WINDOW *win) {
    unsigned char pixel;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            pixel = gfx[x + (64 * y)];
            if (pixel == 1) 
                mvwprintw(win, y, x, "#");
            else
                mvwprintw(win, y, x, " ");
        }
    }    
    wrefresh(win);
}

int main(int argc, char **argv) {
    clock_t start, end;
    double time;

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);

    WINDOW *program = newwin(HEIGHT, WIDTH, 0, 0);
    refresh();

    box(program, 0, 0);

    // load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = fontset[i];

    // clear memory, stack, display and registers
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
        stack[i] = 0;
    }
    for (int i= 0; i < 4096; i++)
        memory[i] = 0;
    for (int i = 0; i < 64 * 32; i++)
        gfx[i] = 0;

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
        start = clock();
        emulateCycle();
        draw(program);
        if (delay_timer > 0)
            --delay_timer;
        if (sound_timer > 0) {
            beep();
            --sound_timer;
        }

        // store keys state
        for (int i = 0; i < 16; i++)
            key[i] = 0;
        char pressed = getch();
        switch (pressed) {
            case 1:
                key[1] = 1;
                break;
            case 2:
                key[2] = 1;
                break;
            case 3:
                key[3] = 1;
                break;
            case 4:
                key[0xc] = 1;
                break;
            case 'a':
                key[4] = 1;
                break;
            case 'z':
                key[5] = 1;
                break;
            case 'e':
                key[6] = 1;
                break;
            case 'r':
                key[0xd] = 1;
                break;
            case 'q':
                key[7] = 1;
                break;
            case 's':
                key[8] = 1;
                break;
            case 'd':
                key[9] = 1;
                break;
            case 'f':
                key[0xe] = 1;
                break;
            case 'w':
                key[0xa] = 1;
                break;
            case 'x':
                key[0] = 1;
                break;
            case 'c':
                key[0xb] = 1;
                break;
            case 'v':
                key[0xf] = 1;
                break;
        }
        
        end = clock();
        time = 1000 * (((double) (end - start)) / CLOCKS_PER_SEC);
        napms((delay - time));
    }

    endwin();

    return 0;
}
