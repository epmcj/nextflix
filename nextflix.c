#include <stdio.h>
#include <stdlib.h>
#include "server.h"

#define PORT 8080

int main(int argc, char const *argv[]) {
    int port;

    if (argc < 2) {
        printf("Missing port parameter.\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    server_listen(port);
    
    return 0;
}
