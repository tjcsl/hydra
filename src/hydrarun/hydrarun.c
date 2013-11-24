#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#include "hydrapacket.h"
#include "hydralog.h"
#include "hydracommon.h"

void display_help(char* argv[]){
    hydra_log(HYDRA_LOG_INFO, "Usage: %s [-d <data> [-d <data ...]] -h <masterhostname> -s NUM -e <executable> [-- [args]]", argv[0]);
}

int main(int argc, char* argv[]){
    // Parse the arguments
    extern char *optarg;
    extern int optind;
    int datafiles_count, slotsset, execset, currarg, hmhostset;
    datafiles_count = slotsset = execset = currarg = hmhostset = 0;
    char** datafiles = malloc(sizeof(char*) * argc); // A safe number
    char* executable;
    char* slots;
    char** hmhost = malloc(sizeof(char*));
    HydraPacket p;
    while((currarg = getopt(argc, argv, "d:s:e:h:")) != -1) {
        switch(currarg) {
            case 'd':
                datafiles[datafiles_count++] = optarg;
                break;
            case 's':
                slots = optarg;
                slotsset = 1;
                break;
            case 'e':
                executable = optarg;
                execset = 1;
                break;
            case 'h':
                (*hmhost) = optarg;
                hmhostset = 1;
                break;
            case '?':
                display_help(argv);
                return 1;
        }
    }
    if(!slotsset || !execset || !hmhostset) {
        display_help(argv);
        return 1;
    }
    // Done with arg parsing, output arguments to user
    hydra_log(HYDRA_LOG_INFO, "Hydra: running \"%s\" with %s slots", executable, slots);
    int i;
    if(datafiles_count > 0){
        hydra_log(HYDRA_LOG_INFO, "Datafiles: ");
        for(i = 0; i < datafiles_count; i++){
            hydra_log(HYDRA_LOG_INFO, "%s ", datafiles[i]);
        }
        hydra_log(HYDRA_LOG_INFO, "");
    }
    hydra_log(HYDRA_LOG_INFO, "Command line: %s ", executable);
    for(i = optind; i < argc; i++){
        hydra_log(HYDRA_LOG_INFO, "%s ", argv[i]);
    }
    hydra_log(HYDRA_LOG_INFO, "");
    /**
     * Make network connection
     */
    int sd = hydra_get_highsock(*hmhost, "51432", 0);
    if(sd < 0){
        hydra_log(HYDRA_LOG_CRIT, "Couldn't open socket (errno %d) (sd = %d), exiting.", errno, sd);
        hydra_log(HYDRA_LOG_CRIT, "%s", gai_strerror(sd));
        return 2;
    }
    // Time for actual socket communication.
    //XXX: TESTING
    int magic = open("/bin/bash", O_RDONLY);
    if (magic < 0) {
        hydra_log(HYDRA_LOG_CRIT, "Couldn't open /bin/bash");
        return -1;
    }
    //XXX: END TESTING
    p.id = HYDRA_PACKET_SUBMIT;
    p.submit.exe_name = executable;
    p.submit.exe_name_length = strlen(executable) + 1;
    p.submit.slots = atoi(slots);
    p.submit.tar = magic;
    if (hydra_write_packet(sd, &p) != 0) {
        hydra_log(HYDRA_LOG_INFO, "Write failed, %d", errno);
    }
    //XXX: TESTING
    close(magic);
    //XXX: END TESTING
    //We don't actually use this yet, but we need to get it or things are sad
    if ((i = hydra_read_packet(sd, &p)) != 0) {
        hydra_log(HYDRA_LOG_CRIT, "Read failed %d %d", errno, i);
        return 1;
    }
    hydra_log(HYDRA_LOG_INFO, "Jobid: %d", p.jobok.jobid);

    // We don't need this any more!
    close(sd);

    return 0;
}
