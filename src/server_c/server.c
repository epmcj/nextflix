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
    sockt =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket == 0) { 
        #if DEBUG_MODE
        printf("Server: failed to create socket.\n"); 
        #endif
        exit(EXIT_FAILURE); 
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port        = htons(port);

    if (bind(sockt, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to bind client socket.\n"); 
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

    // main loop for client requisitions
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
            if (send_video(client, videoID) == 1) {
                #if DEBUG_MODE
                printf("Server: failed to send video %d for client %d.\n", 
                       videoID, client->id);
                #endif
            }

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
    char buffer[SEND_BUFFER_LEN];
    int i, addrLen, rcvd, msize, nframes, *nextMsg;
    int msgsCat[] = MSGS_PER_CAT;
    uint32_t seqNum;
    metadata_t** vinfo;
    segment_t segment;
    message_t *msg;
    mheader_t phdr, *rhdr;
    flow_t flowInfo;
    feedback_t *fb;
    clock_t currTime, diffTime, segDeadline, hperiod;
    ttable_entry_t *ttable;
    struct timeval tv;
    FILE **fps;
    uint32_t ncats, hpsentMsgs, tsentMsgs, lostMsgs, missedDl, msgsPerHP;

    ncats   = NUM_CATS;
    nframes = NUM_FRAMES;

    #if DEBUG_MODE
    printf("Server: starting to send a video.\n");
    #endif

    // initialization of variables for send rate adjustment
    hperiod    = HYPER_PERIOD;
    lostMsgs   = 0;
    tsentMsgs  = 0;
    hpsentMsgs = 0;
    missedDl   = 0;

    // first uses the the buffer to store the file path base.
    buffer[0] = 0;
    get_video_path(videoID, buffer);
    if (buffer[0] == 0) {
        #if DEBUG_MODE
        printf("Server: video %d could not be found.\n", videoID);
        #endif
        return 1;
    }

    // getting references for video files (one per category)
    fps = (FILE **) malloc(ncats * sizeof(FILE *));
    if (fps == NULL) {
        #if DEBUG_MODE
        printf("Server: insufficient memory for fp array.\n");
        #endif
        return 1;
    }

    for (i = 0; i < ncats; i++) {
        sprintf(&buffer[SEND_BUFFER_LEN/2], buffer, i);
        fps[i] = fopen(&buffer[SEND_BUFFER_LEN/2], "r");
        if (fps[i] == NULL) {
            #if DEBUG_MODE
            printf("Server: %s not found.\n", &buffer[SEND_BUFFER_LEN/2]);
            #endif
            return 1;
        }
    }
    
    vinfo = (metadata_t **) malloc(ncats * sizeof(metadata_t *));
	if (vinfo == NULL) {
        #if DEBUG_MODE
        printf("Server: insufficient memory for vinfo structure.\n");
        #endif
		return 1;
	}


    #if DEBUG_MODE
    printf("video has %d categories with ", ncats);
    msgsPerHP = 0;
    for (i = 0; i < ncats; i++) {
        msgsPerHP += msgsCat[i];
        // printf("%d ", vinfo.cat[i].n_msgs);
    }
    printf("messages each.\n");
    #endif

    // removing control socket timeout for feedback reception.
    tv.tv_sec  = 0;
    tv.tv_usec = 30000;
    if (setsockopt(client->ctrl_sockt, SOL_SOCKET, SO_RCVTIMEO, &tv, 
        sizeof(tv)) < 0) {
        #if DEBUG_MODE
        printf("Server: could not remove control socket timeout.\n");
        #endif
        return 1;
    }

    nextMsg = (int *) malloc(ncats * sizeof(int));
    if (nextMsg == NULL) {
        #if DEBUG_MODE
        printf("Server: no memory.\n");
        #endif
        return 1;
    }

    // creating time table (columns: [Release Time, Deadline, Increment])
    ttable = (ttable_entry_t*) malloc(ncats * sizeof(ttable_entry_t));
    if (ttable == NULL) {
        #if DEBUG_MODE
        printf("Server: no memory for the time table.\n");
        #endif
        return 1;
    }

    #if DEBUG_MODE
    printf("Server: time table created.\n");
    #endif

    // filling the increments (they are fixed for a video)
    for (i = 0; i < ncats; i++) {
        ttable[i].inc = (hperiod * CLOCKS_PER_SEC) / msgsCat[i];
    }

    #if DEBUG_MODE
    printf("Server: time table just got filled.\n");
    #endif

    // first sequence number must be random to avoid problems
    flowInfo.seq_num = rand();
    #if DEBUG_MODE
    printf("Server: first sequence number is %u\n", flowInfo.seq_num);
    #endif

    // send the first sequence number + the number of categories (and assure 
    // that the client has received it)
    phdr.type    = INIT_TYPE;
    phdr.seq_num = flowInfo.seq_num;
    memcpy(&buffer[SEND_BUFFER_LEN/2], &ncats, sizeof(uint32_t));
    msize = create_msg(&phdr, &buffer[SEND_BUFFER_LEN/2], sizeof(uint32_t), 
                       buffer);

    i = 0;
    while (i < MAX_TRY) {
        if (sendto(client->ctrl_sockt, buffer, msize, 0, 
            (struct sockaddr *) &client->caddr, 
            sizeof(client->caddr)) < 0) {
            #if DEBUG_MODE
            printf("Server: failed to send first seqNum to client.\n");
            #endif
            // fclose(fp);
            return 1;
        }
        if ((rcvd = recvfrom(client->ctrl_sockt, buffer, SEND_BUFFER_LEN, 0, 
             (struct sockaddr *) &client->caddr, &addrLen)) > -1) {
            // answer must be the first sequence number
            rhdr = read_header(buffer);
            if (rhdr->seq_num == flowInfo.seq_num) {
                break;
            } else {
                #if DEBUG_MODE
                printf("Server: received different seqNum (r:%d, e:%d).\n", 
                       rhdr->seq_num, flowInfo.seq_num);
                #endif
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

    // getting and sending video metadata
    if (create_and_load_metadata(fps, vinfo, ncats, msgsCat, nframes) == 1) {
        #if DEBUG_MODE
        printf("Server: failed to create and load metadata.\n");
        #endif
        return 1;
    }

    for (i = 0; i < ncats; i++) {
        printf("%d: %dx%d - elm[0]=%d, fnum[0]=%d\n", vinfo[i]->cat_id, 
                                                    vinfo[i]->frame_height, 
                                                    vinfo[i]->frame_width,
                                                    vinfo[i]->nElements[0],
                                                    vinfo[i]->frameNums[0]);
    }

    for (i = 0; i < ncats; i++) {
        send_metadata_msg(client, vinfo[i], msgsCat[i] * nframes, buffer);
    }
    
    // filing segment info
    printf("Filing segment\n");
    segment.n_cat  = ncats;
    segment.segNum = 0;
    segment.sets   = (msg_set_t**) malloc(ncats * sizeof(msg_set_t*));
    if (segment.sets == NULL) {
        #if DEBUG_MODE
        printf("Server: no memory for segment sets.\n");
        #endif
        return 1;
    }
    for (i = 0; i < ncats; i++) {
        segment.sets[i] = create_message_set(vinfo[i], msgsCat[i]);
    }
    printf("done\n");
    // sending loop
    printf("Streaming will start now.\n");
    while (load_next_segment(fps, vinfo, msgsCat, &segment) != 1) {
        printf("New segment loaded (with %d categories).\n", segment.n_cat); // TODO: REMOVER
        // time table round initialization
        for (i = 0; i < ncats; i++) {
    printf("%d: %d msgs \n", i, segment.sets[i]->n_msgs);
            nextMsg[i]   = 0;
            ttable[i].rt = clock();
            ttable[i].dl = ttable[i].rt + ttable[i].inc;
        }

        segDeadline = clock() + (hperiod * CLOCKS_PER_SEC);
        while((currTime = clock()) < segDeadline) {
            for (i = 0; i < ncats; i++) {
                if (currTime > ttable[i].dl) {
                    // missed the deadline
                    #if DEBUG_MODE
                    printf("Server: missed a deadline.\n");
                    #endif
                    nextMsg[i]++;
                    if (nextMsg[i] < segment.sets[i]->n_msgs) {
                        ttable[i].rt = ttable[i].dl + 1;
                        ttable[i].dl = ttable[i].dl + ttable[i].inc;
                    } else {
                        ttable[i].rt = segDeadline;
                        ttable[i].dl = segDeadline;
                    }
                    break;

                } else if (currTime > ttable[i].rt) {
                    // can send this message
                    msg = &segment.sets[i]->msgs[nextMsg[i]];
printf("Will send message at %d of index %d.\n", nextMsg[i], segment.sets[i]->msgs[nextMsg[i]].index);
                    if (send_video_msg(client, msg, &flowInfo, buffer) == 0) {
                        hpsentMsgs += 1;
printf("msg sent.\n");
                    }
                    nextMsg[i]++;
                    if (nextMsg[i] < segment.sets[i]->n_msgs) {
                        ttable[i].rt = ttable[i].dl + 1;
                        ttable[i].dl = ttable[i].dl + ttable[i].inc;
                    } else {
                        ttable[i].rt = segDeadline;
                        ttable[i].dl = segDeadline;
                    }
                    break;
                }
            }
        }

        // hiperperiod finalization routine ---
        currTime = clock();
        diffTime = segDeadline - currTime;
        // update global statistics
        tsentMsgs += hpsentMsgs;
        missedDl  += (msgsPerHP - hpsentMsgs);
        // check if it is possible to reduce the hyperperiod
        if (diffTime > (hperiod*HP_DECREASE)) {
            hperiod -= (hperiod*HP_DECREASE);
            #if DEBUG_MODE
            printf("Server: reducing hyperperiod (%ld).\n", hperiod*HP_DECREASE);
            #endif

        } else if (hpsentMsgs < msgsPerHP) {
            // server was not able to sent all messages
            hperiod += (hperiod*HP_INCREASE);
            #if DEBUG_MODE
            printf("Server: increasing hyperperiod (%ld).\n", 
                   hperiod*HP_INCREASE);
            #endif
        }

        hpsentMsgs = 0;

        // try to receive feedback from client
        if ((rcvd = recvfrom(client->ctrl_sockt, buffer, SEND_BUFFER_LEN, 0, 
             (struct sockaddr *) &client->caddr, &addrLen)) == -1) {
            #if DEBUG_MODE
            printf("Server: feedback msg was not received.\n");
            #endif

        } else {
            // handle feedback
            #if DEBUG_MODE
            printf("Server: received a feedback msg.\n");
            #endif
            
            fb = (feedback_t *)(buffer + sizeof(mheader_t));
            lostMsgs += fb->lostMsgs;
            // REDUZIR O NUMERO DE CATEGORIAS SE A PERDA FOR GRANDE?

            #if DEBUG_MODE
            printf("Server: client lost %d messages.\n", fb->lostMsgs);
            #endif
        }
    }

    #if DEBUG_MODE
    printf("Server: video was sent.\n");
    #endif

    // finishing routine
    phdr.type    = FIN_TYPE;
    phdr.seq_num = flowInfo.seq_num;
    msize = create_msg(&phdr, NULL, 0, buffer);
    
    if (sendto(client->ctrl_sockt, buffer, msize, 0, 
        (struct sockaddr *) &client->caddr, 
        sizeof(client->caddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to send fin msg to client.\n");
        #endif
        // fclose(fp);
        return 1;
    }

    printf("Server sent %d messages, client lost %d messages.\n", tsentMsgs, 
           lostMsgs);

    // reseting control socket timeout
    tv.tv_sec  = TIMEOUT_S;
    tv.tv_usec = TIMEOUT_US;
    if (setsockopt(client->ctrl_sockt, SOL_SOCKET, SO_RCVTIMEO, &tv, 
        sizeof(tv)) < 0) {
        #if DEBUG_MODE
        printf("Server: could not reset control socket timeout.\n");
        #endif
        return 1;
    }

    // closing files
    for (i = 0; i < ncats; i++) {
        fclose(fps[i]);
    }

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
    long int msize;

    // header
    hdr.type    = DATA_TYPE;
    hdr.seq_num = info->seq_num++;

    // msize = create_msg(&hdr, (void *) msg->data, (sizeof(float) * msg->size), 
    //                    buffer);

    msize = 0;
    msize += write_header(&hdr, buffer);
printf("header\n");
printf("<<%ld size\n", msg->size*sizeof(float));
    // copying data
    memcpy(buffer + msize, &msg->size, sizeof(uint32_t));
printf("size\n");
printf("%ld size\n", msize);
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, &msg->categoryId, sizeof(uint32_t));
printf("cid\n");
printf("%ld size\n", msize);
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, &msg->index, sizeof(uint32_t));
printf("index\n");
printf("%ld size\n", msize);
    msize += sizeof(uint32_t);
printf("%ld size\n", msg->size*sizeof(float) + msize);
    memcpy(buffer + msize, msg->data, msg->size*sizeof(float));
printf("data\n");
    msize += msg->size*sizeof(float);

    if (sendto(c->data_sockt, buffer, msize, 0, 
        (struct sockaddr *) &c->caddr, sizeof(c->caddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to send data to client.\n");
        #endif
        return 1;
    }
    printf("Server: message sent.\n");
    return 0;
}

/**
 * 
 **/
int send_metadata_msg(cinfo_t* c, metadata_t* catMetadata, int fieldSize,
                      char* buffer) {
    mheader_t hdr;
    int msize;

    // header
    hdr.type    = CTRL_TYPE;
    hdr.seq_num = 0;

    msize = 0;
    msize += write_header(&hdr, buffer);

    // copying metadata (too bad)
    memcpy(buffer + msize, &catMetadata->cat_id, sizeof(uint32_t));
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, &catMetadata->frame_height, sizeof(uint32_t));
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, &catMetadata->frame_width, sizeof(uint32_t));
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, &catMetadata->nChannels, sizeof(uint32_t));
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, &catMetadata->nObjects, sizeof(uint32_t));
    msize += sizeof(uint32_t);
    memcpy(buffer + msize, catMetadata->nElements, fieldSize*sizeof(uint32_t));
    msize += fieldSize*sizeof(uint32_t);
    memcpy(buffer + msize, catMetadata->frameNums, fieldSize*sizeof(uint32_t));
    msize += fieldSize*sizeof(uint32_t);   

    if (sendto(c->data_sockt, buffer, msize, 0, 
        (struct sockaddr *) &c->caddr, sizeof(c->caddr)) < 0) {
        #if DEBUG_MODE
        printf("Server: failed to send data to client.\n");
        #endif
        return 1;
    }
    printf("Server: message sent (size=%d).\n", msize);
    
    return 0;
}