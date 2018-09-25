#include "tools.h"

void int_to_chars(unsigned int n, char* buffer) {
    buffer[0] = (n >> 24) & 0xFF;
    buffer[1] = (n >> 16) & 0xFF;
    buffer[2] = (n >>  8) & 0xFF;
    buffer[3] = n & 0xFF;
}

unsigned int chars_to_int(char* buffer) {
    unsigned int n, i;

    n = 0;
    for (i = 0; i < 4; i++) {
        n = n << 8;
        n |= buffer[i];
    }

    return n;
}
