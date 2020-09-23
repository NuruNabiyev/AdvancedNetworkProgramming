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
    uint8_t reserved: 4,
            data_offset: 4;
    uint8_t fin: 1,
            syn: 1,
            rst: 1,
            push: 1,
            ack: 1,
            urg: 1,
            extra: 2;
    uint16_t window;
    uint16_t csum;
    uint16_t up;
    uint32_t opt_and_pad; // dont need, just eat it
    uint8_t data[];
}__attribute__((packed));

/**
 * Receives incoming tcp packet from IP layer
 * @param sub buffer received
 */
void tcp_rx(struct subuff *sub);

/**
 * Sends buffer back to IP layer for further processing todo
 * @param sub buffer to be st with all proper headers
 */
void tcp_tx(struct subuff *sub);

#endif //ANPNETSTACK_TCP_H
