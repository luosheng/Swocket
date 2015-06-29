//
//  Swocket.h
//  Pods
//
//  Created by Joakim Gyllström on 2015-06-19.
//
//

#ifndef Swocket_h
#define Swocket_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

/**
 Wrapper arround socket call.
 - Parameter port: port to connect to
 - Parameter host: host to connect to
 - Return: A socket descriptor or -1 on error
 */
int swocket_connect(const char * port, const char * host);

/**
 Wrapper arround socket and bind call.
 - Parameter port: port to connect to
 - Parameter host: host to connect to
 - Return: A socket descriptor or -1 on error
 */
int swocket_listen(const char * port, int backlog);

/**
 Waits for incomming connections on sockfd
 */
int swocket_accept(int sockfd);

/**
 Allocates an addrinfo struct. Caller is responsible for freeing it with freeaddrinfo
 */
struct addrinfo * swocket_addrinfo_udp(const char * host, const char * port);

/**
 Allocates a socket. -1 indicates failure
 */
int swocket_socket_udp(const struct addrinfo *servinfo);

/**
 Recieve data over a socket. -1 indicates failure
 */
ssize_t swocket_recieve_udp(int sockfd, void * data, size_t data_length);

/**
 Send data over a socket. -1 indicates failure
 */
ssize_t swocket_send_udp(int sockfd, const void * data, ssize_t length, const struct addrinfo * dest);

#endif /* Swocket_h */
