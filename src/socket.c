//
// Created by Nuru on 9/23/20.
//

#include "systems_headers.h"
#include "socket.h"

struct sock_info *init_sock() {
    struct sock_info *sock_ptr = calloc(sizeof(*sock_ptr), 1);
    sock_ptr->fd = get_random_number(); // random file descriptor in range of 1B-2B
    sock_ptr->state = SOCK_CLOSED; // closed in the beginning
    return sock_ptr;
}
