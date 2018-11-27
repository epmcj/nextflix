#include "message.h"

/**
 * Write the header in a buffer.
 **/
int write_header(mheader_t* header, char* buffer) {
    memcpy(buffer, header, sizeof(mheader_t));
    return 0;
}

int create_msg(mheader_t* header, void* payload, size_t psize, char* buffer) {
    write_header(header, buffer);
    printf("msg header was copied (%ld).\n", sizeof(mheader_t));
    memcpy(buffer + sizeof(mheader_t), payload, psize);
    printf("msg payload was copied (%ld).\n", psize);
    return sizeof(mheader_t) + psize;
}