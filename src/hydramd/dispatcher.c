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
    char *exename;
    void *exeval;
    uint32_t num_files;
    char **filenames;
    void **files;
};

static int semid;
static key_t semkey;
static int shmid;
static key_t shmemkey;

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
}

uint32_t hydra_dispatcher_get_jobid() {
    uint32_t ret;
    struct sembuf ops;
    ops.sem_op = 1;
    ops.sem_num = 0;
    ops.sem_flg = 0;
    if (semop(semid, &ops, 1) == -1) {return -1;}
    
    uint32_t *dat = shmat(shmid, (void *)0, 0);
    ret = *dat;
    *dat = ret + 1;
    shmdt(dat);

    ops.sem_op = -1;
    if (semop(semid, &ops, 1) == -1) {return -1;}

    return ret;
}
