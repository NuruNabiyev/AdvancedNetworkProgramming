#ifndef ANPNETSTACK_CONFIG_H
#define ANPNETSTACK_CONFIG_H

#define ANP_IP_CLIENT_EXT   "10.0.0.4"
#define ANP_IP_LO       "127.0.0.1"
#define ANP_IP_TAP_DEV  "10.0.0.5"

#define ANP_SUBNET_TAP   "10.0.0.0/24"

#define ANP_MAC_CLIENT_LO "00:00:00:00:00:00"
#define ANP_MAC_CLIENT_EXT   "de:ad:be:ef:aa:aa"

#define ANP_MTU_15   1500
#define ANP_MTU_65K  65536
#define ANP_MTU_9K   9000

//https://searchnetworking.techtarget.com/answer/Minimum-and-maximum-Ethernet-frame-sizes
#define ANP_MTU_15_MAX_SIZE 1522

#define TCP_MSL_MSECS 2500 //120000

// TCP specific parameters

// Default to 536 as per spec
// https://tools.ietf.org/html/rfc879
#define TCP_MSS               8
#define TCP_INIT_SND_SEQ      32
#define TCP_INIT_RCV_SEQ       8
#define TCP_INIT_SEND_WND     64
#define TCP_INIT_RCV_WND      8

//https://tools.ietf.org/html/rfc6298
#define TCP_BASIC_RTO_MSEC 10

// https://tools.ietf.org/html/rfc813 <-- window and ACK strategies
#define TCP_MAX_SEGMENT_GROUPS 3

#endif //ANPNETSTACK_CONFIG_H
