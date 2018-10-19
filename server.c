#include "server.h"

void server_listen(int port) {
    int sockt;
    cinfo_t info;
    socklen_t addrlen;
    pthread_t* thread;
    unsigned char buffer[BUFFER_LEN];
    struct sockaddr_in saddr, caddr;

    srand(time(NULL));

    // creating socket to receive solicitations from clients
    sockt = create_and_bind_socket(port);
    if (sockt == 0) { 
        printf("Server: failed to create socket.\n"); 
        exit(EXIT_FAILURE); 
    }

    while (1) {
        printf("Server: Waiting for new client.\n");

        if (recvfrom(sockt, buffer, BUFFER_LEN, 0, 
            (struct sockaddr *) &caddr, &addrlen) == -1) {
            printf("Server: failed to receive data from new client.\n");
            continue;
        }

        info.cid = chars_to_int(buffer);
        printf("id:%d\n", info.cid);
        info.caddr = caddr;

        if (!can_accept(info.cid)) {
            // can not handle this client
            printf("Server: can not handle client %d\n", info.cid);
            // sending answer to client.
            int_to_4chars(-1, buffer);
            buffer[4] = 0;
            if (sendto(sockt, buffer, 5, 0, 
                (struct sockaddr *) &caddr, sizeof(caddr)) < 0) {
                printf("Server: failed to send answer to client.\n");
                exit(EXIT_FAILURE);
            } 
            continue;
        }

        thread = (pthread_t *) malloc(sizeof(pthread_t));
        if (!thread) {
            printf("Server: failed to allocate a new thread.\n");
            continue;
        }

        if (pthread_create(thread, NULL, handle_client, (void *) &info) != 0) {
            printf("Server: failed to create a new thread.\n");
            continue;
        }
    }
}

void* handle_client(void* argument) {
    char buffer[BUFFER_LEN];
    int csocket, port, addrlen, rcvd;
    struct sockaddr_in saddr, caddr;
    struct timeval tv;

    cinfo_t* info = (cinfo_t*) argument;

    csocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (csocket == 0) { 
        printf("Server: failed to create socket.\n"); 
        exit(EXIT_FAILURE); 
    }

    tv.tv_sec  = TIMEOUT_S;
    tv.tv_usec = 0;
    if (setsockopt(csocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("Server: could not set socket timeout.\n");
        exit(EXIT_FAILURE);
    }

    // finding an available port
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    do {
        port = 50000 + (rand() % 10000);
        saddr.sin_port = htons(port);
        if (bind(csocket, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
            port = 0;
        }
    } while (port == 0);

    printf("Server: client %d is connected at port %d.\n", info->cid, port);

    caddr = info->caddr;

    // sending confirmation message to client
    int_to_4chars(port, buffer);
    if (sendto(csocket, buffer, 5, 0, 
        (struct sockaddr *) &caddr, sizeof(caddr)) < 0) {
        printf("Server: failed to send data to client.\n");
        exit(EXIT_FAILURE);
    } 

    // 
    while (1) {
        if ((rcvd = recvfrom(csocket, buffer, BUFFER_LEN, 0, 
             (struct sockaddr *) &caddr, &addrlen)) == -1) {
            printf("Server: failed to receive data from new client.\n");
            break;
        }
    }
}

int can_accept(int cid) {
    return 1;
    // return ((rand() % 2) == 0);
}

int proccess_cmd(int actionID) {

    return 0;
}

int send_video(int videoID) {
    return 0;
}


int get_video_list(vlist_t *videoList) {
    int i;
    FILE *fp;

    fp = fopen(VIDEO_LIST_PATH, "r");
    if (fp == NULL) {
        printf("Server: video list file not found.\n");
        return 1;
    }

    fscanf(fp, "%d", &videoList->length);
    videoList->list = (int *) malloc(videoList->length * sizeof(int));
    if (videoList->list == NULL) {
        printf("Server: memory could not be allocated.\n");
        return 1;
    }

    // reading file and preparing the video list
    // each line is composed by "id;movie_title"
    for (i = 0; i < videoList->length; i++) {
        fscanf(fp, "%d;%*s", &videoList->list[i]);
    }
}