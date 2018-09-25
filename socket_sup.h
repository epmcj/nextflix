#ifndef __SOCKET_SUP_H__
#define __SOCKET_SUP_H__

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Create and bind an UDP socket using its own address and a given port.
 * Returns a socket file descriptor in case of success or 0 otherwise.
 **/
int create_and_bind_socket(int port);

// /**
//  * Similar to recvfrom. Appends a '\0' at the end of the message.
//  **/
// int recv_msg(int sockfd, char* buffer, int len, struct sockaddr *src_addr, 
//               socklen_t *addrlen);

#endif