#include "moniter.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "hydralog.h"
#include "shmem.h"
#include "hydralog.h"
#include "hydracommon.h"

static int status_shmem;
static int status_semid;

typedef struct moniter_data {
    unsigned int num_nodes;
} MoniterData;

typedef struct node_status {
    time_t last_update;
    unsigned int hostname_offset;
    uint16_t slots;
    uint32_t mb_ram;
    uint32_t mb_free;
    uint32_t load_avg;
} NodeStatus;

int hydra_mon_init(const char* whitelist_file_name) {
    size_t strings_size = 0;
    size_t data_size    = sizeof(MoniterData);
    void *status_data   = malloc(data_size);
    void *strings       = NULL;
    char host[128];
    int host_len;
    int offset;
    int i;
    FILE *whitelist;
    MoniterData *mon_data;

    ((MoniterData *)status_data)->num_nodes = 0;

    whitelist = fopen(whitelist_file_name, "r");

    while (fgets(host, 128, whitelist) != NULL) {
        host_len = strlen(host);
        if (host[host_len] != '\n' && host[0] != '#') {
            hydra_log(HYDRA_LOG_CRIT, "Hostname longer than 127 characters");
            hydra_exit_error("Exiting, whitelist has an overly long hostname");
        }
        //Append a node status and update the state of things
        mon_data = (MoniterData *) status_data;
        mon_data->num_nodes++;
        data_size += sizeof(NodeStatus);
        status_data = realloc(status_data, data_size);
        offset = sizeof(MoniterData) + sizeof(NodeStatus) * (mon_data->num_nodes - 1);
        memset(status_data + offset, 0, sizeof(NodeStatus));
        //And put the string in.
        ((NodeStatus*) (status_data + offset))->hostname_offset = strings_size;
        strings_size += host_len + 1;
        strings = realloc(strings, strings_size);
    }

    //Add data_size as an offset so things actually work 'n stuff
    for (i = 0; i < mon_data->num_nodes; ++i) {
        offset = sizeof(MoniterData) + sizeof(NodeStatus) * i;
        ((NodeStatus*)(status_data + offset))->hostname_offset += data_size;
    }

    hydra_shmem_create(HYDRA_MON_LOCK, HYDRA_MON_MEM, data_size + strings_size, &status_shmem, &status_semid);
    void *shmem = hydra_shmem_lock(status_semid, status_shmem);
    memcpy(shmem, status_data, data_size);
    memcpy(shmem + data_size, strings, strings_size);
    free(strings);
    free(status_data);
    fclose(whitelist);
    return 0;
}

int hydra_mon_destroy() {
    return hydra_shmem_destroy(status_semid, status_shmem);
}
