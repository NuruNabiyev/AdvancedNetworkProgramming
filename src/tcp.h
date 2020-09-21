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
    uint16_t doff_and_res; // data_offset 4bit and reserved 12bit, do we actually need it?
    uint16_t window;
    uint16_t csum;
    uint8_t  data[];
}__attribute__((packed));

void tcp_rx(struct subuff *sub);
void tcp_tx(struct subuff *sub);

#endif //ANPNETSTACK_TCP_H
