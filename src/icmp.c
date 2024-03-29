#include "icmp.h"
#include "ip.h"
#include "utilities.h"

void icmp_rx(struct subuff *sub) {
    // get icmp
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct icmp *ic = (struct icmp *) (ih->data);

    /*
     * Get checksum from ICMP Packet.
     * Reset & Recalculate it.
     * */
    uint16_t in_pkt_csum = ic->checksum;
    ic->checksum = 0;
    uint16_t csum = do_csum(ic, IP_PAYLOAD_LEN(ih), 0);
    /* Check if the checksum matches */
    if (csum != in_pkt_csum) {
        printf("Error: invalid checksum, dropping packet");
        goto drop_pkt;
    }
    switch (ic->type) {
        case ICMP_V4_ECHO:
            icmp_reply(sub);
            break;
        case ICMP_V4_REPLY:
            // we already processed the reply
            goto drop_pkt;
        default:
            printf("Error: Unknown IP header proto %d \n", ic->type);
            goto drop_pkt; 
    }
drop_pkt:
    free_sub(sub);
}

void icmp_reply(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct icmp *ic  = (struct icmp *)(ih->data);
    sub_reserve(sub, ETH_HDR_LEN + IP_HDR_LEN + IP_PAYLOAD_LEN(ih));
    sub_push(sub, IP_PAYLOAD_LEN(ih));

    sub->protocol = IPP_NUM_ICMP;

    ic->type = ICMP_V4_REPLY;
    ic->code = ICMP_V4_REPLY;

    ic->checksum = 0;
    ic->checksum = do_csum(ic, IP_PAYLOAD_LEN(ih), 0);
    ic->checksum = htons(ic->checksum);

    ip_output(ih->saddr, sub);
}
