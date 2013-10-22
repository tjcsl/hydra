//Automaticly generated by gen_hydrapacket.py
#ifndef _HYDRAPACKET_H_
#define _HYDRAPACKET_H_
#include <arpa/inet.h>
#include <sys/types.h>
#define HYDRA_PACKET_RUN 13
#define HYDRA_PACKET_JOBDONEACK 12
#define HYDRA_PACKET_EXEC 6
#define HYDRA_PACKET_CHALLENGE 0
#define HYDRA_PACKET_CHOK 2
#define HYDRA_PACKET_PING 3
#define HYDRA_PACKET_FILEACK 10
#define HYDRA_PACKET_SUBMIT 5
#define HYDRA_PACKET_RUNACK 14
#define HYDRA_PACKET_JOBDONE 11
#define HYDRA_PACKET_JOBDATA 8
#define HYDRA_PACKET_HEARTBEAT 4
#define HYDRA_PACKET_CHRESPONSE 1
#define HYDRA_PACKET_JOBOK 7
#define HYDRA_PACKET_FILEDATA 9
extern int hydra_read_RUN (int fd,uint32_t *jobid);
extern int hydra_write_RUN(int fd,uint32_t  jobid);
extern int hydra_read_JOBDONEACK (int fd,uint32_t *jobid);
extern int hydra_write_JOBDONEACK(int fd,uint32_t  jobid);
extern int hydra_read_EXEC (int fd,uint32_t *jobid);
extern int hydra_write_EXEC(int fd,uint32_t  jobid);
extern int hydra_read_CHALLENGE (int fd,uint32_t *challenge_id);
extern int hydra_write_CHALLENGE(int fd,uint32_t  challenge_id);
extern int hydra_read_CHOK (int fd,char *ok);
extern int hydra_write_CHOK(int fd,char  ok);
extern int hydra_read_PING (int fd);
extern int hydra_write_PING(int fd);
extern int hydra_read_FILEACK (int fd,uint32_t *jobid);
extern int hydra_write_FILEACK(int fd,uint32_t  jobid);
extern int hydra_read_SUBMIT (int fd,const void** exe_name_data,int *exe_name_len,uint16_t *slots);
extern int hydra_write_SUBMIT(int fd,const void*  exe_name_data,int exe_name_len,uint16_t  slots);
extern int hydra_read_RUNACK (int fd,uint32_t *jobid);
extern int hydra_write_RUNACK(int fd,uint32_t  jobid);
extern int hydra_read_JOBDONE (int fd,uint32_t *jobid);
extern int hydra_write_JOBDONE(int fd,uint32_t  jobid);
extern int hydra_read_JOBDATA (int fd,uint32_t *jobid);
extern int hydra_write_JOBDATA(int fd,uint32_t  jobid);
extern int hydra_read_HEARTBEAT (int fd,uint16_t *slots,const void** hostname_data,int *hostname_len,uint32_t *mb_free,uint32_t *mb_ram,uint32_t *load_avg);
extern int hydra_write_HEARTBEAT(int fd,uint16_t  slots,const void*  hostname_data,int hostname_len,uint32_t  mb_free,uint32_t  mb_ram,uint32_t  load_avg);
extern int hydra_read_CHRESPONSE (int fd,uint32_t *challenge_resp);
extern int hydra_write_CHRESPONSE(int fd,uint32_t  challenge_resp);
extern int hydra_read_JOBOK (int fd,uint32_t *jobid);
extern int hydra_write_JOBOK(int fd,uint32_t  jobid);
extern int hydra_read_FILEDATA (int fd,uint32_t *jobid);
extern int hydra_write_FILEDATA(int fd,uint32_t  jobid);
extern int hydra_get_next_packettype(int fd);
#endif