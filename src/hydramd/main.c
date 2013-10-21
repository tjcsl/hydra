//Copyright 2013 Reed Koser,James Forcier,Michael Smith,Fox Wilson
//Handels daemonization and then hands control off to the stuff in
//hydramaster.c

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <signal.h>
#include <string.h>
#include "hydramaster.h"

//TODO:Make this stuff load from config file
#define PIDFILE "./hydramd.pid"

void handlesignal(int sig) {
    syslog(LOG_INFO, "Recieved signal %d", sig);
    switch(sig) {
        case SIGTERM:
            syslog(LOG_INFO, "Shutting down hydramd");
            exit(0);
            break;
    }
}

int main(int argc, const char** argv) {
    hydra_daemonize("hydramd", "/tmp", "hydramd.lock", handlesignal);

    hydra_listen();

    return 0;
}
