# Advanced Network Programming (ANP) Skeleton 

ANP skeleton is the basic skeleton code used in the ANP course for developing your 
own networking stack. 

## Code 
Corresponding to the figure 3 in the accompanying assignment, here is a brief 
overview of the various files in this project 

  * anp_netdev.[ch] : implements the application privide network interface with 10.0.0.4 IP address 
  * anpwrapper.[ch] : shared library wrapper code that provides hooks for socket, connect, send, recv, and close calls. 
  * arp.[ch] : ARP protocol implementation. 
  * config.h : various configurations and variables used in the project.
  * debug.h : some basic debugging facility. Feel free to enhance it as you see fit. 
  * ethernet.h : Ethernet header definition  
  * icmp.[ch] : ICMP implementation (your milestone 2). 
  * init.[ch] : various initialization routines. 
  * ip.h : IP header definition 
  * ip rx and rx : IP tranmission and reception paths 
  * linklist.h : basic data structure implementation that you can use to keep track of various networking states (e.g., unacknowledged packets, open connections).
  * route.[ch] : a basic route cache implementation that can be used to find MAC address for a given IP (linked with the ARP implementation).
  * subuffer.[ch] : Linux kernel uses Socket Kernel Buffer (SKB) data strucutre to keep track of packets and data inside the kernel (http://vger.kernel.org/~davem/skb.html). This is our implementation of Socket Userspace Buffer (subuff). It is mostly used to build inplace packet headers.
  * systems_headers.h : a common include file with many commonly used headers. 
  * tap_netdev.[ch] : implementation for sending/receiving packets on the TAP device. It is pretty standard code. 
  * timer.[ch] : A very basic timer facility implementation that can be used to register a callback to do asynchronous processing. Mostly useful in timeout conditions. 
  * utilities.[ch] : various helper routines, printing, debugging, and checksum calculation.           


***You are given quite a bit of skeleton code as a starting point. 
Feel free not to use or use any portion of it when developing your stack. 
You have complete freedom***  
  
 ## How to build 
 
 ```bash
 cmake . 
 make 
 sudo make install  
 ```
 
 This will build and install the shared library. 
 
 ## Scripts 
 
 * sh-make-tun-dev.sh : make a new TUN/TAP device 
 * sh-disable-ipv6.sh : disable IPv6 support 
 * sh-setup-fwd.sh : setup packet forwarding rules from the TAP device to the outgoing interface. This script takes the NIC name which has connectivity to the outside world.  
 * sh-run-arpserver.sh : compiles a dummy main program that can be used to run the shared library to run the ARP functionality 
 * sh-hack-anp.sh : a wrapper library to preload the libanpnetstack and take over networking calls. 
 
 # Setup 
 After a clean reboot, run following scripts in the order 
  1. Make a TAP/TUN device 
  2. Disable IPv6 
  3. Setup packet forwarding rules
 
  
 ## To run ARP setup 
 Make and install the libanpnetstack library. Then run `./sh-run-arpserver.sh` and follow instructions from there. Example 
 
 ```bash
 atr@atr:~/anp-netskeleton/bin$ ./sh-run-arpserver.sh 
 ++ gcc ./arpdummy.c -o arpdummy
 ++ set +x
 -------------------------------------------
 The ANP netstack is ready, now try this:
  1. [terminal 1] ./sh-hack-anp.sh ./arpdummy
  2. [terminal 2] try running arping 10.0.0.4
 
 atr@atr:~/anp-netskeleton/bin$ sudo ./sh-hack-anp.sh ./arpdummy
 [sudo] password for atr: 
 + prog=./arpdummy
 + shift
 + LD_PRELOAD=/usr/local/lib/libanpnetstack.so ./arpdummy
 Hello there, I am ANP networking stack!
 tap device OK, tap0 
 Executing : ip link set dev tap0 up 
 OK: device should be up now, 10.0.0.0/24 
 Executing : ip route add dev tap0 10.0.0.0/24 
 OK: setting the device route, 10.0.0.0/24 
 Executing : ip address add dev tap0 local 10.0.0.5 
 OK: setting the device address 10.0.0.5 
 GXXX  0.0.0.0
 GXXX  0.0.0.0
 GXXX  10.0.0.5
 [ARP] A new entry for 10.0.0.5
 ARP an entry updated 
 ARP an entry updated 
 ^C
 atr@atr:~/anp-netskeleton/bin$
  ```
From a second terminal 
  
```bash
atr@atr:~/home/atr$ arping -I tap0 10.0.0.4 
ARPING 10.0.0.4 from 10.0.0.5 tap0
Unicast reply from 10.0.0.4 [??:??:??:??:??]  0.728ms
Unicast reply from 10.0.0.4 [??:??:??:??:??]  0.922ms
Unicast reply from 10.0.0.4 [??:??:??:??:??]  1.120ms
^CSent 3 probes (1 broadcast(s))
Received 3 response(s) 
```
In place of [??:??:??:??:??] you should see an actual mac address for the TAP device. 

## Getting started with hijacking socket call 

  * Step 1: run the TCP server in one terminal with a specific IP and port number 
  * Step 2: run the TCP client in another terminal, and first check if they connect, run, and pass the buffer matching test.
  
 Then, you can run your client with the `./bin/sh-hack-anp.sh` script as 
 ```bash
 sudo [path_to_anp]/bin/sh-hack-anp.sh [path]/bin/anp_client IP port 
``` 

In case you have some non-standard installation path, please 
update the path `/usr/local/lib/libanpnetstack.so` in the `sh-hack-anp.sh` script.


## Author 
Animesh Trivedi (for the ANP course) 