#include "server.h"

void server_listen(int port) {
    socklen_t addrlen;
    pthread_t* thread;
    pthread_arg arg;
    int ssocket, csocket;
    struct sockaddr_in saddr, caddr;
    char buffer[BUFFER_LEN];

    addrlen = (socklen_t) sizeof(saddr);
    
    srand(time(NULL));

    ssocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ssocket == 0) { 
        printf("Server: failed to create socket.\n"); 
        exit(EXIT_FAILURE); 
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port        = htons(port);

    if (bind(ssocket, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        printf("Server: failed to bind socket to port %d.\n", port);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Server: Waiting for new client.\n");

        if (recvfrom(ssocket, buffer, BUFFER_LEN, 0, 
            (struct sockaddr *) &caddr, &addrlen) == -1) {
            printf("Server: failed to receive data from new client.\n");
            continue;
        }

        thread = (pthread_t *) malloc(sizeof(pthread_t));
        if (!thread) {
            printf("Server: failed to allocate a new thread.\n");
            continue;
        }

        // TEMP !!!
        csocket   = atoi(buffer);
        arg.cid   = csocket;
        arg.caddr = caddr;

        if (pthread_create(thread, NULL, handle_client, (void *) &arg) != 0) {
            printf("Server: failed to create a new thread.\n");
            continue;
        }
    }
}

void* handle_client(void* argument) {
    int csocket, portNum;
    struct sockaddr_in saddr;
    time_t itime, etime;

    pthread_arg* arg = (pthread_arg*) argument;
    
    itime = time(NULL);
    etime = itime + 10;

    // finding an open port
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    do {
        portNum = 50000 + (rand() % 10000);
        saddr.sin_port = htons(portNum);
        if (bind(csocket, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
            portNum = 0;
        }
    } while (portNum != 0);

    printf("Server: client %d is connected at port %d\n.", arg->cid, portNum);

    // Send message to client


    printf(" id: %d, end: %d\n", arg->cid, etime);
}