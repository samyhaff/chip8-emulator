#include <stdio.h>
#include <stdlib.h>

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

    for (int i = 0; i < fsize - 1; i += 2)
        printf("0x%03x: %04x\n", i + 512, buffer[i] << 8 | buffer[i + 1]);

    return 0;
}
