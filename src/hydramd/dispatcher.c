#include "dispatcher.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "hydracommon.h"
#include "hydralog.h"
#include "shmem.h"

struct hydra_job {
    uint32_t id;
    char *tarfile;
    char **exeargs;
};

struct hydra_shared_data {
    uint32_t next_jobid;
    uint32_t active_jobs[8];//bitset
};

static int semid;
static int shmid;

static int safe_create(const char*, mode_t);

void hydra_dispatcher_init() {
    safe_create(HYDRA_JOBS_LOCK, 0644);
    safe_create(HYDRA_JOBS_SHMEM, 0644);
    hydra_shmem_create(HYDRA_JOBS_LOCK, HYDRA_JOBS_SHMEM, 1024, &shmid, &semid);
    if (shmid < 0 || semid < 0) {
        hydra_log(HYDRA_LOG_CRIT, "No good %d", errno);
        hydra_shmem_destroy(semid, shmid);
        hydra_log(HYDRA_LOG_CRIT, "No good %d", errno);
        hydra_exit_error("Couldn't allocate shared memory");
    }
    hydra_log(HYDRA_LOG_INFO, "Dispatcher started");
}

void hydra_dispatcher_destroy() {
    hydra_shmem_destroy(semid, shmid);
    hydra_log(HYDRA_LOG_INFO, "Dispatcher shut down");
}

uint32_t hydra_dispatcher_get_jobid() {
    uint32_t ret;
   
    struct hydra_shared_data *dat = hydra_shmem_lock(semid, shmid);
    if ((long)dat == -1) {return -1;}
    ret = dat->next_jobid;
    dat->next_jobid = ret + 1;

    if (hydra_shmem_ulck(semid, dat) == -1) {return -1;}

    return ret;
}

int hydra_dispatcher_set_job_active(uint32_t jid) {
    struct hydra_shared_data *dat = hydra_shmem_lock(semid, shmid);
    int index;
    if ((long)dat == -1) {return -1;}

    index = (int)(jid % HYDRA_MAX_JOBS);
    dat->active_jobs[index / 32] |= 0x1 << (index % 32);

    if (hydra_shmem_ulck(semid, dat) == -1) {return -1;}
    return 0;
}

int hydra_dispatcher_get_job_active(uint32_t jid) {
    struct hydra_shared_data *dat = hydra_shmem_lock(semid, shmid);
    int index;
    int ret;
    if ((long)dat == -1) {return -1;}

    index = (int)(jid % HYDRA_MAX_JOBS);
    ret = dat->active_jobs[index / 32] & 0x1 << (index % 32);

    if (hydra_shmem_ulck(semid, dat) == -1) {return -1;}
    return ret > 0;
}

int hydra_dispatcher_clr_job_active(uint32_t jid) {
    struct hydra_shared_data *dat = hydra_shmem_lock(semid, shmid);
    uint32_t temp;
    int index;
    if ((long)dat == -1) {return -1;}

    index = (int)(jid % HYDRA_MAX_JOBS);
    temp = dat->active_jobs[index / 32];
    dat->active_jobs[index / 32] = temp & ~(0x1 << (index % 32));

    if (hydra_shmem_ulck(semid, dat) == -1) {return -1;}
    return 0;
}

/*****************************************************************************\
|                                  UTIL FUNCTIONS                             |
\*****************************************************************************/
static int safe_create(const char* fname, mode_t mode) {
    int i = creat(fname, mode);
    if (i < 0) {
        if (errno != EEXIST) {
            hydra_log(HYDRA_LOG_CRIT, "Failed to create file %s", fname);
            hydra_exit_error("failed to start hydramd dispatcher");
        }
    }
    return i;
}
