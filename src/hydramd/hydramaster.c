#include "hydramaster.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <netdb.h>

static int listen4,listen6;

void init_sockets() {
    struct addrinfo* info;
    struct addrinfo hints;
    int i;
    int bound4, bound6;
    listen4 = socket(AF_INET , SOCK_STREAM, 0);
    if (listen4 < 0) {hydra_exit_error("Couldn't create IPv4 socket");}
    listen6 = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen6 < 0) {
        hydra_exit_error("Couldn't create IPv6 socket");
        close(listen4);
    }

    memset((void*)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    i = getaddrinfo(NULL, "51432", &hints, &info);
    if (i != 0) {
        hydra_exit_error(gai_strerror(i));
    }

    bound4 = bound6 = 0;

    while (!bound4 && !bound6 && info) {
        if (!bound4 && info->ai_family == AF_INET) {
            if (bind(listen4, info->ai_addr, info->ai_addrlen) == 0) {
                bound4 = 1;
            }
        }
        if (!bound6 && info->ai_family == AF_INET6) {
            if (bind(listen6, info->ai_addr, info->ai_addrlen) == 0) {
                bound6 = 1;
            }
        }
        syslog(LOG_DEBUG, "Tried family %d", info->ai_family);
    }
    
    syslog(LOG_DEBUG, "Bind status: 6:%d 4:%d", bound6, bound4);

    if (!bound4 && !bound6) {
        close(listen4);
        close(listen6);
        hydra_exit_error("Failed to bind socket, exiting");
    }

    if (!bound4) {
        syslog(LOG_INFO, "Unable to bind IPv4 address");
    }
    if (!bound6) {
        syslog(LOG_INFO, "Unable to bind IPv6 address");
    }
}

void hydra_listen() {
    init_sockets();

    for (;;) {sleep(1);}
}

void hydra_exit_error(const char* err) {
    syslog(LOG_CRIT, "%s", err);
    exit(1);
}
