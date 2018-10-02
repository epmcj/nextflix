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

#define MAX_CONNECTIONS 5
#define TIMEOUT_S 50
#define BUFFER_LEN 512

/**
 * 
 */
typedef struct {
    int cid;    // Client id
    struct sockaddr_in caddr;
} cinfo_t;

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
int send_movie(int movieID);

#endif