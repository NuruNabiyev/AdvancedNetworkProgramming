//
// Created by Nuru on 9/23/20.
//

#include "systems_headers.h"
#include "socket.h"

static LIST_HEAD(fd_cache);

struct sock_info *init_sock() {
    struct list_head *item;
    struct sock_info *entry;
    int size_counter = 0;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct sock_info, list);
        size_counter++;
    }

    struct sock_info *sock_ptr = calloc(1, sizeof(struct sock_info));
    if (size_counter > 0) { sock_ptr->fd = entry->fd + 1; }
    else { sock_ptr->fd = get_random_number(); }
    sock_ptr->state = SOCK_CLOSED; // closed in the beginning
    return sock_ptr;
}

void add_sockfd_to_cache(struct sock_info *si) {
    list_init(&si->list);
    list_add_tail(&si->list, &fd_cache);
}

bool check_sockfd(int fd) {
    struct list_head *item;
    struct sock_info *entry;
    bool is_anp_sockfd = false;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct sock_info, list);
        if (fd == entry->fd) {
            is_anp_sockfd = true;
            break;
        }
    }
    return is_anp_sockfd;
}

struct sock_info *get_sock_info(int fd) {
    struct list_head *item;
    struct sock_info *entry;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct sock_info, list);
        if (fd == entry->fd) {
            return entry;
        }
    }
    return NULL;
}

struct sock_info *get_sock_info_by_seq(uint32_t seq) {
    struct list_head *item;
    struct sock_info *entry;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct sock_info, list);
        if (seq == ntohl(entry->seq)) {
            return entry;
        }
    }
    return NULL;
}

// Helper function in case we need it later.
void free_fc_cache() {
    struct list_head *item, *tmp;
    struct sock_info *entry;
    list_for_each_safe(item, tmp, &fd_cache) {
        entry = list_entry(item, struct sock_info, list);
        list_del(item);
        free(entry);
    }
}

