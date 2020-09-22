// Created by Nuru on 9/21/20.

#include "tcp.h"
#include "ip.h"
#include "utilities.h" // need bcause there is do_tcp_csum

void debug_tcp(struct tcp_hdr *tcp) {
    printf("TCP_DUMP: "
           "src_port %hu, dest_port %hu, "
           "seq %u, ack %u, "
           "data offset and reserved %hx, "
           "window %u, checksum %hx\n\n",
           htons(tcp->src_port), htons(tcp->dest_port),
           ntohl(tcp->seq_num), tcp->ack_num,   // todo ack should be converted too
           htons(tcp->dataoff_res), htons(tcp->window), htons(tcp->csum));
}

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    dump_hex(tcp, 40);
    debug_tcp(tcp);
}

void tcp_tx(struct subuff *sub) {
    // todo
}

