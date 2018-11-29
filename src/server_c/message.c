#include "message.h"

/**
 * Write a message header in a buffer.
 **/
int write_header(mheader_t* header, char* buffer) {
    memcpy(buffer, header, sizeof(mheader_t));
    return 0;
}

mheader_t* read_header(char* msg) {
    return (mheader_t *) msg;
}

int create_msg(mheader_t* header, void* payload, size_t psize, char* buffer) {
    write_header(header, buffer);
    printf("msg header was copied (%ld).\n", sizeof(mheader_t));
    memcpy(buffer + sizeof(mheader_t), payload, psize);
    printf("msg payload was copied (%ld).\n", psize);
    if ((sizeof(mheader_t) + psize) < 10) {
        printf("msg = ");
        int i;
        for (i = 0; i < (sizeof(mheader_t) + psize); i++) {
            printf("\\x%x", buffer[i] & 0xFF);
        }
        printf("\n");
    }
    return sizeof(mheader_t) + psize;
}