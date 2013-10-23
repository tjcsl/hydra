#include "hydramaster.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <netdb.h>
#include "hydracommon.h"
#include "hydranet.h"
#include "hydrapacket.h"
#include "dispatcher.h"

void hydra_read_connection(int fd);

void hydra_listen() {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int fd, i;
    int listen_sock = hydra_get_highsock(NULL, "51432", AI_PASSIVE);
    hydra_dispatcher_init();
    if (listen_sock < 0) {
        syslog(LOG_WARNING, "%d", listen_sock);
        hydra_exit_error("Couldn't get a socket to listen with");
    }
    if (listen(listen_sock, 20) < 0) {
        hydra_exit_error("No way to put our socket into listen mode");
    }
    
    for (;;) {
        fd = accept(listen_sock, (struct sockaddr*) &addr, &addrlen);
        if (fd < 0) {
            syslog(LOG_WARNING, "Error recieving connection: %d", errno);
            break;
        } else {
            syslog(LOG_INFO, "recieved connection");
            
            i = fork();
            if (i == 0){
                close(listen_sock);
                hydra_read_connection(fd);
                syslog(LOG_INFO, "Hydramd thread shutting down");
                return;
            }
        }
    }
    hydra_dispatcher_destroy();
}

void hydra_read_connection(int fd) {
    int pt, exenamelen;
    uint16_t slots;
    uint32_t jobid;
    char* exename;
    for (;;) {
        pt = hydra_get_next_packettype(fd);
        if (pt < 0) {
            syslog(LOG_WARNING, "Read failed %d", errno);
            return;
        }
        switch(pt) {
            case HYDRA_PACKET_SUBMIT:
                hydra_read_SUBMIT(fd, (void**)&exename, &exenamelen, &slots);
                syslog(LOG_INFO, "Submit read: %s %d %d", exename, exenamelen, slots);
                jobid = hydra_dispatcher_get_jobid();
                syslog(LOG_INFO, "Replying with JOBID %d", jobid);
                hydra_write_JOBOK(fd, jobid);
                break;
            default:
                syslog(LOG_INFO, "Packet type: %d", pt);
        }
    }
}
