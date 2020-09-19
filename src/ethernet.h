#include <stdint.h>

#ifndef ANP_NETSTACK_ETHERNET_H
#define ANP_NETSTACK_ETHERNET_H

#include "subuff.h"

struct eth_hdr {
    uint8_t  dmac[6];
    uint8_t  smac[6];
    uint16_t ethertype;
    uint8_t  payload[];
} __attribute__((packed));

#define ETH_HDR_LEN sizeof(struct eth_hdr)

#ifdef DEBUG_ETH
#define eth_debug(msg, hdr)                                               \
    do {                                                                \
        printf("eth (HDR: %lu) "msg" ("                                       \
                "dmac: %.2hhx:%.2hhx:%.2hhx:%.2hhx:%.2hhx:%.2hhx, " \
                "smac: %.2hhx:%.2hhx:%.2hhx:%.2hhx:%.2hhx:%.2hhx, " \
                "ethertype: %.4hx)\n",                               \
                ETH_HDR_LEN, \
                hdr->dmac[0], hdr->dmac[1], hdr->dmac[2], hdr->dmac[3], \
                hdr->dmac[4], hdr->dmac[5], hdr->smac[0], hdr->smac[1], \
                hdr->smac[2], hdr->smac[3], hdr->smac[4], hdr->smac[5], hdr->ethertype); \
    } while (0)
#else
#define eth_debug(msg, hdr)
#endif

static inline struct eth_hdr *eth_hdr(struct subuff *sub) {
    struct eth_hdr *hdr = (struct eth_hdr *)sub_head(sub);
    // we need to reverse the byte order
    hdr->ethertype = ntohs(hdr->ethertype);
    return hdr;
}

#endif //ANP_NETSTACK_ETHERNET_H
