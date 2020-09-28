/**
 @file ctc_port.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-9

 @version v2.0

 This file contains all port related data structure, enum, macro and proto.

*/

#ifndef _CTC_PORT_H
#define _CTC_PORT_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "common/include/ctc_const.h"
#include "common/include/ctc_common.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup port PORT
 @{
*/

#define CTC_PORT_PREFIX_PORT_MAC_NUM    2
#define CTC_PORT_CLASS_ID_RSV_VLAN_CLASS 62
#define CTC_PORT_CLASS_ID_RSV_IP_SRC_GUARD 63
#define CTC_PORT_SERDES_MAX_NUM         4


/**
 @brief Port vlan tag ctl
*/
enum ctc_vlantag_ctl_e
{
    CTC_VLANCTL_ALLOW_ALL_PACKETS,                            /**< [GB.GG.D2.TM] Allow all packet regardless of vlan tag*/
    CTC_VLANCTL_DROP_ALL_UNTAGGED,                            /**< [GB.GG.D2.TM] Drop all untagged packet*/
    CTC_VLANCTL_DROP_ALL_TAGGED,                                /**< [GB.GG.D2.TM] Drop all tagged packet*/
    CTC_VLANCTL_DROP_ALL,                                             /**< [GB.GG.D2.TM] Drop all packet*/
    CTC_VLANCTL_DROP_PACKET_WITHOUT_TWO_TAG,        /**< [GB.GG.D2.TM] Drop packet without double tagged*/
    CTC_VLANCTL_DROP_ALL_PACKET_WITH_TWO_TAG,       /**< [GB.GG.D2.TM] Drop packet with double tagged*/
    CTC_VLANCTL_DROP_ALL_SVLAN_TAG,                          /**< [GB.GG.D2.TM] Drop packet with stag*/
    CTC_VLANCTL_DROP_ALL_NON_SVLAN_TAG,                  /**< [GB.GG.D2.TM] Drop packet without stag*/
    CTC_VLANCTL_DROP_ONLY_SVLAN_TAG,                       /**< [GB.GG.D2.TM] Drop packet with only stag*/
    CTC_VLANCTL_PERMIT_ONLY_SVLAN_TAG,                    /**< [GB.GG.D2.TM] Permit packet with only stag*/
    CTC_VLANCTL_DROP_ALL_CVLAN_TAG,                         /**< [GB.GG.D2.TM] Drop packet with ctag*/
    CTC_VLANCTL_DROP_ALL_NON_CVLAN_TAG,                 /**< [GB.GG.D2.TM] Drop packet without ctag*/
    CTC_VLANCTL_DROP_ONLY_CVLAN_TAG,                       /**< [GB.GG.D2.TM] Drop packet with only ctag*/
    CTC_VLANCTL_PERMIT_ONLY_CVLAN_TAG,                     /**< [GB.GG.D2.TM] Permit packet with only ctag*/

    MAX_CTC_VLANTAG_CTL
};
typedef enum ctc_vlantag_ctl_e ctc_vlantag_ctl_t;

/**
 @brief Port dot1q type
*/
enum ctc_dot1q_type_e
{
    CTC_DOT1Q_TYPE_NONE,        /**< [GB.GG.D2.TM] Packet transmit out with untag*/
    CTC_DOT1Q_TYPE_CVLAN,       /**< [GB.GG.D2.TM] Packet transmit out with ctag*/
    CTC_DOT1Q_TYPE_SVLAN,       /**< [GB.GG.D2.TM] Packet transmit out with stag*/
    CTC_DOT1Q_TYPE_BOTH         /**< [GB.GG.D2.TM] Packet transmit out with double tag*/
};
typedef enum ctc_dot1q_type_e ctc_dot1q_type_t;

/**
 @brief Port speed type
*/

enum ctc_port_speed_e
{
    CTC_PORT_SPEED_1G,          /**< [GB.GG.D2.TM] Port speed 1G mode*/
    CTC_PORT_SPEED_100M,        /**< [GB.GG.D2.TM] Port speed 100M mode*/
    CTC_PORT_SPEED_10M,         /**< [GB.GG.D2.TM] Port speed 10M mode*/
    CTC_PORT_SPEED_2G5,         /**< [GB.GG.D2.TM] Port speed 2.5G mode*/
    CTC_PORT_SPEED_10G,         /**< [GB.GG.D2.TM] Port speed 10G mode*/
    CTC_PORT_SPEED_20G,         /**< [GG.D2.TM] Port speed 20G mode*/
    CTC_PORT_SPEED_40G,         /**< [GG.D2.TM] Port speed 40G mode*/
    CTC_PORT_SPEED_100G,        /**< [GG.D2.TM] Port speed 100G mode*/
    CTC_PORT_SPEED_5G,          /**< [D2.TM] Port speed 5G mode*/
    CTC_PORT_SPEED_25G,         /**< [D2.TM] Port speed 25G mode*/
    CTC_PORT_SPEED_50G,         /**< [D2.TM] Port speed 50G mode*/
    CTC_PORT_SPEED_MAX
};
typedef enum ctc_port_speed_e ctc_port_speed_t;
/**
 @brief define mac type of port
*/
enum ctc_port_mac_type_e
{
    CTC_PORT_MAC_GMAC,      /**< [GB] mac type gmac, speed can be 1G 100M and 10M*/
    CTC_PORT_MAC_XGMAC,     /**< [GB] mac type Xgmac, speed at 10G*/
    CTC_PORT_MAC_SGMAC,     /**< [GB] mac type Sgmac, speed can be 13G*/
    CTC_PORT_MAC_CPUMAC,    /**< [GB] mac type Cpumac, uplink to cpu*/
    CTC_PORT_MAC_MAX
};
typedef enum ctc_port_mac_type_e ctc_port_mac_type_t;

/**
 @brief Port max frame size type
*/
enum ctc_frame_size_e
{
    CTC_FRAME_SIZE_0,       /**< [GG.D2.TM] Max/min frame size0*/
    CTC_FRAME_SIZE_1,       /**< [GG.D2.TM] Max/min frame size1*/
    CTC_FRAME_SIZE_2,       /**< [GG.D2.TM] Max/min frame size2*/
    CTC_FRAME_SIZE_3,       /**< [GG.D2.TM] Max/min frame size3*/
    CTC_FRAME_SIZE_4,       /**< [GG.D2.TM] Max/min frame size4*/
    CTC_FRAME_SIZE_5,       /**< [GG.D2.TM] Max/min frame size5*/
    CTC_FRAME_SIZE_6,       /**< [GG.D2.TM] Max/min frame size6*/
    CTC_FRAME_SIZE_7,       /**< [GG.D2.TM] Max/min frame size7*/
    CTC_FRAME_SIZE_MAX
};
typedef enum ctc_frame_size_e ctc_frame_size_t;

/**
 @brief Port inter-packet gap size
*/
enum ctc_ipg_size_e
{
    CTC_IPG_SIZE_0,       /**< [GB.GG.D2.TM] Ipg size0*/
    CTC_IPG_SIZE_1,       /**< [GB.GG.D2.TM] Ipg size1*/
    CTC_IPG_SIZE_2,       /**< [GB.GG.D2.TM] Ipg size2*/
    CTC_IPG_SIZE_3,       /**< [GB.GG.D2.TM] Ipg size3*/
    CTC_IPG_SIZE_MAX
};
typedef enum ctc_ipg_size_e ctc_ipg_size_t;

/**
 @brief Port loopback action type
*/
enum ctc_port_lbk_type_e
{
    CTC_PORT_LBK_TYPE_SWAP_MAC,  /**< [GB.GG.D2.TM] After port loopback, pakcet mac is swap*/
    CTC_PORT_LBK_TYPE_BYPASS,       /**< [GB.GG.D2.TM] After port loopback, pakcet is no change*/
    CTC_PORT_LBK_TYPE_MAX
};
typedef enum ctc_port_lbk_type_e ctc_port_lbk_type_t;

/**
 @brief Port arp packet processing type
*/
enum ctc_port_arp_action_type_e
{
    CTC_PORT_ARP_ACTION_TYPE_FW_EX       = 0,    /**< [GB.GG.D2.TM] Normal forwarding and exception to CPU*/
    CTC_PORT_ARP_ACTION_TYPE_FW          = 1,       /**< [GB.GG.D2.TM] Normal forwarding, no exception*/
    CTC_PORT_ARP_ACTION_TYPE_EX          = 2,        /**< [GB.GG.D2.TM] Always exception to CPU, and discard*/
    CTC_PORT_ARP_ACTION_TYPE_DISCARD     = 3,    /**< [GB.GG.D2.TM] Discard*/
    CTC_PORT_ARP_ACTION_TYPE_MAX
};
typedef enum ctc_port_arp_action_type_e ctc_port_arp_action_type_t;

/**
 @brief Port dhcp packet processing type
*/
enum ctc_port_dhcp_action_type_e
{
    CTC_PORT_DHCP_ACTION_TYPE_FW_EX = 0,     /**< [GB.GG.D2.TM] Normal forwarding and exception to CPU*/
    CTC_PORT_DHCP_ACTION_TYPE_FW = 1,        /**< [GB.GG.D2.TM] Normal forwarding, no exception*/
    CTC_PORT_DHCP_ACTION_TYPE_EX = 2,        /**< [GB.GG.D2.TM] Always exception to CPU and discard*/
    CTC_PORT_DHCP_ACTION_TYPE_DISCARD = 3,   /**< [GB.GG.D2.TM] No exception and discard*/
    CTC_PORT_DHCP_ACTION_TYPE_MAX
};
typedef enum ctc_port_dhcp_action_type_e ctc_port_dhcp_action_type_t;

/**
 @brief Port rpf type
*/
enum ctc_port_rpf_type_e
{
    CTC_PORT_RPF_TYPE_STRICT = 0,   /**< [GB.GG.D2.TM] Strict*/
    CTC_PORT_RPF_TYPE_LOOSE = 1,    /**< [GB.GG.D2.TM] Loose*/
    CTC_PORT_RPF_TYPE_MAX
};
typedef enum ctc_port_rpf_type_e ctc_port_rpf_type_t;

/**
 @brief Port lbk mode
*/
enum ctc_port_lbk_mode_e
{
    CTC_PORT_LBK_MODE_CC,           /**< [GB.GG.D2.TM] crossconnect loopback*/
    CTC_PORT_LBK_MODE_EFM,          /**< [GB.GG.D2.TM] efm loopback*/
    CTC_PORT_LBK_MODE_TAP           /**< [GG.D2.TM] TAP*/
};
typedef enum ctc_port_lbk_mode_e ctc_port_lbk_mode_t;

/**
 @brief Port loopback parameter
*/
struct ctc_port_lbk_param_s
{
    ctc_port_lbk_type_t lbk_type;     /**< [GB.GG.D2.TM] Port loopback type,
                                                          refer to ctc_port_lbk_type_t*/
    uint32 src_gport;                      /**< [GB.GG.D2.TM] Souce port for loopback*/
    uint32 dst_gport;                      /**< [GB.GG.D2.TM] Destination port for loopback. NOTE: when efm and crossconnect loopback,
                                                           if dst_gport equal src_gport, indicat loopback to self, other port to this port is discard.
                                                           when TAP, mac has been divided into groups. if use one mac as TAP dest,
                                                           all mac included in the group which has the TAP dest mac won't do normal forwarding. */
    uint8   lbk_enable;                    /**< [GB.GG.D2.TM] If set, enable/disable loopback*/
    uint8   lbk_mode;                     /**< [GB.GG.D2.TM] refer to ctc_port_lbk_mode_t*/
    uint32  lbk_gport;                     /**< [GB.GG.D2.TM] [out] lbk_gport is internal port.
                                                         using for EFM OAM loopback,
                                                         when EFM OAM loopback enable,
                                                         packet from cpu to loopback port
                                                         should use the internal port*/

    uint8    rsv1[2];
};
typedef struct ctc_port_lbk_param_s ctc_port_lbk_param_t;

/**
 @brief Port scl key type
*/
enum ctc_port_scl_key_type_e
{
    CTC_SCL_KEY_TYPE_DISABLE,                                      /**< [GB.GG.D2.TM] Disable scl key*/
    CTC_SCL_KEY_TYPE_VLAN_MAPPING_PORT,                   /**< [GB] Scl vlan mapping using port*/
    CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID,                   /**< [GB] Scl vlan mapping using cvlan id*/
    CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID,                   /**< [GB.GG.D2.TM] Scl vlan mapping using svlan id*/
    CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID_CCOS,         /**< [GB] Scl vlan mapping using cvlan id and cos*/
    CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID_SCOS,         /**< [GB] Scl vlan mapping using svlan id and cos*/
    CTC_SCL_KEY_TYPE_VLAN_MAPPING_DVID,                   /**< [GB] Scl vlan mapping using both svlan and cvlan id*/

    CTC_SCL_KEY_TYPE_VLAN_CLASS_MAC_SA,                  /**< [GB] Scl vlan class using macsa*/
    CTC_SCL_KEY_TYPE_VLAN_CLASS_MAC_DA,                  /**< [GB] Scl vlan class using macda*/
    CTC_SCL_KEY_TYPE_VLAN_CLASS_IPV4,                       /**< [GB] Scl vlan class using ipv4 addr*/
    CTC_SCL_KEY_TYPE_VLAN_CLASS_IPV6,                       /**< [GB] Scl vlan class using ipv6 addr*/

    CTC_SCL_KEY_TYPE_IPSG_PORT_MAC,                          /**< [GB] Scl ip source guard check mode:
                                                                                              CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN/
                                                                                              CTC_SECURITY_BINDING_TYPE_MAC/
                                                                                              CTC_SECURITY_BINDING_TYPE_MAC_VLAN*/
    CTC_SCL_KEY_TYPE_IPSG_PORT_IP,                             /**< [GB] Scl ip source guard check mode:
                                                                                              CTC_SECURITY_BINDING_TYPE_IP/
                                                                                              CTC_SECURITY_BINDING_TYPE_IP_VLAN/
                                                                                              CTC_SECURITY_BINDING_TYPE_IP_MAC*/
    CTC_SCL_KEY_TYPE_IPSG_IPV6,                                   /**< [GB] Scl ip source guard check mode:
                                                                                              CTC_SECURITY_BINDING_TYPE_IPV6_MAC*/

    CTC_SCL_KEY_TYPE_IPV4_TUNNEL,                               /**< [GB] Scl tunnel identify for IPv4/IPv6 in IPv4
                                                                                               and GRE with/without GRE key*/
    CTC_SCL_KEY_TYPE_IPV4_TUNNEL_AUTO,                      /**< [GG.D2.TM] Scl tunnel identify for IPv4/IPv6 in IPv4,
                                                                                               6to4 and ISATAP*/
    CTC_SCL_KEY_TYPE_IPV6_TUNNEL,                                /**< [GB] Scl tunnel identify for any in IPv6*/
    CTC_SCL_KEY_TYPE_IPV4_TUNNEL_WITH_RPF,               /**< [GB] Scl tunnel identiry for IPv4/IPv6 in IPv4,
                                                                                               GRE with/without GRE key tunnel, with outer head RPF check*/
    CTC_SCL_KEY_TYPE_IPV6_TUNNEL_WITH_RPF,               /**< [GB] Scl tunnel idenitfy for any in IPv6 tunnel,
                                                                                               with outer head RPF check*/
    CTC_SCL_KET_TYPE_NVGRE,                                          /**< [GG.D2.TM] Scl tunnel identify for NvGre*/
    CTC_SCL_KET_TYPE_VXLAN,                                          /**< [GG.D2.TM] Scl tunnel identify for Vxlan*/

    CTC_SCL_KEY_TYPE_MAX
};
typedef enum ctc_port_scl_key_type_e ctc_port_scl_key_type_t;

/**
 @brief Port vlan domain type
*/
enum ctc_port_vlan_domain_type_e
{
    CTC_PORT_VLAN_DOMAIN_SVLAN                = 0,    /**< [GB.GG.D2.TM] Svlan domain*/
    CTC_PORT_VLAN_DOMAIN_CVLAN                = 1,    /**< [GB.GG.D2.TM] Cvlan domain*/
    CTC_PORT_VLAN_DOMAIN_MAX
};
typedef enum ctc_port_vlan_domain_type_e ctc_port_vlan_domain_type_t;

/*
 @brief Port raw packet type
*/
enum ctc_port_raw_packet_e
{
    CTC_PORT_RAW_PKT_NONE,          /**< [GB.GG.D2.TM] Raw packet parser disable*/
    CTC_PORT_RAW_PKT_ETHERNET,      /**< [GB.GG.D2.TM] Port parser ethernet raw packet*/
    CTC_PORT_RAW_PKT_IPV4,          /**< [GB.GG.D2.TM] Port only parser ipv4 raw packet*/
    CTC_PORT_RAW_PKT_IPV6,          /**< [GB.GG.D2.TM] Port only parser ipv6 raw packet*/
    CTC_PORT_RAW_PKT_MAX
};
typedef enum ctc_port_raw_packet_e ctc_port_raw_packet_t;

/*
 @brief Port restriction type: private vlan, blocking based on port and port isolation
*/
enum ctc_port_restriction_mode_e
{
    CTC_PORT_RESTRICTION_DISABLE = 0,                   /**< [GB.GG.D2.TM] Private vlan, blocking and port isolation is disable on port*/
    CTC_PORT_RESTRICTION_PVLAN,                           /**< [GB.GG.D2.TM] Private vlan is enabled on port*/
    CTC_PORT_RESTRICTION_PORT_BLOCKING,             /**< [GB.GG.D2.TM] Blocking based on port is enabled on port*/
    CTC_PORT_RESTRICTION_PORT_ISOLATION             /**< [GB.GG.D2.TM] Port isolation is enabled on port*/
};
typedef enum ctc_port_restriction_mode_e ctc_port_restriction_mode_t;

/*
 @brief Port private vlan type
*/
enum ctc_port_pvlan_type_e
{
    CTC_PORT_PVLAN_NONE  = 0,          /**< [GB.GG.D2.TM] Port is none port*/
    CTC_PORT_PVLAN_PROMISCUOUS,   /**< [GB.GG.D2.TM] Port is promiscuous port,isolated id must not be configured*/
    CTC_PORT_PVLAN_ISOLATED,          /**< [GB.GG.D2.TM] Port is isolated port,isolated id must not be configured*/
    CTC_PORT_PVLAN_COMMUNITY       /**< [GB.GG.D2.TM] Port is community port,isolated id must be configured*/
};
typedef enum ctc_port_pvlan_type_e ctc_port_pvlan_type_t;

/*
 @brief Port isolation packet isolated type
*/
enum ctc_port_isolation_pkt_type_e
{
    CTC_PORT_ISOLATION_ALL                 = 0x0000,                 /**< [GB.GG.D2.TM] All packet will be isolated*/
    CTC_PORT_ISOLATION_UNKNOW_UCAST        = 0x0001,        /**< [GB.GG.D2.TM] Unknown ucast type packet will be isolated*/
    CTC_PORT_ISOLATION_UNKNOW_MCAST        = 0x0002,        /**< [GB.GG.D2.TM] Unknown mcast type packet will be isolated*/
    CTC_PORT_ISOLATION_KNOW_UCAST          = 0x0004,          /**< [GG.D2.TM] Known ucast type packet will be isolated*/
    CTC_PORT_ISOLATION_KNOW_MCAST          = 0x0008,          /**< [GB.GG.D2.TM] Known mcast type packet will be isolated*/
    CTC_PORT_ISOLATION_BCAST               = 0x0010                /**< [GB.GG.D2.TM] Bcast type packet will be isolated*/
};
typedef enum ctc_port_isolation_pkt_type_e ctc_port_isolation_pkt_type_t;

/*
 @brief Port blocking packet blocked type
*/
enum ctc_port_blocking_pkt_type_e
{
    CTC_PORT_BLOCKING_UNKNOW_UCAST        = 0x0001,        /**< [GB.GG.D2.TM] Unknown ucast type packet will be blocked*/
    CTC_PORT_BLOCKING_UNKNOW_MCAST        = 0x0002,        /**< [GB.GG.D2.TM] Unknown mcast type packet will be blocked*/
    CTC_PORT_BLOCKING_KNOW_UCAST          = 0x0004,           /**< [GB.GG.D2.TM] Known ucast type packet will be blocked*/
    CTC_PORT_BLOCKING_KNOW_MCAST          = 0x0008,          /**< [GB.GG.D2.TM] Known mcast type packet will be blocked*/
    CTC_PORT_BLOCKING_BCAST               = 0x0010                 /**< [GB.GG.D2.TM] Bcast type packet will be blocked*/
};
typedef enum ctc_port_blocking_pkt_type_e ctc_port_blocking_pkt_type_t;

/**
  @brief Port restriction parameter: private vlan, port isolation, blocking based on port
*/
struct ctc_port_restriction_s
{
    ctc_port_restriction_mode_t mode;                     /**< [GB.GG.D2.TM] Restriction mode enable on port, see ctc_port_restriction_mode_t*/
    ctc_direction_t dir;                                  /**< [GB.GG.D2.TM] Direction*/
    uint16 isolated_id;                                   /**< [GB.GG.D2.TM] If port is community port in private vlan, isolated id <0-15>
                                                                       If port isolation is enabled on port, isolated id <0-31>, 0 means disable*/
    uint16 type;                                          /**< [GB.GG.D2.TM] If private vlan is enabled on port,
                                                                       set pvlan type on port(see ctc_port_pvlan_type_e);
                                                                       if port isolation is enabled on port, set isolated id,
                                                                       and if the dir is egress,
                                                                       set isolation packet type on port(see ctc_port_isolation_pkt_type_e)
                                                                       if blocking based on port is enabled on port,
                                                                       set blocking packet type on port(see ctc_port_blocking_pkt_type_e)*/
};
typedef struct ctc_port_restriction_s ctc_port_restriction_t;

/*
 @brief Port isolation configure
*/
struct ctc_port_isolation_s
{
   uint8 use_isolation_id;   /**< [GG.D2.TM] If set, use isolation_id as isolation group's member; else use port as isolation group's member.*/
   uint8 mlag_en;             /**< [GG.D2.TM] enable MLAG port isolation, dest is gport bitmap if is_linkagg = 0.*/
   uint8 is_linkagg;          /**< [D2.TM] if is_linkagg = 1, dest is linkagg bitmap, used for MLAG port isolation*/
   uint8 isolation_pkt_type;  /**< [GG.D2.TM] isolation packet type on port(ctc_port_isolation_pkt_type_t)*/

   uint32 gport;              /**<[GG.D2.TM] If use_isolation_id = 1, means isolation group; else means ingress source port which be isolated.*/
   ctc_port_bitmap_t pbm;     /**< [GG.D2.TM] If use_isolation_id = 1, means source isolation group bitmap;
                                     else means ports be blocked or allow packets from a given ingress port.*/
};
typedef struct ctc_port_isolation_s ctc_port_isolation_t;

/**
  @brief Port untag port default vlan type
*/
enum ctc_port_untag_pvid_type_e
{
    CTC_PORT_UNTAG_PVID_TYPE_NONE,                       /**< [GB.GG.D2.TM] Do nothing for port default svlan or cvlan id*/
    CTC_PORT_UNTAG_PVID_TYPE_SVLAN,                      /**< [GB.GG.D2.TM] Untag port default svlan id*/
    CTC_PORT_UNTAG_PVID_TYPE_CVLAN,                      /**< [GB.GG.D2.TM] Untag port default cvlan id*/
    CTC_PORT_UNTAG_PVID_TYPE_MAX
};
typedef enum ctc_port_untag_pvid_type_e ctc_port_untag_pvid_type_t;

/**
  @brief Port 802.3ap port ability flags
*/
enum ctc_port_cl73_ability_e
{
    CTC_PORT_CL73_10GBASE_KR      = (1 << 0 ),      /**< [GG.D2.TM] 10GBase-KR ability */
    CTC_PORT_CL73_40GBASE_KR4     = (1 << 1 ),     /**< [GG.D2.TM] 40GBase-KR4 ability */
    CTC_PORT_CL73_40GBASE_CR4     = (1 << 2 ),     /**< [GG.D2.TM] 40GBase-CR4 ability */
    CTC_PORT_CL73_100GBASE_KR4    = (1 << 3 ),    /**< [GG.D2.TM] 100GBase-KR4 ability */
    CTC_PORT_CL73_100GBASE_CR4    = (1 << 4 ),    /**< [GG.D2.TM] 100GBase-CR4 ability */
    CTC_PORT_CL73_FEC_ABILITY     = (1 << 5 ),      /**< [GG.D2.TM] BASE-R FEC supported*/
    CTC_PORT_CL73_FEC_REQUESTED   = (1 << 6),      /**< [GG.D2.TM] BASE-R requested */

    CTC_PORT_CL73_25GBASE_KRS                = (1 << 7),  /**< [D2.TM] 25GBase-KRS ability */
    CTC_PORT_CL73_25GBASE_CRS                = (1 << 7),  /**< [D2.TM] 25GBase-CRS ability */
    CTC_PORT_CL73_25GBASE_KR                 = (1 << 8),  /**< [D2.TM] 25GBase-CRS ability */
    CTC_PORT_CL73_25GBASE_CR                 = (1 << 8),  /**< [D2.TM] 25GBase-CR ability */
    CTC_PORT_CL73_25G_RS_FEC_REQUESTED       = (1 << 9),  /**< [D2.TM] 25G RS-FEC requested */
    CTC_PORT_CL73_25G_BASER_FEC_REQUESTED    = (1 << 10),  /**< [D2.TM] 25G BASE-R FEC requested */
    CTC_PORT_CSTM_25GBASE_KR1                = (1 << 11),  /**< [D2.TM] Consortium mode 25GBase-KR1 ability */
    CTC_PORT_CSTM_25GBASE_CR1                = (1 << 12),  /**< [D2.TM] Consortium mode 25GBase-CR1 ability */
    CTC_PORT_CSTM_50GBASE_KR2                = (1 << 13),  /**< [D2.TM] Consortium mode 50GBase-KR2 ability */
    CTC_PORT_CSTM_50GBASE_CR2                = (1 << 14),  /**< [D2.TM] Consortium mode 50GBase-CR2 ability */
    CTC_PORT_CSTM_RS_FEC_ABILITY             = (1 << 15),  /**< [D2.TM] Consortium mode RS-FEC ability */
    CTC_PORT_CSTM_BASER_FEC_ABILITY          = (1 << 16),  /**< [D2.TM] Consortium mode BASE-R FEC ability */
    CTC_PORT_CSTM_RS_FEC_REQUESTED           = (1 << 17),  /**< [D2.TM] Consortium mode RS-FEC requested */
    CTC_PORT_CSTM_BASER_FEC_REQUESTED        = (1 << 18),  /**< [D2.TM] Consortium mode BASE-R FEC requested */

    CTC_PORT_CL73_MAX
};
typedef enum ctc_port_cl73_ability_e ctc_port_cl73_ability_t;

enum ctc_port_fec_type_e
{
    CTC_PORT_FEC_TYPE_NONE,          /**< [D2.TM] FEC OFF, if set, #1)FEC config will ALL be clear
                                                                #2)CL73 Auto-Neg will NOT negotiate FEC abilty */
    CTC_PORT_FEC_TYPE_RS,            /**< [D2.TM] RS FEC, if set, RS FEC config will be set */
    CTC_PORT_FEC_TYPE_BASER,         /**< [D2.TM] BASE-R FEC, if set, BASE-R FEC config will be set */
    CTC_PORT_FEC_TYPE_MAX
};
typedef enum ctc_port_fec_type_e ctc_port_fec_type_t;
enum ctc_port_auto_neg_mode_e
{
    CTC_PORT_AUTO_NEG_MODE_1000BASE_X,             /**< [GB.GG.D2.TM] 1000BASE-X Auto-neg mode */
    CTC_PORT_AUTO_NEG_MODE_SGMII_MASTER,          /**< [GB.GG.D2.TM] SGMII master Auto-neg mode */
    CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER,          /**< [GB.GG.D2.TM] SGMII slaver Auto-neg mode */
    CTC_PORT_AUTO_NEG_MODE_MAX_MODE
};
typedef enum ctc_port_auto_neg_mode_e ctc_port_auto_neg_mode_t;

/**
 @brief Port station move action type
*/
enum ctc_port_station_move_action_type_e
{
    CTC_PORT_STATION_MOVE_ACTION_TYPE_FWD,                      /**< [GB.GG.D2.TM] Packet will forwarding if station move */
    CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD,                /**< [GB.GG.D2.TM] Packet will discard if station move */
    CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD_TOCPU,     /**< [GB.GG.D2.TM] Packet will discard and redirect to cpu if station move */
    CTC_PORT_STATION_MOVE_ACTION_TYPE_MAX
};
typedef enum ctc_port_station_move_action_type_e ctc_port_station_move_action_type_t;


enum ctc_port_wlan_port_type_e
{
    CTC_PORT_WLAN_PORT_TYPE_NONE ,                                       /**< [D2.TM] disable wlan port type */
    CTC_PORT_WLAN_PORT_TYPE_DECAP_WITH_DECRYPT ,             /**< [D2.TM] The port can support the processing plaintext and encrypted packet */
    CTC_PORT_WLAN_PORT_TYPE_DECAP_WITHOUT_DECRYPT ,       /**< [D2.TM] The port only support the processing plaintext*/
    CTC_PORT_WLAN_PORT_TYPE_TYPE_MAX
};
typedef enum ctc_port_wlan_port_type_e ctc_port_wlan_port_type_t;

enum ctc_port_sd_action_type_e
{
    CTC_PORT_SD_ACTION_TYPE_NONE,
    CTC_PORT_SD_ACTION_TYPE_DISCARD_OAM,             /**< [D2.TM] Signal Degrade, discard oam pkt */
    CTC_PORT_SD_ACTION_TYPE_DISCARD_DATA,            /**< [D2.TM] Signal Degrade, discard data pkt */
    CTC_PORT_SD_ACTION_TYPE_DISCARD_BOTH,            /**< [D2.TM] Signal Degrade, discard oam and data pkt */
    CTC_PORT_WLAN_PORT_TYPE_MAX
};
typedef enum ctc_port_sd_action_type_e ctc_port_sd_action_type_t;


/**
  @brief Port property flags
*/
enum ctc_port_property_e
{
    /**< genernal property */
    CTC_PORT_PROP_MAC_EN,                         /**< [GB.GG.D2.TM] Port mac enable*/
    CTC_PORT_PROP_PORT_EN,                        /**< [GB.GG.D2.TM] Port whether the port is enable,
                                                                            the following properties will be set:
                                                                            CTC_PORT_PROP_TRANSMIT_EN,
                                                                            CTC_PORT_PROP_RECEIVE_EN and CTC_PORT_PROP_BRIDGE_EN*/
    CTC_PORT_PROP_LINK_UP,                        /*<  [GB.GG.D2.TM] Port link up status */
    CTC_PORT_PROP_VLAN_CTL,                      /**< [GB.GG.D2.TM] Port's vlan tag control mode*/
    CTC_PORT_PROP_DEFAULT_VLAN,              /**< [GB.GG.D2.TM] Default vlan id, the following properties will be set:
                                                                           untag_default_svlan = 1,untag_default_vlanId = 1,
                                                                           DsPhyPortExt_DefaultVlanId_f and DsDestPort_DefaultVlanId_f*/
    CTC_PORT_PROP_UNTAG_PVID_TYPE,         /**< [GB.GG.D2.TM] Untag default vlan type. refer CTC_PORT_UNTAG_PVID_TYPE_XXX */
    CTC_PORT_PROP_VLAN_DOMAIN,               /**< [GB.GG.D2.TM] vlan domain of the port */

    CTC_PORT_PROP_PROTOCOL_VLAN_EN,               /**< [GB.GG.D2.TM] protocol vlan enable*/
    CTC_PORT_PROP_QOS_POLICY,                          /**< [GB.GG.D2.TM] QOS policy*/
    CTC_PORT_PROP_DEFAULT_PCP,                         /**< [GB.GG.D2.TM] default PCP*/
    CTC_PORT_PROP_DEFAULT_DEI,                         /**< [GB.GG.D2.TM] default DEI*/
    CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP,    /**< [GB.GG.D2.TM] SCL default vlan lookup*/
    CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN,            /**< [GB] SCL ipv4 lookup enable*/
    CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN,            /**< [GB] SCL ipv6 lookup enable*/
    CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC,      /**< [GB] force Ipv4 packet go against MAC SCL entry*/
    CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC,      /**< [GB] force Ipv6 packet go against MAC SCL entry*/
    CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP,      /**< [GG.D2.TM] NvGRE mcast packet do not decapsulation*/
    CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP,      /**< [GG.D2.TM] VxLAN mcast packet do not decapsulation*/
    CTC_PORT_PROP_PTP_EN,                        /**< [GB.GG.D2.TM] set ptp clock id, refer to ctc_ptp_clock_t */

    CTC_PORT_PROP_SPEED,                                  /**< [GB.GG.D2.TM] port speed*/
    CTC_PORT_PROP_MAX_FRAME_SIZE,                 /**< [GB.GG.D2.TM] max frame size*/
    CTC_PORT_PROP_MIN_FRAME_SIZE,                 /**< [GB.GG.D2.TM] min frame size*/
    CTC_PORT_PROP_PREAMBLE,                           /**< [GB.GG.D2.TM] preamble value*/
    CTC_PORT_PROP_PADING_EN,                          /**< [GB.GG.D2.TM] pading enable*/
    CTC_PORT_PROP_STRETCH_MODE_EN,              /**< [GB] port stretch mode enable*/
    CTC_PORT_PROP_IPG,                                     /**< [GB.GG.D2.TM] ipg index per port*/
    CTC_PORT_PROP_RX_PAUSE_TYPE,                  /**< [GB.GG] set port rx pause type, normal or pfc*/
    CTC_PORT_PROP_MAC_TS_EN,                         /**< [GB] Enable Mac for append time-stamp */
    CTC_PORT_PROP_ERROR_CHECK,                     /**< [GB.GG.D2.TM] Mac CRC check, 1:enable, 0:disable */
    /* SYSTEM MODIFIED by yaom for Openflow 20160303 */
    CTC_PORT_PROP_CRC_RECALC_EN,                  /**< [GG] Mac CRC recalculation, 1:enable, 0:disable */
    /* SYSTEM MODIFIED by yaom for Openflow 20160303 */
    CTC_PORT_PROP_CRC_CHECK_EN,                  /**< [GG] Mac CRC recalculation, 1:enable, 0:disable */
    CTC_PORT_PROP_MAC_TX_IPG,                       /**< [GB.GG.D2.TM] mac tx ipg size, the value must be 8bytes or 12bytes(default) */
    CTC_PORT_PROP_OVERSUB_PRI,                     /**< [GG.D2.TM] Oversubscription port bandwidth guarantee priority, oversubscription allows ingress ports bandwidth
                                                                                greater than the core bandwidth of the device and per port set bandwidth guarantee priority, for GG: 0-low,1-high, Default is 0 */

    CTC_PORT_PROP_TRANSMIT_EN,                    /**< [GB.GG.D2.TM] Tx enable*/
    CTC_PORT_PROP_RECEIVE_EN,                      /**< [GB.GG.D2.TM] Rx enable*/
    CTC_PORT_PROP_BRIDGE_EN,                       /**< [GB.GG.D2.TM] bridge enable in both direction*/
    CTC_PORT_PROP_LEARNING_EN,                    /**< [GB.GG.D2.TM] learning enable*/
    CTC_PORT_PROP_PRIORITY_TAG_EN,             /**< [GB.GG.D2.TM] priority tag enable*/

    CTC_PORT_PROP_CROSS_CONNECT_EN,               /**< [GB.GG.D2.TM] port cross connect*/
    CTC_PORT_PROP_DOT1Q_TYPE,                           /**< [GB.GG.D2.TM] dot1q type*/
    CTC_PORT_PROP_USE_OUTER_TTL,                       /**< [GB.GG.D2.TM] use outer ttl in case of tunnel*/
    CTC_PORT_PROP_DISCARD_NON_TRIL_PKT,          /**< [GB.GG.D2.TM] discard non-trill pkg enable*/
    CTC_PORT_PROP_DISCARD_TRIL_PKT,                  /**< [GB.GG.D2.TM] discard trill pkg enable*/
    CTC_PORT_PROP_SRC_DISCARD_EN,                    /**< [GB.GG.D2.TM] port whether the srcdiscard is enable*/
    CTC_PORT_PROP_PORT_CHECK_EN,                     /**< [GB.GG.D2.TM] port port check enable*/
    CTC_PORT_PROP_RAW_PKT_TYPE,                       /**< [GB.GG.D2.TM] raw packet type*/

    CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN,           /**< [GB.GG.D2.TM] bridge to the same port enable*/
    CTC_PORT_PROP_HW_LEARN_EN,                         /**< [GB.GG.D2.TM] hw learning enable*/
    CTC_PORT_PROP_TRILL_EN,                                 /**< [GB.GG.D2.TM] trill enable*/
    CTC_PORT_PROP_TRILL_MCAST_DECAP_EN,           /**< [GG.D2.TM] trill mcast decap when enable*/
    CTC_PORT_PROP_FCOE_EN,                                  /**< [GB.GG.D2.TM] FCOE enable*/
    CTC_PORT_PROP_RPF_EN,                                    /**< [GB.GG.D2.TM] RPF enable*/
    CTC_PORT_PROP_FCMAP,                                     /**< [GG.D2.TM] fcoe FCMAP(0x0EFC00-0x0EFCFF), default is 0x0EFC00 */
    CTC_PORT_PROP_REPLACE_STAG_COS,                 /**< [GB.GG.D2.TM] the STAG COS field is replaced by the classified COS result*/
    CTC_PORT_PROP_REPLACE_STAG_TPID,                /**< [GG.D2.TM] the STAG TPID is replaced by the vlanXlate TPID result*/
    CTC_PORT_PROP_REPLACE_CTAG_COS,                /**< [GB.GG.D2.TM] the STAG COS field is replaced by the classified COS result*/
    CTC_PORT_PROP_REPLACE_DSCP_EN,                  /**< [GB.GG.D2.TM] the dscp field is replaced by the classified qos result, D2 replaced by CTC_PORT_PROP_DSCP_SELECT_MODE*/

    CTC_PORT_PROP_L3PDU_ARP_ACTION,                /**< [GB.GG.D2.TM] l3PDU arp action*/
    CTC_PORT_PROP_L3PDU_DHCP_ACTION,             /**< [GB.GG.D2.TM] l3PDU dhcp action*/
    CTC_PORT_PROP_TUNNEL_RPF_CHECK,               /**< [GB.GG.D2.TM] tunnel RPF check enable*/
    CTC_PORT_PROP_RPF_TYPE,                              /**< [GB.GG.D2.TM] RPF type, refer to ctc_port_rpf_type_t */
    CTC_PORT_PROP_IS_LEAF,                                /**< [GB.GG.D2.TM] the port connect with a leaf node*/
    CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY,       /**< [GB.GG.D2.TM] Packet tag take precedence over all*/
    CTC_PORT_PROP_ROUTE_EN,                             /**< [GB.GG.D2.TM] Enable route on port */

    CTC_PORT_PROP_AUTO_NEG_EN,                       /**< [GB.GG.D2.TM] Enable Auto-neg on port */
    CTC_PORT_PROP_AUTO_NEG_MODE,                  /**< [GB.GG.D2.TM] Config Auto-neg mode on port */
    CTC_PORT_PROP_CL73_ABILITY,                       /**< [GG.D2.TM] cfg cl73 ability */
    CTC_PORT_PROP_FEC_EN,                                /**< [GG.D2.TM] Enable FEC, mac must be disable */
    CTC_PORT_PROP_LINK_INTRRUPT_EN,                /**< [GB.GG.D2.TM] Enable Mac PCS link interrupt */
    CTC_PORT_PROP_UNIDIR_EN,                       /**< [GB.GG.D2.TM] Enable fiber unidirection */
    CTC_PORT_PROP_CUT_THROUGH_EN,                  /**< [GG.D2.TM] enable cut through */

    CTC_PORT_PROP_LINKSCAN_EN,                       /**< [GB.GG.D2.TM] Enable linkscan funciton on port*/
    CTC_PORT_PROP_APS_FAILOVER_EN,                /**< [GB.GG.D2.TM] Enable Aps-failover on port*/
    CTC_PORT_PROP_LINKAGG_FAILOVER_EN,         /**< [GB.GG.D2.TM] Enable Linkagg-failover on port*/
    CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID,     /**< [GG.D2.TM] SCL flow hash field sel id */
    CTC_PORT_PROP_APS_SELECT_GRP_ID,            /**< [GG.D2.TM] The aps select group id, 0xFFFFFFFF means disable aps select*/
    CTC_PORT_PROP_APS_SELECT_WORKING,         /**< [GG.D2.TM] Indicate the flow is working path or protecting path*/

    CTC_PORT_PROP_SNOOPING_PARSER,                         /**< [GG.D2.TM] Enable parser tunnel payload even no tunnel decap */
    CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD,         /**< [GG.D2.TM] If set,indicate tunnel packet will use outer header to do ACL/IPFIX flow lookup */
    CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN,            /**< [GG.D2.TM] If set, indicate use pkt inner info as lookup key, and outer l4 info can also be used as lookup key */
    CTC_PORT_PROP_NVGRE_ENABLE,                   /**< [GG.D2.TM] Enable nvgre */

    CTC_PORT_PROP_METADATA_OVERWRITE_PORT,        /**< [GG.D2.TM] Enable metadata overwrite port */
    CTC_PORT_PROP_METADATA_OVERWRITE_UDF,         /**< [GG] Enable metadata overwrite udf */

    CTC_PORT_PROP_SIGNAL_DETECT,                            /**< [GG.D2.TM] Signal detect on port */

    CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN,      /**< [GB.GG.D2.TM] Enable station move to cpu */
    CTC_PORT_PROP_STATION_MOVE_PRIORITY,               /**< [GB.GG.D2.TM] Station move priority: 0-low,1-high, Default is 0 */
    CTC_PORT_PROP_STATION_MOVE_ACTION,                  /**< [GB.GG.D2.TM] Station move action, refer to ctc_port_station_move_action_type_t */
    CTC_PORT_PROP_EFD_EN,                                          /**< [GG.D2.TM] Enable EFD */
    CTC_PORT_PROP_EEE_EN,                                          /**< [GB.GG.D2.TM] Enable eee on port*/

    CTC_PORT_PROP_LOGIC_PORT,                     /**< [GB.GG.D2.TM] Port logic port value. if set 0, mean logic port disable */
    CTC_PORT_PROP_GPORT,                     /**< [D2.TM] Set port gport value. */
    CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS,           /**< [GB.GG.D2.TM] Will not add default vlan for untagged packet */
    CTC_PORT_PROP_IPFIX_LKUP_BY_OUTER_HEAD,         /**< [D2.TM] If set,indicate tunnel packet will use outer header to do IPFIX flow lookup */
    CTC_PORT_PROP_IPFIX_AWARE_TUNNEL_INFO_EN,     /**< [D2.TM] If set, indicate ipfix lookup key use innner info as key, and outer tunnel info can also be used as lookup key */
    CTC_PORT_PROP_CID,                            /**< [D2.TM] Category Id*/
    CTC_PORT_PROP_TX_CID_HDR_EN,                  /**< [D2.TM] Tx Packet with cid header.*/
    CTC_PORT_PROP_WLAN_PORT_TYPE,                 /**< [D2.TM] WLAN port type, refer to ctc_port_wlan_port_type_t */
    CTC_PORT_PROP_WLAN_PORT_ROUTE_EN,             /**< [D2.TM] Enable inner packet route on the wlan port */
    CTC_PORT_PROP_WLAN_DECAP_WITH_RID,            /**< [D2.TM] If not zero means radio id used as key of WLAN decap */
    CTC_PORT_PROP_DSCP_SELECT_MODE,               /**< [D2.TM] [Egress]Dscp select mode, refer to ctc_dscp_select_mode_t */
    CTC_PORT_PROP_DEFAULT_DSCP,                   /**< [D2.TM] [Egress]Default DSCP, if DSCP_SELECT_MODE is CTC_DSCP_SELECT_ASSIGN, the default dscp will rewrite packet's dscp */
    CTC_PORT_PROP_REPLACE_PCP_DEI,                /**< [D2.TM] [Egress]The classified PCP/DEI is replaced by the Default PCP/DEI */
    CTC_PORT_PROP_LOGIC_PORT_CHECK_EN,            /**< [D2.TM] [Egress]Port logic port check enable */
    CTC_PORT_PROP_LOOP_WITH_SRC_PORT,             /**< [GG.D2.TM] if set, indicate the loop packet will take source port back, or used for taking source port to CPU */
    CTC_PORT_PROP_NET_TX,    /* SYSTEM MODIFIED by liuht for bug42776 2017-02-17*/
    
    CTC_PORT_PROP_BPE_PORT_TYPE,                  /**< [D2.TM] BPE(Bridge port extend) port type, refer to ctc_port_bpe_extend_type_t*/
    CTC_PORT_PROP_BPE_NAMESPACE,                  /**< [D2.TM] BPE port namespace which is used to differentiate same extend vlan*/
    CTC_PORT_PROP_BPE_EXTEND_VLAN,                /**< [D2.TM] BPE extend vlan*/
    CTC_PORT_PROP_SD_ACTION,                      /**< [D2.TM] Port enable Singal Degrade, refer to ctc_port_sd_action_type_t*/

    CTC_PORT_PROP_LB_HASH_ECMP_PROFILE,           /**< [TM] Port sel hash offset profile id for ecmp or packet head, refer to ctc_lb_hash_offset_t*/
    CTC_PORT_PROP_LB_HASH_LAG_PROFILE,	          /**< [TM] Port sel hash offset profile id for linkagg, refer to ctc_lb_hash_offset_t*/

    CTC_PORT_PROP_LEARN_DIS_MODE,                 /**< [TM] Learn disable mode: 1-learn disable with mac sa lookup, 0-learn disable with no mac sa lookup */
    CTC_PORT_PROP_FORCE_MPLS_EN,                  /**< [TM] Force MPLS packet decap */
    CTC_PORT_PROP_FORCE_TUNNEL_EN,                /**< [TM] Force Tunnel packet decap */
    CTC_PORT_PROP_DISCARD_UNENCRYPT_PKT,          /**< [TM] Discard unencrypted packet, except for some special packets */
    CTC_PORT_PROP_XPIPE_EN,                       /**< [TM] Enable/disable XPIPE feature */

    /* Mac Property */
    CTC_PORT_PROP_CHK_CRC_EN = 1000,              /**< [D2.TM] packet check RX CRC Enable*/
    CTC_PORT_PROP_STRIP_CRC_EN,                   /**< [D2.TM] packet strip TX CRC Enable*/
    CTC_PORT_PROP_APPEND_CRC_EN,                  /**< [D2.TM] packet append TX CRC Enable*/
    CTC_PORT_PROP_APPEND_TOD_EN,                  /**< [D2.TM] packet append RX tod Enable*/

    CTC_PORT_PROP_AUTO_NEG_FEC,                   /**< [D2.TM] Config Auto-neg FEC ability on port, refer to 'ctc_port_fec_type_t'
                                                            set value 0 -- disable Auto-Neg FEC ability;
                                                            set value 1 -- enable Auto-Neg FEC RS type ability;
                                                            set value 2 -- enable Auto-Neg FEC Base-R type ability;
                                                            set other value -- return error */
    CTC_PORT_PROP_INBAND_CPU_TRAFFIC_EN,          /**< [D2.TM] Set port inband CPU traffic enable, only used for WDM Pon*/
    CTC_PORT_PROP_MUX_TYPE,                       /**< [GG.D2.TM] Set packet encap format for ingress port identify, refer to ctc_port_mux_type_t */

    /* Phy property*/
    CTC_PORT_PROP_PHY_INIT,                 /**< [D2.TM] phy port init*/
    CTC_PORT_PROP_PHY_EN,                   /**< [D2.TM] phy port enable*/
    CTC_PORT_PROP_PHY_DUPLEX,               /**< [D2.TM] phy duplext*/
    CTC_PORT_PROP_PHY_MEDIUM,               /**< [D2.TM] phy medium*/
    CTC_PORT_PROP_PHY_LOOPBACK,             /**< [D2.TM] phy loopback*/
    CTC_PORT_PROP_PHY_CUSTOM_BASE = 2000,
    CTC_PORT_PROP_PHY_CUSTOM_MAX_TYPE = 2100,

    MAX_CTC_PORT_PROP_NUM
};
typedef enum ctc_port_property_e  ctc_port_property_t;

/**
  @brief Port property flags with direction
*/
enum ctc_port_direction_property_e
{
    CTC_PORT_DIR_PROP_VLAN_FILTER_EN = MAX_CTC_PORT_PROP_NUM, /**< [GB.GG.D2.TM] vlan filter enable*/
    CTC_PORT_DIR_PROP_RANDOM_LOG_EN,                  /**< [GB.GG.D2.TM] random log enable*/
    CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT,         /**< [GB.GG.D2.TM] percent of random threshold*/
    CTC_PORT_DIR_PROP_RANDOM_LOG_RATE,               /**< [GB.GG.D2.TM] rate of random threshold,for GG,D2,TM max rate = 0x7fff,percent = (rate << shift)/0xfffff,
                                                                        shitf default :5,for GB percent = rate /0x7fff */

    CTC_PORT_DIR_PROP_QOS_DOMAIN,                        /**< [GB.GG.D2.TM] QOS domain*/
    CTC_PORT_DIR_PROP_QOS_COS_DOMAIN,                /**< [D2.TM] QOS COS domain*/
    CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN,               /**< [D2.TM] QOS DSCP domain*/

    CTC_PORT_DIR_PROP_PORT_POLICER_VALID,            /**< [GB.GG.D2.TM] policer valid*/
    CTC_PORT_DIR_PROP_STAG_TPID_INDEX,                 /**< [GB.GG.D2.TM] stag TPID index*/
    /* SYSTEM MODIFY by xuwj */    
    CTC_PORT_DIR_PROP_STAG_TPID_INDEX_EN,             /**< [GB.GG] enable stag TPID index*/
    /* SYSTEM MODIFY by xuwj */

    CTC_PORT_DIR_PROP_ACL_EN,                                /**< [GB.GG.D2.TM] port acl enable, refer CTC_ACL_EN_XXX */
    CTC_PORT_DIR_PROP_ACL_CLASSID,                        /**< [GB.GG.D2.TM] port acl classid */
    CTC_PORT_DIR_PROP_ACL_CLASSID_0,                    /**< [GG.D2.TM] port acl0 classid */
    CTC_PORT_DIR_PROP_ACL_CLASSID_1,                    /**< [GG.D2.TM] port acl1 classid */
    CTC_PORT_DIR_PROP_ACL_CLASSID_2,                    /**< [GG.D2.TM] port acl2 classid */
    CTC_PORT_DIR_PROP_ACL_CLASSID_3,                    /**< [GG.D2.TM] port acl3 classid */
    CTC_PORT_DIR_PROP_ACL_IPV4_FORCE_MAC,          /**< [GB] ipv4-packet force use mac-key */
    CTC_PORT_DIR_PROP_ACL_IPV6_FORCE_MAC,          /**< [GB] ipv6-packet force use mac-key */
    CTC_PORT_DIR_PROP_ACL_FORCE_USE_IPV6,           /**< [GB] force use ipv6 key */
    CTC_PORT_DIR_PROP_ACL_USE_CLASSID,                 /**< [GB.GG.D2.TM] use acl classid not bitmap*/
    CTC_PORT_DIR_PROP_ACL_HASH_FIELD_SEL_ID,       /**< [GG.D2.TM] hash field sel id */
    CTC_PORT_DIR_PROP_SERVICE_ACL_EN,                   /**< [GB.GG] service acl enable */
    CTC_PORT_DIR_PROP_ACL_HASH_LKUP_TYPE,           /**< [GG.D2.TM] acl hash type. refer CTC_ACL_HASH_LKUP_TYPE_XXX */
    CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0,        /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1,        /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2,        /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3,        /**< [GG.D2.TM] acl tcam type. refer CTC_ACL_TCAM_LKUP_TYPE_XXX */
    CTC_PORT_DIR_PROP_ACL_USE_MAPPED_VLAN,         /**< [GG.D2.TM] acl use mapped vlan*/
    CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID,          /**< [GB.GG.D2.TM] acl port bitmap id for port bitmap acl */

    CTC_PORT_DIR_PROP_DOT1AE_EN,                   /**< [D2.TM] Dot1AE enable*/
    CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID,              /**< [D2.TM] bind Dot1AE SC ID to port,value = 0 is unbind*/
    CTC_PORT_DIR_PROP_SD_ACTION,                   /**< [D2.TM] Port enable Singal Degrade, refer to ctc_port_sd_action_type_t */
    MAX_CTC_PORT_DIR_PROP_NUM
};
typedef enum ctc_port_direction_property_e ctc_port_direction_property_t;

/**
  @brief Port mac prefix type
*/
enum ctc_port_mac_prefix_type_e
{
    CTC_PORT_MAC_PREFIX_MAC_NONE    = 0x00,         /**< [GB] none */
    CTC_PORT_MAC_PREFIX_MAC_0       = 0x01,            /**< [GB] port mac prefix 0 */
    CTC_PORT_MAC_PREFIX_MAC_1       = 0x02,            /**< [GB] port mac prefix 1 */
    CTC_PORT_MAC_PREFIX_MAC_ALL     = 0x03,           /**< [GB] port mac prefix 0 and prefix 1 */
    CTC_PORT_MAC_PREFIX_48BIT       = 0x04               /**< [GG.D2.TM] port mac 48bit */
};
typedef enum ctc_port_mac_prefix_type_e  ctc_port_mac_prefix_type_t;

/**
  @brief Port mac prefix parameter
*/
struct ctc_port_mac_prefix_s
{
    uint8       prefix_type;                                                       /**< [GB] MAC Prefix bitmap, ctc_port_mac_prefix_type_t*/
    mac_addr_t  port_mac[CTC_PORT_PREFIX_PORT_MAC_NUM];   /**< [GB] Port MAC, high 40 bit. for GB, high 32bit must same, for GG only port_mac array0 is valid */
};
typedef struct ctc_port_mac_prefix_s ctc_port_mac_prefix_t;

/**
  @brief Port mac postfix parameter
*/
struct ctc_port_mac_postfix_s
{
    uint8       prefix_type;                   /**< [GB.GG.D2.TM] MAC Prefix type, CTC_PORT_MAC_PREFIX_MAC_0: prefix mac0, CTC_PORT_MAC_PREFIX_MAC_1: prefix mac1*/
    uint8       low_8bits_mac;              /**< [GB] Port MAC low 8bit*/
    mac_addr_t  port_mac;                  /**< [GG.D2.TM] Port MAC 48bit */
};
typedef struct ctc_port_mac_postfix_s ctc_port_mac_postfix_t;

#define CTC_PORT_LINK_STATUS_1G     0x00000001
#define CTC_PORT_LINK_STATUS_XG     0x00000002

/**
  @brief Port link status
*/
struct ctc_port_link_status_s
{
    uint32  status_1g[2];       /**< [GB] link status of 1G port */
    uint32  status_xg[1];       /**< [GB] link status of XG port */
    uint32  gport;                  /**< [GB.GG.D2.TM] link change Gport Id */
    uint16  rsv;
};
typedef struct ctc_port_link_status_s ctc_port_link_status_t;

/**
  @brief Phy link status
*/
struct ctc_port_phy_status_s
{
    uint32  gport;                  /**< [D2.TM] Phy link changed Gport Id */
    uint8   bus;                    /**< [D2.TM] bus id>*/
    uint8   phy_addr;               /**< [D2.TM] phy address>*/
    uint16   rsv;
};
typedef struct ctc_port_phy_status_s ctc_port_phy_status_t;

/**
  @brief Port fc property parameter
*/
struct ctc_port_fc_prop_s
{
    uint32 gport;                        /**< [GB.GG.D2.TM] Global port of flow control */
    uint8  priority_class;            /**< [GG.D2.TM] Pfc priority class <0-7>, value should be priority/8, priority refer to qos priority, for D2, value should be priority/2 */
    uint8  is_pfc;                       /**< [GB.GG.D2.TM] If set, Priority based flow contol else port base flow control */

    uint8  pfc_class;                  /**< [GG] PFC frame class value <0-7> */
    uint8  dir;                            /**< [GB.GG.D2.TM] Rx or Tx pause frame */
    uint8  enable;                      /**< [GB.GG.D2.TM] enable/disable flow control */
    uint8  rsv;
};
typedef struct ctc_port_fc_prop_s ctc_port_fc_prop_t;

/**
  @brief Port igs scl hash type
*/
enum ctc_port_igs_scl_hash_type_e
{
    CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE,                 /**< [GB.GG.D2.TM] disable igs scl hash*/

    /*The following Key Type, will correspond to the action of DsUserId or DsSclFlow, through flow_en to decide.*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN,           /**< [GB.GG.D2.TM] igs scl port and double vlan id hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN,           /**< [GB.GG.D2.TM] igs scl port and svlan id hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_CVLAN,           /**< [GB.GG.D2.TM] igs scl port and cvlan id hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_COS,   /**< [GB.GG.D2.TM] igs scl port, svlan id and cos hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_CVLAN_COS,   /**< [GB.GG.D2.TM] igs scl port, cvlan id and cos hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_MAC_SA,                  /**< [GB.GG.D2.TM] igs scl macsa hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA,        /**< [GB.GG.D2.TM] igs scl port and macsa hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_MAC_DA,                 /**< [GB.GG.D2.TM] igs scl macda hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_DA,        /**< [GB.GG.D2.TM] igs scl port and macda hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_IP_SA,                     /**< [GB.GG.D2.TM] igs scl ipsa hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_IP_SA,            /**< [GB.GG.D2.TM] igs scl port and ipsa hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_PORT,                      /**< [GB.GG.D2.TM] igs scl port hash type*/

    /*The following Key Type, will correspond to the action for DsSclFlow*/
    CTC_PORT_IGS_SCL_HASH_TYPE_L2,                          /**< [GB.GG.D2.TM] igs scl l2 hash key, include macda, macsa, svlan id/cos/cfi and ether type*/

    /*The following Key Type, will correspond to the action for DsTunnelId*/
    CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL,                   /**< [GB.GG.D2.TM] igs scl for all IPv4/IPv6 manual tunnel which both IPDA IPSA is lookuped, such as IPv4/IPv6 in IPv4/IPv6, GRE with/without GRE type in IPv4/IPv6 tunnel*/
    CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL_RPF,            /**< [GB.GG.D2.TM] igs scl for all IPv4 manual tunnel do outer head RPF check, must used in scl id 1*/
    CTC_PORT_IGS_SCL_HASH_TYPE_IPV4_TUNNEL_AUTO,          /**< [GG.D2.TM] igs scl for IPv4/IPv6 in IPv4/IPv6 which only IPDA is lookuped, 6to4, ISATAP tunnel*/
    CTC_PORT_IGS_SCL_HASH_TYPE_NVGRE,                     /**< [GG.D2.TM] igs scl for NvGre tunnel*/
    CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN,                     /**< [GG.D2.TM] igs scl for VxLan tunnel*/
    CTC_PORT_IGS_SCL_HASH_TYPE_TRILL,                       /**< [GG.D2.TM] igs scl for TRILL tunnel or check*/

    CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_DSCP,           /**< [TM] igs scl port, svlan id and dscp hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_SVLAN,                /**< [D2.TM] igs scl use svlan id hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_SVLAN_MAC,            /**< [D2.TM] igs scl use svlan id and mac hash type*/
    CTC_PORT_IGS_SCL_HASH_TYPE_MAX
};
typedef enum ctc_port_igs_scl_hash_type_e ctc_port_igs_scl_hash_type_t;

/**
  @brief Port egs scl hash type
*/
enum ctc_port_egs_scl_hash_type_e
{
    CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE,                /**< [GB.GG.D2.TM] disable egs scl hash*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN,             /**< [GB.GG.D2.TM] egs scl port and double vlan id hash type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN,             /**< [GB.GG.D2.TM] egs scl port and svlan id hash type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_CVLAN,             /**< [GB.GG.D2.TM] egs scl port and cvlan id hash type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN_COS,         /**< [GB.GG.D2.TM] egs scl port, svlan id and cos hash type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_CVLAN_COS,         /**< [GB.GG.D2.TM] egs scl port, cvlan id and cos hash type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT,                   /**< [GB.GG.D2.TM] egs scl port hash type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_XC,                /**< [GB.D2.TM] egs scl port cross connect type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_PORT_VLAN_XC,           /**< [D2.TM] egs scl port vlan cross type*/
    CTC_PORT_EGS_SCL_HASH_TYPE_MAX
};
typedef enum ctc_port_egs_scl_hash_type_e ctc_port_egs_scl_hash_type_t;

/**
  @brief Port igs scl tcam type
*/
enum ctc_port_igs_scl_tcam_type_e
{
    CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE,                      /**< [GB.GG.D2.TM] disable igs scl tcam*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_MAC,                            /**< [GB.GG.D2.TM] igs scl mac tcam type*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_IP,                                /**< [GB.GG.D2.TM] (GG)auto adujst for ipv6. for other l3-type packet, including l3,mac info*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_IP_SINGLE,                   /**< [GG.D2.TM] (GG)auto adujst for ipv6. for other l3-type packet, only including l3 info*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_VLAN,                           /**< [GB.GG] igs scl tcam type*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL,                        /**< [GB] for all IPv4 auto tunnel which IPSA do not care, such as ISATAP/6TO4 and anyInIPv6 tunnel*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL_RPF,                 /**< [GB] for all IPv4 auto tunnel and anyInIPv6 tunnel do outer head RPF check, must used in scl id 1*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT,      /**< [D2.TM] resolve scl hash confilct*/
    CTC_PORT_IGS_SCL_TCAM_TYPE_UDF,                             /**< [TM] scl udf tcam type */
    CTC_PORT_IGS_SCL_TCAM_TYPE_UDF_EXT,                      /**< [TM] scl udf extern tcam type */
    CTC_PORT_IGS_SCL_TCAM_TYPE_MAX
};
typedef enum ctc_port_igs_scl_tcam_type_e ctc_port_igs_scl_tcam_type_t;

/**
  @brief Port igs scl action type
*/
enum ctc_port_scl_action_type_e
{
    CTC_PORT_SCL_ACTION_TYPE_SCL,                    /**< [GG.D2] scl action type scl*/
    CTC_PORT_SCL_ACTION_TYPE_FLOW,                 /**< [GG.D2] scl action type flow*/
    CTC_PORT_SCL_ACTION_TYPE_TUNNEL,               /**< [GG.D2] scl action type tunnel*/
    CTC_PORT_SCL_ACTION_TYPE_MAX
};
typedef enum ctc_port_scl_action_type_e ctc_port_scl_action_type_t;

/**
  @brief Port scl property parameter
*/
struct ctc_port_scl_property_s
{
    uint8 scl_id;                           /**< [GB.GG.D2.TM] There are 2 scl lookup<0-1>, and each has its own feature*/
    uint8 direction;                       /**< [GB.GG.D2.TM] direction, CTC_INGRESS or CTC_EGRESS*/

/* property */
    uint8 hash_type;                    /**< [GB.GG.D2.TM] hash type. refer ctc_port_igs_scl_hash_type_t or ctc_port_egs_scl_hash_type_t*/
    uint8 action_type;                  /**< [GG.D2.TM] action type. apply on both scl0 and scl1. refer ctc_port_scl_action_type_t*/
    uint8 tcam_type;                    /**< [GB.GG.D2.TM] ingress only. refer ctc_port_igs_scl_tcam_type_t*/
    uint8 class_id_en;                  /**< [GB.GG.D2.TM] use class id to lookup scl*/
    uint16 class_id;                    /**< [GB.GG.D2.TM] class id. If enable ip source guard,it must be CTC_PORT_CLASS_ID_RSV_IP_SRC_GUARD;
                                                        If enable ip VLAN Class,it must be CTC_PORT_CLASS_ID_RSV_VLAN_CLASS;*/
    uint8 use_logic_port_en;        /**< [GB.GG.D2.TM] SCL lookup port use logic port*/
    uint8 hash_vlan_range_dis;       /**< [TM] Disable hash vlan range */
    uint8 tcam_vlan_range_dis;       /**< [TM] Disable tcam vlan range */
    uint8 hash_mcast_dis;            /**< [TM] When packet is mcast, disable hash lookup */
    uint8 rsv0;
};
typedef struct ctc_port_scl_property_s ctc_port_scl_property_t;


/**
 @brief port bpe  extend type
*/
enum ctc_port_bpe_extend_type_e
{
    CTC_PORT_BPE_EXTEND_TYPE_NONE,     /**< [D2.TM] normal port */
    CTC_PORT_BPE_8021BR_PE_EXTEND,     /**< [D2.TM] Extend port of PE */
    CTC_PORT_BPE_8021BR_PE_CASCADE,    /**< [D2.TM] Cascade port of PE */
    CTC_PORT_BPE_8021BR_PE_UPSTREAM,   /**< [D2.TM] Upstream port of PE */
    CTC_PORT_BPE_EXTEND_TYPE_MAX
};
typedef enum ctc_port_bpe_extend_type_e  ctc_port_bpe_extend_type_t;

/**
  @brief Port bpe property parameter
*/
struct ctc_port_bpe_property_s
{
    uint8  extend_type;             /**< [D2.TM] bpe port extend type */
    uint16  name_space;              /**< [D2.TM] bpe 8021br namespace */
	uint16 ecid;                    /**< [D2.TM] bpe extend ecid */
};
typedef struct ctc_port_bpe_property_s ctc_port_bpe_property_t;


/**
 @brief Port global config parameter
*/
struct ctc_port_global_cfg_s
{
    uint8 default_logic_port_en;    /**< [GB.GG.D2.TM] If set, port will enable default logic port for all local phy port and linkagg group*/
    uint8 isolation_group_mode;  /**< [D2.TM] ctc_isolation_group_mode_t*/
    uint8 use_isolation_id;           /**< [D2.TM] choice port isolation basing port or group*/
    uint8 rsv0;
};
typedef struct ctc_port_global_cfg_s ctc_port_global_cfg_t;

/**
 @brief  Define the mode of isolation group
*/
enum ctc_isolation_group_mode_e
{
    CTC_ISOLATION_GROUP_MODE_64 = 0,     /**< [D2.TM] Isolation group num is 64, Max Member is 64*/
    CTC_ISOLATION_GROUP_MODE_32,         /**< [D2.TM] Isolation group num is 32, Max Member is 128*/
    CTC_ISOLATION_GROUP_MODE_16,        /**< [D2.TM] Isolation group num is 16, Max Member is 256*/
    CTC_ISOLATION_GROUP_MODE_MAX
};
typedef enum ctc_isolation_group_mode_e  ctc_isolation_group_mode_t;

/**
 @brief Port with serdes info
*/
struct ctc_port_serdes_info_s
{
   uint8 serdes_id;                     /**<[GB.GG.D2.TM] Serdes ID, if the port have multi-serdes,indicate it's serdes-id base */
   uint8 serdes_id_array[CTC_PORT_SERDES_MAX_NUM];  /**<[D2.TM] Per port all serdes id, refer to serdes_num */
   uint8 serdes_num;                    /**<[GB.GG.D2.TM] Serdes num */
   uint16 overclocking_speed;           /**< [GB.GG.D2.TM] overclocking speed value */
   uint8 serdes_mode;                   /**< [GB.GG.D2.TM] serdes mode, refer to ctc_chip_serdes_mode_t */
   uint8 rsv[3];
   uint32 gport;                        /**< [GB.GG.D2.TM] gport */
};
typedef struct ctc_port_serdes_info_s ctc_port_serdes_info_t;


/**
 @brief Port capability type
*/
enum ctc_port_capability_type_e
{
    CTC_PORT_CAP_TYPE_SERDES_INFO = 0,      /**< [GB.GG.D2.TM] get serdes info, refer to ctc_port_serdes_info_t */
    CTC_PORT_CAP_TYPE_MAC_ID,               /**< [GB.GG.D2.TM] get mac id */
    CTC_PORT_CAP_TYPE_SPEED_MODE,           /**< [D2.TM] get speed mode bitmap, refer to ctc_port_speed_t */
    CTC_PORT_CAP_TYPE_IF_TYPE,              /**< [D2.TM] get interface type bitmap, refer to ctc_port_if_type_t */
    CTC_PORT_CAP_TYPE_FEC_TYPE,             /**< [D2.TM] get FEC type bitmap, refer to ctc_port_fec_type_t */
    CTC_PORT_CAP_TYPE_CL73_ABILITY,         /**< [D2.TM] get CL73 ability, refer to ctc_port_cl73_ability_t */
    CTC_PORT_CAP_TYPE_CL73_REMOTE_ABILITY,  /**< [D2.TM] get CL73 remote ability, refer to ctc_port_cl73_ability_t */
    CTC_PORT_CAP_MAX_TYPE
};
typedef enum ctc_port_capability_type_e ctc_port_capability_type_t;
/**@} end of @defgroup port  */

/**
 @brief Port interface type
*/
enum ctc_port_if_type_e
{
    CTC_PORT_IF_NONE = 0,      /**< [D2.TM] Invalid interface type */
    CTC_PORT_IF_SGMII,         /**< [D2.TM] SGMII type */
    CTC_PORT_IF_2500X,         /**< [D2.TM] 2500X type */
    CTC_PORT_IF_QSGMII,        /**< [D2.TM] QSGMII type */
    CTC_PORT_IF_USXGMII_S,     /**< [D2.TM] USXGMII Single type */
    CTC_PORT_IF_USXGMII_M2G5,  /**< [D2.TM] USXGMII Multi 2.5G type */
    CTC_PORT_IF_USXGMII_M5G,   /**< [D2.TM] USXGMII Multi 5G type */
    CTC_PORT_IF_XAUI,          /**< [D2.TM] XAUI type */
    CTC_PORT_IF_DXAUI,         /**< [D2.TM] DXAUI type */
    CTC_PORT_IF_XFI,           /**< [D2.TM] XFI type */
    CTC_PORT_IF_KR,            /**< [D2.TM] KR type, 1 lane per port */
    CTC_PORT_IF_CR,            /**< [D2.TM] CR type, 1 lane per port */
    CTC_PORT_IF_KR2,           /**< [D2.TM] KR2 type, 2 lanes per port */
    CTC_PORT_IF_CR2,           /**< [D2.TM] CR2 type, 2 lanes per port */
    CTC_PORT_IF_KR4,           /**< [D2.TM] KR4 type, 4 lanes per port */
    CTC_PORT_IF_CR4,           /**< [D2.TM] CR4 type, 4 lanes per port */
    CTC_PORT_IF_FX,            /**< [TM] 100BaseFX */

    CTC_PORT_IF_MAX_TYPE
};
typedef enum ctc_port_if_type_e ctc_port_if_type_t;

/**
 @brief Port interface mode
*/
struct ctc_port_if_mode_s
{
    ctc_port_speed_t     speed;        /**< [D2.TM] port speed*/
    ctc_port_if_type_t   interface_type;      /**< [D2.TM] port interface type*/
};
typedef struct ctc_port_if_mode_s ctc_port_if_mode_t;


/**
  @brief Port mux type
*/
enum ctc_port_mux_type_e
{
    CTC_PORT_MUX_TYPE_NONE                    = 0x00,            /**< [GG] Without outer stacking header encap */
    CTC_PORT_MUX_TYPE_CLOUD_L2_HDR            = 0x01,            /**< [GG] With L2 cloud stacking header format encap */
    CTC_PORT_MUX_TYPE_MAX
};
typedef enum ctc_port_mux_type_e  ctc_port_mux_type_t;


#ifdef __cplusplus
}
#endif

#endif

