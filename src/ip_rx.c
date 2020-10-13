#include "ip.h"
#include "systems_headers.h"
#include "utilities.h"
#include "icmp.h"
#include "tcp.h"

int ip_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    uint16_t csum = -1;

    if (ih->version != IPP_NUM_IP_in_IP) {
        printf("IP packet is not IP\n");
        goto drop_pkt;
    }

    if (ih->ihl < 5) {
        printf("IP packet header is too short, expected atleast 20 bytes, got %d \n", ((ih->ihl)<<2));
        goto drop_pkt;
    }

    if (ih->ttl == 0) {
        printf("ERROR: zero time to live, ttl, dropping packet \n");
        goto drop_pkt;
    }
    //TODO check overhead of doing checksum?
    csum = do_csum(ih, ih->ihl * 4, 0);

    if (csum != 0) {
        printf("Error: invalid checksum, dropping packet");
        goto drop_pkt;
    }

    ih->saddr = ntohl(ih->saddr);
    ih->daddr = ntohl(ih->daddr);
    ih->len   = ntohs(ih->len);
    ih->id    = ntohs(ih->id);

    // TODO delete later because i receive these dns
    if (ih->proto == 17 || ih->proto == 2) {
        printf("nope (%i)", ih->proto);
        return 0;
    }

    /* debug_ip_hdr("in", ih); */

    switch (ih->proto) {
        case IPP_NUM_ICMP:
            /* debug_ip("incoming ICMP packet\n"); */
            icmp_rx(sub);
            return 0;
        case IPP_TCP:
            /* debug_ip("incoming TCP packet\n"); */
            /* TODO:  <13-10-20,matthiasdebernardini> moment we find out we have a TCP*/
            tcp_rx(sub);
            goto drop_pkt;
        default:
            printf("Error: Unknown IP header proto %d \n", ih->proto);
            goto drop_pkt;
    }
drop_pkt:
    free_sub(sub);
    return 0;
}

