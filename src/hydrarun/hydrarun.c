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

void display_help(char* argv[]){
    printf("Usage: %s [-d <data> [-d <data ...]] -h <masterhostname> -s NUM -e <executable> [-- [args]]\n", argv[0]);
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
                display_help(argv);
                return 1;
        }
    }
    if(!slotsset || !execset || !hmhostset) {
        display_help(argv);
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
    struct addrinfo hints, *result;
    int status;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(*hmhost, "51432", &hints, &result);
    if(status != 0){
        fprintf(stderr, "Couldn't getaddrinfo, exiting.\n");
        return 2;
    }
    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(sd == -1){
        fprintf(stderr, "Couldn't open socket, exiting.\n");
        return 2;
    }
    if(connect(sd, result->ai_addr, result->ai_addrlen) == -1){
        fprintf(stderr, "Couldn't connect to socket, exiting. (Errno %d)\n", errno);
        return 2;
    }
    freeaddrinfo(result);
    // Time for actual socket communication.
    int sub_length = 8 + strlen((const char*)executable) + strlen((const char*)slots);
    char submit[sub_length];
    memset(&submit, 0, sub_length);
    strcpy(submit, "SUBMIT ");
    strcat(submit, executable);
    strcat(submit, " ");
    strcat(submit, slots);
    write(sd, submit, sub_length);
    printf("Executable submitted; getting jobid.\n");
    //int jobid;
    //char* submit_resp = fscanf(sd, "JOBID %d", &jobid);

    // We don't need this any more!
    close(sd);

    return 0;
}
