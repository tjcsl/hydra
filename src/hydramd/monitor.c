#include "monitor.h"

#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/signal.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <time.h>
#include "hydralog.h"
#include "shmem.h"
#include "hydralog.h"
#include "hydracommon.h"
#include "hydrapacket.h"

static int status_shmem;
static int status_semid;

typedef struct moniter_data {
    unsigned int num_nodes;
} MoniterData;

typedef struct node_status {
    time_t last_update;
    unsigned int hostname_offset;
    uint16_t slots;
    uint32_t mb_ram;
    uint32_t mb_free;
    uint32_t load_avg;
} NodeStatus;

void hydra_mon_run(void);
void hydra_mon_sighandler(int);
static void ping_node(NodeStatus*, const char*);

int hydra_mon_init(const char* whitelist_file_name) {
    size_t strings_size = 0;
    size_t data_size    = sizeof(MoniterData);
    void *status_data   = malloc(data_size);
    void *strings       = NULL;
    char host[128];
    int host_len;
    int offset;
    int i;
    FILE *whitelist;
    MoniterData *mon_data;
    NodeStatus *node_data;

    ((MoniterData *)status_data)->num_nodes = 0;

    whitelist = fopen(whitelist_file_name, "r");
    if (whitelist == NULL) {
        hydra_exit_error("unable to open whitelist file");
    }

    mon_data = (MoniterData *) status_data;
    mon_data->num_nodes = 0;
    while (fgets(host, 128, whitelist) != NULL) {
        host_len = strlen(host);
        if (host_len == 127 && host[host_len] != '\n' && host[0] != '#') {
            hydra_log(HYDRA_LOG_CRIT, "Hostname longer than 127 characters (%d chars)", host_len);
            hydra_exit_error("Exiting, whitelist has an overly long hostname");
        }
        hydra_log(HYDRA_LOG_DEBUG, "Read hostname %s", host);
        //Append a node status and update the state of things
        mon_data->num_nodes++;
        data_size += sizeof(NodeStatus);

        hydra_log(HYDRA_LOG_DEBUG, "Resizing status storage to %d", data_size);

        status_data = realloc(status_data, data_size);
        mon_data = (MoniterData *) status_data;

        hydra_log(HYDRA_LOG_DEBUG, "Calculating offset into status storage");

        offset = sizeof(MoniterData) + (sizeof(NodeStatus) * (mon_data->num_nodes - 1));
        hydra_log(HYDRA_LOG_DEBUG, "Cleaning status storage for node %d, offset %d length %d", mon_data->num_nodes, offset, sizeof(NodeStatus));
        memset(status_data + offset, 0, sizeof(NodeStatus));

        hydra_log(HYDRA_LOG_DEBUG, "O:%d, D:%d", offset, data_size);

        //And put the string in.
        node_data = ((NodeStatus*) (status_data + offset));
        node_data->hostname_offset = strings_size;
        strings_size += host_len + 1;
        strings = realloc(strings, strings_size);
        memcpy(strings + node_data->hostname_offset, host, host_len + 1);

        hydra_log(HYDRA_LOG_DEBUG, "O:%d, S:%d", offset, strings_size);
    }

    //Add data_size as an offset so things actually work 'n stuff
    for (i = 0; i < mon_data->num_nodes; ++i) {
        offset = sizeof(MoniterData) + sizeof(NodeStatus) * i;
        ((NodeStatus*)(status_data + offset))->hostname_offset += data_size;
    }

    hydra_shmem_create(HYDRA_MON_LOCK, HYDRA_MON_MEM, data_size + strings_size, &status_shmem, &status_semid);
    void *shmem = hydra_shmem_lock(status_semid, status_shmem);
    memcpy(shmem, status_data, data_size);
    memcpy(shmem + data_size, strings, strings_size);
    //XXX: TESTING
    FILE *tfile = fopen("testing.dump", "w");
    fwrite(shmem, 1, data_size + strings_size, tfile);
    fclose(tfile);
    //XXX:TESTING
    free(strings);
    free(status_data);
    fclose(whitelist);

    //and kick off the pinger thingy
    i = fork();
    if (i < 0) {
        return -1;
    } else if (i == 0) {
        hydra_mon_run();
        return 0;
    } else {
        return 0;
    }
}

void hydra_mon_run() {
    struct sigaction act;
    int i;
    MoniterData d;
    void *data;
    NodeStatus *status;
    //Change the open log to hydramd_dispatch
    openlog("hydramd_dispatch", LOG_PID, LOG_DAEMON);
    //Request that the kernel inform us when our parent is murdered
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    //Exit cleanly on deadly signals, we don't want to accidently shut down all
    //the things (twice) if we sigsev =D
    act.sa_handler = hydra_mon_sighandler;
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGKILL, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT , &act, NULL);
    sigaction(SIGHUP , &act, NULL);
    sigaction(SIGPIPE, &act, NULL);
    hydra_log(HYDRA_LOG_INFO, "Hydramon forked and set up, begining pings");
    for (;;) {
        data = hydra_shmem_lock(status_semid, status_shmem);
        d = *((MoniterData*)data);
        hydra_log(HYDRA_LOG_DEBUG, "We have %d nodes to check", d.num_nodes);
        //Macros to save typing
#define GET_NODE_STATUS(off) ((NodeStatus*) (data + sizeof(MoniterData) + (off) * sizeof(NodeStatus)))
        for (i = 0; i < d.num_nodes; ++i) {
            status = GET_NODE_STATUS(i);
            hydra_log(HYDRA_LOG_DEBUG, "Pinging node %d:%s", i, (char*)(data + status->hostname_offset));
            ping_node(status, (char *)(data + status->hostname_offset));
        }
        hydra_shmem_ulck(status_semid, data);
#undef GET_NODE_STATUS
        sleep(120);
    }
}

int hydra_mon_destroy() {
    return hydra_shmem_destroy(status_semid, status_shmem);
}

void hydra_mon_sighandler(int sig) {
    if (sig == SIGPIPE) { /* it's ok, we can ignore this */ return;}
    hydra_log(HYDRA_LOG_CRIT, "Hydra monitor caught deadly signal %d, exiting", sig);
    exit(sig);
}

void invalidate_status(NodeStatus *status) {
    status->load_avg = -1;
    status->mb_ram = -1;
    status->mb_free = -1;
    status->slots = -1;
}

void ping_node(NodeStatus *status, const char* hostname) {
    struct addrinfo hints, *result, *rp;
    struct timeval timeout;
    int i;
    int sock;
    HydraPacket p;

    memset(&p, 0, sizeof(HydraPacket));
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    i = getaddrinfo(hostname, "51444", &hints, &result);
    time(&status->last_update);
    if (i != 0) {
        hydra_log(HYDRA_LOG_WARN,
            "getaddrinfo returned %s for node %s, assuming node is unreachable",
            gai_strerror(i), hostname);
        invalidate_status(status);
        return;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        hydra_log(HYDRA_LOG_DEBUG, "Result: %d %d %d %x", rp->ai_family, rp->ai_socktype, rp->ai_protocol, rp->ai_next);
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) < 0) {continue;}
        if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval)) < 0) {continue;}
        if (sock < 0) continue;
        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) break; /*success*/
        close(sock);
    }

    if (!rp) {
        hydra_log(HYDRA_LOG_WARN,
                "Unable to contact node %s: error %s",
                hostname, strerror(errno));
        invalidate_status(status);
        return;
    }

    freeaddrinfo(result);

    p.id = HYDRA_PACKET_PING;
    hydra_log(HYDRA_LOG_DEBUG, "p.id = %d", p.id);
    if (hydra_write_packet(sock, &p) < 0) {
        hydra_log(HYDRA_LOG_WARN, "Could not ping node: write failed");
        invalidate_status(status);
    }
    hydra_log(HYDRA_LOG_DEBUG, "write socket worked");
    if (hydra_get_next_packettype(sock) != HYDRA_PACKET_HEARTBEAT) {
        hydra_log(HYDRA_LOG_WARN,
                "Got invalid response of type %d from node %s", 
                p.id, hostname);
        hydra_read_packet(sock, &p); //Just so things don't  get clogged up
        invalidate_status(status);
        return;
    }
    if (hydra_read_packet(sock, &p) < 0) {
        hydra_log(HYDRA_LOG_WARN,
                "Couldn't read response from node %s : %s", 
                hostname, strerror(errno));
        invalidate_status(status);
        return;
    }
    hydra_log(HYDRA_LOG_INFO, "Successfuly connected to node %s", hostname);
    status->slots = p.heartbeat.slots;
    status->mb_free = p.heartbeat.mb_free;
    status->mb_ram = p.heartbeat.mb_ram;
    status->load_avg = p.heartbeat.load_avg;
}
