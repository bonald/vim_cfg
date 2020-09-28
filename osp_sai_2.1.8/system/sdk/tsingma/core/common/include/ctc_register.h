/**
 @file ctc_register.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-7-26

 @version v2.0

   This file contains all register related data structure, enum, macro and proto.
*/

#ifndef _CTC_REGISTER_H
#define _CTC_REGISTER_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_parser.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define CTC_LB_HASH_OFFSET_DISABLE      0xFFFF
#define CTC_LB_HASH_ECMP_GLOBAL_PROFILE 0xFF
#define CTC_DUMP_DB_FILE_NAME     256
#define CTC_DUMP_DB_BIT_MAP_NUM     2
/**
 @brief  Define global chip capability type
*/
enum ctc_global_capability_type_e
{
    CTC_GLOBAL_CAPABILITY_LOGIC_PORT_NUM = 0,              /**< [GB.GG.D2.TM] logic port num, include reserved logic port 0 to disable logic port */
    CTC_GLOBAL_CAPABILITY_MAX_FID,                                 /**< [GB.GG.D2.TM] max fid */
    CTC_GLOBAL_CAPABILITY_MAX_VRFID,                            /**< [GB.GG.D2.TM] max vrfid */
    CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM,               /**< [GB.GG.D2.TM] mcast group num, include l2mc, ipmc, vlan default entry, include reserved group id 0 for tx c2c packet on stacking  */
    CTC_GLOBAL_CAPABILITY_VLAN_NUM,                             /**< [GB.GG.D2.TM] vlan num, include reserved vlan 0 */
    CTC_GLOBAL_CAPABILITY_VLAN_RANGE_GROUP_NUM,      /**< [GB.GG.D2.TM] vlan range group num */
    CTC_GLOBAL_CAPABILITY_STP_INSTANCE_NUM,               /**< [GB.GG.D2.TM] stp instance num */
    CTC_GLOBAL_CAPABILITY_LINKAGG_GROUP_NUM,             /**< [GB.GG.D2.TM] linkagg group num */
    CTC_GLOBAL_CAPABILITY_LINKAGG_MEMBER_NUM,           /**< [GB.GG.D2.TM] linkagg per group member num */
    CTC_GLOBAL_CAPABILITY_LINKAGG_DLB_FLOW_NUM,       /**< [GB.GG.D2.TM] linkagg dlb per group flow num */
    CTC_GLOBAL_CAPABILITY_LINKAGG_DLB_MEMBER_NUM,   /**< [GB.GG.D2.TM] linkagg per group dlb member num */
    CTC_GLOBAL_CAPABILITY_LINKAGG_DLB_GROUP_NUM,     /**< [GB.GG.D2.TM] linkagg dlb group num */
    CTC_GLOBAL_CAPABILITY_ECMP_GROUP_NUM,                  /**< [GG.D2.TM] ecmp group num, include reserved ecmp group id 0 to disable ecmp */
    CTC_GLOBAL_CAPABILITY_ECMP_MEMBER_NUM,               /**< [GB.GG.D2.TM] ecmp per group member num */
    CTC_GLOBAL_CAPABILITY_ECMP_DLB_FLOW_NUM,            /**< [GG.D2.TM] ecmp dlb flow num */
    CTC_GLOBAL_CAPABILITY_EXTERNAL_NEXTHOP_NUM,      /**< [GB.GG.D2.TM] external nexthop num, include reserved nhid 0-2 refer to ctc_nh_reserved_nhid_t */
    CTC_GLOBAL_CAPABILITY_GLOBAL_DSNH_NUM,              /**< [GB.GG.D2.TM] nexthop offset num, include reserved nexthop offset 0 for rspan */
    CTC_GLOBAL_CAPABILITY_MPLS_TUNNEL_NUM,                /**< [GB.GG.D2.TM] mpls tunnel id num*/
    CTC_GLOBAL_CAPABILITY_ARP_ID_NUM,                          /**< [GB.GG.D2.TM] max arp id num, include reserved arp id 0 to disable arp */
    CTC_GLOBAL_CAPABILITY_L3IF_NUM,                             /**< [GB.GG.D2.TM] l3 interface num, include reserved ifid 0 to disable L3if */
    CTC_GLOBAL_CAPABILITY_OAM_SESSION_NUM,              /**< [GB.GG.D2.TM] oam session num, include 1 reserved session */
    CTC_GLOBAL_CAPABILITY_NPM_SESSION_NUM,              /**< [GB.GG.D2.TM] npm session num */
    CTC_GLOBAL_CAPABILITY_APS_GROUP_NUM,                 /**< [GB.GG.D2.TM] aps group num */
    CTC_GLOBAL_CAPABILITY_TOTAL_POLICER_NUM,         /**< [GB.GG.D2.TM] total policer num, include port, flow and service, include reserved policer id 0 to disable policer */
    CTC_GLOBAL_CAPABILITY_POLICER_NUM,                    /**< [GB.GG.D2.TM] policer num, include flow and service, include reserved policer id 0 to disable policer */

    CTC_GLOBAL_CAPABILITY_TOTAL_STATS_NUM,              /**< [GB.GG.D2.TM] total stats num, include reserved stats 0 to disable stats */
    CTC_GLOBAL_CAPABILITY_QUEUE_STATS_NUM,              /**< [GB.GG.D2.TM] queue stats num */
    CTC_GLOBAL_CAPABILITY_POLICER_STATS_NUM,            /**< [GB.GG.D2.TM] policer stats num */
    CTC_GLOBAL_CAPABILITY_SHARE1_STATS_NUM,             /**< [GG.D2.TM] share1 stats num, include VRF,IPMC,MPLS VC label,TUNNEL Egress,SCL Egress,Nexthop,Nexthop MPLS PW,Nexthop Mcast,L3IF Egress,FID Ingress*/
    CTC_GLOBAL_CAPABILITY_SHARE2_STATS_NUM,             /**< [GG.D2.TM] share2 stats num, include MPLS TUNNEL Label,TUNNEL Ingress,SCL Ingress,Nexthop MPLS LSP,L3IF Ingress,FID Egress */
    CTC_GLOBAL_CAPABILITY_SHARE3_STATS_NUM,             /**< [D2.TM] global share3 stats num */
    CTC_GLOBAL_CAPABILITY_SHARE4_STATS_NUM,             /**< [D2.TM] global share4 stats num */
    CTC_GLOBAL_CAPABILITY_ACL0_IGS_STATS_NUM,           /**< [GG.D2.TM] acl0 ingress stats num */
    CTC_GLOBAL_CAPABILITY_ACL1_IGS_STATS_NUM,           /**< [GG.D2.TM] acl1 ingress stats num */
    CTC_GLOBAL_CAPABILITY_ACL2_IGS_STATS_NUM,           /**< [GG.D2.TM] acl2 ingress stats num */
    CTC_GLOBAL_CAPABILITY_ACL3_IGS_STATS_NUM,           /**< [GG.D2.TM] acl3 ingress stats num */
    CTC_GLOBAL_CAPABILITY_ACL0_EGS_STATS_NUM,           /**< [GG.D2.TM] acl0 egress stats num */
    CTC_GLOBAL_CAPABILITY_ECMP_STATS_NUM,                  /**< [GG.D2.TM]  ecmp stats num */

    CTC_GLOBAL_CAPABILITY_ROUTE_MAC_ENTRY_NUM,        /**< [GB.GG.D2.TM] route mac entry num */
    CTC_GLOBAL_CAPABILITY_MAC_ENTRY_NUM,                    /**< [GB.GG.D2.TM] mac entry num, include l2mc */
    CTC_GLOBAL_CAPABILITY_BLACK_HOLE_ENTRY_NUM,        /**< [GB.GG.D2.TM] black hole entry num */
    CTC_GLOBAL_CAPABILITY_HOST_ROUTE_ENTRY_NUM,       /**< [GG.D2.TM] host route entry num */
    CTC_GLOBAL_CAPABILITY_LPM_ROUTE_ENTRY_NUM,         /**< [GG.D2.TM] lpm route entry num */
    CTC_GLOBAL_CAPABILITY_IPMC_ENTRY_NUM,                  /**< [GG.D2.TM] ipmc entry num */
    CTC_GLOBAL_CAPABILITY_MPLS_ENTRY_NUM,                  /**< [GB.GG.D2.TM] mpls entry num*/
    CTC_GLOBAL_CAPABILITY_TUNNEL_ENTRY_NUM,              /**< [GG.D2.TM] tunnel entry num */
    CTC_GLOBAL_CAPABILITY_L2PDU_L2HDR_PROTO_ENTRY_NUM,   /**< [GB.GG.D2.TM] l2 pdu based l2 header protocol entry num */
    CTC_GLOBAL_CAPABILITY_L2PDU_MACDA_ENTRY_NUM,             /**< [GB.GG.D2.TM] l2 pdu based macda entry num */
    CTC_GLOBAL_CAPABILITY_L2PDU_MACDA_LOW24_ENTRY_NUM, /**< [GB.GG.D2.TM] l2 pdu based macda low 24bit entry num */
    CTC_GLOBAL_CAPABILITY_L2PDU_L2CP_MAX_ACTION_INDEX,    /**< [GB.GG.D2.TM] l2 pdu l2cp max action index */
    CTC_GLOBAL_CAPABILITY_L3PDU_L3HDR_PROTO_ENTRY_NUM,   /**< [GB.GG.D2.TM] l3 pdu based l3 header protocol entry num */
    CTC_GLOBAL_CAPABILITY_L3PDU_L4PORT_ENTRY_NUM,             /**< [GB.GG.D2.TM] l3 pdu based l4 port entry num */
    CTC_GLOBAL_CAPABILITY_L3PDU_IPDA_ENTRY_NUM,                 /**< [GB.GG.D2.TM] l3 pdu based ipda entry num */
    CTC_GLOBAL_CAPABILITY_L3PDU_MAX_ACTION_INDEX,              /**< [GB.GG.D2.TM] l3 pdu max action index */
    CTC_GLOBAL_CAPABILITY_SCL_HASH_ENTRY_NUM,           /**< [GB.GG.D2.TM] scl hash 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_SCL1_HASH_ENTRY_NUM,           /**< [TM] scl hash 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_SCL_TCAM_ENTRY_NUM,           /**< [GB.GG.D2.TM] scl tcam 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_SCL1_TCAM_ENTRY_NUM,      /**< [GB.GG.D2.TM] scl tcam 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_SCL2_TCAM_ENTRY_NUM,      /**< [TM] scl tcam 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_SCL3_TCAM_ENTRY_NUM,      /**< [TM] scl tcam 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL_HASH_ENTRY_NUM,           /**< [GB.GG.D2.TM] acl hash 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL0_IGS_TCAM_ENTRY_NUM,  /**< [GB.GG.D2.TM] acl0 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL1_IGS_TCAM_ENTRY_NUM,  /**< [GB.GG.D2.TM] acl1 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL2_IGS_TCAM_ENTRY_NUM,  /**< [GB.GG.D2.TM] acl2 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL3_IGS_TCAM_ENTRY_NUM,  /**< [GB.GG.D2.TM] acl3 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL4_IGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl4 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL5_IGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl5 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL6_IGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl6 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL7_IGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl7 ingress 160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL0_EGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl0 egress  160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL1_EGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl1 egress  160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_ACL2_EGS_TCAM_ENTRY_NUM,  /**< [D2.TM] acl2 egress  160bit key size entry num */
    CTC_GLOBAL_CAPABILITY_CID_PAIR_NUM,             /**< [D2.TM] acl tcam cid entry num */
    CTC_GLOBAL_CAPABILITY_UDF_ENTRY_NUM,            /**< [D2.TM] acl udf entry num */
    CTC_GLOBAL_CAPABILITY_IPFIX_ENTRY_NUM,          /**< [GG.D2.TM] ipfix entry num */
    CTC_GLOBAL_CAPABILITY_EFD_FLOW_ENTRY_NUM,       /**< [GG.D2.TM] efd flow entry num */

    CTC_GLOBAL_CAPABILITY_MAX_LCHIP_NUM,            /**< [GB.GG.D2.TM] max local chip num */
    CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM,         /**< [GB.GG.D2.TM] max phy port num */
    CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM,             /**< [GB.GG.D2.TM] max port num per chip */
    CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM,             /**< [GB.GG.D2.TM] max local chip num */

    CTC_GLOBAL_CAPABILITY_MAX
};
typedef enum ctc_global_capability_type_e ctc_global_capability_type_t;


/**
 @brief  Define ECMP dynamic type
*/
enum ctc_global_ecmp_dlb_mode_e
{
    CTC_GLOBAL_ECMP_DLB_MODE_ALL      = 1,         /**< [GG.D2.TM] All flow do ECMP dynamic load balance. This is default mode */
    CTC_GLOBAL_ECMP_DLB_MODE_ELEPHANT = 2,    /**< [GG.D2.TM] Only elephant(big) flow do ECMP dynamic load balance */
    CTC_GLOBAL_ECMP_DLB_MODE_TCP      = 3,        /**< [GG.D2.TM] Only TCP flow do ECMP dynamic load balance */

    CTC_GLOBAL_ECMP_DLB_MODE_MAX
};
typedef enum ctc_global_ecmp_dlb_mode_e ctc_global_ecmp_dlb_mode_t;

/**
 @brief  Define ECMP dynamic rebalance type
*/
enum ctc_global_ecmp_rebalance_mode_e
{
    CTC_GLOBAL_ECMP_REBALANCE_MODE_NORMAL   = 1,    /**< [GG.D2.TM] ECMP dynamic load balancing normal mode:if inactivity duration
                                                                                                       lapsed, use optimal member, else use assigned member.  This is default mode*/
    CTC_GLOBAL_ECMP_REBALANCE_MODE_FIRST    = 2,       /**< [GG.D2.TM] ECMP dynamic load balancing first mode: only new flow
                                                                                                       use optimal member for the first time, than use assigned member */
    CTC_GLOBAL_ECMP_REBALANCE_MODE_PACKET   = 3,      /**< [GG.D2.TM] ECMP dynamic load balancing packet mode: use optimal member every some(default is 256) packets */

    CTC_GLOBAL_ECMP_REBALANCE_MODE_MAX
};
typedef enum ctc_global_ecmp_rebalance_mode_e ctc_global_ecmp_rebalance_mode_t;

/**
 @brief  Define vlan range mode
*/
enum ctc_global_vlan_range_mode_e
{
    CTC_GLOBAL_VLAN_RANGE_MODE_SCL = 0,       /**< [GG.D2.TM] Vlan range only for scl. This is default mode */
    CTC_GLOBAL_VLAN_RANGE_MODE_ACL = 1,       /**< [GG.D2.TM] Vlan range only for acl */
    CTC_GLOBAL_VLAN_RANGE_MODE_SHARE = 2,   /**< [GG.D2.TM] Vlan range for scl and acl */

    CTC_GLOBAL_VLAN_RANGE_MODE_MAX
};
typedef enum ctc_global_vlan_range_mode_e ctc_global_vlan_range_mode_t;


enum ctc_global_igmp_snooping_mode_e
{
    CTC_GLOBAL_IGMP_SNOOPING_MODE_0 = 0,  /**< [D2.TM] redirect igmp_snooping to cpu, default mode.
                                                    D2 can't do copp,TM can do copp. */
    CTC_GLOBAL_IGMP_SNOOPING_MODE_1 = 1,  /**< [D2.TM] redirect igmp_snooping to cpu and user must config one COPP ACL entry based CTC_PKT_CPU_REASON_IGMP_SNOOPING*/
    CTC_GLOBAL_IGMP_SNOOPING_MODE_2 = 2,  /**< [GG.D2.TM]copy igmp_snooping to cpu and do vlan flooding ,
                                                    can do copp at the same time ,[D2]:L3if must enable L2 bridge */
    CTC_GLOBAL_IGMP_SNOOPING_MODE_MAX
};
typedef enum ctc_global_igmp_snooping_mode_e ctc_global_igmp_snooping_mode_t;

struct ctc_global_flow_property_s
{
    uint8 igs_vlan_range_mode;            /**< [GG.D2.TM] Ingress vlan range mode, refer to ctc_global_vlan_range_mode_t */
    uint8 egs_vlan_range_mode;           /**< [GG.D2.TM] Egress vlan range mode, refer to ctc_global_vlan_range_mode_t */
};
typedef struct ctc_global_flow_property_s ctc_global_flow_property_t;

enum ctc_global_ipv6_addr_compress_mode_e
{
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_0 ,       /**< [D2.TM] 127~64 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_1 ,       /**< [D2.TM] 123~60 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_2 ,       /**< [D2.TM] 119~56 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_3 ,       /**< [D2.TM] 115~52 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_4 ,       /**< [D2.TM] 111~48 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_5 ,       /**< [D2.TM] 107~44 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_6 ,       /**< [D2.TM] 103~40 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_7 ,       /**< [D2.TM] 99~36 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_8 ,       /**< [D2.TM] 95~32 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_9 ,       /**< [D2.TM] 91~28 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_10 ,       /**< [D2.TM] 87~24 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_11 ,       /**< [D2.TM] 83~20 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_12 ,       /**< [D2.TM] 79~16 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_13 ,       /**< [D2.TM] 75~12 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_14 ,       /**< [D2.TM] 71~8 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_15 ,       /**< [D2.TM] 67~4 */
    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_16 ,       /**< [D2.TM] 63~0 */

    CTC_GLOBAL_IPV6_ADDR_COMPRESS_MODE_MAX
};
typedef enum ctc_global_ipv6_addr_compress_mode_e ctc_global_ipv6_addr_compress_mode_t;


struct ctc_global_acl_property_s
{
    uint8  dir;                                 /**<[GG.D2.TM]Direction,refer to ctc_direction_t*/
    /*Per ACL Lookup level (acl_priority) control*/
    uint8  lkup_level;                          /**<[GG.D2.TM]ACL Lookup level*/
    uint8  discard_pkt_lkup_en;                 /**<[GG.D2.TM] If set,indicate the discarded packets will do ACL lookup; For GG is not per level ctl,it is global ctl. */
    uint8  key_ipv6_da_addr_mode;               /**<[D2.TM] for CTC_ACL_KEY_FWD/CTC_ACL_KEY_IPV6*/
    uint8  key_ipv6_sa_addr_mode;               /**<[D2.TM] for CTC_ACL_KEY_FWD/CTC_ACL_KEY_IPV6*/
    uint8  key_cid_en;                          /**<[D2.TM] All Acl Key*/
    uint8  random_log_pri;                      /**<[D2.TM] Random log mapped new acl priority*/

    /*global config*/
    uint8  copp_key_use_ext_mode[MAX_CTC_PARSER_L3_TYPE]; /**<[D2.TM] default:CTC_PARSER_L3_TYPE_IPV6 use copp ext key;other use copp key*/
    uint8  l2l3_ext_key_use_l2l3_key[MAX_CTC_PARSER_L3_TYPE]; /**<[D2.TM] default:CTC_PARSER_L3_TYPE_IP/CTC_PARSER_L3_TYPE_IPV4/CTC_PARSER_L3_TYPE_IPV6 use l2l3 ext key;other use l2l3 key*/
    uint8  l3_key_ipv6_use_compress_addr;       /**<[D2.TM] if set,ipv6 address use compress mode (64 bit ipaddr),else ipv6 will use CTC_ACL_KEY_IPV6_EXT even if tcam lkup type is CTC_ACL_TCAM_LKUP_TYPE_L3*/
    uint8  cid_key_ipv6_da_addr_mode;           /**<[D2.TM] for CTC_ACL_KEY_CID*/
    uint8  cid_key_ipv6_sa_addr_mode;           /**<[D2.TM] for CTC_ACL_KEY_CID*/
    uint8  key_ctag_en;                         /**<[D2.TM] default to STag,for CTC_ACL_KEY_INTERFACE/CTC_ACL_KEY_FWD/CTC_ACL_KEY_COPP/CTC_ACL_KEY_COPP_EXT.*/
    uint8  stp_blocked_pkt_lkup_en;             /**<[D2.TM] If set, indicate the stp blocked packet will do ACL lookup.Only ingress valid */
    uint8  l2_type_as_vlan_num;                 /**<[GG.D2]> if set,vlan number can be used as key rather than layer2 type*/
    uint8  fwd_ext_key_l2_hdr_en;               /**<[D2] If set, forward ext key will include layer2 header fields instead of udf fields */
};
typedef struct ctc_global_acl_property_s ctc_global_acl_property_t;

struct ctc_global_cid_property_s
{
    uint32  global_cid;         /**<[D2.TM] global src CID*/
    uint8   global_cid_en;      /**<[D2.TM] enable global src CID */
    uint8   cid_pair_en;        /**<[D2.TM] cid pair lookup enable */
    uint8   cmd_parser_en;      /**<[D2.TM] cid metadata  parser enable*/
    uint8   cross_chip_cid_en;  /**<[D2.TM] if set, stacking header will carry cid info*/
    uint16  cmd_ethtype;        /**<[D2.TM]cid metadata  ethertype*/
    uint8   insert_cid_hdr_en;  /**<[D2.TM]insert CID header*/
    uint8   reassign_cid_pri_en;   /**<[D2.TM]re-assign CID priority*/
   /*direction*/
   uint8   is_dst_cid;          /**<[D2.TM] if set,indicate cfg or get dst cid priority*/
   uint8   default_cid_pri;     /**<[D2.TM] (src-cid|dst-cid) default CID Prioroty */
   uint8   fwd_table_cid_pri;   /**<[D2.TM] (src-cid|dst-cid) forward table(ip/mac) CID Prioroty */
   uint8   flow_table_cid_pri;  /**<[D2.TM] (src-cid|dst-cid) flow table(scl/sclflow) CID Prioroty */
   uint8   global_cid_pri;      /**<[D2.TM] (src-cid) global CID Prioroty */
   uint8   pkt_cid_pri;         /**<[D2.TM] (src-cid) packet's CID Prioroty */
   uint8   if_cid_pri;          /**<[D2.TM] (src-cid) interface CID Prioroty */
   uint8   iloop_cid_pri;         /**<[D2.TM] (src-cid) iloop header's CID Prioroty */
};
typedef struct ctc_global_cid_property_s ctc_global_cid_property_t;

struct ctc_global_ipmc_property_s
{
    uint8 ip_l2mc_mode;            /**< [D2.TM] If set,ip base l2MC lookup failed,will use mac based L2MC lookup result.  */
    uint8 vrf_mode;                /**< [GG] If set, vrfid in IPMC key is svlan id.  */
};
typedef struct ctc_global_ipmc_property_s ctc_global_ipmc_property_t;

struct ctc_global_panel_ports_s
{

    uint16  lport[CTC_MAX_PHY_PORT];       /**< [GB.GG.D2.TM] all the valid ports */
    uint16   count;                                        /**< [GB.GG.D2.TM] Port count num */
    uint8   lchip;                                          /**< [GB.GG.D2.TM] local chip id     */
    uint8   rsv;
};
typedef struct ctc_global_panel_ports_s ctc_global_panel_ports_t;

struct ctc_global_dump_db_s
{
    uint32 bit_map[CTC_DUMP_DB_BIT_MAP_NUM];     /**< [D2.TM] Bit map for dump module ctc_feature_t*/
    char   file[CTC_DUMP_DB_FILE_NAME];          /**< [D2.TM] file of route and file name £¬NULL use default*/
    uint8  detail;                               /**< [D2.TM] flag of dump detail data */
};
typedef struct ctc_global_dump_db_s ctc_global_dump_db_t;

enum ctc_lb_hash_field_e
{
    CTC_LB_HASH_FIELD_MACDA_LO = 0x00000001,     /**< [TM] MAC DA low 16 bits */
    CTC_LB_HASH_FIELD_MACDA_MI = 0x00000002,     /**< [TM] MAC DA middle 16 bits */
    CTC_LB_HASH_FIELD_MACDA_HI = 0x00000004,     /**< [TM] MAC DA high 16 bits */

    CTC_LB_HASH_FIELD_MACSA_LO = 0x00000008,     /**< [TM] MAC SA low 16 bits */
    CTC_LB_HASH_FIELD_MACSA_MI = 0x00000010,     /**< [TM] MAC SA middle 16 bits */
    CTC_LB_HASH_FIELD_MACSA_HI = 0x00000020,     /**< [TM] MAC SA high  16 bits */

    CTC_LB_HASH_FIELD_SVLAN     = 0x00000040,    /**< [TM] SVLAN ID */
    CTC_LB_HASH_FIELD_CVLAN     = 0x00000080,    /**< [TM] CVLAN ID */

    CTC_LB_HASH_FIELD_ETHER_TYPE = 0x00000100,   /**< [TM] Ether type */

    CTC_LB_HASH_FIELD_IP_DA_LO = 0x00000200,     /**< [TM] IP DA low 16 bits */
    CTC_LB_HASH_FIELD_IP_DA_HI = 0x00000400,     /**< [TM] IP DA high 16 bits */
    CTC_LB_HASH_FIELD_IP_SA_LO = 0x00000800,     /**< [TM] IP SA low 16 bits */
    CTC_LB_HASH_FIELD_IP_SA_HI = 0x00001000,     /**< [TM] IP sA high 16 bits */

    CTC_LB_HASH_FIELD_SRC_CHIP_ID = 0x00002000,  /**< [TM] Source chip id */
    CTC_LB_HASH_FIELD_DST_CHIP_ID = 0x00004000,  /**< [TM] Destination chip_id */
    CTC_LB_HASH_FIELD_SRC_PORT = 0x00008000,     /**< [TM] Source port */
    CTC_LB_HASH_FIELD_DST_PORT = 0x00010000,     /**< [TM] Destination port */

    CTC_LB_HASH_FIELD_FLOWLABEL_LO = 0x00020000, /**< [TM] IPv6 flow label low 16 bits */
    CTC_LB_HASH_FIELD_FLOWLABEL_HI = 0x00040000, /**< [TM] IPv6 flow label high 4 bits */

    CTC_LB_HASH_FIELD_PROTOCOL     = 0x00080000, /**< [TM] Layer3 protocol */

    CTC_LB_HASH_FIELD_UDF_LO = 0x00100000,		 /**< [TM] Udf low 16 bits */
    CTC_LB_HASH_FIELD_UDF_HI = 0x00200000,		 /**< [TM] Udf high 16 bits */

    CTC_LB_HASH_FIELD_VNI_LO = 0x00400000,		 /**< [TM] Vni(vxlan/nvgre) low 16 bits */
    CTC_LB_HASH_FIELD_VNI_HI = 0x00800000,		 /**< [TM] Vni(vxlan/nvgre) high 8 bits */

    CTC_LB_HASH_FIELD_L4_DSTPORT = 0x01000000,   /**< [TM] Destination layer4 port */
    CTC_LB_HASH_FIELD_L4_SRCPORT = 0x02000000,   /**< [TM] Source layer4 port */

    CTC_LB_HASH_FIELD_GRE_KEY_LO   = 0x01000000, /**< [TM] GRE key low 16 bits */
    CTC_LB_HASH_FIELD_GRE_KEY_HI   = 0x02000000, /**< [TM] GRE key high 16 bits */
    CTC_LB_HASH_FIELD_GRE_PROTOCOL = 0x04000000, /**< [TM] GRE protocol */

    CTC_LB_HASH_FIELD_LABEL0_LO = 0x01000000,    /**< [TM] MPLS label0 low 16 bits */
    CTC_LB_HASH_FIELD_LABEL0_HI = 0x02000000,	 /**< [TM] MPLS label0 high 16 bits */
    CTC_LB_HASH_FIELD_LABEL1_LO = 0x04000000,	 /**< [TM] MPLS label1 low 16 bits */
    CTC_LB_HASH_FIELD_LABEL1_HI = 0x08000000,	 /**< [TM] MPLS label1 high 16 bits */
    CTC_LB_HASH_FIELD_LABEL2_LO = 0x10000000,	 /**< [TM] MPLS label2 low 16 bits */
    CTC_LB_HASH_FIELD_LABEL2_HI = 0x20000000,	 /**< [TM] MPLS label2 high 16 bits */

    CTC_LB_HASH_FIELD_FCOE_SID_LO  = 0x01000000, /**< [TM] FCOE source fcid low 16 bits */
    CTC_LB_HASH_FIELD_FCOE_SID_HI  = 0x02000000, /**< [TM] FCOE source fcid high 16 bits */
    CTC_LB_HASH_FIELD_FCOE_DID_LO  = 0x04000000, /**< [TM] FCOE dest fcid low 16 bits */
    CTC_LB_HASH_FIELD_FCOE_DID_HI  = 0x08000000, /**< [TM] FCOE dest fcid high 16 bits */

    CTC_LB_HASH_FIELD_INGRESS_NICKNAME  = 0x01000000, /**< [TM] ingress nickname */
    CTC_LB_HASH_FIELD_EGRESS_NICKNAME   = 0x02000000  /**< [TM] egress nickname */
};
typedef enum ctc_lb_hash_field_e ctc_lb_hash_field_t;

enum ctc_lb_hash_select_e
{
    CTC_LB_HASH_SELECT_L2,		                  /**< [TM] Template T0, use layer2 as key */

    CTC_LB_HASH_SELECT_IPV4,		              /**< [TM] Template T4, ipv4 format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV4_TCP_UDP,		      /**< [TM] Template T4, ipv4(tcp/udp) format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV4_TCP_UDP_PORTS_EQUAL,  /**< [TM] Template T4, ipv4(tcp/udp ports equal) format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV4_VXLAN,		          /**< [TM] Template T4, ipv4 vxlan format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV4_GRE,		          /**< [TM] Template T5, ipv4 gre format, use layer3/4 as key */
    CTC_LB_HASH_SELECT_IPV4_NVGRE,		          /**< [TM] Template T5, ipv4 nvgre format, use layer3/4 as key */

    CTC_LB_HASH_SELECT_IPV6,		              /**< [TM] Template T4, ipv6 format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV6_TCP_UDP,		      /**< [TM] Template T4, ipv6(tcp/udp) format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV6_TCP_UDP_PORTS_EQUAL,  /**< [TM] Template T4, ipv6(tcp/udp ports equal) format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV6_VXLAN,		          /**< [TM] Template T4, ipv6 vxlan format, use layer3 as key */
    CTC_LB_HASH_SELECT_IPV6_GRE,	     	      /**< [TM] Template T5, ipv6 gre format, use layer3/4 as key */
    CTC_LB_HASH_SELECT_IPV6_NVGRE,		          /**< [TM] Template T5, ipv6 nvgre format, use layer3/4 as key */

    CTC_LB_HASH_SELECT_NVGRE_INNER_L2,		      /**< [TM] Template T0, nvgre format, decap use layer2 as key */
    CTC_LB_HASH_SELECT_NVGRE_INNER_IPV4,		  /**< [TM] Template T4, nvgre(inner ipv4) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_NVGRE_INNER_IPV6,		  /**< [TM] Template T4, nvgre(inner ipv6) format, decap use layer3 as key */

    CTC_LB_HASH_SELECT_VXLAN_INNER_L2,		      /**< [TM] Template T0, vxlan format, decap use layer2 as key */
    CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4,		  /**< [TM] Template T4, vxlan(inner ipv4) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_VXLAN_INNER_IPV6,		  /**< [TM] Template T4, vxlan(inner ipv6) format, decap use layer3 as key */

    CTC_LB_HASH_SELECT_MPLS,		              /**< [TM] Template T3, mpls format, use mpls as key */
    CTC_LB_HASH_SELECT_MPLS_INNER_IPV4,		      /**< [TM] Template T3, mpls(inner ipv4) format, use mpls as key */
    CTC_LB_HASH_SELECT_MPLS_INNER_IPV6,		      /**< [TM] Template T3, mpls(inner ipv6) format, use mpls as key */
    CTC_LB_HASH_SELECT_MPLS_VPWS_RAW,		      /**< [TM] Template T3, mpls(vpws) format, enable CTC_LB_HASH_CONTROL_VPWS_USE_INNER,
                                                          but not set CTC_GLOBAL_VPWS_SNOOPING_PARSER, use mpls as key */
    CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2,		  /**< [TM] Template T0, mpls(l2vpn) format, decap use layer2 as key */
    CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4,	  /**< [TM] Template T4, mpls(l2vpn inner ipv4) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6,	  /**< [TM] Template T4, mpls(l2vpn inner ipv6) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4,	  /**< [TM] Template T4, mpls(l3vpn inner ipv4) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6,	  /**< [TM] Template T4, mpls(l3vpn inner ipv6) format, decap use layer3 as key */

    CTC_LB_HASH_SELECT_TRILL_OUTER_L2_ONLY,       /**< [TM] Template T0, trill format, use layer2 as key */
    CTC_LB_HASH_SELECT_TRILL,        		      /**< [TM] Template T1, trill format, use trill as key */
    CTC_LB_HASH_SELECT_TRILL_INNER_L2,		      /**< [TM] Template T1, trill(inner l2) format, use trill as key */
    CTC_LB_HASH_SELECT_TRILL_INNER_IPV4,		  /**< [TM] Template T1, trill(inner ipv4) format, use trill as key */
    CTC_LB_HASH_SELECT_TRILL_INNER_IPV6,		  /**< [TM] Template T1, trill(inner ipv6) format, use trill as key */
    CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_L2,	  /**< [TM] Template T0, trill(inner l2) format, decap use layer2 as key */
    CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV4,	  /**< [TM] Template T4, trill(inner ipv4) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV6,	  /**< [TM] Template T4, trill(inner ipv6) format, decap use layer3 as key */

    CTC_LB_HASH_SELECT_FCOE,		              /**< [TM] Template T2, fcoe format, use fcoe as key */

    CTC_LB_HASH_SELECT_FLEX_TNL_INNER_L2,		  /**< [TM] Template T0, flexiable tunnel format, decap use layer2 as key, need CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2 */
    CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV4,		  /**< [TM] Template T4, flexiable tunnel(inner ipv4) format, decap use layer3 as key */
    CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV6,		  /**< [TM] Template T4, flexiable tunnel(inner ipv6) format, decap use layer3 as key */

    CTC_LB_HASH_SELECT_UDF,                       /**< [TM] Template T6, used for scl udf select bitmap, support select id <1-7> which use as profile id for scl module */
    CTC_LB_HASH_SELECT_MAX
};
typedef enum ctc_lb_hash_select_e ctc_lb_hash_select_t;


enum ctc_lb_hash_control_e
{
    CTC_LB_HASH_CONTROL_L2_ONLY,                    /**< [TM] only use CTC_HASH_SELECT_L2 as hash key select */
    CTC_LB_HASH_CONTROL_DISABLE_IPV4,		        /**< [TM] disable ipv4 */
    CTC_LB_HASH_CONTROL_DISABLE_IPV6,		        /**< [TM] disable ipv6 */
    CTC_LB_HASH_CONTROL_DISABLE_MPLS,		        /**< [TM] disable mpls */
    CTC_LB_HASH_CONTROL_DISABLE_FCOE,		        /**< [TM] disable fcoe */
    CTC_LB_HASH_CONTROL_DISABLE_TRILL,		        /**< [TM] disable trill. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN4,	/**< [TM] disable ipv4 tunnel which belongs to ipv4 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN4,	/**< [TM] disable ipv6 tunnel which belongs to ipv4 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN4,	    /**< [TM] disable gre tunnel which belongs to ipv4 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4,	/**< [TM] disable nvgre tunnel which belongs to ipv4 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4,	/**< [TM] disable vxlan tunnel which belongs to ipv4 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN6,	/**< [TM] disable ipv4 tunnel which belongs to ipv6 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN6,	/**< [TM] disable ipv6 tunnel which belongs to ip6 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN6,		/**< [TM] disable gre tunnel which belongs to ipv6 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6,	/**< [TM] disable nvgre tunnel which belongs to ipv6 message. */
    CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6,	/**< [TM] disable vxlan tunnel which belongs to ipv6 message. */
    CTC_LB_HASH_CONTROL_MPLS_EXCLUDE_RSV,			/**< [TM] no care reserve label (0-15) */
    CTC_LB_HASH_CONTROL_IPV6_ADDR_COLLAPSE,		    /**< [TM] use ipv6 address compress mode. */
    CTC_LB_HASH_CONTROL_IPV6_USE_FLOW_LABEL,		/**< [TM] use ipv6 flow label . */
    CTC_LB_HASH_CONTROL_NVGRE_AWARE,				/**< [TM] nvgre aware */
    CTC_LB_HASH_CONTROL_FCOE_SYMMETRIC_EN,			/**< [TM] enable fcoe symmetric hash. */
    CTC_LB_HASH_CONTROL_UDF_SELECT_MODE,			/**< [TM] select udf bits of 128bits,MODE 0:0-31, 1:32-63, 2: 64-95, 3:96-127*/
    CTC_LB_HASH_CONTROL_HASH_TYPE_UDF_DATA_BYTE_MASK, /**< [TM] mask udf-data by byte for generating lb-hash, support select id <1-7> which use as profile id for scl module*/
    CTC_LB_HASH_CONTROL_USE_MAPPED_VLAN,			/**< [TM] select mapped vlan (QinQ/vlanClass/ProtocolVlan/...) */
    CTC_LB_HASH_CONTROL_IPV4_SYMMETRIC_EN,			/**< [TM] enable ipv4 symmetric hash. */
    CTC_LB_HASH_CONTROL_IPV6_SYMMETRIC_EN,			/**< [TM] enable ipv6 symmetric hash. */
    CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2,         /**< [TM] enable l2vpn use l2 else ipv4 or ipv6. */
    CTC_LB_HASH_CONTROL_VPWS_USE_INNER,			    /**< [TM] force vpws use inner do hash. */
    CTC_LB_HASH_CONTROL_TRILL_DECAP_USE_INNER_L2,	/**< [TM] enable trill decap use l2 else ipv4 or ipv6. */
    CTC_LB_HASH_CONTROL_TRILL_TRANSMIT_MODE,		/**< [TM] select trill transmit key select mode.
													        mode 0: select CTC_LB_HASH_SELECT_TRILL_OUTER_L2_ONLY
													        mode 1: need port enable snoopParser   select CTC_LB_HASH_SELECT_TRILL_INNER_L2
													        mode 2: need port enable snoopParser, selectCTC_LB_HASH_SELECT_TRILL_INNER_IPV4/6,CTC_LB_HASH_SELECT_TRILL_INNER_L2
													        mode 3: select CTC_LB_HASH_SELECT_TRILL */
    CTC_LB_HASH_CONTROL_MPLS_ENTROPY_EN,			/**< [TM] use mpls entropy label to hash key*/
    CTC_LB_HASH_CONTROL_MPLS_LABEL2_EN,			    /**< [TM] use mpls label2 to hash key*/
    CTC_LB_HASH_CONTROL_VXLAN_USE_INNER_L2,         /**< [TM] vxlan tunnel use l2 else ipv4 or ipv6. */
    CTC_LB_HASH_CONTROL_NVGRE_USE_INNER_L2,         /**< [TM] nvgre tunnel use l2 else ipv4 or ipv6. */
    CTC_LB_HASH_CONTROL_FLEX_TNL_USE_INNER_L2,      /**< [TM] enable flexiable tunnel use l2 else ipv4 or ipv6. */
    CTC_LB_HASH_CONTROL_HASH_SEED,					/**< [TM] user defined hash key*/
    CTC_LB_HASH_CONTROL_HASH_TYPE_A,  				/**< [TM] hash poly A, refer to ctc_lb_hash_type_t*/
    CTC_LB_HASH_CONTROL_HASH_TYPE_B,				/**< [TM] hash poly B, refer to ctc_lb_hash_type_t*/
    CTC_LB_HASH_CONTROL_MAX
};
typedef enum ctc_lb_hash_control_e ctc_lb_hash_control_t;


enum ctc_lb_hash_type_e
{
    CTC_LB_HASH_TYPE_CRC8_POLY1,		/**< [TM] CRC8_POLY1   x8+x7+x2+x+1*/
    CTC_LB_HASH_TYPE_CRC8_POLY2,		/**< [TM] CRC8_POLY2   x8+x7+x6+x+1*/
    CTC_LB_HASH_TYPE_XOR8,				/**< [TM] XOR8*/
    CTC_LB_HASH_TYPE_CRC10_POLY1,		/**< [TM] CRC10_POLY1  x10+x3+1*/
    CTC_LB_HASH_TYPE_CRC10_POLY2,		/**< [TM] CRC10_POLY2  x10+x7+1*/
    CTC_LB_HASH_TYPE_XOR16,			    /**< [TM] XOR16*/
    CTC_LB_HASH_TYPE_CRC16_POLY1,		/**< [TM] CRC16_POLY1  x16+x5+x3+x2+1*/
    CTC_LB_HASH_TYPE_CRC16_POLY2,		/**< [TM] CRC16_POLY2  x16+x5+x4+x3+1*/
    CTC_LB_HASH_TYPE_CRC16_POLY3,		/**< [TM] CRC16_POLY3  x16+x10+x5+x3+1*/
    CTC_LB_HASH_TYPE_CRC16_POLY4,		/**< [TM] CRC16_POLY4  x16+x9+x4+x2+1*/
    CTC_LB_HASH_TYPE_POLY_MAX

};
typedef enum ctc_lb_hash_type_e ctc_lb_hash_type_t;

enum ctc_lb_hash_config_type_e
{
    CTC_LB_HASH_CFG_HASH_SELECT,		/**< [TM] select hash_select*/
    CTC_LB_HASH_CFG_HASH_CONTROL		/**< [TM] select hash_control*/

};
typedef enum ctc_lb_hash_config_type_e ctc_lb_hash_config_type_t;

struct ctc_lb_hash_config_s
{
    uint8 sel_id;  		/**< [TM] selector id <0-3> */
    uint8 cfg_type; 	/**< [TM] configure type, refer to ctc_lb_hash_config_type_t*/
    uint8 hash_select; 	/**< [TM] configure select, refer to  ctc_lb_hash_select_t*/
    uint8 hash_control; /**< [TM] configure control, refer to  ctc_lb_hash_control_t*/
    uint32 value; 		/**< [TM] value*/
};
typedef struct ctc_lb_hash_config_s ctc_lb_hash_config_t;


enum ctc_lb_hash_module_type_e
{
    CTC_LB_HASH_MODULE_LINKAGG,	    /**< [TM] linkagg module*/
    CTC_LB_HASH_MODULE_ECMP,		/**< [TM] ecmp module*/
    CTC_LB_HASH_MODULE_STACKING,	/**< [TM] stacking module*/
    CTC_LB_HASH_MODULE_HEAD_ECMP,   /**< [TM] for ecmp packet head which use as nexthop edit*/
    CTC_LB_HASH_MODULE_HEAD_LAG,    /**< [TM] for linkagg or stacking packet header hash*/
};
typedef enum ctc_lb_hash_module_type_e ctc_lb_hash_module_type_t;

enum ctc_lb_hash_mode_e
{
    CTC_LB_HASH_MODE_STATIC,		/**< [TM] static mode*/
    CTC_LB_HASH_MODE_DLB_FLOW_SET,	/**< [TM] set dlb flow */
    CTC_LB_HASH_MODE_DLB_MEMBER,	/**< [TM] only for CTC_LB_HASH_MODULE_ECMP */
    CTC_LB_HASH_MODE_RESILIENT,	    /**< [TM] resilient mode*/
};
typedef enum ctc_lb_hash_mode_e ctc_lb_hash_mode_t;


enum ctc_lb_hash_fwd_type_e
{
    CTC_LB_HASH_FWD_UC,		    /**< [TM] unicast message*/
    CTC_LB_HASH_FWD_NON_UC,   	/**< [TM] non unicast message*/
    CTC_LB_HASH_FWD_VXLAN,  	/**< [TM] only for CTC_LB_HASH_MODULE_ECMP */
    CTC_LB_HASH_FWD_NVGRE,  	/**< [TM] only for CTC_LB_HASH_MODULE_ECMP */
    CTC_LB_HASH_FWD_MPLS,   	/**< [TM] only for CTC_LB_HASH_MODULE_ECMP */
    CTC_LB_HASH_FWD_IP,     	/**< [TM] only for CTC_LB_HASH_MODULE_ECMP */
};
typedef enum ctc_lb_hash_fwd_type_e ctc_lb_hash_fwd_type_t;


struct ctc_lb_hash_offset_s
{
    uint8 profile_id;  			/**< [TM] set port acl or scl profile_id*/
    uint8 hash_module;      	/**< [TM] refer to ctc_lb_hash_module_type_t*/
    uint8 hash_mode;        	/**< [TM] refer to ctc_lb_hash_mode_t*/
    uint8 fwd_type;         	/**< [TM] refer to ctc_lb_hash_fwd_type_t*/
    uint16 offset;              /**< [TM] <0-127>, if set 16, means select 16-31 bits, use CTC_LB_HASH_OFFSET_DISABLE to disable */
    uint8 use_entropy_hash;     /**< [TM] only for CTC_LB_HASH_MODULE_ECMP */
    uint8 use_packet_head_hash; /**< [TM] IF enable, use packet head hash, high priority than offset */
};
typedef struct ctc_lb_hash_offset_s ctc_lb_hash_offset_t;

enum ctc_global_macda_derive_mode_e
{
    CTC_GLOBAL_MACDA_DERIVE_FROM_NH,          /**< [TM] macda all derive from nexthop */
    CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE0,   /**< [TM] macda low 2bits derive from route table, others derive from nexthop */
    CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE1,   /**< [TM] macda low 3bits derive from route table, others derive from nexthop */
    CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE2,   /**< [TM] macda low 4bits derive from route table, others derive from nexthop */

    CTC_GLOBAL_MAX_DERIVE_MODE
};
typedef enum ctc_global_macda_derive_mode_e ctc_global_macda_derive_mode_t;

struct ctc_global_overlay_decap_mode_e
{
    uint8 vxlan_mode;                     /**<[GG.D2.TM] mode 0 decap by ipda + ipsa + vni, mode 1 decap by ipda + vni*/
    uint8 nvgre_mode;                     /**<[GG.D2.TM] mode 0 decap by ipda + ipsa + vni, mode 1 decap by ipda + vni*/
    uint8 scl_id;                         /**<[GG.D2.TM] scl id*/
};
typedef struct ctc_global_overlay_decap_mode_e ctc_global_overlay_decap_mode_t;
/**
 @brief  global control type
*/
enum ctc_global_control_type_e
{
    CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT = 0,   /**< [GB.GG.D2.TM]value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT,            /**< [GB.GG.D2.TM]value: bool*, TRUE or FALSE */

    CTC_GLOBAL_DISCARD_TTL_0_PKT,                         /**< [GB.GG.D2.TM]value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_MCAST_SA_PKT,                  /**< [GB.GG.D2.TM] MAC SA is mcast address, value: bool*, TRUE or FALSE */

    CTC_GLOBAL_DISCARD_TCP_SYN_0_PKT,                 /**< [GB.GG.D2.TM] TCP packets with SYN equals 0, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_TCP_NULL_PKT,                   /**< [GB.GG.D2.TM] TCP packets with flags and sequence number equal 0, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_TCP_XMAS_PKT,                  /**< [GB.GG.D2.TM] TCP packets with FIN, URG, PSH bits set and sequence number equals 0, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_TCP_SYN_FIN_PKT,              /**< [GB.GG.D2.TM] TCP packets with SYN & FIN bits set, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_SAME_L4_PORT_PKT,           /**< [GB.GG.D2.TM] Same L4 source and destination port packets, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_DISCARD_ICMP_FRAG_PKT,             /**< [GB.GG.D2.TM] Fragmented ICMP packets, value: bool*, TRUE or FALSE */

    CTC_GLOBAL_ARP_MACDA_CHECK_EN,                 /**< [GB.GG.D2.TM] Drop ARP reply packets which macda not equal to target mac, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_ARP_MACSA_CHECK_EN,                 /**< [GB.GG.D2.TM] Drop ARP packets which macsa not equal to sender mac, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_ARP_IP_CHECK_EN,                   /**< [GB.GG.D2.TM] Drop ARP packets with invalid sender ip or ARP reply packets with invalid target ip(0x0/0xFFFFFFFF/Mcast)value: bool*, TRUE or FALSE */
    CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU,             /**< [GB.GG.D2.TM] ARP packets check fail and copy to cpu, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_IGMP_SNOOPING_MODE,                /**< [D2.TM]  enhanced IGMP Snooping,  value : refer to ctc_global_igmp_snooping_mode_t  */

    CTC_GLOBAL_ACL_CHANGE_COS_ONLY,                  /**< [GG]acl vlan edit actioin only change cos, value: bool*, TRUE or FALSE */

    CTC_GLOBAL_ECMP_DLB_MODE,                             /**< [GG.D2.TM] ecmp dynamic load balancing mode, value uint32* refer to ctc_global_ecmp_dlb_mode_t */
    CTC_GLOBAL_ECMP_REBALANCE_MODE,                  /**< [GG.D2.TM] ecmp dlb rebalance mode, value uint32* refer to ctc_global_ecmp_rebalance_mode_t */
    CTC_GLOBAL_ECMP_FLOW_AGING_INTERVAL,          /**< [GG.D2.TM] ecmp dlb flow aging time, uint:ms, value uint32*, 0:disable flow aging */
    CTC_GLOBAL_ECMP_FLOW_INACTIVE_INTERVAL,      /**< [GG.D2.TM] used for CTC_GLOBAL_ECMP_REBALANCE_MODE_NORMAL dynamic mode, ecmp dlb flow inactive time, uint:us, value uint32* */
    CTC_GLOBAL_ECMP_FLOW_PKT_INTERVAL,               /**< [GG.D2.TM] used for CTC_GLOBAL_ECMP_REBALANCE_MODE_PACKET dynamic mode, select optimal member every 256/1024/8192/32768 packets, value uint32* */

    CTC_GLOBAL_LINKAGG_FLOW_INACTIVE_INTERVAL,        /**< [GB.GG.D2.TM] used for Linkagg DLB mode, flow inactive time, uint:ms, value uint32* */
    CTC_GLOBAL_STACKING_TRUNK_FLOW_INACTIVE_INTERVAL, /**< [GB.GG.D2.TM] used for Stacking trunk DLB mode, flow inactive time, uint:ms, value uint32* */

    CTC_GLOBAL_WARMBOOT_STATUS,                          /**< [GG.D2.TM] warmboot status */
    CTC_GLOBAL_WARMBOOT_CPU_RX_EN,                   /**< [GG.D2.TM] warmboot cpu rx enable, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_FLOW_PROPERTY,                               /**< [GG.D2.TM] Flow property, value:ctc_global_flow_property_t* */
    CTC_GLOBAL_CHIP_CAPABILITY,                              /**< [GB.GG.D2.TM] Chip capability, value: uint32 chip_capability[CTC_GLOBAL_CAPABILITY_MAX], capability invalid value 0xFFFFFFFF means not support */
    CTC_GLOBAL_ACL_PROPERTY,                            /**< [GG.D2.TM] Cfg global acl property, refer to ctc_global_acl_property_t */
    CTC_GLOBAL_CID_PROPERTY,                            /**< [D2.TM] Cfg global cid property, refer to ctc_global_cid_property_t */
    CTC_GLOBAL_ACL_LKUP_PROPERTY,                       /**< [D2.TM] Cfg global acl lookup property, refer to ctc_acl_property_t */
    CTC_GLOBAL_ELOOP_USE_LOGIC_DESTPORT,                /**< [GG.D2.TM] Eloop use logic dest port instand of logic src port, value: bool*, TRUE or FALSE */
    CTC_GLOBAL_PIM_SNOOPING_MODE,                       /**< [GB.GG.D2.TM] PIM snooping mode, mode 0: (GB/GG)PIM snooping recognized as IGMP snooping and per vlan/l3if enable; mode 1:PIM snooping recognized by acl, value uint32* */
    CTC_GLOBAL_IPMC_PROPERTY,                           /**< [GG.D2.TM] Cfg global ipmc property, refer to ctc_global_ipmc_property_t */
    CTC_GLOBAL_VXLAN_UDP_DEST_PORT,                     /**< [GG.D2.TM] vxlan protocol udp dest port, value: uint32, default is 4789 */
    CTC_GLOBAL_GENEVE_UDP_DEST_PORT,                    /**< [GG.D2.TM] geneve protocol udp dest port, value: uint32, default is 6081 */
    CTC_GLOBAL_PANEL_PORTS,                                /**< [GB.GG] get all panel ports in the chip */
    CTC_GLOBAL_NH_FORCE_BRIDGE_DISABLE,                 /**< [GB.GG.D2.TM] Disable force bridge function for IPMC when src l3if match dest l3if */
    CTC_GLOBAL_NH_MCAST_LOGIC_REP_EN,                   /**< [GB.GG.D2.TM] Enable nexthop mcast logic replication*/
    CTC_GLOBAL_LB_HASH_KEY,                             /**< [TM] load banlance hash key select and control value: void* refer to ctc_lb_hash_config_t*/
    CTC_GLOBAL_LB_HASH_OFFSET_PROFILE,                  /**< [TM] load banlance hash offset profile value: void* refer to ctc_lb_hash_offset_s*/
    CTC_GLOBAL_VPWS_SNOOPING_PARSER,                    /**< [TM] Enable parser VPWS payload */
    CTC_GLOBAL_VXLAN_CRYPT_UDP_DEST_PORT,               /**< [TM] vxlan UDP dest port in cloudSec scenario, value: uint32, default is 0 */
    CTC_GLOBAL_OAM_POLICER_EN,                          /**< [D2.TM] Enable oam flow policer*/
    CTC_GLOBAL_PARSER_OUTER_ALWAYS_CVLAN_EN,            /**< [D2.TM] Enable outer vlan is always cvlan when port is cvlan-domain*/
    CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE,                /**< [TM] Config host route nexthop macda derive mode, refer to ctc_global_macda_derive_mode_t*/
    CTC_GLOBAL_ARP_VLAN_CLASS_EN,                       /**< [GG.D2.TM] Enable ARP PDUS vlan classification */

    CTC_GLOBAL_DISCARD_MACSA_0_PKT,                     /**< [D2.TM] Discard the packet that macSa is all zero, value: bool*, TRUE or FALSE*/
    CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST,                   /**< [D2] If set, vpws oam and tp oam can coexit, and vpws oamid only support 0~2K; Else, vpws oam and tp oam can not coexist*/
    CTC_GLOBAL_VXLAN_POLICER_GROUP_ID_BASE,             /**< [TM] vxlan policer group id base, if set, group id value should be base~base+255 */
    CTC_GLOBAL_INBAND_CPU_TRAFFIC_TIMER,                /**< [D2.TM] Set inband CPU traffic timer, 0 means disable, valid range 10(ms) ~100(ms). Only used for WDM Pon*/
    CTC_GLOBAL_DUMP_DB,                                 /**< [D2.TM] Dump module master db to file*/
    CTC_GLOBAL_IGS_RANDOM_LOG_SHIFT,                    /**< [GG.D2.TM] ingress port random log shift,default:5,max rate = 0x7fff,percent = (rate << shift)/0xfffff, value: uint32* */
    CTC_GLOBAL_EGS_RANDOM_LOG_SHIFT,                    /**< [GG.D2.TM] egress port random log shift,default:5,max rate = 0x7fff,percent = (rate << shift)/0xfffff,value: uint32* */
    CTC_GLOBAL_OVERLAY_DECAP_MODE,                      /**< [GG.D2.TM] overlay decap mode, refer to ctc_global_overlay_decap_mode_t*/
    CTC_GLOBAL_EGS_STK_ACL_DIS,                         /**< [GG.D2.TM] Disable acl lookup in statkcing port's egress direction  > */
    CTC_GLOBAL_STK_WITH_IGS_PKT_HDR_EN,                 /**< [D2.TM] Enable or disable to remote cpu in stacking, within ingress chip packet header.  value: bool, default is FALSE*/
    CTC_GLOBAL_NET_RX_EN,                               /**< [TM] Net rx enable/disable>*/

    CTC_GLOBAL_CONTROL_MAX
};
typedef enum ctc_global_control_type_e ctc_global_control_type_t;


#ifdef __cplusplus
}
#endif

#endif

