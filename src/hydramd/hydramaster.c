#include "hydramaster.h"
#include "hydracommon.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <netdb.h>

void hydra_listen() {
    struct sockaddr addr;
    socklen_t addrlen;
    int i;
    int listen_sock = hydra_get_highsock_d(NULL, "51432", AI_PASSIVE);
    listen(listen_sock, 20);
    
    for (;;) {
        i = accept(listen_sock, &addr, &addrlen);
        syslog(LOG_INFO, "recieved connection");
    }
}
