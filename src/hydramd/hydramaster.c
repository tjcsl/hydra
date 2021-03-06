#include "hydramaster.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>

#include "hydracommon.h"
#include "hydranet.h"
#include "hydralog.h"
#include "hydrapacket.h"
#include "dispatcher.h"

void hydra_read_connection(int fd);

void handle_submit(int fd);

void hydra_listen() {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int fd, i;
    int listen_sock = hydra_get_highsock(NULL, "51432", AI_PASSIVE);
    hydra_dispatcher_init();
    if (listen_sock < 0) {
        hydra_log(HYDRA_LOG_WARN, "%d", listen_sock);
        hydra_exit_error("Couldn't get a socket to listen with");
    }
    if (listen(listen_sock, 20) < 0) {
        hydra_exit_error("No way to put our socket into listen mode");
    }
    
    for (;;) {
        fd = accept(listen_sock, (struct sockaddr*) &addr, &addrlen);
        if (fd < 0) {
            hydra_log(HYDRA_LOG_WARN, "Error recieving connection: %d", errno);
            break;
        } else {
            hydra_log(HYDRA_LOG_INFO, "recieved connection");
            
            i = fork();
            if (i == 0){
                close(listen_sock);
                hydra_read_connection(fd);
                hydra_log(HYDRA_LOG_INFO, "Hydramd thread shutting down");
                return;
            }
        }
    }
    hydra_dispatcher_destroy();
}

void hydra_read_connection(int fd) {
    int pt;
    for (;;) {
        pt = hydra_get_next_packettype(fd);
        if (pt < 0) {
            if (errno == 0) {
                hydra_log(HYDRA_LOG_INFO, "Lost connection, remote end probably hung up in a valid manner");
                return;
            }
            hydra_log(HYDRA_LOG_WARN, "Read failed with %s returned %d. Remote end probably hung up unexpectedly.", strerror(errno), pt);
            return;
        }
        switch(pt) {
            case HYDRA_PACKET_SUBMIT:
                handle_submit(fd);
                break;
            default:
                hydra_log(HYDRA_LOG_INFO, "Packet type: %d", pt);
        }
    }
}

void handle_submit(int fd) {
    uint16_t slots;
    uint32_t jobid;
    int tmpfile, exenamelen;
    char tmpfilename[16];
    char* exename;
    strcpy(tmpfilename, "/tmp/XXXXXX");
    mktemp(tmpfilename);
    if (strlen(tmpfilename) == 0) {
        hydra_log(HYDRA_LOG_CRIT, "Error %s", strerror(errno));
        hydra_exit_error("Unable to open file descriptor");
    }
    hydra_log(HYDRA_LOG_INFO, "tempfile name %s", tmpfilename);
    tmpfile = open(tmpfilename, O_RDWR | O_CREAT);
    if (tmpfile < 0) {
        hydra_log(HYDRA_LOG_CRIT, "Error %s", strerror(errno));
        hydra_exit_error("Unable to open file descriptor");
    }
    if (hydra_read_SUBMIT(fd, (void**)&exename, &exenamelen, &slots, tmpfile) < 0) {
        hydra_log(HYDRA_LOG_CRIT, "Failed to read submit data %s", strerror(errno));
        hydra_exit_error("Aborting");
    }
    close(tmpfile);
    //unlink(tmpfilename);
    hydra_log(HYDRA_LOG_INFO, "Submit read: %s %d %d", exename, exenamelen, slots);
    jobid = hydra_dispatcher_get_jobid();
    hydra_log(HYDRA_LOG_INFO, "Replying with JOBID %d", jobid);
    hydra_dispatcher_set_job_active(jobid);
    hydra_log(HYDRA_LOG_INFO, "Job is active: %d", hydra_dispatcher_get_job_active(jobid));
    hydra_dispatcher_clr_job_active(jobid);
    hydra_log(HYDRA_LOG_INFO, "Job is active: %d", hydra_dispatcher_get_job_active(jobid));
    hydra_write_JOBOK(fd, jobid);
}
