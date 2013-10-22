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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hydraslave.h"
#include "hydracommon.h"
#include "system.h"

void display_help(const char *);
void sig_handler(int);

int main(int argc, char **argv) {
    int daemonize = 1;
    int c;
    int sock;
    char *prgname = argv[0];
    while((c = getopt(argc, argv, "Xh")) != -1) {
        switch(c) {
            case 'X':
                daemonize = 0;
                break;
            case 'h':
                display_help(prgname);
                exit(0);
                break;
            case '?':
                display_help(prgname);
                exit(1);
                break;
        }
    }
    
    if(daemonize) {
        hydra_daemonize("hydrasd", "/tmp", "hydrasd.lock", sig_handler);
    }
    sock = hydra_get_highsock_d("localhost", "1997", 0);
    while(1) {
        sleep(10);
    }
    return 0;
}

void display_help(const char *prgname) {
    printf("usage: %s [-hX]", prgname);
}

void sig_handler(int signal) {
    switch (signal) {
        case SIGTERM:
            syslog(LOG_INFO, "Recieved SIGTERM; Exiting");
            exit(0);
            break;
    }
}
