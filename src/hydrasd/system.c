// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson

#include <sys/sysinfo.h>

#include <math.h>
#include <stdlib.h>

#include "hydracommon.h"
#include "system.h"

static double round_2(double);

// Returns Value in Megabytes
unsigned long get_free_ram(void) {
    struct sysinfo *info = malloc(sizeof(struct sysinfo));
    unsigned long free_ram;

    if(sysinfo(info) < 0) {
        free(info);
        return -1;
    }
    free_ram = info->freeram + info->bufferram;
    free(info);
    return free_ram >> 20;
}

// Returns value in Megabytes
unsigned long get_total_ram(void) {
    struct sysinfo *info = malloc(sizeof(struct sysinfo));
    unsigned long total_ram;

    if(sysinfo(info) < 0) {
        free(info);
        return -1;
    }
    total_ram = info->totalram;
    free(info);
    return total_ram >> 20;
}

// 1 min load average
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
    free(info);
    return load_ret;
}

static double round_2(double num) {
    return (double) floorf((float) num * 100 + 0.5) / 100;
}
