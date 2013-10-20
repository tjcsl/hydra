#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void display_help(char* argv[]){
    printf("Usage: %s [-d <data> [-d <data ...]] -h <masterhostname> -s NUM -e <executable> [-- [args]]\n", argv[0]);
}

int main(int argc, char* argv[]){
    // Parse the arguments
    extern char *optarg;
    extern int optind;
    int datafiles_count, slots, slotsset, execset, currarg, hmhostset;
    datafiles_count = slotsset = execset = currarg = hmhostset = 0;
    char** datafiles = malloc(sizeof(char*) * argc); // A safe number
    char** eargs = malloc(sizeof(char*) * argc); // Also a safe number
    char* executable;
    char** hmhost;
    while((currarg = getopt(argc, argv, "d:s:e:h:")) != -1) {
        switch(currarg) {
            case 'd':
                datafiles[datafiles_count++] = optarg;
                break;
            case 's':
                slots = atoi(optarg);
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
    printf("Hydra: running \"%s\" with %d slots\n", executable, slots);
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
    struct addrinfo hints, *result;
    int status;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(*hmhost, "51423", &hints, &result);
    if(status != 0){
        fprintf(stderr, "Network error, exiting.\n");
        return 2;
    }
    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(sd == -1){
        fprintf(stderr, "Network error, exiting.\n");
        return 2;
    }
    if(connect(sd, result->ai_addr, result->ai_addrlen) == -1){
        fprintf(stderr, "Network error, exiting.\n");
        return 2;
    }
    close(sd);
}
