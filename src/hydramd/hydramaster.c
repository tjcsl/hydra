#include "hydramaster.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <netdb.h>
#include "hydracommon.h"
#include "hydranet.h"
#include <stdio.h>
#include <string.h>

void hydra_read_connection(int fd);

void hydra_listen() {
    struct sockaddr addr;
    socklen_t addrlen;
    int fd, i;
    int listen_sock = hydra_get_highsock_d(NULL, "51432", AI_PASSIVE);
    listen(listen_sock, 20);
    int is_child = 0;
    
    for (!is_child) {
        fd = accept(listen_sock, &addr, &addrlen);
        syslog(LOG_INFO, "recieved connection");
        
        i = fork();
        if (i == 0){
            close(listen_sock);
            is_child = 1;
            hydra_read_connection(fd);
        }
    }
}

void hydra_read_connection(int fd) {
    char recv[256];
    memset (recv, 0, 256);
    for (;;) {
        if (read(fd, recv, 1) <= 0) {
            return;
        }
        syslog(LOG_INFO, "%s", recv);
    }
}
