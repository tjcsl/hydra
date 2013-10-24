#ifndef _HYDRACOMMON_H_
#define _HYDRACOMMON_H_

void hydra_daemonize(const char* progname, const char* running_directory, 
                     const char* lockfile, void (*sighandler)(int));

void hydra_exit_error(const char* err);

//Gets an IPv6 socket if we can, IPv4 otherwise.
int hydra_get_highsock(const char* node, const char* service, int flags);

#endif
