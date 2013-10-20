//Copyright 2013 Reed Koser,James Forcier,Michael Smith,Fox Wilson

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>

//TODO:Make this stuff load from config file
#define PIDFILE "./hydramd.pid"

void handlesignal(int sig) {
    int i;
    switch(sig) {
        case SIGTERM:
            i = open("hydra.lock", O_RDONLY);
            if (i > 0) {
                //Unlock the PIDfile
                lockf(i, F_UNLCK, 0);
            }
            //Close logfiles
            exit(0);
            break;
    }
}

int main(int argc, const char** argv) {
    int i, lfp;
    char str[16];

    //We don't want any of the garbage from the environment we were spawned in
    for (i = getdtablesize(); i >= 0; --i) {
        close(i);
    }

    i = fork();
    if (i < 0) exit(1); /*fork failed*/
    if (i > 0) exit(0); /*original process exits.*/
    setsid(); /* We want to be in a new process group*/

    //Make functions that read/write to standard IO work
    i = open("/dev/null", O_RDWR);
    dup(i);
    dup(i);
    //Make sure we don't create files as root with rwx privs
    umask(027);
    //We want to run in a know location.
#ifndef DEBUG
    chdir("/hydra/");
#endif

    lfp = open("hydra.lock", O_RDWR | O_CREAT, 06400);
    if (lfp < 0) exit(1); /*something has already locked the lockfile*/
    if (lockf(lfp,F_TLOCK,0) < 0) exit(0); /*We can't lock */

    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str)); /*record PID to the lockfile*/
    /* and lock the file */
    lockf(lfp, F_LOCK, 0);

    //Register signal handlers
    signal(SIGTERM, handlesignal);

    openlog("hydramd", LOG_PID, LOG_DAEMON);
    syslog(LOG_EMERG, "Hydra Master daemon, fully started up, now accepting connections");

    for (;;) {
        sleep(10);
    }
}
