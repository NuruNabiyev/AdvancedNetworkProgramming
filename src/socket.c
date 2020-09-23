//
// Created by Nuru on 9/23/20.
//

#include "systems_headers.h"
#include "socket.h"

struct sock_info *init_sock() {
    struct sock_info *sock_ptr;
    // i get segfault, todo
    memset(sock_ptr, 0, sizeof(struct sock_info));
    sock_ptr->fd = 1234567 + 123; // some random file descriptor
    sock_ptr->state = SOCK_CLOSED; // closed in the beginning
    return sock_ptr;
}