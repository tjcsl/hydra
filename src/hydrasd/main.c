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
#include "hydracommon.h"

void sig_handler(int);

int main(int argc, char **argv) {
    hydra_daemonize("hydrasd", "/tmp", "hydrasd.lock", sig_handler);

    while(1) {
        sleep(1);
    }
}

void sig_handler(int signal) {
    switch (signal) {
        case SIGTERM:
            syslog(LOG_INFO, "Recieved SIGTERM; Exiting");
            exit(0);
            break;
    }
}
