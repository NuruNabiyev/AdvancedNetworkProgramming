tundev:
    #!/bin/bash
    doas mknod /dev/net/tap c 10 200
    doas chmod 0666 /dev/net/tap

ipvs:
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
    watchexec -c -e c ~/gits/anp-netstack/bin/anp_server $IP $PORT

makerun:
    doas cmake .
    doas make
    doas make install
    doas ./bin/sh-hack-anp.sh $SIMPLE_CLIENT $IP $PORT

makeloop:
    watchexec -c -e c just makerun

hack:
    just tundev
    just ipvs

    just makeloop


