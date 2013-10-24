#include "hydralog.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/syslog.h>

static int target = HYDRA_LOG_STDOUT;

int hydra_log(int level, const char* fmt, ...) {
    int l;
    FILE *o = stdout;
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
            vfprintf(o, fmt, args);
            break;

    }
    va_end(args);
}

void hydra_log_target(int t) {
    target = t;
}
