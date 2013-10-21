// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson

#include <sys/sysinfo.h>

#include <math.h>
#include <stdlib.h>

#include "hydracommon.h"
#include "system.h"

static double round_2(double);

unsigned long get_free_ram(void) {
    return 0.0;
}

unsigned long get_total_ram(void) {
    return 0.0;
}

double get_load_avg(void) {
    struct sysinfo *info = malloc(sizeof(struct sysinfo));
    unsigned long load;
    double load_ret;

    if(sysinfo(info) < 0) {
        free(info);
        return -1;
    }
    load = info->loads[0];
    load_ret = load / pow(2, 16);
    load_ret = round_2(load_ret);
    return load_ret;
}

static double round_2(double num) {
    return (double) floorf((float) num * 100 + 0.5) / 100;
}
