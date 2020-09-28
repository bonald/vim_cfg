
#ifndef _LCSH_CLI_MODE_TYPES_H
#define _LCSH_CLI_MODE_TYPES_H

/* CLI modes.  */
#define LOGIN_MODE              0   
#define AUTH_MODE               1
#define AUTH_ENABLE_MODE        2
#define SERVICE_MODE            3
#define EXEC_MODE               4
#define CONFIG_MODE             5
#define LINE_MODE               6
#define DNS_MODE                7
#define PPPOE_MODE              8
#define DHCP_MODE               9
#define DHCPS_MODE              10
#define DEBUG_MODE              11
    /* Keychain configuration.  */
#define KEYCHAIN_MODE           12
#define KEYCHAIN_KEY_MODE       13
    /* VR. */
#define VR_MODE                 14
    /* IP VRF.  */
#define VRF_MODE                15
    /* Interface.  */
#define INTERFACE_MODE          16 
#define INTERFACE_L2_FAST_MODE  17      
#define INTERFACE_L2_GIGE_MODE  18      
#define INTERFACE_L2_RANGE_MODE 19      
#define INTERFACE_L2_EXT_MODE   20
#define INTERFACE_MANAGE_MODE   21
    /* NSM MPLS node. */
#define NSM_MPLS_MODE           22 
    /* NSM VPLS node. */
#define NSM_VPLS_MODE           23 
    /* NSM MCAST node. */
#define NSM_MCAST_MODE          24 
    /* NSM MCAST6 node. */
#define NSM_MCAST6_MODE         25 

    /* Router ID/Hostname node. */
#define ROUTER_MODE             26
#define BGP_MODE                27
#define BGP_IPV4_MODE           28
#define BGP_IPV4M_MODE          29
#define BGP_IPV4_VRF_MODE       30
#define BGP_IPV6_MODE           31
#define BGP_VPNV4_MODE          32
#define RIP_MODE                33
#define RIP_VRF_MODE            34
#define RIPNG_MODE              35
#define OSPF_MODE               36
#define OSPF6_MODE              37
#define ISIS_MODE               38
#define ISIS_IPV6_MODE          39
#define PDM_MODE                40
#define PDM6_MODE               41
#define PIM_MODE                42
#define PIM6_MODE               43
#define DVMRP_MODE              44
#define VRRP_MODE               45
#define RMM_MODE                46
#define LDP_MODE                47
#define LDP_TRUNK_MODE          48
#define LDP_PATH_MODE           49
#define RSVP_MODE               50
#define RSVP_PATH_MODE          51
#define RSVP_TRUNK_MODE         52
    /* Static routes.  */
#define IP_MODE                 53
    /* Community list.  */
#define COMMUNITY_LIST_MODE     54
    /* Access list and prefix list.  */
#define PREFIX_MODE             55
#define ACCESS_MODE             56
#define L2_ACCESS_MODE          57
    /* IPv6 static routes.  */
#define IPV6_MODE               58
    /* IPv6 access list and prefix list.  */
#define ACCESS_IPV6_MODE        59
#define PREFIX_IPV6_MODE        60
    /* AS path access list.  */
#define AS_LIST_MODE            61
    /* Route-map.  */
#define RMAP_MODE               62
    /* User management. */
#define USER_MODE               63
    /* BGP dump mode. */
#define DUMP_MODE               64
    /* Layer 2 */
#define BRIDGE_MODE             65
#define VLAN_MODE               66
#define STP_MODE                67
#define RSTP_MODE               68
#define MSTP_MODE               69
#define MST_CONFIG_MODE         70
#define AUTH8021X_MODE          71
#define GVRP_MODE               72
#define GMRP_MODE               73
#define IGMP_MODE               74
    /* VTY */
#define VTY_MODE                75
    /* Fake modes used for config-write. */
#define SMUX_MODE               76
    /* Below is special modes.  */
#define EXEC_PRIV_MODE          77   /* Fake mode.  Same as EXEC_MODE.   */
#define IMISH_MODE              78   /* For shell and IMI communication.   */
#define MODIFIER_MODE           79   /* Output modifier node.  */
    /* QoS modes */
#define QOS_MODE                80
    /* Class Map mode */
#define CMAP_MODE               81
    /* Policy Map mode */
#define PMAP_MODE               82
    /* Policy Map Class mode */
#define PMAPC_MODE              83
#define IGMP_IF_MODE            84
#define ARP_MODE                85
/* Static multicast route mode */
#define IP_MROUTE_MODE          86
#define IPV6_MROUTE_MODE        87
#define VLAN_ACCESS_MODE        88
#define VLAN_FILTER_MODE        89
#define MAC_ACL_MODE            90
#define RMON_MODE               91
#define MLD_MODE                92
#define MLD_IF_MODE             93
/* IPSEC Transform Set represents a certain combination of
   security protocols and algorithms */
#define TRANSFORM_MODE          94
/* IPSEC CRYPTOMAP represents all parts needed to setup
   IPSec Security Associations and traffic to be protected
*/
#define CRYPTOMAP_MODE          95
#define ISAKMP_MODE             96
#define STACKING_MODE           97
#define RSVP_BYPASS_MODE        98
/* Firewall Mode */
#define FIREWALL_MODE           99
#define VRRP6_MODE              100

/*Modes for Metro Ethernet Enhancements */
#define ONMD_MODE               101
#define CVLAN_REGIST_MODE       102
#define ETH_CFM_MODE            103
#define VPS_MODE                 104

#define RSA_KEY_MODE            105
#define INTERNAL_DEBUG_MODE     106
#define DIAG_MODE               107
#define PORT_MODE               108

/* CFM enhancement to IEEE 802.1ag 2007 */
#define ETH_CFM_VLAN_MODE       109

#define OSPF6_AF_MODE           110
#define RIPNG_VRF_MODE          111
#define MAX_MODE                112

#endif /* _LCSH_CLI_MODE_TYPES_H */
