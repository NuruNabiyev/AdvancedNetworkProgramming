//XXX: _GNU_SOURCE must be defined before including dlfcn to get RTLD_NEXT symbols
#define _GNU_SOURCE

#include <dlfcn.h>
#include "systems_headers.h"
#include "linklist.h"
#include "anpwrapper.h"
#include "init.h"
#include "socket.h"

static int (*__start_main)(int (*main) (int, char **, char **), int argc, \
        char ** ubp_av, void (*init) (void), void (*fini) (void),            \
        void (*rtld_fini) (void), void (*stack_end));

static ssize_t (*_send)(int fd, const void *buf, size_t n, int flags) = NULL;
static ssize_t (*_recv)(int fd, void *buf, size_t n, int flags)       = NULL;

static int (*_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = NULL;
static int (*_socket)(int domain, int type, int protocol) = NULL;
static int (*_close)(int sockfd) = NULL;

static int is_socket_supported(int domain, int type, int protocol) {
    printf("static int is_socket_supported(int domain %d , int type %d, int protocol %d)\n", domain, type, protocol);
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

/**
 * Initializes socket
 * @param domain AF_INET
 * @param type SOCK_STREAM
 * @param protocol 0
 * @return File descriptor
 */
int socket(int domain, int type, int protocol) {
    printf("int socket(int domain %d, int type %d, int protocol %d) \n",domain, type, protocol);
    if (!is_socket_supported(domain, type, protocol)) {
        // if this is not what anpnetstack support, let it go, let it go!
        return _socket(domain, type, protocol);
    }

    struct sock_info *si = init_sock(); // save this sock_info in fd_cache
    add_sockfd_to_cache(si);
    return si->fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    printf("CONNECT CALLED\n");

    if (!check_sockfd(sockfd)) {
        // the default path
        return _connect(sockfd, addr, addrlen);
    }

    // get our socket
    // populate with local ip/port and remote ip/port
    // set state CONNECTING

    // do 3way handshake
    // prepare TCP struct with related fields in correct network byte order  and checksum
    // add proper IP and Ethernet headers
    // add to sub
    // send to tcp_tx (or just ip_output)

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

ssize_t recv (int sockfd, void *buf, size_t len, int flags) {
    //FIXME -- you can remember the file descriptors that you have generated in the socket call and match them here
    bool is_anp_sockfd = false;
    if (is_anp_sockfd) {
        //TODO: implement your logic here
        return -ENOSYS;
    }
    // the default path
    return _recv(sockfd, buf, len, flags);
}

int close (int sockfd) {
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
    _socket      = dlsym(RTLD_NEXT, "socket");
    _connect     = dlsym(RTLD_NEXT, "connect");
    _send        = dlsym(RTLD_NEXT, "send");
    _recv        = dlsym(RTLD_NEXT, "recv");
    _close       = dlsym(RTLD_NEXT, "close");
}
