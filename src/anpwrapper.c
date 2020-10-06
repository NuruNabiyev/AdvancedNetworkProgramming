//XXX: _GNU_SOURCE must be defined before including dlfcn to get RTLD_NEXT symbols
#define _GNU_SOURCE

#include <dlfcn.h>
#include "systems_headers.h"
#include "linklist.h"
#include "anpwrapper.h"
#include "tcp.h"
#include "utilities.h"
#include "subuff.h"
#include "ethernet.h"
#include "route.h"
#include "anp_netdev.h"
#include "ip.h"
#include <time.h>

static int (*__start_main)(int (*main)(int, char **, char **), int argc, \
        char **ubp_av, void (*init)(void), void (*fini)(void), \
        void (*rtld_fini)(void), void (*stack_end));

static ssize_t (*_send)(int fd, const void *buf, size_t n, int flags) = NULL;

static ssize_t (*_recv)(int fd, void *buf, size_t n, int flags) = NULL;

static int (*_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = NULL;

static int (*_socket)(int domain, int type, int protocol) = NULL;

static int (*_close)(int sockfd) = NULL;

static int is_socket_supported(int domain, int type, int protocol) {
    if (domain != AF_INET) {
        return 0;
    }
    if (!(type & SOCK_STREAM)) {
        return 0;
    }
    if (protocol != 0 && protocol != IPPROTO_TCP) {
        return 0;
    }
    printf("supported socket domain %d type %d and protocol %d \n", domain, type, protocol);
    return 1;
}

/*
 * Parses local and remote ip and ports, assigns to current socket_info
 */
void assign_sockets(struct tcblock *tcb, const struct sockaddr *addr, socklen_t addrlen) {
    // retrieve port and ip and prepare current tcblock
    char rips[NI_MAXHOST], rports[NI_MAXSERV];
    int rc = getnameinfo(addr, addrlen, rips, sizeof(rips), rports, sizeof(rports),
                         NI_NUMERICHOST | NI_NUMERICSERV);
    printf("rc %i, host %s, port %s\n", rc, rips, rports);
    struct sockaddr_in sa;
    inet_pton(AF_INET, rips, &(sa.sin_addr));
    uint32_t rip = htonl(sa.sin_addr.s_addr);
    uint16_t rport = atoi(rports);
    tcb->rip = rip;
    tcb->rport = rport;

    // assign local ip and port
    char lips[NI_MAXHOST] = "10.0.0.4";
    struct sockaddr_in sa_loc;
    inet_pton(AF_INET, lips, &(sa_loc.sin_addr));
    uint32_t lip = htonl(sa_loc.sin_addr.s_addr);
    tcb->lip = lip;
    tcb->lport = rport; // can be what ever we want
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

    uint16_t csum = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN, IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
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

    uint16_t csum = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN, IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
    tcpHdr->csum = csum;
}

struct subuff *alloc_tcp_sub(struct tcblock *tcb, bool syn_or_ack) {
    struct subuff *sub = alloc_sub(ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN);
    sub_reserve(sub, ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN);
    if (!sub) {
        printf("Error: allocation of the arp sub in request failed \n");
        return NULL;
    }
    sub->protocol = IPP_TCP;
    struct tcp_hdr *syntcp = (struct tcp_hdr *) sub_push(sub, TCP_LEN);

    // create tcp packet based on the syn or ack
    if (syn_or_ack) {
        // prepare TCP struct with related fields in correct network byte order  and checksum
        update_tcp_syn(tcb, syntcp);
    } else {
        update_tcp_ack(tcb, syntcp);
    }

    return sub;
}

int send_first_seq(struct tcblock *tcb) {
    struct subuff *sub = alloc_tcp_sub(tcb, true);
    return ip_output(tcb->rip, sub);
}

int wait_for_server(int max_seconds) {
    int             rc;
    struct timespec ts;
    struct timeval  tp;

    rc = pthread_mutex_lock(&tcp_connect_lock);
    gettimeofday(&tp, NULL);
    
    ts.tv_sec = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += max_seconds;

    while(waiting == 0){
        rc = pthread_cond_timedwait(&server_synack_ok, &tcp_connect_lock, &ts);
        if(rc == ETIMEDOUT){
            printf("\nTIMED OUT - ABORT\n");
            rc = pthread_mutex_unlock(&tcp_connect_lock);
            return -1; 
        }
    }
    
    // We have been signae=led, the waiting is over!
    waiting = 0; // Reset thread waiting predicate for next use.
    return 1;
}

/**
 * Initializes socket
 * @param domain AF_INET
 * @param type SOCK_STREAM
 * @param protocol 0
 * @return File descriptor
 */
int socket(int domain, int type, int protocol) {
    if (!is_socket_supported(domain, type, protocol)) {
        // if this is not what anpnetstack support, let it go, let it go!
        return _socket(domain, type, protocol);
    }

    struct tcblock *tcb = init_tcb(); // save this tcblock in fd_cache
    add_sockfd_to_cache(tcb);
    return tcb->fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    printf("CONNECT CALLED:\n");
    if (!check_sockfd(sockfd)) {
        // the default path
        return _connect(sockfd, addr, addrlen);
    }

    // get our socket, populate with local ip/port and remote ip/port, set state CONNECTING
    struct tcblock *tcb = get_tcb_by_fd(sockfd);
    assign_sockets(tcb, addr, addrlen);
    tcb->state = SOCK_CONNECTING;

    // try first one
    int ret = send_first_seq(tcb);
    int count = 1;

    // if our cache is empty and arp in progress
    if (ret == -11) {
        sleep(1);
    }

    if (ret < 0) {
        // send second time (or more)
        do {
            printf("\nRESENDING INITIAL SYN (%i)\n", count);
            if (send_first_seq(tcb) < 0) sleep(1); else break;
            count++;
        } while (count <= 3);
    }

    if (count == 2) {
        tcb->state = SOCK_CLOSED;
        errno = ETIMEDOUT;
        return -ETIMEDOUT;
    }

    // lock until server responds with syn-ack
    ret = wait_for_server(3);
    if (ret < 1) {
        tcb->state = SOCK_CLOSED;
        errno = ETIMEDOUT;
        return -ETIMEDOUT;
    }

    // send ack
    struct subuff *sub_ack = alloc_tcp_sub(tcb, false);
    ret = ip_output(tcb->rip, sub_ack);
    if (ret != -1) {
        tcb->state = SOCK_ESTABLISHED;
        errno = 0;
        return 0;
    }

    errno = ENOSYS;
    return -ENOSYS;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    printf("SEND CALLED:\n");
    if (!check_sockfd(sockfd)) {
        // the default path
        return _send(sockfd, buf, len, flags);
    }

    struct subuff *sub = alloc_sub(ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN + len);
    sub_reserve(sub, ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN + len);
    if (!sub) {
        printf("Error: allocation of the arp sub in request failed \n");
        return NULL;
    }
    sub->protocol = IPP_TCP;
    struct tcp_hdr *tcpHdr = (struct tcp_hdr *) sub_push(sub, TCP_LEN);
    struct tcblock *tcb = get_tcb_by_fd(sockfd);

    tcpHdr->src_port = ntohs(tcb->lport);
    tcpHdr->dest_port = ntohs(tcb->rport);
    tcpHdr->seq_num = htonl(ntohl(tcb->iss) + 1);
    tcpHdr->ack_num = htonl(ntohl(tcb->serv_seq) + 1);
    tcpHdr->data_offset = 10;
    tcpHdr->ack = 1;
    tcpHdr->push = 1;
    tcpHdr->window = ntohs(512);
    tcpHdr->csum = 0;
    uint8_t *subdata = sub_push(sub, len);
    memcpy(subdata, buf, len);

    uint16_t csum = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN, IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
    tcpHdr->csum = csum;

    printf("\n\nABOUT TO SEND \n\n");
    dump_hex(sub, 200);
    int ret = ip_output(tcb->rip, sub);
    sleep(3);

    // todo if server does not acknowledge this in 1-2 seconds, repeat
    return -ENOSYS;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    //FIXME -- you can remember the file descriptors that you have generated in the socket call and match them here
    bool is_anp_sockfd = false;
    if (is_anp_sockfd) {
        //TODO: implement your logic here
        return -ENOSYS;
    }
    // the default path
    return _recv(sockfd, buf, len, flags);
}

int close(int sockfd) {
    //FIXME -- you can remember the file descriptors that you have generated in the socket call and match them here
    bool is_anp_sockfd = false;
    if (is_anp_sockfd) {
        //TODO: implement your logic here
        return -ENOSYS;
    }
    // the default path
    return _close(sockfd);
}

void _function_override_init() {
    printf("void _function_override_init() \n");
    __start_main = dlsym(RTLD_NEXT, "__libc_start_main");
    _socket = dlsym(RTLD_NEXT, "socket");
    _connect = dlsym(RTLD_NEXT, "connect");
    _send = dlsym(RTLD_NEXT, "send");
    _recv = dlsym(RTLD_NEXT, "recv");
    _close = dlsym(RTLD_NEXT, "close");
}
