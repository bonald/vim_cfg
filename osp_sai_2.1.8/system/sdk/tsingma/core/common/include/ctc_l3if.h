/**
 @file ctc_l3if.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-1-7

 @version v2.0

This file contains all L3 interface related data structure, enum, macro and proto.

*/
#ifndef _CTC_L3_IF
#define _CTC_L3_IF
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_stats.h"
#include "common/include/ctc_common.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup l3if L3if
 @{
*/

/*-----------------------L3if id allocation ---------------------
Humber :
 Vlan L3if,Phy L3if and sub L3if share 1023 L3 interface.
 |0...............................................................................................1022|
 |Phy and sub l3if ID(0~511)  ->                      <-(0~1022) vlan l3if ID   |

 vlan l3if             range:  0~1022
 Phy and sub l3if range:   0~511
 Greatbelt:
 Vlan L3if, Phy L3if, sub L3if share 1022 interface ID, the range is 1~1023.
 Goldengate:
 Vlan L3if, Phy L3if share 1023 interface ID(0~1022), Sub interface not support now.
-----------------------------------------------------------*/
#define MAX_CTC_L3IF_VMAC_MAC_INDEX         3       /**< [GB] Max index of layer 3 interface virtual router-mac */
#define MAX_CTC_L3IF_MCAST_TTL_THRESHOLD    256     /**< [GB.GG.D2.TM] Max TTL threshold of layer 3 interface multicast */
#define MAX_CTC_L3IF_MTU_SIZE               16 * 1024 /**< [GB.GG.D2.TM] Max MTU size of layer 3 interface */
#define MAX_CTC_L3IF_PBR_LABEL              0x3F    /**< [GB.GG.D2] Max PBR label value of layer 3 interface */

/**
 @brief l3if global config information
*/
struct ctc_l3if_global_cfg_s
{
    uint16 max_vrfid_num;           /**< [GB.GG.D2.TM]Max vrf id supported in l3 module, MUST be times of 64, default is 256 */
    uint8 ipv4_vrf_en;              /**< [GB.GG.D2.TM]If set, IPv4 enable vrf lookup */
    uint8 ipv6_vrf_en;              /**< [GB.GG.D2.TM]If set, IPv6 enable vrf lookup */
};
typedef struct ctc_l3if_global_cfg_s ctc_l3if_global_cfg_t;

/**
 @brief  Define the prefix type of  l3 interface 40bits Router MAC
*/
enum ctc_l3if_route_mac_type_e
{
    CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE0,                /**< [GB] Type 0*/
    CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE1,                /**< [GB] Type 1*/
    CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_RSV_ROUTER_MAC,  /**< [GB] Type 2,will be reserved for per-system router mac*/
    CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_INVALID,         /**< [GB] Non-support type*/
    MAX_CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE
};
typedef enum ctc_l3if_route_mac_type_e  ctc_l3if_route_mac_type_t;

/**
 @brief  Define the type of l3if
*/
enum ctc_l3if_type_e
{
    CTC_L3IF_TYPE_PHY_IF,    /**< [GB.GG.D2.TM] Physical IP interfaces*/
    CTC_L3IF_TYPE_SUB_IF,    /**< [GB.GG.D2.TM] Sub-interface*/
    CTC_L3IF_TYPE_VLAN_IF,   /**< [GB.GG.D2.TM] Logical IP Vlan interfaces*/
    CTC_L3IF_TYPE_SERVICE_IF,   /**< [GB.GG.D2.TM] Service interfaces for input*/
    MAX_L3IF_TYPE_NUM
};
typedef enum ctc_l3if_type_e  ctc_l3if_type_t;

/**
 @brief  Define the lookup operation for of IPSA lookup
*/
enum ctc_l3if_ipsa_lkup_type_e
{
    CTC_L3IF_IPSA_LKUP_TYPE_NONE, /**< [GB.GG.D2.TM] Don't do IPSA lookup*/
    CTC_L3IF_IPSA_LKUP_TYPE_RPF,  /**< [GB.GG.D2.TM] IPSA lookup operation used as RPF check */
    CTC_L3IF_IPSA_LKUP_TYPE_NAT,  /**< [GB.GG.D2.TM] IPSA lookup operation used as NAT */
    CTC_L3IF_IPSA_LKUP_TYPE_PBR,  /**< [GB.GG.D2.TM] IPSA lookup operation used as PBR*/
    MAX_CTC_L3IF_IPSA_LKUP_TYPE
};
typedef enum ctc_l3if_ipsa_lkup_type_e ctc_l3if_ipsa_lkup_type_t;

/**
  @brief   Define l3if property flags
*/
enum ctc_l3if_property_e
{
    CTC_L3IF_PROP_ROUTE_EN,               /**< [GB.GG.D2.TM] Enable IP Routing */
    CTC_L3IF_PROP_PBR_LABEL,              /**< [GB.GG.D2] Set Policy-based routing label*/
    CTC_L3IF_PROP_NAT_IFTYPE,             /**< [GB.GG.D2.TM] The type of NAT,0- external,1- internal*/
    CTC_L3IF_PROP_ROUTE_ALL_PKT,          /**< [GB.GG.D2.TM] If set,all packets are routed*/
    CTC_L3IF_PROP_IPV4_UCAST,             /**< [GB.GG.D2.TM] If set,IpV4 Ucast Routing will be enabled*/
    CTC_L3IF_PROP_IPV4_MCAST,             /**< [GB.GG.D2.TM] If set,IpV4 Mcast Routing will be enabled*/
    CTC_L3IF_PROP_IPV4_SA_TYPE,           /**< [GB.GG.D2.TM] SA lkup operation for IPV4 Lookup, refer to ctc_l3if_ipsa_lkup_type_t */
    CTC_L3IF_PROP_IPV6_UCAST,             /**< [GB.GG.D2.TM] If set,IpV6 Ucast Routing will be enabled*/
    CTC_L3IF_PROP_IPV6_MCAST,             /**< [GB.GG.D2.TM] If set,IpV6 Mcast Routing will be enabled*/
    CTC_L3IF_PROP_IPV6_SA_TYPE,           /**< [GB.GG.D2.TM] SA lkup operation for IPV6 Lookup, refer to ctc_l3if_ipsa_lkup_type_t  */
    CTC_L3IF_PROP_VRF_ID,                 /**< [GB.GG.D2.TM] The ID for virtual route forward table */
    CTC_L3IF_PROP_MTU_EN,                 /**< [GB.GG.D2.TM] MTU check is enabled on the egress interface*/
    CTC_L3IF_PROP_MTU_SIZE,               /**< [GB.GG.D2.TM] MTU size of the egress interface*/
    CTC_L3IF_PROP_MTU_EXCEPTION_EN,       /**< [GB.GG.D2.TM] MTU Exception enable for packet to cpu to do fragment */
    CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE,  /**< [GB] The prefix type of router MAC, value should be ctc_l3if_route_mac_type_e */
    CTC_L3IF_PROP_ROUTE_MAC_LOW_8BITS,    /**< [GB] Low 8 bits of router MAC  */
    CTC_L3IF_PROP_EGS_MAC_SA_PREFIX_TYPE, /**< [GB] The prefix type of router MAC,to be used in the MAC SA for routed packets EPE edit*/
    CTC_L3IF_PROP_EGS_MAC_SA_LOW_8BITS,   /**< [GB] Low 8 bits for the MAC SA for routed packets EPE edit*/
    CTC_L3IF_PROP_EGS_MCAST_TTL_THRESHOLD,/**< [GB.GG.D2.TM] Multicast TTL threshold */
    CTC_L3IF_PROP_MPLS_EN,                /**< [GB.GG.D2.TM] Enable MPLS */
    CTC_L3IF_PROP_MPLS_LABEL_SPACE,       /**< [GB.GG.D2.TM] Set MPLS label space */
    CTC_L3IF_PROP_VRF_EN,                 /**< [GB.GG.D2.TM] Enable VRF lookup */
    CTC_L3IF_PROP_IGMP_SNOOPING_EN,       /**< [GG.D2.TM] Enable IGMP snooping */
    CTC_L3IF_PROP_RPF_CHECK_TYPE,         /**< [GB.GG.D2.TM] RPF check type */
    CTC_L3IF_PROP_CONTEXT_LABEL_EXIST,    /**< [GG.D2.TM] If set, identify mpls packets on this interface have context label*/
    CTC_L3IF_PROP_RPF_PERMIT_DEFAULT,     /**< [GB.GG.D2.TM] Permit default ipsa pass RPF Check */
    CTC_L3IF_PROP_STATS,                  /**< [GB.GG.D2.TM] Stats en for interface(GB only support sub-if). if value is 0,disable stats, or value is statsid*/
    CTC_L3IF_PROP_EGS_STATS,              /**< [GG.D2.TM] Stats en for interface. if value is 0,disable stats, or value is statsid*/
    CTC_L3IF_PROP_CID,                    /**< [D2.TM] Category Id. */
    CTC_L3IF_PROP_PUB_ROUTE_LKUP_EN,      /**< [D2.TM] Enable public route lookup */
    CTC_L3IF_PROP_IPMC_USE_VLAN_EN,       /**< [D2.TM] Enable vlan of ipmc */
    CTC_L3IF_PROP_INTERFACE_LABEL_EN,     /**< [D2.TM] Enable interface lable */
    CTC_L3IF_PROP_PHB_USE_TUNNEL_HDR,     /**< [D2.TM] Phb use tunnel hdr */
    CTC_L3IF_PROP_TRUST_DSCP,             /**< [D2.TM] [Ingress]Trust dscp */
    CTC_L3IF_PROP_DSCP_SELECT_MODE,       /**< [D2.TM] [Egress]Dscp select mode, refer to ctc_dscp_select_mode_t */
    CTC_L3IF_PROP_IGS_QOS_DSCP_DOMAIN,    /**< [D2.TM] Ingress QOS DSCP domain */
    CTC_L3IF_PROP_EGS_QOS_DSCP_DOMAIN,    /**< [D2.TM] Egress QOS DSCP domain */
    CTC_L3IF_PROP_DEFAULT_DSCP,           /**< [D2.TM] [Egress]Default DSCP, if DSCP_SELECT_MODE is CTC_DSCP_SELECT_ASSIGN, the default dscp will rewrite packet's dscp */

    MAX_CTC_L3IF_PROP_NUM
};
typedef enum ctc_l3if_property_e  ctc_l3if_property_t;

/**
 @brief  Define vrf stats type
*/
enum ctc_l3if_vrf_stats_type_e
{
    CTC_L3IF_VRF_STATS_UCAST,    /**< [GB.GG.D2.TM] Only stats ucast route packet */
    CTC_L3IF_VRF_STATS_MCAST,    /**< Only stats mcast route packet */
    CTC_L3IF_VRF_STATS_ALL,      /**< [GG.D2.TM] Stats all route packet */
    MAX_L3IF_VRF_STATS_TYPE
};
typedef enum ctc_l3if_vrf_stats_type_e  ctc_l3if_vrf_stats_type_t;

/**
 @brief  Define vrf stats
*/
struct ctc_l3if_vrf_stats_s
{
    ctc_l3if_vrf_stats_type_t type;     /**< [GB.GG.D2.TM] VRF stats type */
    uint16 vrf_id;                      /**< [GB.GG.D2.TM] The ID for virtual route forward table */
    uint8 enable;                       /**< [GB.GG.D2.TM] Enable stats */
    uint32 stats_id;                    /**< [GB.GG.D2.TM] Stats id*/

    ctc_stats_basic_t stats;            /**< [GB] Output stats info */
};
typedef struct ctc_l3if_vrf_stats_s  ctc_l3if_vrf_stats_t;

/**
  @brief  Define L3if structure
 */
struct ctc_l3if_s
{
    uint8   l3if_type; /**< [GB.GG.D2.TM] The type of l3interface, CTC_L3IF_TYPE_XXX */
    uint8   rsv;
    uint32  gport;     /**< [GB.GG.D2.TM] Global port ID */
    uint16  vlan_id;   /**< [GB.GG.D2.TM] Vlan Id */
};
typedef struct ctc_l3if_s  ctc_l3if_t;

/**
  @brief   Define L3if  VMAC structure
 */
struct ctc_l3if_vmac_s
{
    uint8   low_8bits_mac_index; /**< [GB] The index of VMAC entry,0~3*/
    uint8   prefix_type;         /**< [GB] ctc_l3if_route_mac_type_t */
    uint8   low_8bits_mac;       /**< [GB] The id of L3 ingress interface */
    uint8   rsv;
};
typedef struct ctc_l3if_vmac_s  ctc_l3if_vmac_t;

enum ctc_l3if_router_mac_op_e
{
    CTC_L3IF_UPDATE_ROUTE_MAC,  /**< [D2.TM] Update all routermac */
    CTC_L3IF_APPEND_ROUTE_MAC,  /**< [D2.TM] Append a routermac */
    CTC_L3IF_DELETE_ROUTE_MAC,  /**< [D2.TM] Delete a routermac */
};
typedef enum ctc_l3if_router_mac_op_e ctc_l3if_router_mac_op_t;
/**
  @brief   Define L3if router mac
 */
struct ctc_l3if_router_mac_s
{
    mac_addr_t  mac[4];     /**< [GG.D2.TM] Router mac*/
    uint32      num;        /**< [GG.D2.TM] Valid router mac number in mac*/
    uint8       mode;       /**< [D2.TM] only for ctc_l3if_set_interface_router_mac, refer to ctc_l3if_router_mac_op_t */
    uint8       dir;        /**< [D2.TM] CTC_INGRESS/CTC_EGRESS. when it's CTC_EGRESS, only mac[0] is valid*/
    uint32  next_query_index;   /**< [TM] [in][out] only for ctc_l3if_get_interface_router_mac,
                                                    indicate the start index and return index of the next query,
                                                    return 0xffffffff means end*/
};
typedef struct ctc_l3if_router_mac_s  ctc_l3if_router_mac_t;

/**
  @brief   Define L3if ECMP interface
 */
struct ctc_l3if_ecmp_if_s
{
    uint16 ecmp_if_id;          /**< [GG.D2.TM] ecmp interface id <1-1024> */
    uint16 ecmp_type;           /**< [GG.D2.TM] ecmp type, refer to ctc_nh_ecmp_type_t, only support static and DLB */
    uint8  failover_en;         /**< [GG.D2.TM] Indicate linkdown use hw based linkup member select, only used for static ecmp */
    uint32 stats_id;            /**< [GG.D2.TM] Stats id*/
};
typedef struct ctc_l3if_ecmp_if_s  ctc_l3if_ecmp_if_t;

#ifdef __cplusplus
}
#endif

#endif
/**@} end of @defgroup  l3if L3if */

