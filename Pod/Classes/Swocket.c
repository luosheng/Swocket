//
//  Swocket.c
//  Pods
//
//  Created by Joakim Gyllström on 2015-06-19.
//
//

#include "Swocket.h"

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int swocket_connect(const char * port, const char * host) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        return -1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            continue; }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue; }
        break; }
    if (p == NULL) {
        return -1;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    
    freeaddrinfo(servinfo); // all done with this structure
    
    // Set socket to ignore SIGPIPE
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &yes,sizeof(yes));
    
    return sockfd;
}

int swocket_listen(const char * port, int backlog) {
    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage; // connector's address information
    struct sigaction sa;
    int yes=1;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        return -1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL)  {
        return -1;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        return -1;
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    
    return sockfd;
}

int swocket_accept(int sockfd) {
    int new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    
    inet_ntop(their_addr.ss_family,
              get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    
    return new_fd;
}

// Caller should do a freeaddrinfo when done with addrinfo pointer
struct addrinfo * swocket_addrinfo_udp(const char * host, const char * port) {
    struct addrinfo hints, *servinfo;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(host, port, &hints, &servinfo) != 0) {
        return NULL;
    }
    
    return servinfo;
}

int swocket_socket_udp(const struct addrinfo *servinfo) {
    int sockfd;
    const struct addrinfo *p;
    
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        return -1;
    }
    
    return sockfd;
}

ssize_t swocket_recieve_udp(int sockfd, void * data, size_t data_length) {
    struct sockaddr_storage their_addr;
    ssize_t numbytes;
    socklen_t addr_len;
    
    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, data, data_length-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        return -1;
    }
    
    return numbytes;
}

ssize_t swocket_send_udp(int sockfd, const void * data, ssize_t length, const struct addrinfo * dest) {
    return sendto(sockfd, data, length, 0, dest->ai_addr, dest->ai_addrlen);
}
