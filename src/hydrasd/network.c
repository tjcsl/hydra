#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network.h"

int gethydrasocket(const char *node, const char *service, int flags) {
    int status;
    int sock;
    struct addrinfo hints;
    struct addrinfo *info, *curr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = flags;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo(node, service, &hints, &info);
    if(status < 0) {
        return -1;
    }
    puts("IP Family Test:");
    for(curr = info; curr != NULL; curr = curr->ai_next) {
        if(curr->ai_family == AF_INET6) {
            puts("IPV6!");
        } else {
            puts("IPV4!");
        }
    }
    for(curr = info; curr != NULL; curr = curr->ai_next) {
        sock = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
        if(sock < 0) {
            continue;
        }
        status = connect(sock, curr->ai_addr, curr->ai_addrlen);
        if(status < 0) {
            close(sock);
            continue;
        }
        break;
    }
    if(curr == NULL) {
        freeaddrinfo(info);
        return -2;
    }
    return sock;
}
