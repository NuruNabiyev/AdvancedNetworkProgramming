#!/bin/bash
set -ex 
doas mknod /dev/net/tap c 10 200
doas chmod 0666 /dev/net/tap
