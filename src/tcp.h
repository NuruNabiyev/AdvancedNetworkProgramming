// Created by Nuru on 9/21/20.

#ifndef ANPNETSTACK_TCP_H
#define ANPNETSTACK_TCP_H

#include "systems_headers.h"
#include "subuff.h"

struct tcp_hdr {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    //uint8_t data_offset: 4;    // also indicated header length by *4
    //uint16_t res: 12;
    uint16_t dataoff_res;
    uint16_t window;
    uint16_t csum;
    uint16_t up;
    uint32_t opt_and_pad; // dont need, just eat it
    uint8_t  data[];
}__attribute__((packed));

void tcp_rx(struct subuff *sub);
void tcp_tx(struct subuff *sub);

inline void debug_tcp(struct tcp_hdr *tcp) {
    printf("TCP_DUMP: "
           "src_port %hu, dest_port %hu, "
           "seq %u, ack %u, "
           "data offset and reserved %hx, "
           "window %u, checksum %hx\n\n",
           htons(tcp->src_port), htons(tcp->dest_port),
           ntohl(tcp->seq_num), tcp->ack_num,   // todo ack should be converted too
           htons(tcp->dataoff_res), htons(tcp->window), htons(tcp->csum));
}

#endif //ANPNETSTACK_TCP_H
