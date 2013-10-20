#include "hydranet.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *hydra_read_packet(int fd) {
    char buff[4];
    char *ret;
    int *i;
    if (read(fd, buff, 4) < 0) {
        return NULL;
    }

    i = (int*)buff; //Hacky casting to make it an integer
    *i = ntohl(*i);
    
    ret = malloc(*i);

    if (read(fd, ret, *i) < 0) {
        free(ret);
        return NULL;
    }
    return ret;
}

void hydra_write_packet(int fd, const char* data, int datalen) {
    char *obuff = malloc (datalen + 4);
    int ndatalen = htonl(datalen);

    memcpy(obuff + 4, data, datalen);
    memcpy(obuff, &ndatalen, 4);
    write(fd, obuff, datalen + 4);

    free(obuff);
}
