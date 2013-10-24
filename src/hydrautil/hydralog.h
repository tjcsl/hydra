//Provides utilites for when we need to switch between printf() and syslog()
#ifndef _HYDRA_LOG_H_
#define _HYDRA_LOG_H_

#define HYDRA_LOG_STDOUT 0
#define HYDRA_LOG_SYSLOG 1

#define HYDRA_LOG_DEBUG 0
#define HYDRA_LOG_INFO 1
#define HYDRA_LOG_WARN 2
#define HYDRA_LOG_CRIT 3

extern int hydra_log(int level, const char* fmt, ...);

extern void hydra_log_target(int target);
#endif
