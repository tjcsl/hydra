#include "hydramaster.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <sys/signal.h>
#include <netdb.h>
#include <fcntl.h>

#include "hydracommon.h"
#include "hydranet.h"
#include "hydralog.h"
#include "hydrapacket.h"
#include "dispatcher.h"
#include "moniter.h"

void hydra_read_connection(int fd);

void handle_submit(int fd);
void hydra_read_signal(int);

void hydra_listen(const char* service) {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int fd, i;
    int listen_sock = hydra_get_highsock(NULL, service, AI_PASSIVE);

    if (listen_sock < 0) {
        hydra_log(HYDRA_LOG_WARN, "%d", listen_sock);
        hydra_exit_error("Couldn't get a socket to listen with");
    }
    if (listen(listen_sock, 20) < 0) {
        hydra_exit_error("No way to put our socket into listen mode");
    }

    hydra_log(HYDRA_LOG_INFO, "Begining listen");
    for (;;) {
        hydra_log(HYDRA_LOG_DEBUG, "Waiting for more connections");

        fd = accept(listen_sock, (struct sockaddr*) &addr, &addrlen);
        hydra_log(HYDRA_LOG_DEBUG, "accept returned");
        if (fd < 0) {
            hydra_log(HYDRA_LOG_WARN, "Error recieving connection: %d", errno);
        } else {
            hydra_log(HYDRA_LOG_DEBUG, "Got connection");
            i = fork();
            if (i == 0) {
                close(listen_sock);
                hydra_read_connection(fd);
                hydra_log(HYDRA_LOG_DEBUG, "Hydramd thread shutting down");
                exit(0);
            }
        }
    }
    hydra_dispatcher_destroy();
}

void hydra_read_connection(int fd) {
    int id;
    //We need to ignore signals because otherwise we clean up twice
    hydra_register_signal_handler(hydra_read_signal);
    for (;;) {
        id = hydra_get_next_packettype(fd);
        if (id < 0) {
            if (errno == 0) {
                hydra_log(HYDRA_LOG_DEBUG, "Lost connection, remote end probably hung up in a valid manner");
                return;
            }
            hydra_log(HYDRA_LOG_WARN, "Read failed with %s returned %d. Remote end probably hung up unexpectedly.", strerror(errno), id);
            return;
        }
        switch(id) {
            case HYDRA_PACKET_SUBMIT:
                handle_submit(fd);
                hydra_log(HYDRA_LOG_DEBUG, "Submit handled");
                break;
            default:
                hydra_log(HYDRA_LOG_DEBUG, "Packet type: %d", id);
        }
    }
    close(fd);
}

void hydra_read_signal(int sig) {
    hydra_log(HYDRA_LOG_DEBUG, "Hydra listner recieved signal %d", sig);
    switch(sig) {
        case SIGTERM:
        case SIGTSTP:
            exit(-1);
            break;
    }
}

void handle_submit(int fd) {
    int tmpfile;
    char tmpfilename[16];
    struct hydra_packet_submit sub;
    HydraPacket p;
    uint32_t jobid;
    //Get a temporary file
    strcpy(tmpfilename, "/tmp/XXXXXX");
    mktemp(tmpfilename);
    if (strlen(tmpfilename) == 0) {
        hydra_log(HYDRA_LOG_CRIT, "Error %s", strerror(errno));
        hydra_exit_error("Unable to open file descriptor");
    }
    hydra_log(HYDRA_LOG_DEBUG, "tempfile name %s", tmpfilename);
    tmpfile = open(tmpfilename, O_RDWR | O_CREAT);
    if (tmpfile < 0) {
        hydra_log(HYDRA_LOG_CRIT, "Error %s", strerror(errno));
        hydra_exit_error("Unable to open file descriptor");
    }
    //Read the packet
    p.id = HYDRA_PACKET_SUBMIT;
    p.submit.tar = tmpfile;
    hydra_read_packet(fd, &p);
    close(tmpfile);
    //unlink(tmpfilename);
    sub = p.submit;
    hydra_log(HYDRA_LOG_DEBUG, "Submit read: %s %d %d", sub.exe_name, sub.exe_name_length, sub.slots);
    free(sub.exe_name);
    jobid = hydra_dispatcher_get_jobid();
    hydra_log(HYDRA_LOG_DEBUG, "Replying with JOBID %d", jobid);
    hydra_dispatcher_set_job_active(jobid);
    hydra_log(HYDRA_LOG_DEBUG, "Job is active: %d", hydra_dispatcher_get_job_active(jobid));
    hydra_dispatcher_clr_job_active(jobid);
    hydra_log(HYDRA_LOG_DEBUG, "Job is active: %d", hydra_dispatcher_get_job_active(jobid));
    p.id = HYDRA_PACKET_JOBOK;
    p.jobok.jobid = jobid;
    hydra_write_packet(fd, &p);
}
