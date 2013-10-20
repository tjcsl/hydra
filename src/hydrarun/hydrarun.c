#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MASTER_HOST "hydra.csl.tjhsst.edu"

void display_help(char* argv[]){
    printf("Usage: %s [--datafile|-d <data> [--datafile|-d <data ...]] --slots|-s NUM <executable> [-- [args]]\n", argv[0]);
}

int main(int argc, char* argv[]){
    if(argc < 4){
        display_help(argv);
        return 1;
    }
    /**
     * Parse arguments
     */
    int current_arg, datafiles_count, slots, argparse_mode, current_earg;
    datafiles_count = argparse_mode = current_earg = 0;
    char** datafiles = malloc(sizeof(char*) * argc); // A safe number
    char** eargs = malloc(sizeof(char*) * argc); // Also a safe number
    char* executable;
    for(current_arg = 1; current_arg < argc; current_arg++){
        if(argparse_mode == 0){
            if(strcmp(argv[current_arg], "--datafile") == 0 || strcmp(argv[current_arg], "-d") == 0){
                current_arg++;
                datafiles[datafiles_count] = argv[current_arg];
                datafiles_count++;
                continue;
            }
            else if(strcmp(argv[current_arg], "--slots") == 0 || strcmp(argv[current_arg], "-s") == 0){
                current_arg++;
                slots = atoi(argv[current_arg]);
                continue;
            }
            else if(strcmp(argv[current_arg], "--") == 0){
                argparse_mode = 1;
                continue;
            }
            else{
                executable = argv[current_arg];
            }
        }
        else{
            eargs[current_earg] = argv[current_arg];
            current_earg++;
        }
    }
    /**
     * Output parsed args
     */
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
    for(i = 0; i < current_earg; i++){
        printf("%s ", eargs[i]);
    }
    printf("\n");
    /**
     * Make network connection
     */
    struct addrinfo hints, *result, *q;
    int status;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(MASTER_HOST, "51423", &hints, &result);
    if(status != 0){
        printf("Network error, exiting.\n");
        return 2;
    }
    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(sd == -1){
        printf("Network error, exiting.\n");
        return 2;
    }
    if(connect(sd, result->ai_addr, result->ai_addrlen) == -1){
        printf("Network error, exiting.\n");
        return 2;
    }
    close(sd);
}
