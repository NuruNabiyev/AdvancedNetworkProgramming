#!/bin/bash
sudo ./bin/sh-make-tun-dev.sh
sudo ./bin/sh-disable-ipv6.sh

# set errors in order to catch compilation stuff
set -e

sudo cmake .
sudo make
sudo make install
sudo ./bin/sh-run-arpserver.sh
sudo ./bin/sh-hack-anp.sh ./arpdummy
