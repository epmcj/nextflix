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
    socklen_t addrlen;
    int ssocket, rsocket, serverPort, ownPort, id;
    char* serverAddr;
    char msg[BUFFER_LEN];
    struct sockaddr_in saddr, addr;

    addrlen = (socklen_t) sizeof(addr);
    
    if (argc < 4) {
        printf("Missing parameters: ./client [SERVER_ADDR] [SERVER_PORT]\
 [CLIENT_PORT]\n");
        exit(EXIT_FAILURE);
    }
    serverAddr = argv[1];
    serverPort = atoi(argv[2]);
    ownPort    = atoi(argv[3]);

    // creating socket to receive data
    rsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(ownPort);

    if (bind(rsocket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("Client: failed to bind socket to port %d.\n", ownPort);
        exit(EXIT_FAILURE);
    }

    // creating socket to send data
    ssocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ssocket == 0) { 
        printf("Client: failed to create socket to send data.\n"); 
        exit(EXIT_FAILURE); 
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, serverAddr, &saddr.sin_addr.s_addr) <= 0) {
        printf("Client: invalid server address: %s.\n", serverAddr);
        exit(EXIT_FAILURE);
    }
    saddr.sin_port = htons(serverPort);

    //
    id = 123456;
    sprintf(msg, "%d-%d", id, ownPort);

    if (sendto(ssocket, msg, strlen(msg), 0, (struct sockaddr *) &saddr, 
        sizeof(saddr)) < 0) {
        printf("Client: failed to send data to server.\n");
        exit(EXIT_FAILURE);
    }

    if (recvfrom(ssocket, msg, BUFFER_LEN, 0, (struct sockaddr *) &saddr, 
        &addrlen) == -1) {
        printf("Client: failed to receive answer from servert.\n");
        exit(EXIT_FAILURE);
    }  

    id = atoi(msg);
    printf("port: %d\n", id); 
    
    return 0;
}
