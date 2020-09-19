#include "systems_headers.h"
#include "ip.h"
#include "utilities.h"
#include "route.h"
#include "subuff.h"
#include "anp_netdev.h"
#include "arp.h"

void ip_send_check(struct iphdr *ihdr)
{
    uint32_t csum = do_csum(ihdr, ihdr->ihl * 4, 0);
    ihdr->csum = csum;
}

int dst_neigh_output(struct subuff *sub)
{
    struct iphdr *iphdr = IP_HDR_FROM_SUB(sub);
    struct anp_netdev *anp_netdev = sub->dev;
    struct rtentry *rt = sub->rt;
    uint32_t dst_addr = ntohl(iphdr->daddr);
    uint32_t src_addr = ntohl(iphdr->saddr);

    uint8_t *target_dst_mac;

    if (rt->flags & RT_GATEWAY) {
        // in case, we are not briged but NAT'ed with a gateway
        dst_addr = rt->gateway;
    }
    target_dst_mac = arp_get_hwaddr(dst_addr);

    if (target_dst_mac) {
        return netdev_transmit(sub, target_dst_mac, ETH_P_IP);
    } else {
        arp_request(src_addr, dst_addr, anp_netdev);
        return -EAGAIN;
    }
}

int ip_output(uint32_t dst_ip_addr, struct subuff *sub)
{
    struct rtentry *rt;
    struct iphdr *ihdr = IP_HDR_FROM_SUB(sub);

    rt = route_lookup(dst_ip_addr);

    if (!rt) {
        printf("IP output route lookup failed \n");
        return -1;
    }

    sub->dev = rt->dev;
    sub->rt = rt;

    sub_push(sub, IP_HDR_LEN);

    ihdr->version = IPP_NUM_IP_in_IP;
    ihdr->ihl = 0x05;
    ihdr->tos = 0;
    ihdr->len = sub->len;
    ihdr->id = ihdr->id;
    ihdr->frag_offset = 0x4000;
    ihdr->ttl = 0;
    ihdr->proto = sub->protocol;
    ihdr->saddr = sub->dev->addr;
    ihdr->daddr = dst_ip_addr;
    ihdr->csum = 0;

    debug_ip_hdr("out", ihdr);

    ihdr->len = htons(ihdr->len);
    ihdr->id = htons(ihdr->id);
    ihdr->daddr = htonl(ihdr->daddr);
    ihdr->saddr = htonl(ihdr->saddr);
    ihdr->csum = htons(ihdr->csum);
    ihdr->frag_offset = htons(ihdr->frag_offset);

    ip_send_check(ihdr);
    return dst_neigh_output(sub);
}

