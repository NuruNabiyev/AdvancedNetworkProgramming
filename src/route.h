#ifndef ANPNETSTACK_ROUTE_H
#define ANPNETSTACK_ROUTE_H

#include "systems_headers.h"
#include "linklist.h"

#define RT_LOOPBACK 0x01
#define RT_GATEWAY  0x02
#define RT_HOST     0x04
#define RT_REJECT   0x08
#define RT_UP       0x10

struct rtentry {
    struct   list_head list;
    uint32_t dst;
    uint32_t gateway;
    uint32_t netmask;
    uint8_t  flags;
    struct   anp_netdev *dev;
};

void   route_init();
struct rtentry *route_lookup(uint32_t daddr);
void   free_routes();

#endif //ANPNETSTACK_ROUTE_H
