#!/bin/bash
doas ./bin/sh-make-tun-dev.sh
doas ./bin/sh-disable-ipv6.sh

# set errors in order to catch compilation stuff
set -e

doas cmake .
doas make
doas make install
doas ./bin/sh-run-arpserver.sh
doas ./bin/sh-hack-anp.sh ./arpdummy
