/**
 @file ctc_ipmc.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-01-25

 @version v2.0

   This file contains all IP Multicast related data structure, enum, macro and proto.
*/

#ifndef _CTC_IPMC_H_
#define _CTC_IPMC_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_l3if.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup ipmc IPMC
 @{
*/
#define CTC_IPMC_FLAG_DROP                      0x00000001  /**< [GB.GG.D2.TM] Packets will be dropped */
#define CTC_IPMC_FLAG_COPY_TOCPU                0x00000002  /**< [GB.GG.D2.TM] Packets will be copied to CPU */
#define CTC_IPMC_FLAG_RPF_CHECK                 0x00000004  /**< [GB.GG.D2.TM] Packets will do RPF check */
#define CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU 0x00000010  /**< [GB.GG.D2.TM] Packets will be sent to CPU and fallback bridged */
#define CTC_IPMC_FLAG_BIDPIM_CHECK              0x00000020  /**< [GB.GG.D2.TM] Packets will do bidirection PIM check */
#define CTC_IPMC_FLAG_TTL_CHECK                 0x00000040  /**< [GB.GG.D2.TM] Packets will do TTL check */
#define CTC_IPMC_FLAG_KEEP_EMPTY_ENTRY          0x00000100  /**< [GB.GG.D2.TM] When removing group,  only delete all member */
#define CTC_IPMC_FLAG_STATS                     0x00000200  /**< [GB.GG.D2.TM] Statistic will be supported*/
#define CTC_IPMC_FLAG_REDIRECT_TOCPU            0x00000400  /**< [GB.GG.D2.TM] Packets will be redirected to CPU*/
#define CTC_IPMC_FLAG_TUNNEL                    0x00000800  /**< [GB.D2.TM] Packets will be encap */
#define CTC_IPMC_FLAG_L2MC                      0x00001000  /**< [GB.GG.D2.TM] Packets will do l2 mcast by ip */
#define CTC_IPMC_FLAG_PTP_ENTRY                 0x00002000  /**< [GB.GG.D2.TM] Mcast PTP IP address, using for IP DA */
#define CTC_IPMC_FLAG_BIDIPIM_FAIL_BRIDGE_AND_TOCPU 0x00004000  /**< [GG.D2.TM] Packets will be sent to CPU and fallback bridged */
#define CTC_IPMC_FLAG_SHARE_GROUP           0x00008000  /**< [GG.D2.TM] if set, means share the same mcast group*/
#define CTC_IPMC_FLAG_USE_VLAN                   0x00010000 /**< [D2.TM] if set, means vrfid in IPMC key is svlan id*/
#define CTC_IPMC_FLAG_WITH_NEXTHOP               0x00020000 /**< [GB.GG.D2.TM] if set, means add ipmc group with nexthop id*/
#define CTC_IPMC_MAX_MEMBER_PER_GROUP           64          /**< [GB.GG.D2.TM] Define max member that an ipmc group can contain*/

/**
 @brief Ipv4/6 multicast default actions
*/
enum ctc_ipmc_default_action_e
{
    CTC_IPMC_DEFAULT_ACTION_DROP = 0,           /**< [GB.GG.D2.TM] packet will be discarded*/
    CTC_IPMC_DEFAULT_ACTION_TO_CPU = 1,         /**< [GB.GG.D2.TM] packet will be sent to cpu*/
    CTC_IPMC_DEFAULT_ACTION_FALLBACK_BRIDGE = 2, /**< [GB.GG.D2.TM] packet will fall back bridge and to cpu*/
    CTC_IPMC_DEFAULT_ACTION_MAX
};
typedef enum ctc_ipmc_default_action_e ctc_ipmc_default_action_t;

/**
 @brief Ipmc force route
*/
struct ctc_ipmc_force_route_s
{
    uint8 ip_version;       /**< [GB.GG.D2.TM] ctc_ip_ver_t, v4 or v6*/
    uint8 force_bridge_en;  /**< [GB.GG.D2.TM] force special mcast address to bridge*/
    uint8 force_ucast_en;   /**< [GB.GG.D2.TM] force special mcast address to do ucast, lower priority than force bridge*/
    uint8 rsv;              /**< reserved*/

    uint8 ipaddr0_valid;    /**< [GB.GG.D2.TM] first ip address is enable*/
    uint8 ipaddr1_valid;    /**< [GB.GG.D2.TM] second ip address is enable*/
    uint8 addr0_mask;       /**< [GB.GG.D2.TM] first ip address mask length*/
    uint8 addr1_mask;       /**< [GB.GG.D2.TM] second ip address mask length*/

    union
    {
        ip_addr_t ipv4;     /**< [GB.GG.D2.TM] Ipv4 address*/
        ipv6_addr_t ipv6;   /**< [GB.GG.D2.TM] Ipv6 address*/
    } ip_addr0;             /**< [GB.GG.D2.TM] first ip address*/

    union
    {
        ip_addr_t ipv4;     /**< [GB.GG.D2.TM] Ipv4 address*/
        ipv6_addr_t ipv6;   /**< [GB.GG.D2.TM] Ipv6 address*/
    } ip_addr1;             /**< [GB.GG.D2.TM] second ip address*/

};
typedef struct ctc_ipmc_force_route_s ctc_ipmc_force_route_t;

/**
 @brief Ipv4 multicast address.
*/
struct ctc_ipmc_ipv4_addr_s
{
    uint32 src_addr;           /**< [GB.GG.D2.TM] IPv4 Source address */
    uint32 group_addr;         /**< [GB.GG.D2.TM] IPv4 Group address */
    uint32 vrfid;              /**< [GB.GG.D2.TM] VRF Id ,if IP-L2MC,used as FID.
                                    [GG.D2.TM] When set vrf_mode by CTC_GLOBAL_IPMC_PROPERTY, vrfid is svlan id.
                                    for D2, When set CTC_L3IF_PROP_IPMC_USE_VLAN_EN, vrfid is svlan id */

};
typedef struct ctc_ipmc_ipv4_addr_s ctc_ipmc_ipv4_addr_t;

/**
 @brief Ipv6 multicast address.
*/
struct ctc_ipmc_ipv6_addr_s
{
    ipv6_addr_t src_addr;          /**< [GB.GG.D2.TM] IPv6 Source address */
    ipv6_addr_t group_addr;        /**< [GB.GG.D2.TM] IPv6 Group address */
    uint32 vrfid;                  /**< [GB.GG.D2.TM] VRF Id ,if IP-L2MC,used as FID.
                                        [GG.D2.TM] When set vrf_mode by CTC_GLOBAL_IPMC_PROPERTY, vrfid is svlan id.
                                        for D2, When set CTC_L3IF_PROP_IPMC_USE_VLAN_EN, vrfid is svlan id */
};
typedef struct ctc_ipmc_ipv6_addr_s ctc_ipmc_ipv6_addr_t;

/**
 @brief Union structure that stores ipv4 or ipv6 address.
*/
union ctc_ipmc_addr_info_u
{
    ctc_ipmc_ipv4_addr_t ipv4;      /**< [GB.GG.D2.TM] IPV4 address structure */
    ctc_ipmc_ipv6_addr_t ipv6;      /**< [GB.GG.D2.TM] IPV6 address structure */
};
typedef union ctc_ipmc_addr_info_u ctc_ipmc_addr_info_t;

/**
 @brief  Define ipmc member flag
*/
enum ctc_ipmc_member_flag_e
{
    CTC_IPMC_MEMBER_FLAG_ASSIGN_PORT    = 0x00000001,    /**< [GB.GG.D2.TM]Use global_port as the assign port when set is_nh */
    CTC_IPMC_MEMBER_FLAG_LEAF_CHECK_EN  = 0x00000002,    /**< [GG.D2.TM] Enable leaf check */
    CTC_IPMC_MEMBER_FLAG_MTU_NO_CHECK   = 0x00000004,    /**< [GB.GG.D2.TM] If set , means mtu check disable */
    CTC_IPMC_MEMBER_FLAG_MAX
};
typedef enum ctc_ipmc_member_flag_e ctc_ipmc_member_flag_t;

/**
 @brief Data structure that stores member information.\
     ip based L2mc, if out port is switchport,it should be l3_if_type set to MAX_L3IF_TYPE_NUM or vlan_port set to 1.
                    if out port is L3if, it should be l3_if_type set to ctc_l3if_type_t valid
*/
struct ctc_ipmc_member_info_s
{
    uint32 flag;                /**< [GB.GG.D2.TM] Member flag, refer to ctc_ipmc_member_flag_t */
    uint32 global_port;         /**< [GB.GG.D2.TM] Member port if member is local member, gport: gchip(8bit) + local phy port(8bit);
                                     else if member is remote chip entry, gport: gchip(8bit) + mcast_profile_id(8bit) */
    ctc_port_bitmap_t  port_bmp;/**< [GB.GG.D2.TM] port bitmap, used for add/remove members */
    uint8   port_bmp_en;        /**< [GB.GG.D2.TM] if set, means add/remove members by bitmap */
    uint8   gchip_id;           /**< [GB.GG.D2.TM] global chip id, used for add/remove port bitmap member,gchip_id=0x1f means linkagg*/
    uint16 vlan_id;             /**< [GB.GG.D2.TM] Vlan id */
    ctc_l3if_type_t l3_if_type; /**< [GB.GG.D2.TM] Layer3 interface type */
    bool remote_chip;           /**< [GB.GG.D2.TM] If set, member is remote chip entry */
    bool vlan_port;             /**< [GB.GG.D2.TM] If set and  l3if is vlan interface , output packet only do L2 bridging */
    bool is_nh;                 /**< [GB.GG.D2.TM] If set add member by nexthop */
    bool re_route;              /**< [GG.D2.TM] If set add loop back member */

    uint32 nh_id;               /**< [GB.GG.D2.TM] Nexthop Id */
    uint16 logic_dest_port;     /**< [GG.D2.TM] logic dest port, not zero will do logic port check*/
};
typedef struct ctc_ipmc_member_info_s ctc_ipmc_member_info_t;

/**
 @brief Data structure that stores group information
*/
struct ctc_ipmc_group_info_s
{
    uint8  ip_version;                             /**< [GB.GG.D2.TM] 0 IPV4, 1 IPV6 */
    uint8  member_number;                          /**< [GB.GG.D2.TM] The numbers of member added to this group, not support this info for reply */
    uint8  src_ip_mask_len;                        /**< [GB.GG.D2.TM] IP Source address mask length, V4: 0, 32; V6: 0, 128 */
    uint8  group_ip_mask_len;                      /**< [GB.GG.D2.TM] IP Group address mask length, V4: 0, 32; V6: 0, 128 */
    uint8  rpf_intf_valid[CTC_IP_MAX_RPF_IF];      /**< [GB.GG.D2.TM] RPF check valid flag */
    uint16 rpf_intf[CTC_IP_MAX_RPF_IF];            /**< [GB.GG.D2.TM] RPF check interface id or port */
    uint32 flag;                                   /**< [GB.GG.D2.TM] Rules that define actions for packets of this group,CTC_IPMC_FLAG_XXX,
                                      CTC_IPMC_FLAG_TTL_CHECK flag to enable TTL check;CTC_IPMC_FLAG_RPF_CHECK flag to enable RPF check;
                                      CTC_IPMC_FLAG_L2MC flag to enable L2MC based on IP;CTC_IPMC_FLAG_STATS flag to enable  ipmc statistic
                                      CTC_IPMC_FLAG_COPY_TOCPU, CTC_IPMC_FLAG_REDIRECT_TOCPU flag to enable ipda exception. CTC_IPMC_FLAG_BIDPIM_CHECK and
                                      CTC_IPMC_FLAG_RPF_CHECK can not enable together */
    uint32 stats_id;                               /**< [GB.GG.D2.TM] Stats id */
    uint16 group_id;                               /**< [GB.GG.D2.TM] Identify a group */
    uint8   rp_id;                                 /**< [GG.D2.TM] rp id */
    uint8   statsFail;                             /**< [GB] Return value,if ipmc stats don't get stats resource, not support this info for reply */
    ctc_ipmc_member_info_t ipmc_member[CTC_IPMC_MAX_MEMBER_PER_GROUP];  /**< [GB.GG.D2.TM] record member information, not support this info for reply */
    ctc_ipmc_addr_info_t address;                  /**< [GB.GG.D2.TM] Union structure stores ipv4 or ipv6 address */
    uint32  nhid;                                  /**< [GB.GG.D2.TM] Add ipmc group with nexthop id, used when flag CTC_IPMC_FLAG_WITH_NEXTHOP is set*/
};
typedef struct ctc_ipmc_group_info_s  ctc_ipmc_group_info_t;

/**
 @brief Rp information
*/
struct ctc_ipmc_rp_s
{
    uint8   rp_id;                                 /**< [GG.D2.TM] rp id */
    uint8   rp_intf_count;                         /**< [GG.D2.TM] rp interface count valid */
    uint8   rsv[2];
    uint16  rp_intf[CTC_IPMC_RP_INTF];             /**< [GG.D2.TM] rp interface configured */
};
typedef struct ctc_ipmc_rp_s  ctc_ipmc_rp_t;

typedef int32 (* ctc_ipmc_traverse_fn)(ctc_ipmc_group_info_t* p_ipmc_param, void* user_data);

/**@} end of @defgroup  ipmc IPMC */
#ifdef __cplusplus
}
#endif

#endif

