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
    // todo compare checksum, return if not correct

    // get icmp
    struct iphdr *ih = IP_HDR_FROM_SUB(sub);
    struct icmp *ic = (struct icmp *) (ih->data);

    // todo check if type is 8 -> icmp_reply
    printf("\nICMP type is %i; code %i; check %hu; data %s\n",
           ic->type, ic->code, ic->checksum, ic->data);
    free_sub(sub);
}

void icmp_reply(struct subuff *sub) {
    //FIXME: implement your ICMP reply implementation here
    // preapre an ICMP response buffer
    // send it out on ip_ouput(...)
}
