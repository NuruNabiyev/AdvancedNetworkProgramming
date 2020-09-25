//XXX: _GNU_SOURCE must be defined before including dlfcn to get RTLD_NEXT symbols
#define _GNU_SOURCE

#include <dlfcn.h>
#include "systems_headers.h"
#include "linklist.h"
#include "anpwrapper.h"
#include "init.h"
#include "socket.h"
#include "tcp.h"
#include "utilities.h"
#include "subuff.h"
#include "ethernet.h"
#include "route.h"
#include "anp_netdev.h"

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
void assign_sockets(struct sock_info *current_si, const struct sockaddr *addr, socklen_t addrlen) {
    // retrieve port and ip and prepare current sock_info
    char rips[NI_MAXHOST], rports[NI_MAXSERV];
    int rc = getnameinfo(addr, addrlen, rips, sizeof(rips), rports, sizeof(rports),
                         NI_NUMERICHOST | NI_NUMERICSERV);
    printf("rc %i, host %s, port %s\n", rc, rips, rports);
    struct sockaddr_in sa;
    inet_pton(AF_INET, rips, &(sa.sin_addr));
    uint32_t rip = htonl(sa.sin_addr.s_addr);
    uint16_t rport = atoi(rports);
    current_si->rip = rip;
    current_si->rport = rport;

    // assign local ip and port
    char lips[NI_MAXHOST] = "10.0.0.4";
    struct sockaddr_in sa_loc;
    inet_pton(AF_INET, lips, &(sa_loc.sin_addr));
    uint32_t lip = htonl(sa_loc.sin_addr.s_addr);
    current_si->lip = lip;
    current_si->lport = rport;  // same as remote

    // debug
    debug_ip(current_si->lip);
    debug_ip(current_si->rip);
}

struct tcp_hdr *create_syn_tcp(const struct sock_info *si) {
    struct tcp_hdr *syntcp = calloc(1, sizeof(struct tcp_hdr));
    syntcp->src_port = ntohs(si->lport); //ntohs(53224);
    syntcp->dest_port = ntohs(si->rport); // ntohs(43211);
    syntcp->seq_num = htonl(92957434); //todo generate yourself and save SYN in sockinfo
    syntcp->ack_num = 0;
    syntcp->data_offset = 10;
    syntcp->reserved = 0;
    syntcp->syn = 1;
    syntcp->window = ntohs(64240);

    uint8_t *tcp_as_data = (uint8_t *) syntcp;

    char saddr[NI_MAXHOST] = "10.0.0.5";
    struct sockaddr_in saddr_loc;
    inet_pton(AF_INET, saddr, &(saddr_loc.sin_addr));
    uint32_t saddress = htonl(saddr_loc.sin_addr.s_addr);

    char daddr[NI_MAXHOST] = "10.0.0.4";
    struct sockaddr_in daddr_loc2;
    inet_pton(AF_INET, daddr, &(daddr_loc2.sin_addr));
    uint32_t daddrerss = htonl(daddr_loc2.sin_addr.s_addr);
    uint16_t csum = do_tcp_csum(tcp_as_data, 10, 6, saddress, daddrerss);
    syntcp->csum = ntohs(csum);

    return syntcp;
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

    struct sock_info *si = init_sock(); // save this sock_info in fd_cache
    add_sockfd_to_cache(si);
    return si->fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    printf("CONNECT CALLED:\n");

    if (!check_sockfd(sockfd)) {
        // the default path
        return _connect(sockfd, addr, addrlen);
    }

    // get our socket, populate with local ip/port and remote ip/port, set state CONNECTING
    struct sock_info *current_si = get_sock_info(sockfd);
    assign_sockets(current_si, addr, addrlen);
    current_si->state = SOCK_CONNECTING;

    // prepare TCP struct with related fields in correct network byte order  and checksum
    struct tcp_hdr *syntcp = create_syn_tcp(current_si);

    // -------- ???? ---------

//    struct subuff *sub = alloc_sub(ETH_HDR_LEN + TCP_LEN);
//    if (!sub) {
//        printf("Error: allocation of the arp sub in request failed \n");
//        return -ENOSYS;
//    }
//    sub_reserve(sub, ETH_HDR_LEN + TCP_LEN);
//    sub->protocol = htons(ETH_P_IP);

    struct rtentry *lo_rt = route_lookup(current_si->lip);
    printf("anp_r %hhx:%hhx:%hhx:%hhx:%hhx:%hhx \n",
           lo_rt->dev->hwaddr[0],
           lo_rt->dev->hwaddr[1],
           lo_rt->dev->hwaddr[2],
           lo_rt->dev->hwaddr[3],
           lo_rt->dev->hwaddr[4],
           lo_rt->dev->hwaddr[5]);

    // add proper IP and Ethernet headers
    // add to sub
    // send to tcp_tx (or just tod ip_output directly, up to you)

    return -ENOSYS;
}

// TODO: ANP milestone 5 -- implement the send, recv, and close calls
ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    //FIXME -- you can remember the file descriptors that you have generated in the socket call and match them here
    bool is_anp_sockfd = false;
    if (is_anp_sockfd) {
        //TODO: implement your logic here
        return -ENOSYS;
    }
    // the default path
    return _send(sockfd, buf, len, flags);
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
