#!/bin/sh

#echo "Create default and mgmt ns"
ip netns add mgmt

mkdir -p /var/run/netns/
touch /var/run/netns/default
mount --bind /proc/1/ns/net /var/run/netns/default

ip link set eth0 netns mgmt
ip netns exec mgmt ifconfig eth0 up
ip netns exec mgmt ifconfig lo 127.0.0.1 up
