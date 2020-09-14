/*
 * Copyright [2020] [Animesh Trivedi]
 *
 * This code is part of the Advanced Network Programming (ANP) course
 * at VU Amsterdam.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *        http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "icmp.h"
#include "ip.h"
#include "utilities.h"

//FIXME: implement your ICMP packet processing implementation here
//figure out various type of ICMP packets, and implement the ECHO response type (icmp_reply)
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
  //FIXME: implement your ICMP reply implementation here
  // preapre an ICMP response buffer
  // send it out on ip_ouput(...)
  struct iphdr *ih = IP_HDR_FROM_SUB(sub);
  struct icmp *ic = (struct icmp *)(ih->data);
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
