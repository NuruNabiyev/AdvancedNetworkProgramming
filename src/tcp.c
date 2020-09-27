// Created by Nuru on 9/21/20.

#include "systems_headers.h"
#include "tcp.h"
#include "ip.h"
#include "utilities.h" // need bcause there is do_tcp_csum
#include "anpwrapper.h"

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    printf("dump sub: ");
    dump_hex(sub, 170);

    printf("dump ip: ");
    dump_hex(ih, 70);
    dump_hex(tcp, 40);
    debug_tcp(tcp);
    
    if(tcp->ack == 1 && tcp->syn == 1){
        // calculate the checksum
        uint16_t packet_csum = tcp->csum;
        tcp->csum = 0;
        uint16_t check_csum = do_tcp_csum((uint8_t *) tcp, 24, IPP_TCP, ntohl(ih->saddr), ntohl(ih->daddr)); 
        printf("--------- CHECKSUM=%hx! --------\n", htons(check_csum));

        if(packet_csum != check_csum) goto dropkt;

        //... send the final ACK here
    }

dropkt:
    freesub(sub);
}

void tcp_tx(struct subuff *sub) {
    // todo
}

