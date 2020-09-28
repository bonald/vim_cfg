/****************************************************************************
* $Id$
*  Centec Iptables related MACRO and ENUM, Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Dongya Zhang
* Date          : 2015-03-19 10:00
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_NS_PORT_FORWARDING_H__
#define __GLB_NS_PORT_FORWARDING_H__

#ifdef _GLB_UML_SYSTEM_
#define GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX     "ip netns exec mgmt iptables"
#define GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX_V6  "ip netns exec mgmt ip6tables"
#else
#define GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX     "ip netns exec mgmt xtables-multi iptables"
#define GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX_V6  "ip netns exec mgmt xtables-multi ip6tables"
#endif

#define GLB_NS_PORT_FORWARDING_CONNTRACK_MGMT_PREFIX  "ip netns exec mgmt conntrack"

#define GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ    1024  
#define GLB_NS_PORT_FORWARDING_IPTABLES_MAX_LINE_BUFSZ   256

#define GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_CMD_BUFSZ    1024  
#define GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_LINE_BUFSZ   256

/* Defined in cdb const. */
#if 0
typedef enum
{
    GLB_NS_PORT_FORWARDING_LOCAL = 0,
    GLB_NS_PORT_FORWARDING_REMOTE,
    GLB_NS_PORT_FORWARDING_MAX
} glb_ns_port_forwarding_type_t;
#endif

/* Link local address for veth pair used for port forwarding cross mgmt and default namespace. */

#define GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IP_ADDR "169.254.1.1"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT_IP_ADDR    "169.254.1.2"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT_IFNAME  "vemgmt"

#define GLB_NS_PORT_FORWARDING_VETH_DEFAULT2_IP_ADDR "169.254.2.1"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT2_IP_ADDR    "169.254.2.2"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT2_IFNAME  "vemgmt2"

#define GLB_NS_PORT_FORWARDING_VETH_DEFAULT3_IP_ADDR "169.254.3.1"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT3_IP_ADDR    "169.254.3.2"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT3_IFNAME  "vemgmt3"

#define GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IPV6_ADDR   "fec0:fffe:fffe::1:1"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT_IPV6_ADDR      "fec0:fffe:fffe::1:2"

#define GLB_NS_PORT_FORWARDING_VETH_DEFAULT2_IPV6_ADDR  "fec0:fffe:fffe::2:1"
#define GLB_NS_PORT_FORWARDING_VETH_MGMT2_IPV6_ADDR     "fec0:fffe:fffe::2:2"

#ifdef _GLB_UML_SYSTEM_
#define GLB_NS_PORT_FORWARDING_MGMT_IFNAME "eth99"
#else
#define GLB_NS_PORT_FORWARDING_MGMT_IFNAME "eth0"
#endif


#define GLB_NS_ROUTE_DEFAULT_IP_ADDR "169.254.2.1"
#define GLB_NS_ROUTE_MGMT_IP_ADDR    "169.254.2.2"
#define GLB_NS_ROUTE_MGMT_IFNAME     "vemgmt2"

#endif /* __GLB_NS_PORT_FORWARDING_H__ */


