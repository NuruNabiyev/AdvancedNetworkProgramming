#!/bin/bash
sudo ./bin/sh-make-tun-dev.sh
sudo ./bin/sh-disable-ipv6.sh

# set errors in order to catch compilation stuff
set -e

sudo cmake .
sudo make
sudo make install

# path to your client
anp_client_path="/home/m/gits/anp-netstack/bin/anp_client"
myIp="192.168.178.115"  # use hostname -I and select first one
myPort="12345"
sudo ./bin/sh-hack-anp.sh $anp_client_path $myIp $myPort
