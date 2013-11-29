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
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "ini.h"

#include "hydralog.h"
#include "hydramaster.h"
#include "hydracommon.h"
#include "dispatcher.h"
#include "monitor.h"

void handlesignal(int);
int parse_config(void *, const char*, const char*, const char*);

struct config {
     char* whitelist_location;
     char* run_location;
     char* pid_file;
     char* port;
};

typedef struct config MasterConfig;

static MasterConfig config;

int main(int argc, char** argv) {
    int daemonize = 1;
    int c;
    char* config_file = NULL;
    char* run_location = NULL;
    char* lockfile_name = NULL;
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
                run_location = strdup(optarg);
                break;
            case 'l':
                lockfile_name = strdup(optarg);
                break;
            case '?':
                exit(1);
                break;
        }
    }
    
    if (config_file == NULL || strlen(config_file) == 0) {
        hydra_exit_error("You must pass -c");
    }

    config.pid_file = NULL;
    config.run_location = NULL;
    config.whitelist_location = NULL;

    if (ini_parse(config_file, parse_config, &config) < 0) {
        hydra_exit_error("Failed to parse config");
    }

    if (run_location != NULL && strlen(run_location) != 0) {
        free(config.run_location);
        config.run_location = run_location;
    }

    if (lockfile_name != NULL && strlen(lockfile_name) != 0) {
        free(config.pid_file);
        config.pid_file = lockfile_name;
    }

    umask(0027);
    i = mkdir(config.run_location, 0777);
    if (i == -1) {
        if (errno != EEXIST) {
            hydra_log(HYDRA_LOG_CRIT, "Couldn't create running directory %s, error %d", config.run_location, errno);
            exit(1);
        }
        //XXX:Handle run_location being a file
    }

    if (daemonize) {
        hydra_log_target(HYDRA_LOG_SYSLOG);
        hydra_daemonize("hydramd", config.run_location, config.pid_file, handlesignal);
    }

    hydra_dispatcher_init();
    hydra_mon_init(config.whitelist_location);
    hydra_listen(config.port);

    free(config.port);
    free(config.run_location);
    free(config.pid_file);
    free(config.whitelist_location);

    return 0;
}

int parse_config(void * user_data, const char* section, const char* name, const char* value) {
    MasterConfig *conf = (MasterConfig*)user_data;

    //Temporary macro def to save typing
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    //hydra_log(HYDRA_LOG_INFO, "parsing [%s] %s %s", section, name, value);
    if (MATCH("main", "whitelist_file")) {
        conf->whitelist_location = strdup(value);
    } else if (MATCH("main", "pid_file")) {
        conf->pid_file = strdup(value);
    } else if (MATCH("main", "run_loc")) {
        conf->run_location = strdup(value);
    } else if (MATCH("main", "port")) {
        conf->port = strdup(value);
    } else {
        return 1;
    }
#undef MATCH
    return 0;
}

void handlesignal(int sig) {
    int i;
    hydra_log(HYDRA_LOG_INFO, "Received signal %d", sig);
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            hydra_log(HYDRA_LOG_INFO, "Shutting down hydramd");
            hydra_dispatcher_destroy();
            hydra_mon_destroy();
            free(config.port);
            free(config.run_location);
            free(config.pid_file);
            free(config.whitelist_location);
            hydra_log(HYDRA_LOG_INFO, "Hydramd shut down, calling exit(0)");
            exit(0);
            break;
        case SIGCHLD:
            waitpid(-1, &i, WNOHANG);
            if (WEXITSTATUS(i) != 0 && WEXITSTATUS(i) != SIGTERM) {
                hydra_log(HYDRA_LOG_WARN, "WARNING: Child exited with status code %d", WEXITSTATUS(i));
            }
            break;
    }
}
