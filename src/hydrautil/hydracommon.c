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
int hydra_get_highsock_d(const char* host, const char* service, int flags) {
    struct addrinfo *ret, *info;
    struct addrinfo hints;
    int i;
    int listen_sock, bound;
    
    memset((void*)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = flags; 
    i = getaddrinfo(host, service, &hints, &ret);
    if (i != 0) {
        hydra_exit_error(gai_strerror(i));
    }

    bound = 0;
    info = ret;

    while (!(bound & BOUND6) && info) {
        syslog(LOG_DEBUG, "Trying family %d", info->ai_family);
        if (info->ai_family == AF_INET6) {
            listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
            if (listen_sock < 0) {hydra_exit_error("Couldn't create IPv6 socket");}
            if (bind(listen_sock, info->ai_addr, info->ai_addrlen) == 0) {
                bound |= BOUND6;
                break; //We got this
            } else {
                close(listen_sock);
                syslog(LOG_DEBUG, "Attempted IPv6 bind failed, errno %d", errno);
                if (errno = EADDRINUSE) {
                    hydra_exit_error("Couldn't bind IPv6 address: already in use");
                }
            }
        }
        info = info->ai_next;
    }

    freeaddrinfo(ret);

    memset((void*)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = flags; 
    i = getaddrinfo(host, service, &hints, &ret);
    if (i != 0) {
        hydra_exit_error(gai_strerror(i));
    }

    info = ret;

    while (!bound && info) {
        if (info->ai_family == AF_INET) {
            listen_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (listen_sock < 0) {hydra_exit_error("Couldn't create IPv4 socket");}
            if (bind(listen_sock, info->ai_addr, info->ai_addrlen) == 0) {
                bound |= BOUND4;
                break;
            } else {
                close(listen_sock);
                syslog(LOG_DEBUG, "Attempted IPv4 bind failed, errno %d", errno);
                if (errno == EADDRINUSE) {
                    hydra_exit_error("Couldn't bind IPv4 address: already in use");
                }
            }
        }
    }

    freeaddrinfo(ret);
    
    if (!bound) {
        hydra_exit_error("Failed to bind socket, exiting");
    }

    return listen_sock;
}
