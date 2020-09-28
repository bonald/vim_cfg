#!/bin/sh

# This script is used to setup veth pair used as proxy between default
# and mgmt namespaces.

# Note: should prefix each command with correct namespace to prevent do operation
# in wrong namespace.

# Enable mgmt ip_forward firstly.
ip netns exec mgmt sysctl -w net.ipv4.ip_forward=1
ip netns exec mgmt sysctl -w net.ipv6.conf.all.forwarding=1

sysctl -w net.ipv6.conf.all.disable_ipv6=0
sysctl -w net.ipv6.conf.default.disable_ipv6=0

# Support mappping same port on different ve address, up to 3 address are supported. 
VE_LIST="vedefault vedefault2"
BASE_ADDR_VAR=1
for VEDEFAULT in $VE_LIST; do
    VEMGMT=${VEDEFAULT/default/mgmt}
    # Create veth pair and move the vemgmt to mgmt namespace.
    ip netns exec default ip link add $VEDEFAULT type veth  peer name $VEMGMT
    VEDEFAULT_MAC=`ip netns exec default cat /sys/class/net/$VEDEFAULT/address`
    VEMGMT_MAC=`ip netns exec default cat /sys/class/net/$VEMGMT/address`
    # Move vemgmt* to mgmt namespace.
    ip netns exec default ip link set $VEMGMT netns mgmt
    # Bring up the interface.
    ip netns exec default ip link set $VEDEFAULT up
    ip netns exec mgmt ip link set $VEMGMT up
    # Assign link local address to the veth pair.
    ip netns exec default ip addr add 169.254.${BASE_ADDR_VAR}.1/24 dev $VEDEFAULT
    ip netns exec mgmt ip addr add 169.254.${BASE_ADDR_VAR}.2/24 dev $VEMGMT
    # Set static arp permanently.
    ip netns exec default ip neigh add 169.254.${BASE_ADDR_VAR}.2 lladdr $VEMGMT_MAC dev $VEDEFAULT
    ip netns exec mgmt ip neigh add 169.254.${BASE_ADDR_VAR}.1 lladdr $VEDEFAULT_MAC dev $VEMGMT
    # Assign IPv6 link local address to the veth pair.
    ip netns exec default ip -6 addr add fec0:fffe:fffe::${BASE_ADDR_VAR}:1/112 dev $VEDEFAULT
    ip netns exec mgmt ip -6 addr add fec0:fffe:fffe::${BASE_ADDR_VAR}:2/112 dev $VEMGMT
    # Set static arp permanently.
    ip netns exec default ip neigh add fec0:fffe:fffe::${BASE_ADDR_VAR}:2 lladdr $VEMGMT_MAC dev $VEDEFAULT
    ip netns exec mgmt ip neigh add fec0:fffe:fffe::${BASE_ADDR_VAR}:1 lladdr $VEDEFAULT_MAC dev $VEMGMT
    # Incr addr base.
    let BASE_ADDR_VAR=BASE_ADDR_VAR+1    
done
