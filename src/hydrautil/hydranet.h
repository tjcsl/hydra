#ifndef _HYDRANET_H_
#define _HYDRANET_H_

//reads a hydra packet from fd. They user _MUST_ call free() on the returned
//buffer. Returns NULL on failure.

extern char* hydra_read_packet(int fd);

extern void hydra_write_packet(int fd, const char *data, int datalen);

#endif
