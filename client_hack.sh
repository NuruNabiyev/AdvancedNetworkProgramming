#!/bin/bash
doas ./bin/sh-make-tun-dev.sh
doas ./bin/sh-disable-ipv6.sh

# set errors in order to catch compilation stuff
set -e

doas cmake .
doas make
doas make install

# path to your client
anp_client_path="/home/nuru/Documents/simple-server-client-master/bin/anp_client"
myIp="192.168.178.52"  # use hostname -I and select first one
myPort="44556"         # use whatever easy for you
doas ./bin/sh-hack-anp.sh $anp_client_path $myIp $myPort