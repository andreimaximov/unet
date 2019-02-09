#!/bin/bash

# Setup a Linux TAP device. Couples of notes on this...
#
# 1) Our network topology looks roughly like so...
#
#    [ Stack @ 10.255.255.102/24 ]  <->  [ VM tap0 @ 10.255.255.101/24 ]
#                                                        |
#                                        [  VM Kernel IPv4 Forwarding  ]
#                                                        |
#    [          Host             ]  <->  [          VM eth0 @ ?        ]
#
#    Here "Stack" is the unet network stack running in a process on the VM. We
#    enable NAT forwarding so the VM kernel can act as a router/NAT for the
#    network stack.
#
# 2) tap0 will only receive frames if it is UP. As of Linux kernel 2.6.36 TAP
#    interfaces are UP only if a program has opened the interface. You can run
#    the ./stack example to bring tap0 UP.
#
# 3) Do not use the same MAC address for Stack as tap0. Otherwise tap0 swallows
#    frames (or something like that...).
#
# - https://wiki.archlinux.org/index.php/Iptables
# - https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/4/html/Security_Guide/s1-firewall-ipt-fwd.html

if [ -d /sys/class/net/tap0 ]; then
    exit 0
fi

if [ -z "$TRAVIS_OS_NAME" ]; then
IFR="eth0";
else
IFR="ens4";
fi

# Show debug info...
echo "sudo ip addr:"
sudo ip addr
echo "sudo ip link:"
sudo ip link
echo "IFR=$IFR"

# Setup tap0...
sudo ip tuntap add name tap0 mode tap user $USER
sudo ip addr add 10.255.255.101/24 dev tap0
sudo ip link set tap0 up

# Enable IPv4 forwarding and NAT routing...
sudo iptables -t nat -A POSTROUTING -o $IFR -j MASQUERADE
sudo sysctl net.ipv4.ip_forward=1
