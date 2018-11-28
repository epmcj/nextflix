#include <stdio.h>
#include <stdlib.h>
#include "message.h"

#define BUFFER_LEN 512

struct st1 {
    unsigned char f1;
};

struct st2 {
    uint32_t f1;
};

struct st3 {
    unsigned char f2;
    uint32_t f1;
};

int main(int argc, char const *argv[]) {
    /* code */
    mheader_t hdr, hdr2;
    char buffer[BUFFER_LEN];
    int msize;

    hdr.type    = INIT_TYPE;
    hdr.seq_num = 68;

    msize = create_msg(&hdr, NULL, 0, buffer);
    printf("msize = %d \n", msize);
    int i;
    for (i = 0; i < msize; i++) {
        printf("%d,", buffer[i]);
    }
    printf("\n");

    hdr.type    = DATA_TYPE;
    hdr.seq_num = 75065;
    char payload[] = {1,2,3,4,5,6};
    msize = create_msg(&hdr, payload, 6, buffer);
    printf("msize = %d \n", msize);  
    for (i = 0; i < msize; i++) {
        printf("%d,", buffer[i]);
    }
    printf("\n");

    printf("--%ld\n", sizeof(unsigned char));
    printf("--%ld\n", sizeof(uint32_t));
    printf("--%ld\n", sizeof(mheader_t));
    printf("--%ld\n", sizeof(struct st1));
    printf("--%ld\n", sizeof(struct st2));
    printf("--%ld\n", sizeof(struct st3));

    return 0;
}

