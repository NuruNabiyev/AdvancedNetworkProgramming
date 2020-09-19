#/bin/bash
set -e
echo "
#include <unistd.h>
#include <limits.h>

int main(int argv, char **argc){
    return sleep(INT_MAX);
}" > ./arpdummy.c
set -x
gcc ./arpdummy.c -o arpdummy
set +x
echo "-------------------------------------------"
echo "The ANP netstack is ready, now try this:"
echo " 1. [terminal 1] ./sh-hack-anp.sh ./arpdummy"
echo " 2. [terminal 2] try running arping 10.0.0.4"
