#!/bin/bash
sudo ./bin/sh-make-tun-dev.sh
sudo ./bin/sh-disable-ipv6.sh

# set errors in order to catch compilation stuff
set -e

sudo cmake .
sudo make
sudo make install

# path to your client
anp_client_path="/home/nuru/Documents/simple-server-client-master/bin/anp_client"
myIp="192.168.178.52"  # use hostname -I and select first one
myPort="12345"         # note: should be same as servers?
sudo ./bin/sh-hack-anp.sh $anp_client_path $myIp $myPort