#include <stdio.h>
#include <stdlib.h>
#include "message.h"

#define BUFFER_LEN 512

int main(int argc, char const *argv[]) {
    /* code */
    mheader_t hdr;
    char buffer[BUFFER_LEN];
    int msize;

    hdr.type    = INIT_TYPE;
    hdr.seq_num = 68;

    msize = create_msg(&hdr, NULL, 0, buffer);
    print("msize = %d \n", msize);  

    return 0;
}

