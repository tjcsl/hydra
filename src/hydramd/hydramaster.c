#include "hydramaster.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <netdb.h>

#define BOUND6 0x1
#define BOUND4 0x2

static int listen_sock;
static int bound;

void init_sockets() {
    struct addrinfo *ret, *info;
    struct addrinfo hints;
    int i;

    memset((void*)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    i = getaddrinfo(NULL, "51432", &hints, &ret);
    if (i != 0) {
        hydra_exit_error(gai_strerror(i));
    }

    bound = 0;
    info = ret;

    while (!(bound & BOUND6) && info) {
        syslog(LOG_DEBUG, "Trying family %d", info->ai_family);
        if (info->ai_family == AF_INET6) {
            listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
            if (listen_sock < 0) {hydra_exit_error("Couldn't create IPv6 socket");}
            if (bind(listen_sock, info->ai_addr, info->ai_addrlen) == 0) {
                bound |= BOUND6;
                break; //We got this
            } else {
                close(listen_sock);
                syslog(LOG_DEBUG, "Attempted IPv6 bind failed, errno %d", errno);
            }
        }
        info = info->ai_next;
    }

    freeaddrinfo(ret);

    memset((void*)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    i = getaddrinfo(NULL, "51432", &hints, &ret);
    if (i != 0) {
        hydra_exit_error(gai_strerror(i));
    }

    info = ret;

    while (!bound && info) {
        if (info->ai_family == AF_INET) {
            listen_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (listen_sock < 0) {hydra_exit_error("Couldn't create IPv4 socket");}
            if (bind(listen_sock, info->ai_addr, info->ai_addrlen) == 0) {
                bound |= BOUND4;
                break;
            } else {
                close(listen_sock);
                syslog(LOG_DEBUG, "Attempted IPv4 bind failed, errno %d", errno);
            }
        }
    }

    freeaddrinfo(ret);

    syslog(LOG_DEBUG, "Bind status:%d", bound);

    if (!bound) {
        hydra_exit_error("Failed to bind socket, exiting");
    }

    listen(listen_sock, 20);
}

void hydra_listen() {
    struct sockaddr addr;
    socklen_t addrlen;
    int ret;
    init_sockets();

    while (1) {
        ret = accept(listen_sock, &addr, &addrlen);
        if(ret == -1)
            hydra_exit_error("accept failed");
        else
            syslog(LOG_INFO, "recieved connection");
    }
}

void hydra_exit_error(const char* err) {
    syslog(LOG_CRIT, "%s", err);
    exit(1);
}
