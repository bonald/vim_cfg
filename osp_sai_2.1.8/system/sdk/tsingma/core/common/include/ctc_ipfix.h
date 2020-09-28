/**
 @file ctc_ipfix.h

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-13

 @version v3.0

 This file contains all port related data structure, enum, macro and proto.

*/

#ifndef _CTC_IPFIX_H
#define _CTC_IPFIX_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "common/include/ctc_const.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup ipfix IPFIX
 @{
*/

#define CTC_IPFIX_LABEL_NUM          3

/**
 @brief  IPFIX flag define
*/
enum ctc_ipfix_data_flags_e
{
    CTC_IPFIX_DATA_L2_MCAST_DETECTED      = (1 << 0),             /**< [GG.D2.TM] Session destid is L2Mcast*/
    CTC_IPFIX_DATA_L3_MCAST_DETECTED      = (1 << 1),             /**< [GG.D2.TM] Session destid is L3Mcast*/
    CTC_IPFIX_DATA_BCAST_DETECTED         = (1 << 2),             /**< [GG.D2.TM] Session destid is BMcast*/
    CTC_IPFIX_DATA_APS_DETECTED           = (1 << 3),             /**< [GG.D2.TM] Session destid is APS Group*/
    CTC_IPFIX_DATA_ECMP_DETECTED          = (1 << 4),             /**< [GG.D2.TM] Session destid is ECMP Group*/
    CTC_IPFIX_DATA_LINKAGG_DETECTED       = (1 << 5),             /**< [GG.D2.TM] Session destid is Linkagg Group*/
    CTC_IPFIX_DATA_UNKNOW_PKT_DETECTED    = (1 << 6),             /**< [GG.D2.TM] Unkown Packet detect*/
    CTC_IPFIX_DATA_FRAGMENT_PKT_DETECTED  = (1 << 7),             /**< [GG.D2.TM] Fragment Packet detect*/
    CTC_IPFIX_DATA_DROP_DETECTED          = (1 << 8),             /**< [GG.D2.TM] Drop Packrt detect*/
    CTC_IPFIX_DATA_TTL_CHANGE             = (1 << 9),             /**< [GG.D2.TM] TTL change in this session*/
    CTC_IPFIX_DATA_CVLAN_TAGGED           = (1 << 10),            /**< [GG.D2.TM] Ipfix key cvlan valid*/
    CTC_IPFIX_DATA_SVLAN_TAGGED           = (1 << 11),            /**< [GG.D2.TM] Ipfix key svlan valid*/
    CTC_IPFIX_DATA_SRC_CID_VALID          = (1 << 12),            /**< [D2.TM]    Ipfix key src cid valid*/
    CTC_IPFIX_DATA_DST_CID_VALID          = (1 << 13),            /**< [D2.TM]    Ipfix key dst cid valid*/
    CTC_IPFIX_DATA_MFP_VALID              = (1 << 14)             /**< [TM]       Session enable micro flow policer */
};
typedef enum ctc_ipfix_data_flags_e ctc_ipfix_data_flags_t;

/**
 @brief  IPFIX Export Reason define
*/
enum ctc_ipfix_export_reason_e
{
    CTC_IPFIX_NO_EXPORT,
    CTC_IPFIX_REASON_EXPIRED,                                  /**< [GG.D2.TM] Export reason is time expired */
    CTC_IPFIX_REASON_TCP_CLOSE,                                /**< [GG.D2.TM] Export reason is Tcp session close */
    CTC_IPFIX_REASON_PACKET_CNT_OVERFLOW,                      /**< [GG.D2.TM] Export reason is packet count overflow */
    CTC_IPFIX_REASON_BYTE_CNT_OVERFLOW,                        /**< [GG.D2.TM] Export reason is packet byte overflow */
    CTC_IPFIX_REASON_LAST_TS_OVERFLOW,                         /**< [GG.D2.TM] Export reason is ts overflow */
    CTC_IPFIX_REASON_HASH_CONFLICT,                            /**< [GG] Export reason is hash conflict */
    CTC_IPFIX_REASON_NEW_FLOW_INSERT,                          /**< [GG.D2.TM] Export reason is new flow insert */
    CTC_IPFIX_REASON_DEST_CHANGE                               /**< [GG.D2.TM] Export reason is dest info change */
};
typedef enum ctc_ipfix_export_reason_e ctc_ipfix_export_reason_t;

/**
 @brief  IPFIX aim to packet type
*/
enum ctc_ipfix_flow_type_e
{
    CTC_IPFIX_FLOW_TYPE_ALL_PACKET,                      /**< [GG.D2.TM] Do ipfix for all packets including Discard and Non-discard */
    CTC_IPFIX_FLOW_TYPE_NON_DISCARD_PACKET,              /**< [GG.D2.TM] Only do ipfix for Non-discard packets */
    CTC_IPFIX_FLOW_TYPE_DISCARD_PACKET,                  /**< [GG.D2.TM] Only do ipfix for Discard packets */
    CTC_IPFIX_FLOW_TYPE_MAX
};
typedef enum ctc_ipfix_flow_type_e ctc_ipfix_flow_type_t;

/**
 @brief  IPFIX mpls label info
*/
struct ctc_ipfix_label_info_s
{
    uint8  ttl;        /**< [GG.D2.TM] TTL value*/
    uint8  exp;        /**< [GG.D2.TM] EXP value*/
    uint8  sbit;       /**< [GG.D2.TM] Sbit value*/
    uint32 label;      /**< [GG.D2.TM] MPLS label value*/
};
typedef struct ctc_ipfix_label_info_s ctc_ipfix_label_info_t;

/**
 @brief  IPFIX l3 info struct
*/
struct ctc_ipfix_l3_info_s
{
    struct
    {
        uint8 ttl;                             /**< [GG.D2.TM] TTL value*/
        uint8 dscp;                            /**< [GG.D2.TM] DSCP value*/
        uint8 ecn;                             /**< [GG.D2.TM] Ip Ecn value*/
        uint8 ipda_masklen;                    /**< [GG.D2.TM] Only used for Cpu construct key, length should be 1~32*/
        uint8 ipsa_masklen;                    /**< [GG.D2.TM] Only used for Cpu construct key, length should be 1~32*/
        uint16  ip_pkt_len;                    /**< [D2.TM] Ip packet length,only for ipv4 key*/
        uint16  ip_identification;             /**< [D2.TM] Ip packet identification.*/
        ip_addr_t ipda;                        /**< [GG.D2.TM] IPV4 ip da*/
        ip_addr_t ipsa;                        /**< [GG.D2.TM] IPV4 ip sa*/
    }ipv4;
    struct
    {
        uint32 flow_label;                     /**< [GG.D2.TM] Flow Label*/
        uint8 dscp;                            /**< [GG.D2.TM] DSCP value*/
        uint8 ttl;                             /**< [D2.TM] TTL value*/
        uint8 ecn;                             /**< [D2.TM] Ip Ecn value*/
        uint8 ipda_masklen;                    /**< [GG.D2.TM] Only used for Cpu construct key, length should be 4*n*/
        uint8 ipsa_masklen;                    /**< [GG.D2.TM] Only used for Cpu construct key, length should be 4*n*/
        ipv6_addr_t ipda;                      /**< [GG.D2.TM] IPV6 ip da*/
        ipv6_addr_t ipsa;                      /**< [GG.D2.TM] IPV6 ip sa*/
    }ipv6;

    struct
    {
        uint8 label_num;                                         /**< [GG.D2.TM] MPLS lable number*/
        ctc_ipfix_label_info_t label[CTC_IPFIX_LABEL_NUM];       /**< [GG.D2.TM] MPLS lable*/
    }mpls;

    uint32 vrfid;                            /**< [GG.D2.TM] Vrfid value*/
    uint8  ip_frag;                          /**< [D2.TM] Ip fragment, refer to ctc_ip_frag_t*/

};
typedef struct ctc_ipfix_l3_info_s ctc_ipfix_l3_info_t;

/**
 @brief  IPFIX l4 info struct
*/
struct ctc_ipfix_l4_info_s
{
    union
    {
        uint16 l4_type;                   /**< [GG] refer to ctc_ipfix_l4_type_t, used for l2l3 and ipv6 key*/
        uint16 ip_protocol;               /**< [GG.D2.TM] used for ipv4 key */
    }type;

    struct
    {
        uint16 source_port;               /**< [GG.D2.TM] Source port value*/
        uint16 dest_port;                 /**< [GG.D2.TM] Destination port value*/
    }l4_port;

    struct
    {
        uint8 icmpcode;                   /**< [GG.D2.TM] Icmpcode value*/
        uint8 icmp_type;                  /**< [GG.D2.TM] Icmp type*/
    }icmp;

    struct
    {
        uint8 igmp_type;                  /**< [D2.TM] Igmp type value*/
    }igmp;
    uint32 l4_sub_type;                   /**< [GG] refer to ctc_ipfix_l4_sub_type_t, used for l2l3 ipv4 ipv6 key. When used for ipv6, only support vxlan_vin*/
    uint32 gre_key;                       /**< [GG.D2.TM] GRE key or NVGRE vsid*/
    uint32 vni;                           /**< [GG.D2.TM] Vni key*/
    uint8  aware_tunnel_info_en;          /**< [GG.D2.TM] if enable, indicate lookup key use innner info as key, and outer tunnel info can also be used as lookup key*/


    struct
    {
        uint8      radio_id;              /**< [D2.TM] Radio id value*/
        uint8      is_ctl_pkt;            /**< [D2.TM] Is wlan control packet*/
        mac_addr_t radio_mac;             /**< [D2.TM] Radio mac*/
    }wlan;

    uint8 tcp_flags;                      /**< [D2.TM]tcp flags, consist of CWR|ECE|URG|ACK|PSH|RST|SYN|FIN*/


};
typedef struct ctc_ipfix_l4_info_s ctc_ipfix_l4_info_t;

/**
 @brief  IPFIX Port Type
*/
enum ctc_ipfix_port_type_s
{
    CTC_IPFIX_PORT_TYPE_NONE,           /**< [GG.D2.TM] Donot using Gport/LogicPort/metedara as key */
    CTC_IPFIX_PORT_TYPE_GPORT,          /**< [GG.D2.TM] Using GlobalPort as key */
    CTC_IPFIX_PORT_TYPE_LOGIC_PORT,     /**< [GG.D2.TM] Using LogicPort as key */
    CTC_IPFIX_PORT_TYPE_METADATA,        /**< [GG.D2.TM] Using Metadata as key */
};
typedef enum ctc_ipfix_port_type_s ctc_ipfix_port_type_t;

/** @brief  IPFIX Key Type */
enum ctc_ipfix_key_type_e
{
    CTC_IPFIX_KEY_HASH_MAC,           /**< [GG.D2.TM] MAC Hash key type */
    CTC_IPFIX_KEY_HASH_L2_L3,         /**< [GG.D2.TM] L2 L3 Hash key type */
    CTC_IPFIX_KEY_HASH_IPV4,          /**< [GG.D2.TM] IPv4 Hash key type */
    CTC_IPFIX_KEY_HASH_IPV6,          /**< [GG.D2.TM] IPv6 Hash key type */
    CTC_IPFIX_KEY_HASH_MPLS,          /**< [GG.D2.TM] MPLS Hash key type */
    CTC_IPFIX_KEY_NUM
};
typedef enum ctc_ipfix_key_type_e ctc_ipfix_key_type_t;

/**
 @brief  Define flex nexthop use packet type
*/
enum ctc_ipfix_l4_type_e
{
    CTC_IPFIX_L4_TYPE_NONE,             /**< [GG] no l4 type*/
    CTC_IPFIX_L4_TYPE_TCP,              /**< [GG] 1: l4 type is TCP*/
    CTC_IPFIX_L4_TYPE_UDP,              /**< [GG] 2: l4 type is UDP*/
    CTC_IPFIX_L4_TYPE_GRE,              /**< [GG] 3: l4 type is GRE*/
    CTC_IPFIX_L4_TYPE_IPINIP,           /**< [GG] 4: l4 type is IPINIP*/
    CTC_IPFIX_L4_TYPE_V6INIP,           /**< [GG] 5: l4 type is V6INIP*/
    CTC_IPFIX_L4_TYPE_ICMP,             /**< [GG] 6: l4 type is ICMP*/
    CTC_IPFIX_L4_TYPE_IGMP,             /**< [GG] 7: l4 type is IGMP*/
    CTC_IPFIX_L4_TYPE_SCTP,             /**< [GG] 8: l4 type is SCTP*/
    CTC_IPFIX_L4_TYPE_MAX
};
typedef enum ctc_ipfix_l4_type_e ctc_ipfix_l4_type_t;

/**
 @brief  Define flex nexthop use packet type
*/
enum ctc_ipfix_l4_sub_type_e
{
    CTC_IPFIX_L4_SUB_TYPE_NONE,           /**< [GG] no l4 sub type, for l4 type is UDP*/
    CTC_IPFIX_L4_SUB_TYPE_BFD,            /**< [GG] 1: l4 sub type is BFD, for l4 type is UDP*/
    CTC_IPFIX_L4_SUB_TYPE_UDPPTP,         /**< [GG] 2: l4 sub type is UDPPTP,for l4 type is UDP*/
    CTC_IPFIX_L4_SUB_TYPE_WLAN,           /**< [GG] 3: l4 sub type is WLAN, for l4 type is UDP*/
    CTC_IPFIX_L4_SUB_TYPE_NTP,            /**< [GG] 4: l4 sub type is NTP, for l4 type is UDP*/
    CTC_IPFIX_L4_SUB_TYPE_VXLAN,          /**< [GG] 5: l4 sub type is VXLAN, for l4 type is UDP*/
    CTC_IPFIX_L4_SUB_TYPE_MAX
};
typedef enum ctc_ipfix_l4_sub_type_e ctc_ipfix_l4_sub_type_t;

/**
 @brief  IPFIX Data struct, including key and ad info
 for Export all info is usefull; for lookup and cpu add/del only key info is usefull
*/
struct ctc_ipfix_data_s
{
    uint32 flags;                    /**< [GG.D2.TM] reger to ctc_ipfix_data_flags_t */
    uint32 port_type;                /**< [GG.D2.TM] refer to ctc_ipfix_port_type_t*/
    uint32 key_type;                 /**< [GG.D2.TM] refer to ctc_ipfix_key_type_t*/

    uint32 gport;                    /**< [GG.D2.TM] Used to identify local phy port, also identify GlobalPort if port_type is CTC_IPFIX_PORT_TYPE_GPORT */
    uint16 logic_port;               /**< [GG.D2.TM] Used to identify logic port or meter data, depends on port_type. */
    uint16 svlan;                    /**< [GG.D2.TM] VLAN identifier. */
    uint16 svlan_prio;               /**< [GG.D2.TM] VLAN priority. */
    uint16 svlan_cfi;                /**< [GG.D2.TM] VLAN CFI*/
    uint16 cvlan;                    /**< [GG.D2.TM] VLAN identifier. */
    uint16 cvlan_prio;               /**< [GG.D2.TM] VLAN priority. */
    uint16 cvlan_cfi;                /**< [GG.D2.TM] VLAN CFI*/
    uint16 ether_type;               /**< [GG.D2.TM] Type in ethernet II frame or 802.1Q tag. */
    uint8  src_cid;                  /**< [D2.TM] Source category id*/
    uint8  dst_cid;                  /**< [D2.TM] Dest category id*/
    uint8  profile_id;               /**< [D2.TM] config profile ID. */
    uint8  field_sel_id;             /**< [D2.GG] Hash select ID. */
    mac_addr_t src_mac;              /**< [GG.D2.TM] Source MAC address. */
    mac_addr_t dst_mac;              /**< [GG.D2.TM] Destination MAC address. */

    ctc_ipfix_l3_info_t l3_info;     /**< [GG.D2.TM] Layer3 data. */
    ctc_ipfix_l4_info_t l4_info;     /**< [GG.D2.TM] Layer4 data. */

    ctc_direction_t dir;             /**< [GG.D2.TM] Ingress or egress. */

    /*export information*/
    uint32  export_reason;           /**< [GG.D2.TM] Export reason used to construct ad information */
    uint32  start_timestamp;         /**< [GG.D2.TM] Start timestamp in this session. */
    uint32  last_timestamp;          /**< [GG.D2.TM] Last timestamp in this session. */
    uint64  byte_count;              /**< [GG.D2.TM] Total byts count in this session. */
    uint32  pkt_count;               /**< [GG.D2.TM] Total packets count in this session. */
    uint8   ttl;                     /**< [GG] TTL value of the first packet. */
    uint8   min_ttl;                 /**< [GG.D2.TM] Min ttl value in  all packets. */
    uint8   max_ttl;                 /**< [GG.D2.TM] Max ttl value in all packets. */
    uint8   tcp_flags;               /**< [D2.TM] Tcp flags status, consist of CWR|ECE|URG|ACK|PSH|RST|SYN|FIN */
    uint16  dest_gport;              /**< [GG.D2.TM] Destination global port ID for unicast, include LinkAgg, valid if CTC_PKT_FLAG_MCAST is not set */
    uint16  dest_group_id;           /**< [GG.D2.TM] Destination group ID for multicast, valid if CTC_PKT_FLAG_MCAST is set */

    uint32 max_latency;              /**< [TM] max latency stats, uint:ns*/
    uint32 min_latency;              /**< [TM] min latency stats, uint:ns*/
    uint32 max_jitter;               /**< [TM] max jitter stats, uint:ns*/
    uint32 min_jitter;               /**< [TM] min jitter stats, uint:ns*/

} ;
typedef struct ctc_ipfix_data_s ctc_ipfix_data_t;

/** @brief IPFIX LOOKUP type */
enum ctc_ipfix_lkup_type_e
{
    CTC_IPFIX_HASH_LKUP_TYPE_DISABLE,     /**< [GG] Disable */
    CTC_IPFIX_HASH_LKUP_TYPE_L2,          /**< [GG] only use l2 field*/
    CTC_IPFIX_HASH_LKUP_TYPE_L3,          /**< [GG] only use l3 field*/
    CTC_IPFIX_HASH_LKUP_TYPE_L2L3,        /**< [GG] L2+ L3, Notice if using l2l3 type, must using etherType as one of hashsel elements*/
    CTC_IPFIX_HASH_LKUP_TYPE_MAX
};
typedef enum ctc_ipfix_lkup_type_e ctc_ipfix_lkup_type_t;

struct ctc_ipfix_hash_mac_field_sel_s
{
    uint8 mac_da;                /**< [GG.D2.TM] MAC DA select enable*/
    uint8 mac_sa;                /**< [GG.D2.TM] MAC SA select enable*/
    uint8 eth_type;              /**< [GG.D2.TM] Eth-Type select enable*/
    uint8 gport;                 /**< [GG.D2.TM] Using gport hash, not gport value*/
    uint8 logic_port;            /**< [GG.D2.TM] Logic port select enable*/
    uint8 metadata;              /**< [GG.D2.TM] Metadata select enable*/
    uint8 cos;                   /**< [GG.D2.TM] COS select enable*/
    uint8 cfi;                   /**< [GG.D2.TM] CFI SA select enable*/
    uint8 vlan_id;               /**< [GG.D2.TM] VLAN select enable*/
    uint8 src_cid;               /**< [D2.TM] src category id select enable*/
    uint8 dst_cid;               /**< [D2.TM] dst category id select enable*/
    uint8 profile_id;            /**< [D2.TM] Config profile id select enable*/
};
typedef struct ctc_ipfix_hash_mac_field_sel_s ctc_ipfix_hash_mac_field_sel_t;


struct ctc_ipfix_hash_ipv4_field_sel_s
{
    uint8  ip_da;           /**< [GG.D2.TM] IP DA select enable*/
    uint8  ip_da_mask;      /**< [GG.D2.TM] IP DA mask length*/
    uint8  ip_sa;           /**< [GG.D2.TM] IP SA select enable*/
    uint8  ip_sa_mask;      /**< [GG.D2.TM] IP SA mask length*/
    uint8  l4_src_port;     /**< [GG.D2.TM] l4 source port select enable*/
    uint8  l4_dst_port;     /**< [GG.D2.TM] l4 destination port select enable*/
    uint8  gport;           /**< [GG.D2.TM] Gport select enable*/
    uint8  logic_port;      /**< [GG.D2.TM] Logic port select enable*/
    uint8  metadata;        /**< [GG.D2.TM] Metadata select enable*/
    uint8  dscp;            /**< [GG.D2.TM] DSCP select enable*/
    uint8  ttl;             /**< [D2.TM] TTL select enable*/
    uint8  ecn;             /**< [D2.TM] ECN select enable*/
    uint8  ip_protocol;     /**< [GG.D2.TM] IP protocol select enable*/
    uint8  l4_sub_type;     /**< [GG] Before cfg l4_sub_type, first enable ip_protocol*/

    uint8  ip_identification; /**< [D2.TM] Ip identification select enable*/
    uint8  ip_pkt_len;        /**< [D2.TM] Ip packet length select enable*/
    uint8  ip_frag;           /**< [D2.TM] Ip fragment select enable*/
    uint8  tcp_flags;         /**< [D2.TM] Tcp flags select enable, consist of CWR|ECE|URG|ACK|PSH|RST|SYN|FIN */
    uint8  vrfid;             /**< [D2.TM] Vrfid select enable*/

    uint8  icmp_type;       /**< [GG.D2.TM] ICMP type select enable*/
    uint8  icmp_code;       /**< [GG.D2.TM] ICMP code select enable*/
    uint8  igmp_type;       /**< [D2.TM] IGMP type select enable*/
    uint8  vxlan_vni;       /**< [GG.D2.TM] VXLAN select enable*/
    uint8  gre_key;         /**< [GG.D2.TM] GRE select enable*/
    uint8  nvgre_key;    /**< [D2.TM] NVGRE vsid select enable*/
    uint8  src_cid;         /**< [D2.TM] src category id select enable*/
    uint8  dst_cid;         /**< [D2.TM] dst category id select enable*/
    uint8  profile_id;      /**< [D2.TM] config profile id select enable*/
};
typedef struct ctc_ipfix_hash_ipv4_field_sel_s ctc_ipfix_hash_ipv4_field_sel_t;

struct ctc_ipfix_hash_mpls_field_sel_s
{
    uint8 logic_port;           /**< [GG.D2.TM] Logic port select enable*/
    uint8 gport;                /**< [GG.D2.TM] Gport select enable*/
    uint8 metadata;             /**< [GG.D2.TM] Metadata select enable*/
    uint8 label_num;            /**< [GG.D2.TM] MPLS lable number select enable*/
    uint8 mpls_label0_label;    /**< [GG.D2.TM] MPLS lable0 select enable*/
    uint8 mpls_label0_exp;      /**< [GG.D2.TM] MPLS lable0_exp select enable*/
    uint8 mpls_label0_s;        /**< [GG.D2.TM] MPLS lable0_s select enable*/
    uint8 mpls_label0_ttl;      /**< [GG.D2.TM] MPLS lable0_ttl select enable*/
    uint8 mpls_label1_label;    /**< [GG.D2.TM] MPLS lable1 select enable*/
    uint8 mpls_label1_exp;      /**< [GG.D2.TM] MPLS lable1_exp select enable*/
    uint8 mpls_label1_s;        /**< [GG.D2.TM] MPLS lable1_s select enable*/
    uint8 mpls_label1_ttl;      /**< [GG.D2.TM] MPLS lable1_ttl select enable*/
    uint8 mpls_label2_label;    /**< [GG.D2.TM] MPLS lable2 select enable*/
    uint8 mpls_label2_exp;      /**< [GG.D2.TM] MPLS lable2_exp select enable*/
    uint8 mpls_label2_s;        /**< [GG.D2.TM] MPLS lable2_s select enable*/
    uint8 mpls_label2_ttl;      /**< [GG.D2.TM] MPLS lable2_ttl select enable*/
    uint8 profile_id;           /**< [D2.TM] config profile id select enable*/
};
typedef struct ctc_ipfix_hash_mpls_field_sel_s ctc_ipfix_hash_mpls_field_sel_t;

struct ctc_ipfix_hash_ipv6_field_sel_s
{
    uint8 ip_da;                /**< [GG.D2.TM] IP DA select enable*/
    uint8 ip_da_mask;           /**< [GG.D2.TM] Mask len value4,8,12,...,128 */
    uint8 ip_sa;                /**< [GG.D2.TM] IP SA select enable*/
    uint8 ip_sa_mask;           /**< [GG.D2.TM] Mask len value4,8,12,...,128 */
    uint8 l4_src_port;          /**< [GG.D2.TM] l4 source port select enable*/
    uint8 l4_dst_port;          /**< [GG.D2.TM] l4 destination port select enable*/
    uint8 gport;                /**< [GG.D2.TM] Gport select enable*/
    uint8 logic_port;           /**< [GG.D2.TM] Logic port select enable*/
    uint8 metadata;             /**< [GG.D2.TM] Metadata select enable*/
    uint8 dscp;                 /**< [GG.D2.TM] DSCP select enable*/
    uint8 ttl;                  /**< [D2.TM] TTL select enable*/
    uint8 ecn;                  /**< [D2.TM] ECN select enable*/
    uint8 flow_label;           /**< [GG.D2.TM] flow_label select enable*/
    uint8 l4_type;              /**< [GG] l4_type select enable*/
    uint8 l4_sub_type;          /**< [GG] Before cfg l4_sub_type, first enable l4_type. Only support VXLAN*/

    uint8 icmp_type;            /**< [GG.D2.TM] ICMP type select enable*/
    uint8 icmp_code;            /**< [GG.D2.TM] ICMP code select enable*/
    uint8 igmp_type;            /**< [D2.TM] IGMP type select enable*/

    uint8 vxlan_vni;            /**< [GG.D2.TM] VXLAN select enable*/
    uint8 gre_key;              /**< [GG.D2.TM] GRE select enable*/
    uint8 nvgre_key;         /**< [D2.TM] NVGRE vsid select enable*/
    uint8 aware_tunnel_info_en; /**< [D2.TM] if enable, indicate lookup key use innner info as key, and outer tunnel info can also be used as lookup key*/
    uint8 wlan_radio_mac;       /**< [D2.TM] Wlan radio mac enable*/
    uint8 wlan_radio_id;        /**< [D2.TM] Wlan radio id enable*/
    uint8 wlan_ctl_packet;      /**< [D2.TM] Wlan control packet enable */
    uint8 ip_protocol;          /**< [D2.TM] Ip protocol select enable*/
    uint8 ip_frag;              /**< [D2.TM] Ip fragment select enable*/
    uint8 tcp_flags;            /**< [D2.TM] Tcp flags select enable, consist of CWR|ECE|URG|ACK|PSH|RST|SYN|FIN*/
    uint8 src_cid;              /**< [D2.TM] src category id select enable*/
    uint8 dst_cid;              /**< [D2.TM] dst category id select enable*/
    uint8 profile_id;           /**< [D2.TM] config profile id select enable*/

};
typedef struct ctc_ipfix_hash_ipv6_field_sel_s ctc_ipfix_hash_ipv6_field_sel_t;


struct ctc_ipfix_hash_l2_l3_field_sel_s
{
    uint8 gport;                /**< [GG.D2.TM] Gport select enable*/
    uint8 logic_port;           /**< [GG.D2.TM] Logic port select enable*/
    uint8 metadata;             /**< [GG.D2.TM] Metadata select enable*/
    uint8 mac_da;               /**< [GG.D2.TM] MAC DA select enable*/
    uint8 mac_sa;               /**< [GG.D2.TM] MAC SA select enable*/
    uint8 stag_cos;             /**< [GG.D2.TM] SVLAN COS select enable*/
    uint8 stag_cfi;             /**< [GG.D2.TM] SVLAN CFI select enable*/
    uint8 stag_vlan;            /**< [GG.D2.TM] SVLAN select enable*/
    uint8 ctag_cos;             /**< [GG] CVLAN COS select enable*/
    uint8 ctag_cfi;             /**< [GG] CVLAN COS select enable*/
    uint8 ctag_vlan;            /**< [GG] CVLAN select enable*/

    uint8 ip_da;                /**< [GG.D2.TM] IP DA select enable*/
    uint8 ip_da_mask;           /**< [GG.D2.TM] IP DA mask length*/
    uint8 ip_sa;                /**< [GG.D2.TM] IP SA select enable*/
    uint8 ip_sa_mask;           /**< [GG.D2.TM] IP SA mask length*/
    uint8 dscp;                 /**< [GG.D2.TM] DSCP select enable*/
    uint8 ecn;                  /**< [GG.D2.TM] ECN select enable*/
    uint8 ttl;                  /**< [GG.D2.TM] TTL select enable*/
    uint8 eth_type;             /**< [GG.D2.TM] Eth_type select enable*/
    uint8 vrfid;                /**< [GG.D2.TM] vrfid select enable*/
    uint8 l4_type;              /**< [GG] L4 type select enable*/
    uint8 l4_sub_type;          /**< [GG] Before cfg l4_sub_type, first enable l4_type*/
    uint8 ip_protocol;          /**< [D2.TM] IP protocol select enable*/
    uint8 ip_identification;    /**< [D2.TM] Ip identification select enable*/
    uint8 ip_frag;              /**< [D2.TM] Ip fragment select enable*/
    uint8 tcp_flags;            /**< [D2.TM] Tcp flags select enable, consist of CWR|ECE|URG|ACK|PSH|RST|SYN|FIN*/

    uint8 gre_key;              /**< [GG.D2.TM] GRE select enable*/
    uint8 nvgre_key;          /**< [D2.TM] NVGRE vsid select enable*/
    uint8 vxlan_vni;            /**< [GG.D2.TM] Vxlan select enable*/
    uint8 aware_tunnel_info_en; /**< [D2.TM] if enable, indicate lookup key use innner info as key, and outer tunnel info can also be used as lookup key*/
    uint8 wlan_radio_mac;       /**< [D2.TM] Wlan radio mac enable*/
    uint8 wlan_radio_id;        /**< [D2.TM] Wlan radio id enable*/
    uint8 wlan_ctl_packet;      /**< [D2.TM] Wlan control packet enable */
    uint8 l4_src_port;          /**< [GG.D2.TM] l4 source port select enable*/
    uint8 l4_dst_port;          /**< [GG.D2.TM] l4 destination port select enable*/
    uint8 icmp_type;            /**< [GG.D2.TM] ICMP type select enable*/
    uint8 icmp_code;            /**< [GG.D2.TM] ICMP code select enable*/
    uint8 igmp_type;            /**< [D2.TM] IGMP type select enable*/

    uint8 label_num;            /**< [GG.D2.TM] MPLS lable number select enable*/
    uint8 mpls_label0_label;    /**< [GG.D2.TM] MPLS lable0 select enable*/
    uint8 mpls_label0_exp;      /**< [GG.D2.TM] MPLS lable0_exp select enable*/
    uint8 mpls_label0_s;        /**< [GG.D2.TM] MPLS lable0_s select enable*/
    uint8 mpls_label0_ttl;      /**< [GG.D2.TM] MPLS lable0_ttl select enable*/
    uint8 mpls_label1_label;    /**< [GG.D2.TM] MPLS lable1 select enable*/
    uint8 mpls_label1_exp;      /**< [GG.D2.TM] MPLS lable1_exp select enable*/
    uint8 mpls_label1_s;        /**< [GG.D2.TM] MPLS lable1_s select enable*/
    uint8 mpls_label1_ttl;      /**< [GG.D2.TM] MPLS lable1_ttl select enable*/
    uint8 mpls_label2_label;    /**< [GG.D2.TM] MPLS lable2 select enable*/
    uint8 mpls_label2_exp;      /**< [GG.D2.TM] MPLS lable2_exp select enable*/
    uint8 mpls_label2_s;        /**< [GG.D2.TM] MPLS lable2_s select enable*/
    uint8 mpls_label2_ttl;      /**< [GG.D2.TM] MPLS lable2_ttl select enable*/
    uint8 src_cid;              /**< [D2.TM] src category id select enable*/
    uint8 dst_cid;              /**< [D2.TM] dst category id select enable*/
    uint8 profile_id;           /**< [D2.TM] config profile id select enable*/

};
typedef struct ctc_ipfix_hash_l2_l3_field_sel_s ctc_ipfix_hash_l2_l3_field_sel_t;


struct ctc_ipfix_hash_field_sel_s
{
    uint8  key_type;                            /**< [GG.D2.TM] Refer ctc_ipfix_key_type_t */
    uint8  field_sel_id;                        /**< [GG.D2.TM] Hash select ID */

    union
    {
        ctc_ipfix_hash_mac_field_sel_t   mac;   /**< [GG.D2.TM] Hash key type is mac key*/
        ctc_ipfix_hash_mpls_field_sel_t  mpls;  /**< [GG.D2.TM] Hash key type is mpls key*/
        ctc_ipfix_hash_ipv4_field_sel_t  ipv4;  /**< [GG.D2.TM] Hash key type is ipv4 key*/
        ctc_ipfix_hash_ipv6_field_sel_t  ipv6;  /**< [GG.D2.TM] Hash key type is ipv6 key*/
        ctc_ipfix_hash_l2_l3_field_sel_t l2_l3; /**< [GG.D2.TM] Hash key type is l2l3 key*/
    }u;
};
typedef struct ctc_ipfix_hash_field_sel_s ctc_ipfix_hash_field_sel_t;

/* IPFIX port configuration structure */
struct ctc_ipfix_port_cfg_s
{
    uint8   dir;                    /**< [GG] Direction*/
    uint8   lkup_type;              /**< [GG] Refer to ctc_ipfix_lkup_type_t */
    uint8   field_sel_id;           /**< [GG] Field select id , used associate with ctc_ipfix_key_type_t */
    uint8   tcp_end_detect_disable; /**< [GG] Tcp end detect disable*/
    uint8   flow_type;              /**< [GG] IPFIX aim to packet type, refer to ctc_ipfix_flow_type_t*/
    uint8   learn_disable;          /**< [GG] Disable learn new flow based port*/
    uint16  sample_interval;        /**< [GG] Global support for packet interval*/
};
typedef struct ctc_ipfix_port_cfg_s ctc_ipfix_port_cfg_t;


struct ctc_ipfix_flow_cfg_s
{
    uint8   profile_id;             /**< [D2.TM] Config profile id.*/
    uint8   dir;                    /**< [D2.TM] Direction*/
    uint8   tcp_end_detect_disable; /**< [D2.TM] Tcp end detect disable based on flow. */
    uint8   log_pkt_count;          /**< [D2.TM] Log/mirror the first N packets for new flow. */
    uint8   flow_type;              /**< [D2.TM] IPFIX aim to packet type, refer to ctc_ipfix_flow_type_t*/
    uint8   learn_disable;          /**< [D2.TM] Disable learn new flow based on flow. */
    uint8   sample_mode;            /**< [D2.TM] Sample mode,0:fixed interval 1:random mode*/
    uint8   sample_type;            /**< [D2.TM] Sample type,0:based on all packets, 1: based on unkown packet*/
    uint16  sample_interval;        /**< [D2.TM] if sample_interval = N, then do ipfix probability 1/N. */
};
typedef struct ctc_ipfix_flow_cfg_s ctc_ipfix_flow_cfg_t;


/**
 @brief define learn & Aging  global config information
*/
struct ctc_ipfix_global_cfg_s
{
    uint32  aging_interval;        /**< [GG.D2.TM] Aging interval (s) */
    uint32  pkt_cnt;               /**< [GG.D2.TM] Number of packets to be export.*/
    uint64  bytes_cnt;             /**< [GG.D2.TM] Number of bytes threshold to be export.*/
    uint32  times_interval;        /**< [GG.D2.TM] Number of times interval to be export. */
    uint8   sample_mode;           /**< [GG] Sample mode, 0: based on all packet, 1: based on unknown packet*/
    uint8   conflict_export;       /**< [GG.D2.TM] Conflict entry export to cpu */
    uint8   tcp_end_detect_en;     /**< [GG.D2.TM] Global enable tcp end detect */
    uint8   new_flow_export_en;    /**< [GG.D2.TM] New flow export control */
    uint8   sw_learning_en;        /**< [GG.D2.TM] Cpu add key and delete key, asic only do update, not insert */
    uint8   unkown_pkt_dest_type;  /**< [GG.D2.TM] 0:using group id, 1:using vlan id */
    uint16  threshold;             /**< [D2.TM] if flow count up/down to threshold will trigger interrupt */

	uint8   latency_type;          /**< [TM] Monitor latency type, 0: monitor latency stats, 1:monitor jitter stats*/
	uint32  conflict_cnt;          /**< [D2.TM] Conflict entry counter*/

};
typedef struct ctc_ipfix_global_cfg_s ctc_ipfix_global_cfg_t;

enum ctc_ipfix_traverse_type_e
{
    CTC_IPFIX_TRAVERSE_BY_ALL,     /**< [GG.D2.TM] Traverse by all */
    CTC_IPFIX_TRAVERSE_BY_PORT,    /**< [GG.D2.TM] Traverse by port */
    CTC_IPFIX_TRAVERSE_BY_DIR,     /**< [GG.D2.TM] Traverse by direction */
    CTC_IPFIX_TRAVERSE_BY_PORT_DIR /**< [GG.D2.TM] Traverse by port and direction */
};
typedef enum ctc_ipfix_traverse_type_e ctc_ipfix_traverse_type_t;

struct ctc_ipfix_traverse_s
{
    void*   user_data;           /**< [GG.D2.TM] User data*/
    uint32  start_index;         /**< [GG.D2.TM] If it is the first traverse, it is equal to 0, else it is equal to the last next_traverse_index */
    uint32  next_traverse_index; /**< [GG.D2.TM] [out] return index of the next traverse */
    uint8   is_end;              /**< [GG.D2.TM] [out] if is_end == 1, indicate the end of traverse */
    uint16  entry_num;           /**< [GG.D2.TM] indicate entry number for one traverse function, total entry num is 8k */
    uint8   lchip_id;            /**< [D2.TM]    Traverse entry for specific local chip id */
};
typedef struct ctc_ipfix_traverse_s ctc_ipfix_traverse_t;

/* ctc_ipfix_fn_t */
typedef void (*ctc_ipfix_fn_t)( ctc_ipfix_data_t* info, void* userdata);
typedef int32 (* ctc_ipfix_traverse_fn)(ctc_ipfix_data_t* p_data, void* user_data);
typedef ctc_ipfix_traverse_fn ctc_ipfix_traverse_remove_cmp;
struct ctc_ipfix_event_s
{
    uint8 exceed_threshold;       /**< [D2.TM] if set, measns flow count exceed threshold else down to threshold */
    uint8 rsv[3];
};
typedef struct ctc_ipfix_event_s ctc_ipfix_event_t;
/**@} end of @defgroup ipfix  */

#ifdef __cplusplus
}
#endif

#endif
