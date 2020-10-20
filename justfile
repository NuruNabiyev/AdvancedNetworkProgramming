timestamp := `date +%s`
new_port := `python3 -c 'import socket; s=socket.socket(); s.bind(("", 0)); print(s.getsockname()[1]); s.close()'`
current_port := `netstat -lt | rg 'smolfrosty' | cut -d':' -f2,5 | awk -F ' ' '{print $1}'`

default:
    @just --choose

webserver:
    python3 -m http.server 8000

tundev:
    #!/bin/bash
    doas mknod /dev/net/tap c 10 200
    doas chmod 0666 /dev/net/tap

prep:
    just pfwd
    just ipv6
    just tundev

ipv6:
    #!/bin/bash
    doas sysctl -w net.ipv6.conf.all.disable_ipv6=1
    doas sysctl -w net.ipv6.conf.default.disable_ipv6=1
    doas sysctl -w net.ipv6.conf.lo.disable_ipv6=1

pfwd:
    #!/bin/bash
    doas sysctl -w net.ipv4.ip_forward=1
    doas iptables -I INPUT --source 10.0.0.0/24 -j ACCEPT
    doas iptables -t nat -I POSTROUTING --out-interface $CONNECT_OUT -j MASQUERADE
    doas iptables -I FORWARD --in-interface $CONNECT_OUT --out-interface tap0 -j ACCEPT
    doas iptables -I FORWARD --in-interface tap0 --out-interface $CONNECT_OUT -j ACCEPT

arp:
    gcc ./arpdummy.c -o arpdummy

serve:
    nohup ~/gits/anp-netstack/bin/anp_server $IP {{new_port}} &

makerun:
    doas cmake .
    doas make
    doas make install
    doas ./bin/sh-hack-anp.sh $SIMPLE_CLIENT $IP {{current_port}}

makeloop:
    watchexec -c -w .git -- just makerun

benchmark:
    just makerun >> benchmark_results/{{timestamp}} 

onerun:
    just serve
    just benchmark

sample:
    for i in `seq 100`; do \
        just onerun;      \
    done

hack:
    just tundev
    just ipvs

    just makeloop


