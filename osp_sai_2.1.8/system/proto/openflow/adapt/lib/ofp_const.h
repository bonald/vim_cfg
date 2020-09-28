#ifndef __OFP_CONST_H__
#define __OFP_CONST_H__


#define OFP_DEFAULT_NETDEV_TYPE     "switch"
/* Added by weizj for tunnel */
#define OFP_L2GRE_NETDEV_TYPE       "l2gre"
#define OFP_NVGRE_NETDEV_TYPE       "nvgre"
#define OFP_VXLAN_NETDEV_TYPE       "vxlan"
/* End by weizj for tunnel */
#define OFP_BOND_NETDEV_TYPE        "bond"
#define OFP_STUB_NETDEV_TYPE        "stub"
#define OFP_AGG_NETDEV_TYPE        "agg"
#define OFP_IFNAME_SIZE             GLB_IFNAME_SZ
#define OFP_ETH_ADDR_LEN            MAC_ADDR_LEN

#define ETH_TYPE_IP            0x0800
#define ETH_TYPE_ARP           0x0806
#define ETH_TYPE_VLAN_8021Q    0x8100
#define ETH_TYPE_VLAN          ETH_TYPE_VLAN_8021Q
/* added by hansf for support push_vlan select ether_type*/
#define ETH_TYPE_VLAN_QINQ    0x9100
#define ETH_TYPE_VLAN_8021AD   0x88a8
#define ETH_TYPE_IPV6          0x86dd
#define ETH_TYPE_LACP          0x8809
#define ETH_TYPE_RARP          0x8035
#define ETH_TYPE_MPLS          0x8847
#define ETH_TYPE_MPLS_MCAST    0x8848

#define IP_PROTO_ICMP     1
#define IP_PROTO_IGMP     2
#define IP_PROTO_IP_IN_IP 4
#define IP_PROTO_TCP      6
#define IP_PROTO_UDP      17
#define IP_PROTO_RDP      27
#define IP_PROTO_DCCP     33
#define IP_PROTO_V6_IN_IP 41
#define IP_PROTO_GRE      47
#define IP_PROTO_SCTP     132

#define IPV6_PROTO_TCP          6
#define IPV6_PROTO_UDP          17
#define IPV6_PROTO_ICMP         58
#define IPV6_PROTO_GRE          47
#define IPV6_PROTO_IP_IN_IPV6   4
#define IPV6_PROTO_V6_IN_IPV6   41
#define IPV6_PROTO_RDP          27
#define IPV6_PROTO_SCTP     132


#define OFP_UINT8_MAX       0xff
#define OFP_UINT16_MAX      0xffff
#define OFP_UINT24_MAX      0xffffff      /* Added by weizj for tunnel */
#define OFP_UINT32_MAX      0xffffffff
#define OFP_UINT64_MAX      0xffffffffffffffffLLU

#define OFP_VLAN_MAX                 4094              /* Max VLANs.   */
#define OFP_VLAN_ALL                 (OFP_VLAN_MAX + 1)/* All VLANs.   */

/**
 @brief define direction
*/
enum ofp_direction_e
{
    OFP_INGRESS,        /**< Ingress direction */
    OFP_EGRESS,         /**< Egress direction */
    OFP_BOTH_DIRECTION  /**< Both Ingress and Egress direction */
};
typedef enum ofp_direction_e ofp_direction_t;

#endif  /* !__OFP_CONST_H__ */
