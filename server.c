#include "server.h"

void server_listen(int port) {
    socklen_t addrlen;
    pthread_t* thread;
    pthread_arg arg;
    int ssocket, csocket;
    struct sockaddr_in saddr, caddr;
    char buffer[BUFFER_LEN];

    addrlen = (socklen_t) sizeof(saddr);

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
        csocket = atoi(buffer);
        arg.csocket = csocket;

        if (pthread_create(thread, NULL, handle_client, (void *) &arg) != 0) {
            printf("Server: failed to create a new thread.\n");
            continue;
        }
    }
}

void* handle_client(void* argument) {
    time_t itime, etime;

    pthread_arg* arg = (pthread_arg*) argument;
    
    itime = time(NULL);
    etime = itime + 10;

    printf("New client:\n id: %d, begin: %d\n", arg->csocket, itime);
    
    while(time(NULL) < etime);

    printf(" id: %d, end: %d\n", arg->csocket, etime);
}