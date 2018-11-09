#include "server.h"

void server_listen(int port) {
    int sockt;
    cinfo_t info;
    socklen_t addrLen;
    pthread_t* thread;
    unsigned char buffer[BUFFER_LEN];
    struct sockaddr_in saddr, caddr;

    srand(time(NULL));

    // creating socket to receive solicitations from clients
    sockt = create_and_bind_socket(port);
    if (sockt == 0) { 
        #if DEBUG_MODE
        printf("Server: failed to create socket.\n"); 
        #endif
        exit(EXIT_FAILURE); 
    }

    while (1) {
        #if DEBUG_MODE
        printf("Server: Waiting for new client.\n");
        #endif

        if (recvfrom(sockt, buffer, BUFFER_LEN, 0, 
            (struct sockaddr *) &caddr, &addrLen) == -1) {
            #if DEBUG_MODE
            printf("Server: failed to receive data from new client.\n");
            #endif
            continue;
        }

        info.id = chars_to_int(buffer);
        #if DEBUG_MODE
        printf("id:%d\n", info.id);
        #endif
        info.caddr = caddr;

        if (!can_accept(info.id)) {
            // can not handle this client
            #if DEBUG_MODE
            printf("Server: can not handle client %d\n", info.id);
            #endif
            // sending answer to client.
            int_to_4chars(-1, buffer);
            buffer[4] = 0;
            if (sendto(sockt, buffer, 5, 0, 
                (struct sockaddr *) &caddr, sizeof(caddr)) < 0) {
                #if DEBUG_MODE
                printf("Server: failed to send answer to client.\n");
                #endif
                exit(EXIT_FAILURE);
            } 
            continue;
        }

        thread = (pthread_t *) malloc(sizeof(pthread_t));
        if (!thread) {
            #if DEBUG_MODE
            printf("Server: failed to allocate a new thread.\n");
            #endif
            continue;
        }

        if (pthread_create(thread, NULL, handle_client, (void *) &info) != 0) {
            #if DEBUG_MODE
            printf("Server: failed to create a new thread.\n");
            #endif
            continue;
        }
    }
}

void* handle_client(void* argument) {
    char buffer[BUFFER_LEN];
    int ctrlPort, dataPort, addrLen, rcvd, cmdID, videoID;
    struct sockaddr_in saddr;
    struct timeval tv;

    cinfo_t* client = (cinfo_t*) argument;

    client->ctrl_sockt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client->ctrl_sockt == 0) { 
        #if DEBUG_MODE
        printf("Server: failed to create control socket.\n"); 
        #endif
        exit(EXIT_FAILURE); 
    }
    
    client->data_sockt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client->data_sockt == 0) { 
        #if DEBUG_MODE
        printf("Server: failed to create data socket.\n"); 
        #endif
        exit(EXIT_FAILURE); 
    }

    tv.tv_sec  = TIMEOUT_S;
    tv.tv_usec = TIMEOUT_US;
    if (setsockopt(client->ctrl_sockt, SOL_SOCKET, SO_RCVTIMEO, &tv, 
        sizeof(tv)) < 0) {
        #if DEBUG_MODE
        printf("Server: could not set socket timeout.\n");
        #endif
        exit(EXIT_FAILURE);
    }

    tv.tv_sec  = TIMEOUT_S;
    tv.tv_usec = 0;
    if (setsockopt(client->ctrl_sockt, SOL_SOCKET, SO_RCVTIMEO, &tv, 
        sizeof(tv)) < 0) {
        #if DEBUG_MODE
        printf("Server: could not set socket timeout.\n");
        #endif
        exit(EXIT_FAILURE);
    }

    // finding an available port
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    do {
        // control port must be even
        ctrlPort = 40000 + (rand() % 20000);
        saddr.sin_port = htons(ctrlPort);
        if (bind(client->ctrl_sockt, (struct sockaddr *) &saddr, 
            sizeof(saddr)) < 0) {
            ctrlPort = 0;
        }
    } while (ctrlPort == 0);
    #if DEBUG_MODE
    printf("Server: client %d => port %d.\n", client->id, ctrlPort);
    #endif

    // sending confirmation message to client
    int_to_4chars(ctrlPort, buffer);
    if (sendto(client->ctrl_sockt, buffer, 4, 0, 
        (struct sockaddr *) &client->caddr, sizeof(client->caddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to send data to client.\n");
        #endif
        exit(EXIT_FAILURE);
    } 

    // 
    while (1) {
        if ((rcvd = recvfrom(client->ctrl_sockt, buffer, BUFFER_LEN, 0, 
             (struct sockaddr *) &client->caddr, &addrLen)) == -1) {
            #if DEBUG_MODE
            printf("Server: failed to receive data from new client.\n");
            #endif
            break;
        }

        cmdID = chars_to_int(buffer);
        if (cmdID == LIST_CODE) {
            #if DEBUG_MODE
            printf("Server: list cmd received from client %d.\n", client->id);
            #endif
            if (send_video_list(client)) {
                #if DEBUG_MODE
                printf("Server: failed to send list of videos.\n");
                #endif
                break;
            }

        } else if (cmdID == PLAY_CODE) {
            videoID = chars_to_int(&buffer[4]);
            send_video(client, videoID);

        } else if (cmdID == EXIT_CODE) {
            #if DEBUG_MODE
            printf("Server: client %d left.\n", client->id);
            #endif
            break;
        
        } else {
            #if DEBUG_MODE
            printf("Server: unknown command from client %d.\n", client->id);
            #endif
        }
    }

    close(client->ctrl_sockt);
    close(client->data_sockt);
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
        #if DEBUG_MODE
        printf("Server: video list file not found.\n");
        #endif
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

void* handle_feedback(void* parameters) {

}

int send_video(cinfo_t* client, int videoID) {
    char buffer[BUFFER_LEN];
    int i, addrLen, rcvd, *nextMsg;
    uint32_t seqNum;
    video_metadata_t vinfo;
    segment_t *segment;
    message_t msg;
    flow_t flowInfo;
    clock_t currTime, segDeadline, **ttable;
    FILE *fp;
printf("starting sending function\n");
    // first uses the the buffer to store the file path.
    buffer[0] = 0;
    get_video_path(videoID, buffer);
    if (buffer[0] == 0) {
        #if DEBUG_MODE
        printf("Server: video %d could not be found.\n", videoID);
        #endif
        return 1;
    }
    
    fp = fopen(buffer, "r");
    if (fp == NULL) {
        #if DEBUG_MODE
        printf("Server: video file not found.\n");
        #endif
        return 1;
    }

//    get_video_metadata(fp, &vinfo);

    nextMsg = (int *) malloc(vinfo.n_cat * sizeof(int));
    if (nextMsg == NULL) {
        #if DEBUG_MODE
        printf("Server: no memory.\n");
        #endif
        return 1;
    }

    // creating time table (columns: [Release Time, Deadline, Increment])
    ttable = (clock_t**) malloc(3 * sizeof(clock_t *));
    if (ttable == NULL) {
        #if DEBUG_MODE
        printf("Server: no memory for the time table.\n");
        #endif
        return 1;
    }

    for (i = 0; i < 3; i++) {
        ttable[i] = (clock_t*) malloc(vinfo.n_cat * sizeof(clock_t));
        if (ttable[i] == NULL) {
            #if DEBUG_MODE
            printf("Server: no memory for the time table.\n");
            #endif
            return 1;
        }
    }
    #if DEBUG_MODE
    printf("Server: time table created.\n");
    #endif

    // filling the increments (they are fixed for a video)
    // for (i = 0; i < vinfo.n_cat; i++) {
    //     ttable[2][i] = (HYPER_PERIOD * CLOCKS_PER_SEC) / vinfo.cat[i].n_msgs;
    // }

    #if DEBUG_MODE
    printf("Server: time table just got filled.\n");
    #endif

    // first sequence number must be random to avoid problems
    flowInfo.seq_num = rand();
    #if DEBUG_MODE
    printf("Server: first sequence number is %u\n", flowInfo.seq_num);
    #endif
    // send the first sequence number (and assure that the client has received 
    // it)
    int_to_4chars(flowInfo.seq_num, buffer);
    i = 0;
    while (i < MAX_TRY) {
        if (sendto(client->ctrl_sockt, buffer, 4, 0, 
            (struct sockaddr *) &client->caddr, 
            sizeof(client->caddr)) < 0) {
            #if DEBUG_MODE
            printf("Server: failed to send first seqNum to client.\n");
            #endif
            // fclose(fp);
            return 1;
        }
        if ((rcvd = recvfrom(client->ctrl_sockt, buffer, BUFFER_LEN, 0, 
             (struct sockaddr *) &client->caddr, &addrLen)) > -1) {
            // answer must be the first sequence number
            if (chars_to_int(buffer) == flowInfo.seq_num) {
                break;
            }
        }
        #if DEBUG_MODE
        printf("Server: failed to receive data from new client.\n");
        #endif
        i++;
    }
    if (i == MAX_TRY) {
        #if DEBUG_MODE
        printf("Server: failed to initiate sending routine.\n");
        #endif
        return 1;
    }
    printf("Streaming will start now.\n");
    return 0;
    // sending loop
    while (loag_segment(fp, segment) != 1) {
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
                    if (nextMsg[i] < segment->cats[i].n_msgs) {
                        ttable[0][i] = ttable[1][i] + 1;
                        ttable[1][i] = ttable[1][i] + ttable[2][i];
                    } else {
                        ttable[0][i] = segDeadline;
                        ttable[1][i] = segDeadline;
                    }
                } else if (currTime > ttable[0][i]) {
                    // can send this message
                    msg = segment->cats[i].msgs[nextMsg[i]];
                    // MUST SEND THE MESSAGE
                    send_video_msg(client, &msg, &flowInfo, buffer);
                    nextMsg[i]++;
                    if (nextMsg[i] < segment->cats[i].n_msgs) {
                        ttable[0][i] = ttable[1][i] + 1;
                        ttable[1][i] = ttable[1][i] + ttable[2][i];
                    } else {
                        ttable[0][i] = segDeadline;
                        ttable[1][i] = segDeadline;
                    }
                }
            }
        }

        // try to receive feedback from client
        if ((rcvd = recvfrom(client->ctrl_sockt, buffer, BUFFER_LEN, 0, 
             (struct sockaddr *) &client->caddr, &addrLen)) == -1) {
            #if DEBUG_MODE
            printf("Server: failed to receive feedback from new client.\n");
            #endif

        } else {
            // handle feedback

        }
        
    }
/**/
    return 0;
}

int send_video_list(cinfo_t* client) {
    int i, msgSize, vid, numVideos;
    char buffer[BUFFER_LEN];
    FILE *fp;

    fp = fopen(VIDEO_LIST_PATH, "r");
    if (fp == NULL) {
        #if DEBUG_MODE
        printf("Server: video list file not found.\n");
        #endif
        return 1;
    }

    fgets(buffer, BUFFER_LEN, fp);
    sscanf(buffer, "%d", &numVideos);
    int_to_4chars(numVideos, buffer);
    if (sendto(client->data_sockt, buffer, 4, 0, 
        (struct sockaddr *) &client->caddr, sizeof(client->caddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to send data to client.\n");
        #endif
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
        if (sendto(client->ctrl_sockt, buffer, msgSize, 0, 
            (struct sockaddr *) &client->caddr, 
            sizeof(client->caddr)) < 0) {
            #if DEBUG_MODE
            printf("Server: failed to send data to client.\n");
            #endif
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/**
 * 
 **/
int send_video_msg(cinfo_t* c, message_t* msg, flow_t* info, char* buffer) {
    mheader_t hdr;

    hdr.seq_num = info->seq_num++;

    memcpy(buffer, &hdr, sizeof(mheader_t));
    memcpy(buffer + sizeof(mheader_t), msg, sizeof(message_t));

    if (sendto(c->data_sockt, buffer, 4, 0, 
        (struct sockaddr *) &c->caddr, sizeof(c->caddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to send data to client.\n");
        #endif
        return 1;
    }
}