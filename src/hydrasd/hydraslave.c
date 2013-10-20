// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson

#include <sys/syslog.h>

#include <stdlib.h>

#include "hydraslave.h"

void hydra_exit_error(const char *err_msg) {
    syslog(LOG_CRIT, "%s", err_msg);
    exit(1);
}
