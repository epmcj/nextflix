#ifndef __SERVER_MOD_H__
#define __SERVER_MOD_H__

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 5
#define BUFFER_LEN 512

/**
 * 
 */
typedef struct {
    int csocket;    // Client socket
} pthread_arg;

/**
 * 
 **/
void server_listen(int port);

/**
 * 
 **/
void* handle_client(void* arg);

#endif