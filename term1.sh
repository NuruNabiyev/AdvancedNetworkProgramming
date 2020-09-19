#!/bin/bash
# this allows easily to compile and run server on terminal
cmake .
make
doas make install
./bin/sh-make-tun-dev.sh
./bin/sh-disable-ipv6.sh
./bin/sh-run-arpserver.sh
sudo ./bin/sh-hack-anp.sh ./arpdummy
