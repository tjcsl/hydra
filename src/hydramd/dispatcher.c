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
    syslog(LOG_INFO, "Dispatcher shut down")
}

uint32_t hydra_dispatcher_get_jobid() {
    uint32_t ret;
   
    uint32_t *dat = lock_shmem();
    if ((long)dat == -1) {return -1;}
    ret = *dat;
    *dat = ret + 1;

    if (ulock_shmem(dat) == -1) {return -1;}

    return ret;
}

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
