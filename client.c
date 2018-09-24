#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_LEN 512

int main(int argc, char const *argv[]) {
    int csocket, serverPort, id;
    char* serverAddr;
    char msg[BUFFER_LEN];
    struct sockaddr_in address;
    
    if (argc < 3) {
        printf("Missing parameters: ./client [SERVER_ADDR] [SERVER_PORT]\n");
        exit(EXIT_FAILURE);
    }
    serverAddr = argv[1];
    serverPort = atoi(argv[2]);

    csocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (csocket == 0) { 
        printf("Client: failed to create socket.\n"); 
        exit(EXIT_FAILURE); 
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, serverAddr, &address.sin_addr.s_addr) <= 0) {
        printf("Client: invalid server address: %s.\n", serverAddr);
        exit(EXIT_FAILURE);
    }
    address.sin_port = htons(serverPort);

    id = 123456;
    sprintf(msg, "%d\0", id);

    if (sendto(csocket, msg, strlen(msg), 0, (struct sockaddr *) &address, 
        sizeof(address)) < 0) {
        printf("Client: failed to send data to server.\n");
        exit(EXIT_FAILURE);
    }    
    
    return 0;
}
