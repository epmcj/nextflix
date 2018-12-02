#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tools.h"
#include "video.h"
#include "ttable.h"
#include "message.h"

#define MAX_CONNECTIONS 5
#define MAX_TRY         3
#define HYPER_PERIOD    4 // seconds
#define HP_DECREASE     10/100 // 10 %
#define HP_INCREASE     50/100 // 50 %
#define TIMEOUT_S       60
#define TIMEOUT_US      0
#define BUFFER_LEN      512
#define SEND_BUFFER_LEN 8192
#define VIDEO_LIST_PATH "video/list.txt"

#define LIST_CODE 7673 
#define PLAY_CODE 8076
#define EXIT_CODE 9999

#define DEBUG_MODE 1

/**
 * 
 */
typedef struct {
    int                   id;
    int           ctrl_sockt;
    int           data_sockt;
    struct sockaddr_in caddr;
} cinfo_t;

typedef struct {
    uint32_t seq_num;
} flow_t;

typedef struct {
    int length;
    int  *list; 
} vlist_t;

typedef struct {
    uint32_t lostMsgs;
} feedback_t;

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
 **/
int send_message(message_t msg, uint32_t seqNum);

/**
 * 
 */ 
int send_video_list(cinfo_t* client);

/**
 * 
 **/
int send_video_msg(cinfo_t* client, message_t* msg, flow_t* info, char* buffer);

/**
 * 
 **/
int send_metadata_msg(cinfo_t* client, metadata_t* catMetadata, int fieldSize,
                      char* buffer);

#endif