// Created by Nuru on 9/21/20.

#ifndef ANPNETSTACK_TCP_H
#define ANPNETSTACK_TCP_H

#include "systems_headers.h"
#include "subuff.h"
#include "linklist.h"

#define TCP_LEN 40

// todo add other states, rfc page 21, also look at diagram at page 23
#define SOCK_CLOSED 0
#define SOCK_CONNECTING 1
#define SOCK_ESTABLISHED 2

extern volatile int waiting;

extern pthread_cond_t server_synack_ok;
extern pthread_mutex_t tcp_connect_lock;

struct tcp_hdr {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t reserved: 4, data_offset: 4;
    uint8_t fin: 1, syn: 1, rst: 1, push: 1, ack: 1, urg: 1, extra: 2;
    uint16_t window;
    uint16_t csum;
    uint16_t up;
    uint32_t opt_and_pad; // dont need, just eat it
    uint8_t data[];
}__attribute__((packed));

/**
 * Holds all sockets of this program and connections states
 */
struct tcblock {
    // todo add other important stuff (rfc page 19-21, and 53)
    struct list_head list;
    // should be unique for this program
    int fd;
    uint8_t state;
    uint32_t iss;       // initial send sequence number
    uint32_t serv_seq;  // server's seq that needs to be ACKed by us todo delete?
    // local ip and port
    uint32_t lip;
    uint16_t lport;
    // remote ip and port
    uint32_t rip;
    uint16_t rport;
}__attribute__((packed));

/**
 * Initializes socket for this computer.
 * Also generates FD
 */
struct tcblock *init_tcb();

/*
 * Adds a given socket to the fd_cache
 */
void add_sockfd_to_cache(struct tcblock *tcb);

/*
 * Checks if a given socket is already in the cache.
 */
bool check_sockfd(int fd);

/*
 * Frees the cache from all the sockets.
 */
void free_fc_cache();

/**
 * @return socket info based on @param fd
 */
struct tcblock *get_tcb_by_fd(int fd);

struct tcblock *get_tcb_by_iss(uint32_t seq);

/**
 * Receives incoming tcp packet from IP layer
 * @param sub buffer received
 */
void tcp_rx(struct subuff *sub);

static inline void debug_tcp(struct tcp_hdr *tcp) {
    printf("TCP_DUMP: "
           "src_port %hu, dest_port %hu, "
           "seq_num %u, ack_num %u, "
           "data offset %i bytes, reserved %i "
           "urg %i ack %i push %i rst %i syn %i, fin %i extra %i, "
           "window %u, checksum %hx\n\n",
           htons(tcp->src_port), htons(tcp->dest_port),
           ntohl(tcp->seq_num), tcp->ack_num,
           tcp->data_offset * 4, tcp->reserved,
           tcp->urg, tcp->ack, tcp->push, tcp->rst, tcp->syn, tcp->fin, tcp->extra,
           htons(tcp->window), htons(tcp->csum));
}

#endif //ANPNETSTACK_TCP_H
