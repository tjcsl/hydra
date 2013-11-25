#include "hydralog.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <string.h>
#include <stdlib.h>

static int target = HYDRA_LOG_STDOUT;

int hydra_log(int level, const char* fmt, ...) {
    int l, bufflen;
    FILE *o = stdout;
    char *fbuff;
    va_list args;
    va_start(args, fmt);
    switch (target) {
        case HYDRA_LOG_SYSLOG:
            switch (level) {
                case HYDRA_LOG_DEBUG: l = LOG_DEBUG; break;
                case HYDRA_LOG_INFO : l = LOG_INFO; break;
                case HYDRA_LOG_WARN : l = LOG_WARNING; break;
                case HYDRA_LOG_CRIT : l = LOG_CRIT; break;
                default: l = LOG_CRIT; break;
            }
            vsyslog(l, fmt, args);
            break;
        case HYDRA_LOG_STDOUT:
            if (level == HYDRA_LOG_CRIT) o = stderr;
            bufflen = strlen(fmt) + 2;
            fbuff = malloc(bufflen);
            strcpy(fbuff, fmt);
            fbuff[bufflen - 2] = '\n';
            fbuff[bufflen - 1] = '\0';
            vfprintf(o, fbuff, args);
            free(fbuff);
            break;
        default:
            fprintf(stderr, "Invalid log target %d", target);
            return -1;
    }
    va_end(args);
    return 0;
}

void hydra_log_target(int t) {
    target = t;
}
