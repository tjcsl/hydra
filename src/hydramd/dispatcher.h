#ifndef _HYDRA_DISPATCHER_H_
#define _HYDRA_DISPATCHER_H_
/*****************************************************************************\
 *  The dispatcher is responsible for assigning JobIDs and kicking off remote*
 *  processies                                                               *
\*****************************************************************************/
//File for semaphores
#define HYDRA_JOBS_LOCK "jobs.lock"
#define HYDRA_JOBS_SHMEM "jobs.mem"

//for uint16_t

#include <stdint.h>

struct hydra_job;

extern void hydra_dispatcher_init();
extern void hydra_dispatcher_destroy();
extern uint32_t hydra_dispatcher_get_jobid();
#endif
