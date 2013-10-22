#include "hydracommon.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BOUND6 0x1
#define BOUND4 0x2

void hydra_daemonize(const char* progname, const char* running_directory 
                    ,const char* lockfile, void (*sighandler)(int)) {
    int i, lfp;
    char str[16];

    umask(0);
    for (i = getdtablesize(); i >= 0; --i) {
        close(i);
    }

    i = fork();
    if (i < 0) {
        hydra_exit_error("Fork failed");
    }
    if (i > 0) {
        exit(0);
    }

    setsid();
    
    openlog(progname, LOG_PID, LOG_DAEMON);

    i = open("/dev/null", O_RDWR);
    dup(i);
    dup(i);

    umask(027);
    if (chdir(running_directory)) {
        hydra_exit_error("Failed to change running directory");
    }
    
    lfp = open(lockfile, O_RDWR | O_CREAT, 0640);
    if (lfp < 0) {
        hydra_exit_error("Something has already locked the lockfile");
    }
    if (lockf(lfp,F_TLOCK,0) < 0) {
        hydra_exit_error("We failed to lock our lockfile");
    }

    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str));

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);

    syslog(LOG_INFO, "Daemon successfully daemonized");
}

void hydra_exit_error(const char* err) {
    syslog(LOG_CRIT, "%s", err);
    exit(1);
}

//TODO:Make less bad
int hydra_get_highsock_d(const char* node, const char* service, int flags) {
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
