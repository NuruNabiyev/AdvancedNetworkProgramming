//
// Created by Nuru on 9/23/20.
//

#ifndef ANPNETSTACK_SOCKET_H
#define ANPNETSTACK_SOCKET_H

#include "systems_headers.h"

#define SOCK_CLOSED 0
#define SOCK_CONNECTING 1
#define SOCK_LISTENING 2
#define SOCK_ESTABLISHED 3

/**
 * Holds all sockets of this program and connections states
 */
struct sock_info {
    // todo include linklist to save other sockets for this computer
    // should be unique for this program
    int fd; // todo lowest-numbered file descriptor not currently open for the process
    uint8_t state;  // Closed (0) Connecting(1) Listening(2) Established(3)
    // local ip and port
    uint32_t lip;
    uint16_t lport;
    // remote ip and port
    uint32_t rip;
    uint16_t rport;
}__attribute__((packed));

/**
 * Initializes socket for this computer.
 * Also generates FD (todo check whether it existed before or not)
 */
struct sock_info *init_sock();

/**
 * todo should be called just before TCP's connect
 * todo set state to connecting
 * @param lip is local ip of this computer
 * @param lport is local port
 * @param rip is remote ip that we are willing to connect
 * @param rport is remote port
 */
void *connect_sock(uint32_t lip, uint16_t lport,
                   uint32_t rip, uint16_t rport);

static inline int get_random_number(){
    int upper = 2000000;
    int lower = 1000000;
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

#endif //ANPNETSTACK_SOCKET_H
