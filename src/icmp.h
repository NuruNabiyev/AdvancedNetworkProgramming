#ifndef ANPNETSTACK_ICMP_H
#define ANPNETSTACK_ICMP_H

#include "systems_headers.h"
#include "subuff.h"

//https://www.iana.org/assignments/icmp-parameters/icmp-parameters.xhtml

#define ICMP_V4_REPLY           0x00
#define ICMP_V4_ECHO            0x08

// ICMP packet header https://www.researchgate.net/figure/ICMP-packet-structure_fig5_316727741
struct icmp {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint8_t  data[];
} __attribute__((packed));

void icmp_rx(struct subuff *sub);
void icmp_reply(struct subuff *sub);

#endif //ANPNETSTACK_ICMP_H
