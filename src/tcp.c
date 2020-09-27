// Created by Nuru on 9/21/20.

#include "systems_headers.h"
#include "tcp.h"
#include "ip.h"
#include "utilities.h" // need bcause there is do_tcp_csum
#include "anpwrapper.h"
#include "socket.h"
#include "socket.h"

// todo this is going to be initialized to false again once another tcp comes in
//  so we should use proper pthread locks
volatile bool server_synack_ok = false;

void tcp_rx(struct subuff *sub) {
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct tcp_hdr *tcp = (struct tcp_hdr *) (ih->data);

    dump_hex(tcp, 40);
    debug_tcp(tcp);

    if (tcp->ack == 1 && tcp->syn == 1) {
        // check the checksum
        uint16_t packet_csum = tcp->csum;
        tcp->csum = 0;
        uint16_t check_csum = do_tcp_csum((uint8_t *) tcp, 24, IPP_TCP, ntohl(ih->saddr), ntohl(ih->daddr));
        if (packet_csum != check_csum) goto dropkt;

        // check our sequence from SYN to this SYN-ACK
        struct sock_info *si = get_sock_info_by_seq(ntohl(tcp->ack_num) - 1);
        if (si == NULL) {
            printf("\n---NO SUCH ITEM FOUND---\n");
            goto dropkt;
        }
        printf("--------- CHECKSUM=%hx! --------\n", htons(check_csum));
        server_synack_ok = true;

        //... send the final ACK here
    }

    dropkt:
    // todo if checksum is not correct and we are dropping packet,
    //  then we should notify connect() to re-transmit
    printf("todo drop packet");
    //freesub(sub); // this throws error
}

void tcp_tx(struct subuff *sub) {
    // todo
}

