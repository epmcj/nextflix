#include "tools.h"
// #include <stdio.h>

void int_to_4chars(int n, unsigned char* buffer) {
    buffer[0] = (n >> 24) & 0xFF;
    buffer[1] = (n >> 16) & 0xFF;
    buffer[2] = (n >>  8) & 0xFF;
    buffer[3] = n & 0xFF;
    // printf("%x-%x-%x-%x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
}

int chars_to_int(unsigned char* buffer) {
    int n, i;

    n = 0;
    for (i = 0; i < 4; i++) {
        n = n << 8;
        n |= buffer[i];
    }

    return n;
}
