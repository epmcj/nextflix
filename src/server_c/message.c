#include "message.h"

/**
 * Write a message header in a buffer.
 **/
int write_header(mheader_t* header, char* buffer) {
    memcpy(buffer, header, sizeof(mheader_t));
    return sizeof(mheader_t);
}

mheader_t* read_header(char* msg) {
    return (mheader_t *) msg;
}

int create_msg(mheader_t* header, void* payload, size_t psize, char* buffer) {
    write_header(header, buffer);
    memcpy(buffer + sizeof(mheader_t), payload, psize);
    return sizeof(mheader_t) + psize;
}