#ifndef ANPNETSTACK_ANP_NETDEV_H
#define ANPNETSTACK_ANP_NETDEV_H

#include <stdint.h>
#include "subuff.h"

struct eth_hdr;

struct anp_netdev {
  uint32_t addr;
  uint8_t  addr_len;
  uint8_t  hwaddr[6];
  uint32_t mtu;
};

struct anp_netdev* netdev_get(uint32_t sip);
void   client_netdev_init();
void   *netdev_rx_loop();
void   free_netdev();
int    netdev_transmit(struct subuff *skb, uint8_t *dst, uint16_t ethertype);

#endif //ANPNETSTACK_ANP_NETDEV_H
