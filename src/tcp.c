// Created by Nuru on 9/21/20.

#include "systems_headers.h"
#include "tcp.h"
#include "ip.h"
#include "utilities.h" // need bcause there is do_tcp_csum
#include "anpwrapper.h"

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    printf("dump sub: ");
    dump_hex(sub, 70);

    printf("dump ip: ");
    dump_hex(ih, 70);
    dump_hex(tcp, 40);
    debug_tcp(tcp);


    if (tcp->syn == 1) {
        // socket(AF_INET, SOCK_STREAM, 0);
        // todo and connect?
    }
}

void tcp_tx(struct subuff *sub) {
    // todo
}

