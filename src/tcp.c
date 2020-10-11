// Created by Nuru on 9/21/20.

#include "systems_headers.h"
#include "tcp.h"
#include "ip.h"
#include "utilities.h"

// todo this is going to be initialized to false again once another tcp comes in
//  so we should use proper pthread locks
//volatile bool server_synack_ok = false;
volatile int waiting = 0;
pthread_cond_t server_synack_ok = PTHREAD_COND_INITIALIZER;
pthread_mutex_t tcp_connect_lock = PTHREAD_MUTEX_INITIALIZER;

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

struct subuff *alloc_tcp_connect(struct tcblock *tcb, bool syn_or_ack) {
    struct subuff *sub = alloc_sub(ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN_40);
    sub_reserve(sub, ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN_40);
    if (!sub) {
        printf("Error: allocation of the arp sub in request failed \n");
        return NULL;
    }
    sub->protocol = IPP_TCP;
    struct tcp_hdr *syntcp = (struct tcp_hdr *) sub_push(sub, TCP_LEN_40);

    // create tcp packet based on the syn or ack
    if (syn_or_ack) {
        // prepare TCP struct with related fields in correct network byte order  and checksum
        update_tcp_syn(tcb, syntcp);
    } else {
        update_tcp_ack(tcb, syntcp);
    }

    return sub;
}

void update_tcp_syn(struct tcblock *tcb, struct tcp_hdr *tcpHdr) {
    tcpHdr->src_port = ntohs(tcb->lport);
    tcpHdr->dest_port = ntohs(tcb->rport);
    tcpHdr->seq_num = htonl(get_random_number()); // todo save on repeat
    tcb->iss = tcpHdr->seq_num;
    tcpHdr->data_offset = 10;
    tcpHdr->syn = 1;
    tcpHdr->window = ntohs(65495);
    tcpHdr->csum = 0;

    uint16_t csum = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN_40, IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
    tcpHdr->csum = csum;
}

void update_tcp_ack(struct tcblock *tcb, struct tcp_hdr *tcpHdr) {
    tcpHdr->src_port = ntohs(tcb->lport);
    tcpHdr->dest_port = ntohs(tcb->rport);
    tcpHdr->seq_num = htonl(ntohl(tcb->iss) + 1);
    tcpHdr->ack_num = htonl(ntohl(tcb->serv_seq) + 1);

    tcpHdr->data_offset = 10;
    tcpHdr->ack = 1;
    tcpHdr->window = ntohs(512);
    tcpHdr->csum = 0;

    uint16_t csum = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN_40, IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
    tcpHdr->csum = csum;
}

struct subuff *allocate_tcp_send(struct tcblock *tcb, const void *buf, size_t len) {
    struct subuff *sub = alloc_sub(ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN_32 + len);
    sub_reserve(sub, ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN_32 + len);
    if (!sub) {
        printf("Error: allocation of the arp sub in request failed \n");
        return NULL;
    }
    sub->protocol = IPP_TCP;

    // first push data
    uint8_t *subdata = sub_push(sub, len);
    memcpy(subdata, buf, len);

    // now push header
    struct tcp_hdr *tcpHdr = (struct tcp_hdr *) sub_push(sub, TCP_LEN_32);
    tcpHdr->src_port = ntohs(tcb->lport);
    tcpHdr->dest_port = ntohs(tcb->rport);
    tcpHdr->seq_num = htonl(ntohl(tcb->iss) + 1);
    tcpHdr->ack_num = htonl(ntohl(tcb->serv_seq) + 1);
    tcb->snd_nxt = htonl(ntohl(tcb->iss) + 1 + len);
    tcpHdr->data_offset = 8;
    tcpHdr->ack = 1;
    tcpHdr->push = 1;
    tcpHdr->window = ntohs(502);
    tcpHdr->csum = 0;

    uint16_t csum = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN_32 + len,
                                IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
    tcpHdr->csum = csum;

    return sub;
}

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    // receiving handshake
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
        pthread_mutex_lock(&tcp_connect_lock);
        waiting = 1;
        printf("---------------------------SENING SIGNAL..\n\n");
        pthread_cond_signal(&server_synack_ok);
        pthread_mutex_unlock(&tcp_connect_lock);
        return;
    }

    // receiving ack for our packet  todo if its receive packet not whole
    if (tcp->ack == 1 && tcp->push == 0) {
        printf("\n\nRECEIVED ACK FOR PACKET\n");
        debug_tcp(tcp);

        // check the checksum
        uint16_t packet_csum = tcp->csum;
        tcp->csum = 0;
        uint16_t check_csum = do_tcp_csum((uint8_t *) tcp, 20, IPP_TCP, ntohl(ih->saddr), ntohl(ih->daddr));
        if (packet_csum != check_csum) goto dropkt;

        // release the lock
        pthread_mutex_lock(&tcp_connect_lock);
        waiting = 1;
        printf("---------------------------SENING SIGNAL.. 2\n\n");
        pthread_cond_signal(&server_synack_ok);
        pthread_mutex_unlock(&tcp_connect_lock);
        return;
    }

    dropkt:
    // todo if checksum is not correct and we are dropping packet,
    //  then we should notify connect() to re-transmit
    printf("todo drop packet\n");
    //freesub(sub); // this throws error
}
