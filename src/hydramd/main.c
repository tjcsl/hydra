//Copyright 2013 Reed Koser,James Forcier,Michael Smith,Fox Wilson
//Handels daemonization and then hands control off to the stuff in
//hydramaster.c

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    char *run_location = "/tmp/hydramd";
    char *lockfile_name = "hydramd.lock";
    int c, index;
    int i;

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
    
    umask(027);
    i = mkdir(run_location, 0777);
    if (i == -1) {
        if (errno != EEXIST) {
            printf("Couldn't create running directory %s, error %d", run_location, errno);
        }
        //XXX:Handle run_location being a file
    }

    if (daemonize) {
        hydra_daemonize("hydramd", run_location, lockfile_name, handlesignal);
    }

    hydra_listen();

    return 0;
}
