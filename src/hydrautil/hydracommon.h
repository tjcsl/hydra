#ifndef _HYDRACOMMON_H_
#define _HYDRACOMMON_H_

void hydra_daemonize(const char* progname, const char* running_directory, 
                     const char* lockfile, void (*sighandler)(int));

void hydra_exit_error(const char* err);

//Gets an IPv6 socket if we can, IPv4 otherwise.
//Logs to syslog, so should only be used in daemons
int hydra_get_highsock_d(const char* node, const char* service, int flags);

#endif
