//BEGIN CODE INCLUDED FROM hydrapacket.template.c
#include "hydrapacket.h"

#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "hydralog.h"

int read_data(int fd, int *len, void **data) {
    int i;
    if ((i = read(fd, len, 4)) < 4) {
        return i;
    }
    *len = ntohl(*len);
    *data = malloc(*len);
    if ((i = read(fd, *data, *len)) != *len) {return -1;}
    return 0;
}

int write_data(int fd, int len, void *data) {
    int i;
    uint32_t u32;
    u32 = len;
    u32 = htonl(u32);
    if ((i = write(fd, &u32, 4)) != 4) {return -1;}
    if ((i = write(fd, data, len)) != len) {return -1;}
    return 0;
}

int read_file(int fd, int out) {
    uint32_t l;
    int nbytes; 
    if (read(fd, &l, sizeof(uint32_t)) < 0) {return -1;}
    l = ntohl(l);
    char buff[4096];
    while (l > 0) {
        nbytes = (l < 4096) ? l : 4096;
        int res;
        res = read(fd, buff, nbytes);
        if (res < 0) {
            return res;
        }
        l -= res;
        res = write(out, buff, nbytes);
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

int read_u32(int fd, uint32_t *u32) {
    if (read(fd, u32, sizeof(uint32_t)) < 0) {return -1;}
    *u32 = ntohl(*u32);
    return 0;
}

int write_u32(int fd, uint32_t u32) {
    u32 = htonl(u32);
    if (write(fd, &u32, sizeof(uint32_t)) < 0) {return -1;}
    return 0;
}

int read_u16(int fd, uint16_t *u16) {
    if (read(fd, &u16, sizeof(uint16_t)) < 0) {return -1;}
    *u16 = ntohs(*u16);
    return 0;
}

int write_u16(int fd, uint16_t u16) {
    u16 = htons(u16);
    if (write(fd, &u16, sizeof(uint16_t)) < 0) {return -1;}
    return 0;
}

int hydra_get_next_packettype(int fd) {
    char c;
    if (read(fd, &c, 1) != 1) {
        return -1;
    }
    return c;
}

//END CODE INCLUDED FROM hydrapacket.template.c
