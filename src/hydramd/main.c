//Copyright 2013 Reed Koser,James Forcier,Michael Smith,Fox Wilson
//Handels daemonization and then hands control off to the stuff in
//hydramaster.c

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <signal.h>
#include <string.h>
#include "hydramaster.h"
#include "hydracommon.h"

//TODO:Make this stuff load from config file
#define PIDFILE "./hydramd.pid"

void handlesignal(int sig) {
    syslog(LOG_INFO, "Received signal %d", sig);
    switch(sig) {
        case SIGTERM:
            syslog(LOG_INFO, "Shutting down hydramd");
            exit(0);
            break;
    }
}

int main(int argc, char** argv) {
    int daemonize = 1;
    char *config_file = "./conf/hydramd.conf";
    char *run_location = "/tmp";
    char *lockfile_name = "hydramd.lock";
    int c, index;

    while ((c = getopt(argc, argv, "Xc:r:l:")) != -1) {
        switch (c) {
            case 'X':
                daemonize = 0;
                break;
            case 'c':
                config_file = optarg;
                break;
            case 'r':
                run_location = optarg;
                break;
            case 'l':
                lockfile_name = optarg;
                break;
            case '?':
                exit(1);
                break;
        }
    }

    if (daemonize) {
        hydra_daemonize("hydramd", run_location, lockfile_name, handlesignal);
    }

    hydra_listen();

    return 0;
}
