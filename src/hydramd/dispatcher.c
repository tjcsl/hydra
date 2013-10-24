#include "dispatcher.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "hydracommon.h"

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
static key_t semkey;
static int shmid;
static key_t shmemkey;

static void *lock_shmem();
static int ulock_shmem(void *);
static int safe_create(const char*, mode_t);

void hydra_dispatcher_init() {
    safe_create(HYDRA_JOBS_LOCK, 0644);
    semkey = ftok(HYDRA_JOBS_LOCK, 0);
    semid = semget(semkey, 10, 0644 | IPC_CREAT);
    if (semid < 0) {
        hydra_exit_error("Couldn't allocate semaphore");
    }
    safe_create(HYDRA_JOBS_SHMEM, 0644);
    shmemkey = ftok(HYDRA_JOBS_SHMEM, 0);
    if (shmemkey < 0) {
        semctl(semid, 0, IPC_RMID);
        syslog(LOG_CRIT, "Badness %d", errno);
        hydra_exit_error("Couldn't get shmemkey");
    }
    shmid = shmget(shmemkey, 1024, 0644 | IPC_CREAT);
    if (shmid < 0) {
        semctl(semid, 0, IPC_RMID);
        syslog(LOG_CRIT, "No good %d", errno);
        hydra_exit_error("Couldn't allocate shared memory");
    }
    syslog(LOG_INFO, "Dispatcher started");
}

void hydra_dispatcher_destroy() {
    struct sembuf ops;
    ops.sem_op = 0;
    ops.sem_num = 0;
    ops.sem_flg = 0;
    //Wait for any remaining locks to be freed
    semop(semid, &ops, 1);
    //and delete
    semctl(semid, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
    syslog(LOG_INFO, "Dispatcher shut down");
}

uint32_t hydra_dispatcher_get_jobid() {
    uint32_t ret;
   
    struct hydra_shared_data *dat = lock_shmem();
    if ((long)dat == -1) {return -1;}
    ret = dat->next_jobid;
    dat->next_jobid = ret + 1;

    if (ulock_shmem(dat) == -1) {return -1;}

    return ret;
}

int hydra_dispatcher_set_job_active(uint32_t jid) {
    struct hydra_shared_data *dat = lock_shmem();
    int index;
    if ((long)dat == -1) {return -1;}

    index = (int)(jid % HYDRA_MAX_JOBS);
    dat->active_jobs[index / 32] |= 0x1 << (index % 32);

    if (ulock_shmem(dat) == -1) {return -1;}
    return 0;
}

int hydra_dispatcher_get_job_active(uint32_t jid) {
    struct hydra_shared_data *dat = lock_shmem();
    int index;
    int ret;
    if ((long)dat == -1) {return -1;}

    index = (int)(jid % HYDRA_MAX_JOBS);
    ret = dat->active_jobs[index / 32] & 0x1 << (index % 32);

    if (ulock_shmem(dat) == -1) {return -1;}
    return ret > 0;
}

int hydra_dispatcher_clr_job_active(uint32_t jid) {
    struct hydra_shared_data *dat = lock_shmem();
    uint32_t temp;
    int index;
    if ((long)dat == -1) {return -1;}

    index = (int)(jid % HYDRA_MAX_JOBS);
    temp = dat->active_jobs[index / 32];
    dat->active_jobs[index / 32] = temp & ~(0x1 << (index % 32));

    if (ulock_shmem(dat) == -1) {return -1;}
    return 0;
}

/*****************************************************************************\
|                                  UTIL FUNCTIONS                             |
\*****************************************************************************/
static void *lock_shmem() {
    struct sembuf ops;
    ops.sem_op = 1;
    ops.sem_num = 0;
    ops.sem_flg = 0;
    if (semop(semid, &ops, 1) == -1) {return (void*)-1;}
    return shmat(shmid, (void *)0, 0);
}

static int ulock_shmem(void* data) {
    struct sembuf ops;
    ops.sem_op = -1;
    ops.sem_num = 0;
    ops.sem_flg = 0;
    if (semop(semid, &ops, 1) == -1) {return -1;}
    return shmdt(data);
}

static int safe_create(const char* fname, mode_t mode) {
    int i = creat(fname, mode);
    if (i < 0) {
        if (errno != EEXIST) {
            syslog(LOG_CRIT, "Failed to create file %s", fname);
            hydra_exit_error("failed to start hydramd dispatcher");
        }
    }
    return i;
}
