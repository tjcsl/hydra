//BEGIN CODE INCLUDED FROM hydrapacket.template.c
#include "hydrapacket.h"

#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int read_data(int fd, int *len, void **data) {
    int i;
    if ((i = read(fd, len, 4)) < 4) {
        return i;
    }
    *len = ntohl(*len);
    *data = malloc(*len);
    if ((i = read(fd, *data, *len)) != *len) {
        return i;
    }
}

int write_data(int fd, int len, void *data) {
    int i;
    uint32_t u32;
    u32 = len;
    u32 = htonl(u32);
    if ((i = write(fd, &u32, 4)) != 4) {return i;} 
    if ((i = write(fd, data, len)) != len) {return i;}
}

int read_file(int fd, int out) {
    uint32_t l;
    int nbytes; 
    if (read(fd, &l, sizeof(uint32_t)) < 0) {return -1;}
    l = ntohl(l);
    char buff[4096];
    while (l > 0) {
        int to_read = (l < 4096) ? l : 4096;
        int res;
        res = read(fd, buff, to_read);
        if (res < 0) {
            return res;
        }
        l -= res;
        res = write(out, buff, to_read);
        if (res < 0) {
            return res;
        }
    }

    return 0;
}

int write_file(int fd, int in) {
    struct stat info;
    uint32_t w;
    if (fstat(in, &info) < 0) {return -1;}
    w = htonl((uint32_t)(info.st_size));
    if (write(fd, &w, sizeof(uint32_t)) < 0) {return -1;}
    return sendfile(fd, in, 0, info.st_size);
}

int hydra_get_next_packettype(int fd) {
    char c;
    if (read(fd, &c, 1) != 1) {
        return -1;
    }
    return c;
}
//END CODE INCLUDED FROM hydrapacket.template.c
