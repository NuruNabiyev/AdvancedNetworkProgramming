#!/bin/bash
read lower_port upper_port < /proc/sys/net/ipv4/ip_local_port_range
while :; do
    for (( port = lower_port ; port <= upper_port ; port++ )); do
        nc -l -p "$port" 2>/dev/null && break 2
    done
done
