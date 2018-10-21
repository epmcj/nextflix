#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_sup.h"
#include "tools.h"
#include "video.h"

#define MAX_CONNECTIONS 5
#define TIMEOUT_S 50
#define BUFFER_LEN 512
#define VIDEO_LIST_PATH "./video/list.txt"

#define LIST_CODE 7673 
#define PLAY_CODE 8076
#define EXIT_CODE 9999

/**
 * 
 */
typedef struct {
    int                   id;
    int                sockt;
    struct sockaddr_in caddr;
} cinfo_t;

typedef struct {
    int length;
    int  *list; 
} vlist_t;

/**
 * 
 **/
void server_listen(int port);

/**
 * 
 **/
void* handle_client(void* arg);

/**
 * 
 **/
int can_accept(int cid);

/**
 * 
 **/
int proccess_cmd(int actionID);

/**
 * 
 **/
int send_video(cinfo_t* client, int videoID);

/**
 * 
 */ 
int send_video_list(cinfo_t* client);

#endif