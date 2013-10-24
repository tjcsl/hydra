#ifndef _HYDRA_DISPATCHER_H_
#define _HYDRA_DISPATCHER_H_
/*****************************************************************************\
 *  The dispatcher is responsible for assigning JobIDs and kicking off remote*
 *  processies                                                               *
\*****************************************************************************/
//File for semaphores
#define HYDRA_JOBS_LOCK "jobs.lock"
#define HYDRA_JOBS_SHMEM "jobs.mem"

#define HYDRA_MAX_JOBS 32 * 8
//for uint16_t

#include <stdint.h>

struct hydra_job;

extern void hydra_dispatcher_init();
extern void hydra_dispatcher_destroy();
extern uint32_t hydra_dispatcher_get_jobid();
extern int hydra_dispatcher_set_job_active(uint32_t jid);
extern int hydra_dispatcher_get_job_active(uint32_t jid);
extern int hydra_dispatcher_clr_job_active(uint32_t jid);
#endif
