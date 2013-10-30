#ifndef _HYDRA_MON_H_
#define _HYDRA_MON_H_

#define HYDRA_MON_LOCK "./hydramon.lock"
#define HYDRA_MON_MEM "./hydramon.mem"

extern int hydramon_init(const char* whitelist_file_name);
extern int hydramon_destroy();

#endif
