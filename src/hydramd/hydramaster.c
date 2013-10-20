#include "hydramaster.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <netdb.h>

static int listen4,listen6;
static int bound4, bound6;

void init_sockets() {
    struct addrinfo *ret, *info;
    struct addrinfo hints;
    int i;
    listen4 = socket(AF_INET , SOCK_STREAM, 0);
    if (listen4 < 0) {hydra_exit_error("Couldn't create IPv4 socket");}
    listen6 = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen6 < 0) {
        hydra_exit_error("Couldn't create IPv6 socket");
        close(listen4);
    }

    memset((void*)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    i = getaddrinfo(NULL, "51432", &hints, &ret);
    if (i != 0) {
        hydra_exit_error(gai_strerror(i));
    }

    bound4 = bound6 = 0;
    info = ret;

    while (!bound4 && !bound6 && info) {
        syslog(LOG_DEBUG, "Trying family %d", info->ai_family);
        if (!bound4 && info->ai_family == AF_INET) {
            if (bind(listen4, info->ai_addr, info->ai_addrlen) == 0) {
                bound4 = 1;
            } else {
                syslog(LOG_DEBUG, "Attempted IPv4 bind failed");
            }
        }
        if (!bound6 && info->ai_family == AF_INET6) {
            if (bind(listen6, info->ai_addr, info->ai_addrlen) == 0) {
                bound6 = 1;
            } else {
                syslog(LOG_DEBUG, "Attempted IPv6 bind failed");
            }
        }
        info = info->ai_next;
    }

    freeaddrinfo(ret);
    
    syslog(LOG_DEBUG, "Bind status: 6:%d 4:%d", bound6, bound4);

    if (!bound4 && !bound6) {
        close(listen4);
        close(listen6);
        hydra_exit_error("Failed to bind socket, exiting");
    }

    if (!bound4) {
        syslog(LOG_INFO, "Unable to bind IPv4 address, continuing");
    }
    if (!bound6) {
        syslog(LOG_INFO, "Unable to bind IPv6 address, continuing");
    }
}

void hydra_listen() {
    struct epoll_event ev;
    struct sockaddr addr;
    socklen_t addrlen;
    int epoll_dev;
    int i;
    init_sockets();

    epoll_dev = epoll_create(2); //paramater is ignored since 2.4, but must be not 0
    
    if (epoll_dev < 0) {
        hydra_exit_error("Epoll device creation failed");
    }

    if (bound4) {
        ev.events = EPOLLIN;
        ev.data.fd = listen4;
        i = epoll_ctl(epoll_dev, EPOLL_CTL_ADD, listen4, &ev);
        if (i < 0) {
            hydra_exit_error("Failed to EPOLL_ADD IPv4 listener");
        }
        listen(listen4, 20);
    }

    if (bound6) {
        ev.events = EPOLLIN;
        ev.data.fd = listen6;
        i = epoll_ctl(epoll_dev, EPOLL_CTL_ADD, listen6, &ev);
        if (i < 0) { hydra_exit_error("Failed to EPOLL_ADD IPv6 listener");
        }
        listen(listen6, 20);
    }
    
    for (;;) {
        i = epoll_wait(epoll_dev, &ev, 1, -1);
        if (i < 0) {
            syslog(LOG_WARNING, "epoll_wait returned a negative value");
        }
        i = accept(ev.data.fd, &addr, &addrlen);
        syslog(LOG_INFO, "recieved connection");
    }
}

void hydra_exit_error(const char* err) {
    syslog(LOG_CRIT, "%s", err);
    exit(1);
}
