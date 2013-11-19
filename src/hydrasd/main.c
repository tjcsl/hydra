// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson


#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "hydraslave.h"
#include "hydracommon.h"
#include "hydralog.h"
#include "system.h"

void display_help(const char *);
void sig_handler(int);

int main(int argc, char **argv) {
    int daemonize = 1;
    int c;
    char *prgname = argv[0];
    ConfigFile *config, *curr;
    ConfigEntry *entry;
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
        hydra_log_target(HYDRA_LOG_SYSLOG);
        hydra_daemonize("hydrasd", "/tmp", "hydrasd.lock", sig_handler);
    }
    config = parse_config("config.test");
    if (config == (void*)-1) {
        hydra_exit_error("Failed to parse config.");
    }
    hydra_log(HYDRA_LOG_INFO, "Config == NULL: %d\n", config == NULL);
    hydra_log(HYDRA_LOG_INFO,"Config Entries:\n");
    for(curr = config; curr != NULL; curr = curr->next) {
        if(curr->entry == NULL)
            continue;
        entry = curr->entry;
        hydra_log(HYDRA_LOG_INFO,"Config Entry: %s = %s, Type: %d\n", entry->key, entry->value, entry->value_type);
    }
    while(1) {
        sleep(10);
    }
    return 0;
}

void display_help(const char *prgname) {
    hydra_log(HYDRA_LOG_INFO,"usage: %s [-hX]", prgname);
}

void sig_handler(int signal) {
    switch (signal) {
        case SIGTERM:
            hydra_log(HYDRA_LOG_INFO, "Recieved SIGTERM; Exiting");
            exit(0);
            break;
    }
}
