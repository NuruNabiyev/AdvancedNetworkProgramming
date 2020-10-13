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
#include <sys/time.h>

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

int send_first_seq(struct tcblock *tcb) {
    struct subuff *sub = alloc_tcp_connect(tcb, true);
    return ip_output(tcb->rip, sub);
}

int wait_for_server(int max_seconds) {
    int rc;
    struct timespec ts;
    struct timeval tp;

    pthread_mutex_trylock(&tcp_connect_lock);
    gettimeofday(&tp, NULL);

    ts.tv_sec = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += max_seconds;

    while (waiting == 0) {
        printf("\nLOCKING FOR PACKET ACK\n");
        rc = pthread_cond_timedwait(&server_synack_ok, &tcp_connect_lock, &ts);
        if (rc == ETIMEDOUT) {
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
    struct subuff *sub_ack = alloc_tcp_connect(tcb, false);
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

    struct tcblock *tcb = get_tcb_by_fd(sockfd);
    struct subuff *sub = allocate_tcp_send(tcb, buf, len);
    int ret = ip_output(tcb->rip, sub);
    int count = 1;

    if (ret < 0) {
        // send second time (or more)
        do {
            printf("\nRESENDING PACKET (%i)\n", count);
            struct subuff *sub2 = allocate_tcp_send(tcb, buf, len);
            int ret = ip_output(tcb->rip, sub2);
            if (ret < 0) sleep(1); else break;
            count++;
        } while (count <= 3);
    }
    if (count == 2) {
        printf("\nABORT - SENT 3 times\n", count);
        tcb->state = SOCK_CLOSED;
        errno = ETIMEDOUT;
        return -ETIMEDOUT;
    }

    // lock until server responds with syn-ack
    waiting = 0;
    int wait_ret = wait_for_server(3);
    if (wait_ret < 1) {
        printf("\n\nSERVER DID NOT RESPOND WITH ACK\n\n");
        tcb->state = SOCK_CLOSED;
        errno = ETIMEDOUT;
        return -ETIMEDOUT;
    }

    return ret;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    printf("RECEIVE packet with length %zi:\n", len);
    if (!check_sockfd(sockfd)) {
        // the default path
        return _recv(sockfd, buf, len, flags);
    }

    // skipping checksum checking
    struct tcblock *tcb    = get_tcb_by_fd(sockfd);
    struct subuff *sub     = alloc_sub(ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN_32);
    sub_reserve(sub, ETH_HDR_LEN + IP_HDR_LEN + TCP_LEN_32);
    if (!sub) {
        printf("Error: allocation of the arp sub in request failed \n");
        return NULL;
    }
    sub->protocol          = IPP_TCP;
    struct tcp_hdr *tcpHdr = (struct tcp_hdr *) sub_push(sub, TCP_LEN_32);
    tcpHdr->src_port       = ntohs(tcb->lport);
    tcpHdr->dest_port      = ntohs(tcb->rport);
    tcpHdr->seq_num        = tcb->snd_nxt;
    tcpHdr->ack_num        = htonl(tcb->rcv_nxt + len);
    tcb->rcv_nxt           = ntohl(tcpHdr->ack_num);
    tcpHdr->data_offset    = 8;
    tcpHdr->ack            = 1;
    tcpHdr->window         = ntohs(5480);
    tcpHdr->csum           = 0;
    uint16_t csum          = do_tcp_csum((uint8_t *) tcpHdr, TCP_LEN_32, IPP_TCP, htonl(tcb->lip), htonl(tcb->rip));
    tcpHdr->csum           = csum;
    int ret                = ip_output(tcb->rip, sub);
    return len;
}

int close(int sockfd) {
    printf("\n\n---------------------------CLOSE CALLED------------\n\n");
    if (!check_sockfd(sockfd)) {
        // the default path
        return _close(sockfd);
    }

    struct tcblock *tcb = get_tcb_by_fd(sockfd);
    struct subuff *sub = alloc_tcp_finack(tcb);
    int ret = ip_output(tcb->rip, sub);

    // lock and wait here until in tcp_rx receives fin/ack from server
    ret = wait_for_server(3);
    if (ret < 1) {
        tcb->state = SOCK_CLOSED;
        errno = ETIMEDOUT;
        return -ETIMEDOUT;
    }

    // send our last ack
    struct subuff *sub_ack = alloc_tcp_lastack(tcb);
    ret = ip_output(tcb->rip, sub_ack);
    if (ret != -1) {
        // clear tcb and remove from list, return 0 on success
        tcb->state = SOCK_CLOSED;
        errno = 0;
        return 0;
    }
    sleep(1);
    errno = ENOSYS;
    return -ENOSYS;
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
