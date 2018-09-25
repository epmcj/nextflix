#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_sup.h"
// #include <arpa/inet.h>

#define BUFFER_LEN 512

int main(int argc, char const *argv[]) {
    socklen_t addrlen;
    int sockt, sport, cport, id;
    char serverAddr[16];
    char buffer[BUFFER_LEN];
    struct sockaddr_in saddr, caddr;

    if (argc < 3) {
        printf("Use: ./client [SERVER ADDR] [SERVER PORT] [CLIENT PORT]\n");
        exit(EXIT_FAILURE);
    }
    strcpy(serverAddr, argv[1]);
    sport = atoi(argv[2]);
    cport = atoi(argv[3]);

    // creating socket for communication
    sockt = create_and_bind_socket(cport);
    if (sockt == 0) { 
        printf("Client: failed to create socket.\n"); 
        exit(EXIT_FAILURE); 
    }

    // filling server information
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, serverAddr, &saddr.sin_addr.s_addr) <= 0) {
        printf("Client: invalid server address: %s.\n", serverAddr);
        exit(EXIT_FAILURE);
    }
    saddr.sin_port = htons(sport);

    // sending hello message
    id = 123456;
    sprintf(buffer, "%d", id);

    if (sendto(sockt, buffer, strlen(buffer), 0, (struct sockaddr *) &saddr, 
        sizeof(saddr)) < 0) {
        printf("Client: failed to send data to server.\n");
        exit(EXIT_FAILURE);
    }

    if (recvfrom(sockt, buffer, BUFFER_LEN, 0, (struct sockaddr *) &saddr, 
        &addrlen) == -1) {
        printf("Client: failed to receive answer from server.\n");
        exit(EXIT_FAILURE);
    }  

    id = atoi(buffer);
    printf("port: %d\n", id); 
    
    return 0;
}
