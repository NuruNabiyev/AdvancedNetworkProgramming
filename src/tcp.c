// Created by Nuru on 9/21/20.

#include "systems_headers.h"
#include "tcp.h"
#include "ip.h"
#include "utilities.h"

// todo this is going to be initialized to false again once another tcp comes in
//  so we should use proper pthread locks
volatile bool server_synack_ok = false;

static LIST_HEAD(fd_cache);

struct tcblock *init_tcb() {
    struct list_head *item;
    struct tcblock *entry;
    int size_counter = 0;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct tcblock, list);
        size_counter++;
    }

    struct tcblock *tcb_ptr = calloc(1, sizeof(struct tcblock));
    if (size_counter > 0) { tcb_ptr->fd = entry->fd + 1; }
    else { tcb_ptr->fd = get_random_number(); }
    tcb_ptr->state = SOCK_CLOSED; // closed in the beginning
    return tcb_ptr;
}

void add_sockfd_to_cache(struct tcblock *tcb) {
    list_init(&tcb->list);
    list_add_tail(&tcb->list, &fd_cache);
}

bool check_sockfd(int fd) {
    struct list_head *item;
    struct tcblock *entry;
    bool is_anp_sockfd = false;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct tcblock, list);
        if (fd == entry->fd) {
            is_anp_sockfd = true;
            break;
        }
    }
    return is_anp_sockfd;
}

struct tcblock *get_tcb_by_fd(int fd) {
    struct list_head *item;
    struct tcblock *entry;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct tcblock, list);
        if (fd == entry->fd) {
            return entry;
        }
    }
    return NULL;
}

struct tcblock *get_tcb_by_iss(uint32_t seq) {
    struct list_head *item;
    struct tcblock *entry;
    list_for_each(item, &fd_cache) {
        entry = list_entry(item, struct tcblock, list);
        if (seq == ntohl(entry->iss)) {
            return entry;
        }
    }
    return NULL;
}

// Helper function in case we need it later.
void free_fc_cache() {
    struct list_head *item, *tmp;
    struct tcblock *entry;
    list_for_each_safe(item, tmp, &fd_cache) {
        entry = list_entry(item, struct tcblock, list);
        list_del(item);
        free(entry);
    }
}

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    if (tcp->ack == 1 && tcp->syn == 1) {
        // check the checksum
        uint16_t packet_csum = tcp->csum;
        tcp->csum = 0;
        uint16_t check_csum = do_tcp_csum((uint8_t *) tcp, 24, IPP_TCP, ntohl(ih->saddr), ntohl(ih->daddr));
        if (packet_csum != check_csum) goto dropkt;

        // check our sequence from SYN to this SYN-ACK
        struct tcblock *si = get_tcb_by_iss(ntohl(tcp->ack_num) - 1);
        if (si == NULL) {
            printf("\n---NO SUCH ITEM FOUND---\n");
            goto dropkt;
        }
        si->serv_seq = tcp->seq_num;
        // release the lock
        server_synack_ok = true;
    }

    dropkt:
    // todo if checksum is not correct and we are dropping packet,
    //  then we should notify connect() to re-transmit
    printf("todo drop packet");
    //freesub(sub); // this throws error
}
