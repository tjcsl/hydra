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
#include "hydrapacket.h"
#include "hydracommon.h"

void display_help(char* argv[], FILE* fd){
    fprintf(fd, "Usage: %s [-d <data> [-d <data ...]] -h <masterhostname> -s NUM -e <executable> [-- [args]]\n", argv[0]);
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
                display_help(argv, stdout);
                return 1;
        }
    }
    if(!slotsset || !execset || !hmhostset) {
        display_help(argv, stderr);
        return 1;
    }
    // Done with arg parsing, output arguments to user
    printf("Hydra: running \"%s\" with %s slots\n", executable, slots);
    int i;
    if(datafiles_count > 0){
        printf("Datafiles: ");
        for(i = 0; i < datafiles_count; i++){
            printf("%s ", datafiles[i]);
        }
        printf("\n");
    }
    printf("Command line: %s ", executable);
    for(i = optind; i < argc; i++){
        printf("%s ", argv[i]);
    }
    printf("\n");
    /**
     * Make network connection
     */
    int sd = hydra_get_highsock(*hmhost, "51432", 0);
    if(sd < 0){
        fprintf(stderr, "Couldn't open socket (errno %d) (sd = %d), exiting.\n", errno, sd);
        fprintf(stderr, "%s\n", gai_strerror(sd));
        return 2;
    }
    // Time for actual socket communication.
    uint32_t jobid;
    if (hydra_write_SUBMIT(sd, executable, strlen(executable) + 1, atoi(slots)) != 0) {
        printf("Write failed, %d\n", errno);
    }
    int pt;
    //We don't actually use this yet, but we need to get it or things are sad
    if((pt = hydra_get_next_packettype(sd)) != HYDRA_PACKET_JOBOK) {
        fprintf(stderr, "Received malformed packet: %d", pt);
        return 1;
    }
    if ((i = hydra_read_JOBOK(sd, &jobid)) != 0) {
        fprintf(stderr, "Read failed %d %d\n", errno, i);
        return 1;
    }
    printf("Jobid: %d\n", jobid);

    // We don't need this any more!
    close(sd);

    return 0;
}
