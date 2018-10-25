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

        info.id = chars_to_int(buffer);
        printf("id:%d\n", info.id);
        info.caddr = caddr;

        if (!can_accept(info.id)) {
            // can not handle this client
            printf("Server: can not handle client %d\n", info.id);
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
    int port, addrlen, rcvd, cmdID, videoID;
    struct sockaddr_in saddr;
    struct timeval tv;

    cinfo_t* client = (cinfo_t*) argument;

    client->sockt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client->sockt == 0) { 
        printf("Server: failed to create socket.\n"); 
        exit(EXIT_FAILURE); 
    }

    tv.tv_sec  = TIMEOUT_S;
    tv.tv_usec = 0;
    if (setsockopt(client->sockt, SOL_SOCKET, SO_RCVTIMEO, &tv, 
        sizeof(tv)) < 0) {
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
        if (bind(client->sockt, (struct sockaddr *) &saddr, 
            sizeof(saddr)) < 0) {
            port = 0;
        }
    } while (port == 0);
    printf("Server: client %d is connected at port %d.\n", client->id, port);

    // sending confirmation message to client
    int_to_4chars(port, buffer);
    if (sendto(client->sockt, buffer, 4, 0, (struct sockaddr *) &client->caddr, 
        sizeof(client->caddr)) < 0) {
        printf("Server: failed to send data to client.\n");
        exit(EXIT_FAILURE);
    } 

    // 
    while (1) {
        if ((rcvd = recvfrom(client->sockt, buffer, BUFFER_LEN, 0, 
             (struct sockaddr *) &client->caddr, &addrlen)) == -1) {
            printf("Server: failed to receive data from new client.\n");
            break;
        }

        cmdID = chars_to_int(buffer);
        if (cmdID == LIST_CODE) {
            printf("Server: list cmd received from client %d.\n", client->id);
            if (send_video_list(client)) {
                printf("Server: Failed to send list of videos.\n");
                break;
            }

        } else if (cmdID == PLAY_CODE) {
            videoID = chars_to_int(&buffer[4]);
            send_video(client, videoID);

        } else if (cmdID == EXIT_CODE) {
            printf("Server: Client %d leave.\n", client->id);
            break;
        
        } else {
            printf("Server: Unknown command from client %d.\n", client->id);
        }
    }
}

int can_accept(int cid) {
    return 1;
    // return ((rand() % 2) == 0);
}

void get_video_path(int videoID, char* path) {
    char buffer[BUFFER_LEN];
    int i, vid, numVideos;
    FILE *fp;

    fp = fopen(VIDEO_LIST_PATH, "r");
    if (fp == NULL) {
        printf("Server: video list file not found.\n");
        return;
    }

    // reading the number of available videos
    fgets(buffer, BUFFER_LEN, fp);
    sscanf(buffer, "%d", &numVideos);
    for (i = 0; i < numVideos; i++) {
        fgets(buffer, BUFFER_LEN, fp);
        sscanf(buffer, "%d", &vid);
        if (vid == videoID) {
            sscanf(buffer, "%*d;%*[^;];%s", path);
            break;
        }
    }

    fclose(fp);
}

int send_video(cinfo_t* client, int videoID) {
    char fpath[BUFFER_LEN];
    int i, *nextMsg;
    uint32_t seqNum;
    video_metadata_t vinfo;
    segment_t *buffer;
    message_t msg;
    clock_t currTime, segDeadline, **ttable;
    FILE *fp;

    fpath[0] = 0;
    get_video_path(videoID, fpath);
    if (fpath[0] == 0) {
        printf("Server: video %d could not be found.\n", videoID);
    }
    
    fp = fopen(fpath, "r");
    if (fp == NULL) {
        printf("Server: video file not found.\n");
        return 1;
    }

    get_video_metadata(fp, &vinfo);

    nextMsg = (int *) malloc(vinfo.n_cat * sizeof(int));
    if (nextMsg == NULL) {
        printf("Server: no memory.\n");
        return 1;
    }

    // creating time table (columns: [Release Time, Deadline, Increment])
    ttable = (clock_t**) malloc(3 * sizeof(clock_t *));
    if (ttable == NULL) {
        printf("Server: no memory for the time table.\n");
        return 1;
    }

    for (i = 0; i < 3; i++) {
        ttable[i] = (clock_t*) malloc(vinfo.n_cat * sizeof(clock_t));
        if (ttable[i] == NULL) {
            printf("Server: no memory for the time table.\n");
            return 1;
        }
    }

    // filling the increments (they are fixed for a video)
    for (i = 0; i < vinfo.n_cat; i++) {
        ttable[2][i] = (HYPER_PERIOD * CLOCKS_PER_SEC) / vinfo.cat[i].n_msgs;
    }

    // first sequence number must be random to avoid problems
    seqNum = rand();

    // sending loop
    while (loag_segment(fp, buffer) != 1) {
        // time table round initialization
        for (i = 0; i < vinfo.n_cat; i++) {
            nextMsg[0]   = 0;
            ttable[0][i] = clock();
            ttable[1][i] = ttable[0][i] + ttable[2][i];
        }

        segDeadline = clock() + (HYPER_PERIOD * CLOCKS_PER_SEC);
        while((currTime = clock()) < segDeadline) {
            for (i = 0; i < vinfo.n_cat; i++) {
                if (currTime > ttable[1][i]) {
                    // missed the deadline
                    nextMsg[i]++;
                    if (nextMsg[i] < buffer->cats[i].n_msgs) {
                        ttable[0][i] = ttable[1][i] + 1;
                        ttable[1][i] = ttable[1][i] + ttable[2][i];
                    } else {
                        ttable[0][i] = segDeadline;
                        ttable[1][i] = segDeadline;
                    }
                } else if (currTime > ttable[0][i]) {
                    // can send this message
                    msg = buffer->cats[i].msgs[nextMsg[i]];
                    // MUST SEND THE MESSAGE
                    nextMsg[i]++;
                    if (nextMsg[i] < buffer->cats[i].n_msgs) {
                        ttable[0][i] = ttable[1][i] + 1;
                        ttable[1][i] = ttable[1][i] + ttable[2][i];
                    } else {
                        ttable[0][i] = segDeadline;
                        ttable[1][i] = segDeadline;
                    }
                }
            }
        }
    }

    return 0;
}

int send_video_list(cinfo_t* client) {
    int i, msgSize, vid, numVideos;
    char buffer[BUFFER_LEN];
    FILE *fp;

    fp = fopen(VIDEO_LIST_PATH, "r");
    if (fp == NULL) {
        printf("Server: video list file not found.\n");
        return 1;
    }

    fgets(buffer, BUFFER_LEN, fp);
    sscanf(buffer, "%d", &numVideos);
    int_to_4chars(numVideos, buffer);
    if (sendto(client->sockt, buffer, 4, 0, (struct sockaddr *) &client->caddr, 
        sizeof(client->caddr)) < 0) {
        printf("Server: failed to send data to client.\n");
        fclose(fp);
        return 1;
    } 

    // reading file and preparing the video list
    // each line is composed by "id;video title;video_path"
    for (i = 0; i < numVideos; i++) {
        fgets(buffer, BUFFER_LEN, fp);
        sscanf(buffer, "%d;%[^;]s;%*s", &vid, &buffer[4]);
        int_to_4chars(vid, buffer);
        msgSize = 4 + strlen(&buffer[4]); // 4 bytes for the vid
        if (sendto(client->sockt, buffer, msgSize, 0, 
            (struct sockaddr *) &client->caddr, 
            sizeof(client->caddr)) < 0) {
            printf("Server: failed to send data to client.\n");
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}