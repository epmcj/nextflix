#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <stdlib.h>
#include <stdint.h>

// message types
#define INIT_TYPE 0x00
#define DATA_TYPE 0x01
#define FIN_TYPE  0x02
#define FBCK_TYPE 0x04

// Message header
typedef struct {
    unsigned char type;
    uint32_t   seq_num;
} mheader_t;

int write_header(mheader_t* header, char* buffer);

int create_msg(mheader_t* header, void* payload, size_t psize, char* buffer);

#endif