#include "shmem.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int hydra_shmem_create(const char* lock, const char* mem, size_t size, int *out_shmem, int *out_semid) {
    key_t mem_key = ftok(mem, 0);
    key_t sem_key = ftok(lock, 0);

    *out_shmem = shmget(mem_key, size, 0644 | IPC_CREAT);
    if (out_shmem == (int *) -1) {return -1;}

    *out_semid = semget(sem_key, 1, 0644 | IPC_CREAT);
    if (out_semid == (int *) -1) {
        shmctl(*out_shmem, IPC_RMID, NULL);
        return -1;
    }

    return 0;
}

void *hydra_shmem_lock(int semid, int shmemid) {
    struct sembuf ops;
    ops.sem_op = 1;
    ops.sem_num = 0;
    ops.sem_flg = 0;
    if (semop(semid, &ops, 1) == -1) {return (void*)-1;}
    return shmat(shmemid, (void *)0, 0);
}

int hydra_shmem_ulck(int semid, void *shmem) {
    struct sembuf ops;
    ops.sem_op = 1;
    ops.sem_num = 0;
    ops.sem_flg = 0;
    if (shmdt(shmem) < 0) {return -1;}
    return semop(semid, &ops, 1);
}

int hydra_shmem_destroy(int semid, int shmemid) {
    struct sembuf ops;
    ops.sem_op = 0;
    ops.sem_num = 0;
    ops. sem_flg = 0;
    //We probably should have this but if things get kilt while locked we have a deadlock shutting down which is
    //a Bad Thing (TM)
//    if (semop(semid, &ops, 1) < 0) {return -1;}
    if (semctl(semid, 0, IPC_RMID) < 0) {return -1;}
    return shmctl(shmemid, IPC_RMID, NULL);
}
