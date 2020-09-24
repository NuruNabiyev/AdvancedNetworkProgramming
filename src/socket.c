//
// Created by Nuru on 9/23/20.
//

#include "systems_headers.h"
#include "socket.h"

struct sock_info *init_sock(struct list_head *fd_cache) {
    struct list_head *item;
    struct sock_info *entry;
    int size_counter = 0;
    list_for_each(item, fd_cache){
        entry = list_entry(item, struct sock_info, list);
        size_counter++;
    }
    
    struct sock_info *sock_ptr = calloc(1, sizeof(struct sock_info));
    if(size_counter > 0){ sock_ptr->fd = entry->fd + 1; } 
    else{ sock_ptr->fd = get_random_number(); }
    sock_ptr->state = SOCK_CLOSED; // closed in the beginning
    return sock_ptr;
}
