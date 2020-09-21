// Created by Nuru on 9/21/20.

#include "tcp.h"
#include "ip.h"
#include "utilities.h" // need bcause there is do_tcp_csum

void dump_hex(void *something, int len){
    printf("DUMPING_HEX: [");
    for(int i = 0; i < len; i++)
    {
        printf("%02x ", ((unsigned char*)something)[i]);
    }
    printf("]\n");
}

void print_tcp(struct tcp_hdr *tcp) {
    printf("TCP_DUMP: "
           "src_port: %hu, dest_port %hu, "
           "\n\n",
           htons(tcp->src_port), htons(tcp->dest_port)
           // todo print seq, ack
           );
}

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    dump_hex(tcp, 40);
    print_tcp(tcp);
}

void tcp_tx(struct subuff *sub) {
    // todo
}

