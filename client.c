#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_sup.h"
#include "tools.h"
// #include <arpa/inet.h>

#define BUFFER_LEN 512

int main(int argc, char const *argv[]) {
    socklen_t addrlen;
    int sockt, sport, cport, id, rport, rcvd;
    char serverAddr[16];
    unsigned char buffer[BUFFER_LEN];
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
    int_to_4chars(id, buffer);
    if (sendto(sockt, buffer, 5, 0, (struct sockaddr *) &saddr, 
        sizeof(saddr)) < 0) {
        printf("Client: failed to send data to server.\n");
        exit(EXIT_FAILURE);
    }

    rcvd = recvfrom(sockt, buffer, BUFFER_LEN, 0, (struct sockaddr *) &saddr, 
                    &addrlen);
    if (rcvd == -1) {
        printf("Client: failed to receive answer from server.\n");
        exit(EXIT_FAILURE);
    }  else {
        printf("Client: recevied %d bytes from server.\n", rcvd);
    }

    rport = chars_to_int(buffer);
    if (rport < 0) {
        printf("Client: connection denied.\n");
        exit(EXIT_FAILURE);

    }
    printf("Client: should connect to port %d\n", rport);
    
    return 0;
}
