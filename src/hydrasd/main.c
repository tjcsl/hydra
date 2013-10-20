// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson

// TODO:
// * Move sigterm_handler
// * Move daemonize to lib
// * Actually add some functionality!

#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "hydraslave.h"

void daemonize(char *);
void sigterm_handler(int);

int main(int argc, char **argv) {
    daemonize(SLAVE_NAME);
    signal(SIGTERM, sigterm_handler);
    while(1) {
        sleep(1);
    }
}

void daemonize(char *d_name) {
    pid_t fork_pid;
    int fd_table_size, lock_fd;
    
    umask(0);
    fork_pid = fork();
    if(fork_pid < 0) {
        exit(1);
    } else if(fork_pid > 0) {
        exit(0);
    }
    setsid();
    
    if(chdir("/tmp/") < 0) {
        exit(1);
    }
    for(fd_table_size = getdtablesize(); fd_table_size >= 0; fd_table_size--) {
        close(fd_table_size);
    }
    fd_table_size = open("/dev/null", O_RDWR);
    dup(fd_table_size);
    dup(fd_table_size);
    umask(027);

    openlog(d_name, LOG_PID, LOG_DAEMON);

    lock_fd = open("hydrasd.lock", O_RDWR | O_CREAT, 0640);
    if(lock_fd < 0) {
        hydra_exit_error("Lock file already in use");
    }
    if(lockf(lock_fd, F_TLOCK, 0) < 0) {
        hydra_exit_error("Lock file failed to lock");
    }
    
    syslog(LOG_INFO, "Daemonization successful");
}

void sigterm_handler(int signal) {
    syslog(LOG_INFO, "Recieved SIGTERM; Exiting");
    exit(0);
}
