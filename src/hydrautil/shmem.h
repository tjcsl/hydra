#ifndef _HUTIL_SHMEM_H_
#define _HUTIL_SHMEM_H_

#include <stddef.h>

extern int   hydra_shmem_create(const char* lock, const char* mem, size_t size,
                                int *out_shmem, int *out_semid);
extern void *hydra_shmem_lock(int semid, int shmemid);
extern int   hydra_shmem_ulck(int semid, void* shmem);
extern int   hydra_shmem_destroy(int semid, int shmemid);

#endif
