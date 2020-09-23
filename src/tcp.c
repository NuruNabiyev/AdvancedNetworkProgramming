// Created by Nuru on 9/21/20.

#include "tcp.h"
#include "ip.h"
#include "utilities.h" // need bcause there is do_tcp_csum
#include "anpwrapper.h"

void debug_tcp(struct tcp_hdr *tcp) {
    printf("TCP_DUMP: "
           "src_port %hu, dest_port %hu, "
           "seq_num %u, ack_num %u, "
           "data offset %i bytes, reserved %i "
           "urg %i ack %i push %i rst %i syn %i, fin %i extra %i, "
           "window %u, checksum %hx\n\n",
           htons(tcp->src_port), htons(tcp->dest_port),
           ntohl(tcp->seq_num), tcp->ack_num,   // todo ack should be converted too
           tcp->data_offset * 4, tcp->reserved,
           tcp->urg, tcp->ack, tcp->push, tcp->rst, tcp->syn, tcp->fin, tcp->extra,
           htons(tcp->window), htons(tcp->csum));
}

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    dump_hex(tcp, 40);
    debug_tcp(tcp);

    // todo if syn is 1 and other syuff... then go to anpwrapper connect()
}

void tcp_tx(struct subuff *sub) {
    // todo
}

