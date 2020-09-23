//
// Created by Nuru on 9/23/20.
//

#ifndef ANPNETSTACK_SOCKET_H
#define ANPNETSTACK_SOCKET_H

#include "systems_headers.h"

/**
 * Holds all sockets of this program and connections states
 */
struct sock_info {
    // todo include linklist to save other sockets for this computer
    // should be unique for this program
    uint32_t fd;
    uint8_t state;  // Closed (0) Connecting(1) Listening(2) Established(3)
    // local ip and port
    uint32_t lip;
    uint16_t lport;
    // remote ip and port
    uint32_t rip;
    uint16_t rport;
}__attribute__((packed));

/**
 * Initializes socket for two computers.
 * Also generates FD (todo check whether it existed before or not)
 * @param lip is local ip of this computer
 * @param lport is local port
 * @param rip is remote ip that we are willing to connect
 * @param rport is remote port
 * @return newly created socket and add todo to list of other sockets
 */
struct sock_info *init_sock(uint32_t lip,
                            uint16_t lport,
                            uint32_t rip,
                            uint16_t rport);

#endif //ANPNETSTACK_SOCKET_H
