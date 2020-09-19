#!/bin/bash
set -ex
if [ $# -ne 1 ]; then 
	echo "which device?"
	exit 1;
fi 
echo "setting up with $1 , on the tun subet of 10.0.0.0/24" 

doas isysctl -w net.ipv4.ip_forward=
doas iptables -I INPUT --source 10.0.0.0/24 -j ACCEPT
doas iptables -t nat -I POSTROUTING --out-interface $1 -j MASQUERADE
doas iptables -I FORWARD --in-interface $1 --out-interface tap0 -j ACCEPT
doas iptables -I FORWARD --in-interface tap0 --out-interface $1 -j ACCEPT
