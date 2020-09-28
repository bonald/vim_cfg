/**
 @file ctc_parser.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-15

 @version v2.0

This file contains all parser related data structure, enum, macro and proto.
*/

#ifndef CTC_PARSER_H_
#define CTC_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 @defgroup parser PARSER
 @{
*/

/**
 @defgroup parser_l2 PARSER_L2
 @{
*/
/**
 @brief  the packet layer2 type
*/
enum ctc_parser_pkt_type_e
{
    CTC_PARSER_PKT_TYPE_ETHERNET,    /**< [GB.GG.D2.TM] 0: parser packet type,it's config in system initialization*/
    CTC_PARSER_PKT_TYPE_IP_OR_IPV6,  /**< [GB.GG.D2.TM] 1: ipv4 in gb,parser packet type,it's config in system intialization*/
    CTC_PARSER_PKT_TYPE_MPLS,        /**< [GB.GG.D2.TM] 2: parser packet type,it's config in system intialization*/
    CTC_PARSER_PKT_TYPE_IPV6,        /**< [GB.GG.D2.TM] 3: parser packet type,it's config in system intialization*/
    CTC_PARSER_PKT_TYPE_MCAST_MPLS,  /**< [GB.GG.D2.TM] 4: parser packet type,it's config in system intialization*/
    CTC_PARSER_PKT_TYPE_CESOETH,     /**< [GB.GG.D2.TM] 5: trill in gb,parser packet type,it's config in system intialization*/
    CTC_PARSER_PKT_TYPE_FLEXIBLE,    /**< [GB.GG.D2.TM] 6: parser packet type,it's config in system intialization*/
    CTC_PARSER_PKT_TYPE_RESERVED,    /**< [GB.GG.D2.TM] 7: parser packet type,it's config in system intialization*/

    MAX_CTC_PARSER_PKT_TYPE
};
typedef enum ctc_parser_pkt_type_e ctc_parser_pkt_type_t;

/**
 @brief  the packet layer2 type
*/
enum ctc_parser_l2_type_e
{
    CTC_PARSER_L2_TYPE_NONE,                    /**< [GB.GG.D2.TM] 0: parser layer2 type,it's config in system intialization*/
    CTC_PARSER_L2_TYPE_ETH_V2,                  /**< [GB.GG.D2.TM] 1: parser layer2 type,it's config in system initialization*/
    CTC_PARSER_L2_TYPE_ETH_SAP,                 /**< [GB.GG.D2.TM] 2: parser layer2 type,it's config in system initialization*/
    CTC_PARSER_L2_TYPE_ETH_SNAP,                /**< [GB.GG.D2.TM] 3: parser layer2 type,it's config in system initialization*/
    CTC_PARSER_L2_TYPE_PPP_2B,                  /**< [GB]    4: parser layer2 type,it's config in system initialization*/
    CTC_PARSER_L2_TYPE_PPP_1B,                  /**< [GB]    5: parser layer2 type,it's config in system initialization*/
    MAX_CTC_PARSER_L2_TYPE
};
typedef enum ctc_parser_l2_type_e ctc_parser_l2_type_t;

/**
 @brief  the packet layer3 type
*/
enum ctc_parser_l3_type_e
{
    CTC_PARSER_L3_TYPE_NONE,             /**< [GB.GG.D2.TM] 0: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_IP,               /**< [GB.GG.D2.TM] 1: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_IPV4,             /**< [GB.GG.D2.TM] 2: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_IPV6,             /**< [GB.GG.D2.TM] 3: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_MPLS,             /**< [GB.GG.D2.TM] 4: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_MPLS_MCAST,       /**< [GB.GG.D2.TM] 5: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_ARP,              /**< [GB.GG.D2.TM] 6: parser layer3 type,it's config in system intialization*/

    CTC_PARSER_L3_TYPE_FCOE = 7,         /**< [GB.GG.D2.TM] 7: parser layer3 type */
    CTC_PARSER_L3_TYPE_TRILL = 8,        /**< [GB.GG.D2.TM] 8: parser layer3 type */
    CTC_PARSER_L3_TYPE_ETHER_OAM = 9,        /**< [GB.GG.D2.TM] 9: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_SLOW_PROTO =10,       /**< [GB.GG.D2.TM] 10: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_CMAC =11,             /**< [GB.GG.D2.TM] 11: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_PTP = 12,              /**< [GB.GG.D2.TM] 12: parser layer3 type,it's config in system intialization*/

    CTC_PARSER_L3_TYPE_DOT1AE = 13,             /**< [D2.TM] 13: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_RSV_USER_DEFINE0 = 13,   /**< [GB.GG] 13: parser layer3 type, it can be configed by user*/
    CTC_PARSER_L3_TYPE_SATPDU = 14,             /**< [D2.TM] 14: parser layer3 type,it's config in system intialization*/
    CTC_PARSER_L3_TYPE_RSV_USER_DEFINE1 = 14,   /**< [GB.GG] 14: parser layer3 type, it can be configed by user*/
    CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3,         /**< [GB.GG.D2.TM] 15: parser layer3 type,it's config in system intialization*/
    MAX_CTC_PARSER_L3_TYPE
};
typedef enum ctc_parser_l3_type_e ctc_parser_l3_type_t;

/**
 @brief  the packet layer4 type
*/
enum ctc_parser_l4_type_e
{
    CTC_PARSER_L4_TYPE_NONE,             /**< [GB.GG.D2.TM] 0: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_TCP,              /**< [GB.GG.D2.TM] 1: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_UDP,              /**< [GB.GG.D2.TM] 2: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_GRE,              /**< [GB.GG.D2.TM] 3: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_IPINIP,           /**< [GB.GG.D2.TM] 4: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_V6INIP,           /**< [GB.GG.D2.TM] 5: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_ICMP,             /**< [GB.GG.D2.TM] 6: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_IGMP,             /**< [GB.GG.D2.TM] 7: parser layer4 type,it's config in system intialization*/
    CTC_PARSER_L4_TYPE_IPINV6 = 8,       /**< [GB.GG.D2.TM] 8: parser layer4 type*/
    CTC_PARSER_L4_TYPE_PBB_ITAG_OAM = 9, /**< [GB.GG.D2.TM] 9: parser layer4 type*/
    CTC_PARSER_L4_TYPE_ACH_OAM = 10,     /**< [GB.GG.D2.TM] 10: parser layer4 type*/
    CTC_PARSER_L4_TYPE_V6INV6 = 11,      /**< [GB.GG.D2.TM] 11: parser layer4 type*/
    CTC_PARSER_L4_TYPE_RDP = 12,         /**< [GB.GG.D2.TM] 12: parser layer4 type*/
    CTC_PARSER_L4_TYPE_SCTP = 13,        /**< [GB.GG.D2.TM] 13: parser layer4 type*/
    CTC_PARSER_L4_TYPE_DCCP = 14,        /**< [GB.GG.D2.TM] 14: parser layer4 type*/
    CTC_PARSER_L4_TYPE_ANY_PROTO,        /**< [GB] 15: parser layer4 type,it's config in system intialization*/
    MAX_CTC_PARSER_L4_TYPE
};
typedef enum ctc_parser_l4_type_e  ctc_parser_l4_type_t;

/**
 @brief  the packet layer4 user type
*/
enum ctc_parser_l4_usertype_e
{
    CTC_PARSER_L4_USER_TYPE_NONE = 0,            /**< [D2.TM] 0: parser layer4 user type, it's config in system intialization*/

    /* L4UserType used for L4Type is CTC_PARSER_L4_TYPE_ACH_OAM */
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHY1731,     /**< [D2.TM] 1: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHBFD,       /**< [D2.TM] 2: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHCC,        /**< [D2.TM] 3: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHCV,        /**< [D2.TM] 4: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHDLM,       /**< [D2.TM] 5: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHILM,       /**< [D2.TM] 6: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHILMDM,     /**< [D2.TM] 7: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHLMDM,      /**< [D2.TM] 8: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_ACHDM,        /**< [D2.TM] 9: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_MCC,          /**< [D2.TM] 10: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_ACHOAM_SCC,          /**< [D2.TM] 11: parser layer4 user type, it's config in system intialization*/

    /* L4UserType used for L4Type is CTC_PARSER_L4_TYPE_UDP */
    CTC_PARSER_L4_USER_TYPE_UDP_BFD = 1,             /**< [D2.TM] 1: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_UDP_UDPPTP,          /**< [D2.TM] 2: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_UDP_CAPWAP,          /**< [D2.TM] 3: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_UDP_NTP,             /**< [D2.TM] 4: parser layer4 user type, it's config in system intialization*/
    CTC_PARSER_L4_USER_TYPE_UDP_VXLAN,           /**< [D2.TM] 5: parser layer4 user type, it's config in system intialization*/
    MAX_CTC_PARSER_L4_USER_TYPE
};
typedef enum ctc_parser_l4_usertype_e ctc_parser_l4_usertype_t;

/**
 @brief  packet type
*/
enum packet_type_e
{
    PKT_TYPE_ETH = 0,       /**< [GB.GG.D2.TM] pkt type map type0*/
    PKT_TYPE_IPV4,          /**< [GB.GG.D2.TM] pkt type map type1*/
    PKT_TYPE_MPLS,          /**< [GB.GG.D2.TM] pkt type map type2*/
    PKT_TYPE_IPV6,          /**< [GB.GG.D2.TM] pkt type map type3*/
    PKT_TYPE_IP = 5,        /**<[GG.D2.TM] pkt type map type5*/
    PKT_TYPE_FLEXIBLE = 6,  /**<[GB.GG.D2.TM] pkt type map type6*/

    PKT_TYPE_RESERVED = 7   /**< [GB.GG.D2.TM] pkt type maptype7*/
};
typedef enum packet_type_e packet_type_t;

/**
 @brief  tpid enum
*/
enum ctc_parser_l2_tpid_e
{
    CTC_PARSER_L2_TPID_CVLAN_TPID,      /**< [GB.GG.D2.TM] cvlan tpid*/
    CTC_PARSER_L2_TPID_ITAG_TPID,       /**< [GB.GG.D2] itag tpid in pbb packet*/
    CTC_PARSER_L2_TPID_BLVAN_TPID,      /**< [GB.GG.D2] bvlan tpid in pbb packet*/
    CTC_PARSER_L2_TPID_SVLAN_TPID_0,    /**< [GB.GG.D2.TM] svlan tpid0*/
    CTC_PARSER_L2_TPID_SVLAN_TPID_1,    /**< [GB.GG.D2.TM] svlan tpid1*/
    CTC_PARSER_L2_TPID_SVLAN_TPID_2,    /**< [GB.GG.D2.TM] svlan tpid2*/
    CTC_PARSER_L2_TPID_SVLAN_TPID_3,    /**< [GB.GG.D2.TM] svlan tpid3*/
    CTC_PARSER_L2_TPID_CNTAG_TPID,      /**< [GB.GG.D2.TM] cntag tpid in Congestion Notification (CN) message, defined in the 802.1Qau*/
    CTC_PARSER_L2_TPID_EVB_TPID         /**< [GG.D2.TM] evb tpid */
};
typedef enum ctc_parser_l2_tpid_e ctc_parser_l2_tpid_t;

/**
 @brief  pbb header field
*/
struct ctc_parser_pbb_header_s
{
    uint8 nca_value_en;              /**< [GB.GG.D2.TM] nca value enable set*/
    uint8 outer_vlan_is_cvlan;       /**< [GB.GG.D2.TM] outer vlan is cvlan */
    uint8 vlan_parsing_num;          /**< [GB.GG.D2.TM] vlan parsing num */
    uint8 resv;
};
typedef struct ctc_parser_pbb_header_s ctc_parser_pbb_header_t;

/**
 @brief  layer2 flex header field
*/
struct ctc_parser_l2flex_header_s
{
    uint8 mac_da_basic_offset;          /**< [GB] macda  basic offset, 0-26*/
    uint8 header_protocol_basic_offset; /**< [GB] protocol type (Ethernet Type) basic offset, 0-30*/
    uint8 min_length;                   /**< [GB] min length for parser check*/
    uint8 l2_basic_offset;              /**< [GB] layer2 basic offset, 0-63*/
};
typedef struct ctc_parser_l2flex_header_s ctc_parser_l2flex_header_t;

/**
 @brief  layer2 protocol entry fields
*/
struct ctc_parser_l2_protocol_entry_s
{
    uint32 mask;                        /**< [GB.GG.D2.TM] mask for layer2 header protocol,layer2 type,isEth,isPPP,isSAP,0-0x7FFFFF*/

    uint16 l2_header_protocol;          /**< [GB.GG.D2.TM] layer2 header protocol(ethertype)*/
    uint8 l3_type;                      /**<[GB.GG.D2.TM] layer3 type, 13-15,CTC_PARSER_L3_TYPE_RSV_XXX*/
    uint8 addition_offset;              /**< [GB.GG.D2.TM] addition offset for layer3 info parser, 0-15*/

    ctc_parser_l2_type_t l2_type;       /**< [GB.GG.D2.TM] layer2 type, CTC_PARSER_L2_TYPE_XXX*/
};
typedef struct ctc_parser_l2_protocol_entry_s ctc_parser_l2_protocol_entry_t;

/**@} end of @defgroup  parser_l2 PARSER_L2 */

/**
 @defgroup parser_l3 PARSER_L3
 @{
*/

/**
 @brief  layer3 flex header fields
*/
struct ctc_parser_l3flex_header_s
{
    uint8 ipda_basic_offset;              /**< [GB] the basic offset of ipda, 0-24*/
    uint8 l3min_length;                   /**< [GB] layer3 min length, 0-255*/
    uint8 protocol_byte_sel;              /**< [GB] protocol byte selection, 0-31*/
    uint8 l3_basic_offset;                /**< [GB] layer3 basic offset, 0-255*/
};
typedef struct ctc_parser_l3flex_header_s ctc_parser_l3flex_header_t;

/**
 @brief  layer3 protocol entry fields
*/
struct ctc_parser_l3_protocol_entry_s
{
    uint8 l4_type;                        /**< [GB.GG.D2.TM] layer4 type,CTC_PARSER_L4_TYPE_XXX*/
    uint8 addition_offset;                /**< [GB.GG.D2.TM] additional offset for layer4 basic offset*/
    uint8 l3_type_mask;                   /**< [GB.GG.D2.TM] layer3 type mask*/
    uint8 l3_type;                        /**< [GB.GG.D2.TM] layer3 type, CTC_PARSER_L3_TYPE_XXX*/

    uint8 l3_header_protocol;             /**< [GB.GG.D2.TM] layer3 header protocol*/
    uint8 l3_header_protocol_mask;        /**< [GB.GG.D2.TM] layer3 header protocol mask*/
    uint16 resv;
};
typedef struct ctc_parser_l3_protocol_entry_s ctc_parser_l3_protocol_entry_t;

/**
 @brief  trill header field
*/
struct ctc_parser_trill_header_s
{
    uint16 inner_tpid;                    /**< [GB.GG.D2.TM] inner vlan tag tpid*/
    uint16 rbridge_channel_ether_type;    /**< [GB.GG.D2.TM] rbridge channel ether type*/
};
typedef struct ctc_parser_trill_header_s ctc_parser_trill_header_t;

/**@} end of @defgroup  parser_l3 PARSER_L3*/

/**
 @defgroup parser_l4 PARSER_L4
 @{
*/

/**
 @brief  layer4 flex header fields
*/
struct ctc_parser_l4flex_header_s
{
    uint8 dest_port_basic_offset;        /**< [GB.GG.D2.TM] byte selection to put as layer4 dest port, 0-30*/
    uint8 l4_min_len;                    /**< [GB.GG.D2.TM] layer4 min length, 0-31*/
    uint8 resv[2];
};
typedef struct ctc_parser_l4flex_header_s ctc_parser_l4flex_header_t;

/**
@brief  layer4 app ctl fields
*/
struct ctc_parser_l4app_ctl_s
{
    uint8 ptp_en;                        /**< [GB.GG.D2.TM] ptp enable*/
    uint8 ntp_en;                        /**< [GB.GG.D2.TM] ntp enable*/
    uint8 bfd_en;                        /**< [GB.GG.D2.TM] bfd enable*/
    uint8 vxlan_en;                      /**< [GG.D2.TM] vxlan enable */
};
typedef struct ctc_parser_l4app_ctl_s ctc_parser_l4app_ctl_t;

/**
 @brief  layer4 app data entry fields
*/
struct ctc_parser_l4_app_data_entry_s
{
    uint16 l4_dst_port_mask;      /**< [GB] dest port mask*/
    uint16 l4_dst_port_value;     /**< [GB] dest port value*/
    uint8 is_udp_mask;            /**< [GB] is udp mask*/
    uint8 is_udp_value;           /**< [GB] is udp value*/
    uint8 is_tcp_mask;            /**< [GB] is tcp mask*/
    uint8 is_tcp_value;           /**< [GB] is tcp value*/
};
typedef struct ctc_parser_l4_app_data_entry_s ctc_parser_l4_app_data_entry_t;

/**
 @brief  calculate and generate hash key method type
*/
enum ctc_parser_gen_hash_type_e
{
    CTC_PARSER_GEN_HASH_TYPE_XOR, /**< [GB.GG.D2.TM] xor hash type */
    CTC_PARSER_GEN_HASH_TYPE_CRC, /**< [GB.GG.D2.TM] crc hash type */
    CTC_PARSER_GEN_HASH_TYPE_NUM
};
typedef enum ctc_parser_gen_hash_type_e ctc_parser_gen_hash_type_t;

/**
 @brief  layer2 hash flags
*/
enum ctc_parser_l2_hash_flags_e
{
    CTC_PARSER_L2_HASH_FLAGS_COS           = 0x00000001,      /**< [GB.GG.D2.TM] hash key includes cos, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_MACSA         = 0x00000002,      /**< [GB.GG.D2.TM] hash key includes macsa, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_MACDA         = 0x00000004,      /**< [GB.GG.D2.TM] hash key includes macda, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_CTAG_VID      = 0x00000008,      /**< [GG.D2.TM] hash key includes cvlan vlan id, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_CTAG_CFI      = 0x00000010,      /**< [GG.D2.TM] hash key includes cvlan vlan cfi, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_CTAG_COS      = 0x00000020,      /**< [GG.D2.TM] hash key includes cvlan vlan cos, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_STAG_VID      = 0x00000040,      /**< [GG.D2.TM] hash key includes stag vlan id, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_STAG_CFI      = 0x00000080,      /**< [GG.D2.TM] hash key includes stag vlan cfi, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_STAG_COS      = 0x00000100,      /**< [GG.D2.TM] hash key includes stag vlan cos, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE     = 0x00000200,      /**< [GG.D2.TM] hash key includes ethertype, support inner packet */
    CTC_PARSER_L2_HASH_FLAGS_PORT          = 0x00000400       /**< [GG.D2.TM] hash key includes gport, support inner packet */
};
typedef enum  ctc_parser_l2_hash_flags_e  ctc_parser_l2_hash_flags_t;

/**
 @brief  ip hash flags
*/
enum ctc_parser_ip_ecmp_hash_flags_e
{
    CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL        = 0x00000001,    /**< [GB.GG.D2.TM] use protocol num to compute hash flags, support inner packet*/
    CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP            = 0x00000002,    /**< [GB.GG.D2.TM] use dscp to compute hash flags, support inner packet*/
    CTC_PARSER_IP_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL = 0x00000004,    /**< [GB.GG.D2.TM] use ipv6 flow label to compute hash flags, support inner packet*/
    CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA            = 0x00000008,    /**< [GB.GG.D2.TM] use ipsa to compute hash flags, support inner packet*/
    CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA            = 0x00000010     /**< [GB.GG.D2.TM] use ipda to compute hash flags, support inner packet*/
};
typedef enum ctc_parser_ip_ecmp_hash_flags_e ctc_parser_ip_ecmp_hash_flags_t;

/**
 @brief  ip hash flags
*/
enum ctc_parser_ip_hash_flags_e
{
    CTC_PARSER_IP_HASH_FLAGS_PROTOCOL        = 0x00000001,    /**< [GG.D2.TM] use protocol num to compute hash flags, support inner packet*/
    CTC_PARSER_IP_HASH_FLAGS_DSCP            = 0x00000002,    /**< [GG.D2.TM] use dscp to compute hash flags, support inner packet*/
    CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL = 0x00000004,    /**< [GG.D2.TM] use ipv6 flow label to compute hash flags, support inner packet*/
    CTC_PARSER_IP_HASH_FLAGS_IPSA            = 0x00000008,    /**< [GG.D2.TM] use ipsa to compute hash flags, support inner packet*/
    CTC_PARSER_IP_HASH_FLAGS_IPDA            = 0x00000010,    /**< [GG.D2.TM] use ipda to compute hash flags, support inner packet*/
    CTC_PARSER_IP_HASH_FLAGS_ECN             = 0x00000020     /**< [GG.D2.TM] use ecn to compute hash flags, support inner packet*/
};
typedef enum ctc_parser_ip_hash_flags_e ctc_parser_ip_hash_flags_t;

/**
 @brief  layer4 hash flags
*/
enum ctc_parser_l4_ecmp_hash_flags_e
{
    CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT            = 0x00000001,     /**< [GB.GG.D2.TM] use source port to compute hash flags, support inner packet*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT            = 0x00000002,     /**< [GB.GG.D2.TM] use dest port to compute hash flags, support inner packet*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_GRE_KEY             = 0x00000004,     /**< [GB.GG.D2.TM] use dest port to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TYPE             = 0x00000008,     /**< [GG.D2.TM] use layer4 type to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_USER_TYPE        = 0x00000010,     /**< [GG.D2.TM] use layer4 user type to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT   = 0x00000020,     /**< [GG.D2.TM] use vxlan layer4 source port to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_FLAG         = 0x00000040,     /**< [GG.D2.TM] use layer4 tcp flag to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_ECN          = 0x00000080,     /**< [GG.D2.TM] use layer4 ecn flag to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_VSID       = 0x00000100,     /**< [GG.D2.TM] use layer4 nvgre vsid to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_FLOW_ID    = 0x00000200,     /**< [GG.D2.TM] use layer4 nvgre flow id to compute hash flags*/
    CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_VXLAN_VNI        = 0x00000400      /**< [GG.D2.TM] use layer4 vxlan vni to compute hash flags*/
};
typedef enum ctc_parser_l4_ecmp_hash_flags_e ctc_parser_l4_ecmp_hash_flags_t;

/**
 @brief  layer4 hash flags
*/
enum ctc_parser_l4_hash_flags_e
{
    CTC_PARSER_L4_HASH_FLAGS_SRC_PORT            = 0x00000001,     /**< [GG.D2.TM] use source port to compute hash flags, support inner packet */
    CTC_PARSER_L4_HASH_FLAGS_DST_PORT            = 0x00000002,     /**< [GG.D2.TM] use dest port to compute hash flags, support inner packet */
    CTC_PARSER_L4_HASH_FLAGS_GRE_KEY             = 0x00000004,     /**< [GG.D2.TM] use dest port to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_TYPE             = 0x00000008,     /**< [GG.D2.TM] use layer4 type to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_USER_TYPE        = 0x00000010,     /**< [GG.D2.TM] use layer4 user type to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT   = 0x00000020,     /**< [GG.D2.TM] use vxlan layer4 source port to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_TCP_FLAG         = 0x00000040,     /**< [GG.D2.TM] use layer4 tcp flag to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_TCP_ECN          = 0x00000080,     /**< [GG.D2.TM] use layer4 ecn flag to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID       = 0x00000100,     /**< [GG.D2.TM] use layer4 nvgre vsid to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_FLOW_ID    = 0x00000200,     /**< [GG.D2.TM] use layer4 nvgre flow id to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI        = 0x00000400,     /**< [GG.D2.TM] use layer4 vxlan vni to compute hash flags*/
    CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_DST_PORT   = 0x00000800      /**< [TM] use vxlan layer4 dest port to compute hash flags*/
};
typedef enum ctc_parser_l4_hash_flags_e ctc_parser_l4_hash_flags_t;

/**
 @brief  pbb hash flags
*/
enum ctc_parser_pbb_hash_flags_e
{
    CTC_PARSER_PBB_HASH_FLAGS_CMAC_DA            = 0x00000001,     /**< [GG.D2] ecmp/linkagg/flow hash include CMAC DA*/
    CTC_PARSER_PBB_HASH_FLAGS_CMAC_SA            = 0x00000002,     /**< [GG.D2] ecmp/linkagg/flow hash include CMAC SA*/
    CTC_PARSER_PBB_HASH_FLAGS_ISID               = 0x00000004,     /**< [GG.D2] ecmp/linkagg/flow hash include ISID*/
    CTC_PARSER_PBB_HASH_FLAGS_STAG_VID           = 0x00000008,     /**< [GG.D2] ecmp/linkagg/flow hash includestag vlan id*/
    CTC_PARSER_PBB_HASH_FLAGS_STAG_COS           = 0x00000010,     /**< [GG.D2] ecmp/linkagg/flow hash includestag vlan cos*/
    CTC_PARSER_PBB_HASH_FLAGS_STAG_CFI           = 0x00000020,     /**< [GG.D2] ecmp/linkagg/flow hash includestag vlan cfi*/
    CTC_PARSER_PBB_HASH_FLAGS_CTAG_VID           = 0x00000040,     /**< [GG.D2] ecmp/linkagg/flow hash includectag vlan id*/
    CTC_PARSER_PBB_HASH_FLAGS_CTAG_COS           = 0x00000080,     /**< [GG.D2] ecmp/linkagg/flow hash includectag vlan cos*/
    CTC_PARSER_PBB_HASH_FLAGS_CTAG_CFI           = 0x00000100      /**< [GG.D2] ecmp/linkagg/flow hash includectag vlan cfi*/
};
typedef enum ctc_parser_pbb_hash_flags_e ctc_parser_pbb_hash_flags_t;

/**
 @brief  fcoe hash flags
*/
enum ctc_parser_fcoe_ecmp_hash_flags_e
{
    CTC_PARSER_FCOE_ECMP_HASH_FLAGS_SID    = 0x00000001,     /**< [GB.GG.D2.TM] use sid to compute ecmp hash flags*/
    CTC_PARSER_FCOE_ECMP_HASH_FLAGS_DID    = 0x00000002      /**< [GB.GG.D2.TM] use did to compute ecmp hash flags*/
};
typedef enum ctc_parser_fcoe_ecmp_hash_flags_e  ctc_parser_fcoe_ecmp_hash_flags_t;

/**
 @brief  fcoe hash flags
*/
enum ctc_parser_fcoe_hash_flags_e
{
    CTC_PARSER_FCOE_HASH_FLAGS_SID    = 0x00000001,     /**< [GG.D2.TM] use sid to compute hash flags*/
    CTC_PARSER_FCOE_HASH_FLAGS_DID    = 0x00000002      /**< [GG.D2.TM] use did to compute hash flags*/
};
typedef enum ctc_parser_fcoe_hash_flags_e  ctc_parser_fcoe_hash_flags_t;

/**
 @brief  trill hash flags
*/
enum ctc_parser_trill_ecmp_hash_flags_e
{
    CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INNER_VLAN_ID       = 0x00000001,     /**< [GB.GG.D2.TM] use inner vlan id to compute hash flags*/
    CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INGRESS_NICKNAME    = 0x00000002,     /**< [GB.GG.D2.TM] use ingress nickname to compute hash flags*/
    CTC_PARSER_TRILL_ECMP_HASH_FLAGS_EGRESS_NICKNAME     = 0x00000004      /**< [GB.GG.D2.TM] use egress nickname to compute hash flags*/
};
typedef enum ctc_parser_trill_ecmp_hash_flags_e  ctc_parser_trill_ecmp_hash_flags_t;

/**
 @brief  trill hash flags
*/
enum ctc_parser_trill_hash_flags_e
{
    CTC_PARSER_TRILL_HASH_FLAGS_INNER_VLAN_ID       = 0x00000001,     /**< [GG.D2.TM] use inner vlan id to compute hash flags*/
    CTC_PARSER_TRILL_HASH_FLAGS_INGRESS_NICKNAME    = 0x00000002,     /**< [GG.D2.TM] use ingress nickname to compute hash flags*/
    CTC_PARSER_TRILL_HASH_FLAGS_EGRESS_NICKNAME     = 0x00000004      /**< [GG.D2.TM] use egress nickname to compute hash flags*/
};
typedef enum ctc_parser_trill_hash_flags_e  ctc_parser_trill_hash_flags_t;

/**
 @brief  mpls hash flags
*/
enum ctc_parser_mpls_ecmp_hash_flag_e
{
    CTC_PARSER_MPLS_ECMP_HASH_FLAGS_PROTOCOL        = 0x00000001,     /**< [GB.GG.D2.TM] use protocol to compute hash flags*/
    CTC_PARSER_MPLS_ECMP_HASH_FLAGS_DSCP            = 0x00000002,     /**< [GB.GG.D2.TM] use dscp to compute hash flags*/
    CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL = 0x00000004,     /**< [GB.GG.D2.TM] use ipv6 flow label to compute hash flags*/
    CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPSA            = 0x00000008,     /**< [GB.GG.D2.TM] use ipsa to compute hash flags*/
    CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPDA            = 0x00000010      /**< [GB.GG.D2.TM] use ipda to compute ecmp hash flags*/
};
typedef enum ctc_parser_mpls_ecmp_hash_flag_e  ctc_parser_mpls_ecmp_hash_flag_t;

/**
 @brief  mpls hash flags
*/
enum ctc_parser_mpls_hash_flag_e
{
    CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL        = 0x00000001,         /**<[GG.D2.TM] use protocol to compute hash flags*/
    CTC_PARSER_MPLS_HASH_FLAGS_DSCP            = 0x00000002,         /**<[GG.D2.TM] use dscp to compute hash flags*/
    CTC_PARSER_MPLS_HASH_FLAGS_IPV6_FLOW_LABEL = 0x00000004,         /**<[GG.D2.TM] use ipv6 flow label to compute hash flags*/
    CTC_PARSER_MPLS_HASH_FLAGS_IPSA            = 0x00000008,         /**<[GG.D2.TM] use ipsa to compute hash flags*/
    CTC_PARSER_MPLS_HASH_FLAGS_IPDA            = 0x00000010          /**<[GG.D2.TM] use ipda to compute hash flags*/
};
typedef enum ctc_parser_mpls_hash_flag_e  ctc_parser_mpls_hash_flag_t;

/**
 @brief  l2 hash flags
*/
enum ctc_parser_l2_ecmp_hash_flag_e
{
    CTC_PARSER_L2_ECMP_HASH_FLAGS_COS   = 0x00000001,     /**<[GB] use cos   to compute ecmp hash */
    CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA = 0x00000002,     /**<[GB] use macsa to compute ecmp hash */
    CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA = 0x00000004,      /**<[GB] use macda to compute ecmp hash */
    /* SYSTEM MODIFIED by yaom for Openflow 20160304 */
    CTC_PARSER_L2_ECMP_HASH_FLAGS_ETHERTYPE  = 0x00000200,      /**< [GG] hash key includes ethertype, support inner packet */
};
typedef enum ctc_parser_l2_ecmp_hash_flag_e  ctc_parser_l2_ecmp_hash_flag_t;

/**
 @brief  common hash flags
*/
enum ctc_parser_common_hash_flags_e
{
    CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO        = 0x00000001,         /**<[D2.TM] use deviceinfo to compute hash flags*/
};
typedef enum ctc_parser_common_hash_flags_e  ctc_parser_common_hash_flags_t;

/**
 @brief  select to config hash type flags
*/
enum ctc_parser_ecmp_hash_type_e
{
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP     = 0x00000001,     /**< [GB.GG.D2.TM] layer3 ip hash ctl flags*/
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4     = 0x00000002,     /**< [GB.GG.D2.TM] layer4 hash ctl flags*/
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS   = 0x00000004,     /**< [GB.GG.D2.TM] layer3 mpls hash ctl flags*/
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_FCOE   = 0x00000008,     /**< [GB.GG.D2.TM] layer3 fcoe hash ctl flags*/
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_TRILL  = 0x00000010,     /**< [GB.GG.D2.TM] layer3 trill hash ctl flags*/
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2     = 0x00000020,     /**< [GB.GG.D2.TM] layer2 hash ctl flags*/

    /* following is hash type control */
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER  = 0x01000000,     /**< [GG.D2.TM] if set, config inner hash key */
    CTC_PARSER_ECMP_HASH_TYPE_FLAGS_DLB_EFD= 0x02000000      /**< [GG.D2.TM] if set, config dlb&efd hash key */
};
typedef enum ctc_parser_ecmp_hash_type_e ctc_parser_ecmp_hash_type_t;

enum ctc_parser_hash_type_e
{
    CTC_PARSER_HASH_TYPE_FLAGS_IP    = 0x00000001,           /**< [GB.GG.D2.TM] layer3 ip hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_L4    = 0x00000002,           /**< [GB.GG.D2.TM] layer4 hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_MPLS  = 0x00000004,           /**< [GB.GG.D2.TM] layer3 mpls hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_FCOE  = 0x00000008,           /**< [GB.GG.D2.TM] layer3 fcoe hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_TRILL = 0x00000010,           /**< [GB.GG.D2.TM] layer3 trill hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_L2    = 0x00000020,           /**< [GG.D2.TM] layer2 hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_PBB   = 0x00000040,           /**< [GG.D2] pbb hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_COMMON   = 0x00000080,        /**< [D2.TM] common hash ctl flags*/
    CTC_PARSER_HASH_TYPE_FLAGS_UDF   = 0x00000100,           /**< [D2.TM] udf hash ctl flags,per udf entry control*/

    /* following is hash type control */
    CTC_PARSER_HASH_TYPE_FLAGS_INNER        = 0x01000000,    /**< [GG.D2.TM] if set, config inner hash key */
    CTC_PARSER_HASH_TYPE_FLAGS_DLB_EFD      = 0x02000000,    /**< [GG.D2.TM] if set, config dlb&efd hash key */
    CTC_PARSER_HASH_TYPE_FLAGS_EFD    = 0x04000000,           /**< [TM] if set, config efd hash key */
    CTC_PARSER_HASH_TYPE_FLAGS_DLB    = 0x08000000           /**< [TM] if set, config dlb hash key */
};
typedef enum ctc_parser_hash_type_e ctc_parser_hash_type_t;

enum ctc_parser_tunnel_type_e
{
    CTC_PARSER_TUNNEL_TYPE_UDP,             /**< [GG.D2.TM] udp tunnel */
    CTC_PARSER_TUNNEL_TYPE_MPLS_VPN,        /**< [GG.D2.TM] mpls vpn tunnel*/
    CTC_PARSER_TUNNEL_TYPE_MPLS,            /**< [GG.D2.TM] mpls tunnel*/
    CTC_PARSER_TUNNEL_TYPE_IP,              /**< [GG.D2.TM] ipv4 or ipv6 tunnel*/
    CTC_PARSER_TUNNEL_TYPE_TRILL,           /**< [GG.D2.TM] trill tunnel*/
    CTC_PARSER_TUNNEL_TYPE_PBB,             /**< [GG.D2] pbb tunnel*/
    CTC_PARSER_TUNNEL_TYPE_GRE,             /**< [GG.D2.TM] gre tunnel*/
    CTC_PARSER_TUNNEL_TYPE_NVGRE,           /**< [GG.D2.TM] nvgre tunnels*/
    CTC_PARSER_TUNNEL_TYPE_VXLAN,           /**< [GG.D2.TM] vxlan tunnel*/

    CTC_PARSER_TUNNEL_TYPE_MAX
};
typedef enum ctc_parser_tunnel_type_e ctc_parser_tunnel_type_t;

enum ctc_parser_tunnel_hash_mode_e
{
    CTC_PARSER_TUNNEL_HASH_MODE_OUTER,      /**< [GG.D2.TM] use outer hash key*/
    CTC_PARSER_TUNNEL_HASH_MODE_MERGE,      /**< [GG.D2.TM] merge inner and outer hash key*/
    CTC_PARSER_TUNNEL_HASH_MODE_INNER,      /**< [GG.D2.TM] use inner hash key*/
};
typedef enum ctc_parser_tunnel_hash_mode_e ctc_parser_tunnel_hash_mode_t;

/**
 @brief  ecmp hash ctl fields
*/
struct ctc_parser_hash_ctl_s
{
    uint32  hash_type_bitmap;             /**< [GB.GG.D2.TM] hash type flag defined in ctc_parser_hash_type_t or compatible ctc_parser_ecmp_hash_type_t for ecmp*/
    uint32  udf_id;                       /**< [D2.TM] UDF ID, is the key of udf_bitmap */
    uint32  udf_bitmap;                   /**< [D2.TM] bitmap control UDF field hash key£¬1 bit control 8 bit of udf field,total udf field is 128*/

    uint32  ip_flag;                      /**< [GB.GG.D2.TM] layer3 ip hash ctl flag defined in ctc_parser_ip_hash_flags_t or compatible ctc_parser_ip_ecmp_hash_flags_t for gb ecmp*/
    uint32  l4_flag;                      /**< [GB.GG.D2.TM] layer4 hash ctl flag defined in ctc_parser_l4_hash_flags_t or compatible ctc_parser_l4_ecmp_hash_flags_t for gb ecmp*/
    uint32  mpls_flag;                    /**< [GB.GG.D2.TM] layer3 mpls hash ctl flag defined in ctc_parser_mpls_hash_flag_t or compatible ctc_parser_mpls_ecmp_hash_flag_t for gb ecmp*/
    uint32  fcoe_flag;                    /**< [GB.GG.D2.TM] layer3 fcoe hash ctl flag defined in ctc_parser_fcoe_hash_flags_t or compatible ctc_parser_fcoe_ecmp_hash_flags_t for gb ecmp*/
    uint32  trill_flag;                   /**< [GB.GG.D2.TM] trill hash ctl flag defined in ctc_parser_trill_hash_flags_t or compatible ctc_parser_trill_ecmp_hash_flags_t for gb ecmp*/
    uint32  l2_flag;                      /**< [GB.GG.D2.TM] layer2 hash ctl flag defined in ctc_parser_l2_hash_flags_t or compatible ctc_parser_l2_ecmp_hash_flag_t for gb emcp*/
    uint32  pbb_flag;                     /**< [GG.D2] pbb hash flag defined in ctc_parser_pbb_hash_flags_t*/
    uint32  common_flag;                  /**< [D2.TM] common hash flag defined in ctc_parser_common_hash_flags_t*/
};
typedef struct ctc_parser_hash_ctl_s ctc_parser_hash_ctl_t;

typedef ctc_parser_hash_ctl_t ctc_parser_ecmp_hash_ctl_t;
typedef ctc_parser_hash_ctl_t ctc_parser_linkagg_hash_ctl_t;
typedef ctc_parser_hash_ctl_t ctc_parser_flow_hash_ctl_t;
typedef ctc_parser_hash_ctl_t ctc_parser_efd_hash_ctl_t;

/**
 @brief  parser global config fields
*/
struct ctc_parser_global_cfg_s
{
    uint8   ecmp_hash_type;          /**< [GB.GG.D2.TM] ecmp hash algorithm use xor or crc, if set use crc ,else use xor*/
    uint8   linkagg_hash_type;       /**< [GB.GG.D2.TM] linkagg hash algorithm use xor or crc, if set use crc ,else use xor*/
    uint8   small_frag_offset;       /**< [GB.GG.D2.TM] ipv4 small fragment offset, 0~3, means 0,8,16,24 bytes of small fragment length*/
    uint8   symmetric_hash_en;       /**< [D2.TM] if set, src/dest ip, src/dest l4 port or src/dest fcid is normalized before hashing*/
    uint8   ecmp_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_MAX];      /**< [GG.D2.TM] refer to ctc_parser_tunnel_hash_mode_t*/
    uint8   linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_MAX];   /**< [GG.D2.TM] refer to ctc_parser_tunnel_hash_mode_t*/
    uint8   dlb_efd_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_MAX];   /**< [GG.D2.TM] refer to ctc_parser_tunnel_hash_mode_t*/
};
typedef struct ctc_parser_global_cfg_s ctc_parser_global_cfg_t;

/**@} end of @defgroup  parser_l4 PARSER_L4*/

/**
 @defgroup parser_udf
 @{
*/
#define CTC_PARSER_UDF_FIELD_NUM 4

enum ctc_parser_udf_type_e
{
    CTC_PARSER_UDF_TYPE_L3_UDF, /**< [GG] UDF based L3 filed*/
    CTC_PARSER_UDF_TYPE_L4_UDF, /**< [GG] UDF based L4 filed*/
    CTC_PARSER_UDF_TYPE_NUM
};
typedef enum ctc_parser_udf_type_e ctc_parser_udf_type_t;

/**
@brief layer2/layer3/layer4 based DATA packet qualifier fields
*/
struct ctc_parser_udf_s
{
    uint8  type;                                        /**< [GG] UDF based L3 or L4 filed, CTC_PARSER_UDF_TYPE_XX*/

    /*L3 UDF key*/
    uint16 ether_type;                                  /**< [GG] Ether type*/

    /*L4 UDF key*/
    uint8  ip_version;                                  /**< [GG] IP version, CTC_IP_VER_X, must set for L4 UDF*/
    uint8 l3_header_protocol;                          /**< [GG] L3 header protocol, must set for L4 UDF*/

    uint8  is_l4_src_port;                              /**< [GG] Select L4 source port as L4 UDF Key*/
    uint8  is_l4_dst_port;                              /**< [GG] Select L4 dest port as L4 UDF Key*/
    uint16 l4_src_port;                                 /**< [GG] Optional field, L4 source port*/
    uint16 l4_dst_port;                                 /**< [GG] Optional field, L4 dest port*/

    /*
     * L3/L4 UDF offset:
     * +-------+-------+------+-----------+-----------+--------+---------------------+
     * | MacDa | MacSa | Vlan | EtherType |   Layer3  | Layer4 |      Payload        |
     * +-------+-------+------+-----------+-----------+--------+---------------------+
     *                                    ^                                          |
     *                                    |              L3 UDF Offset               |
     *                                    |<---------------------------------------->|
     *
     * +-------+-------+------+-----------+-----------+--------------------+---------+
     * | MacDa | MacSa | Vlan | EtherType | IP Header | Layer4(TCP or UDP) | Payload |
     * +-------+-------+------+-----------+-----------+--------------------+---------+
     *                                                ^                              |
     *                                                |        L4 UDF Offset         |
     *                                                |<---------------------------->|
     */
    uint8  udf_offset[CTC_PARSER_UDF_FIELD_NUM];        /**< [GG] The offset of each UDF field*/
    uint8  udf_num;                                     /**< [GG] The number of UDF field*/
};
typedef struct ctc_parser_udf_s ctc_parser_udf_t;


/**@} end of @defgroup  parser_udf*/
/**@} end of @defgroup  parser PARSER */

#ifdef __cplusplus
}
#endif

#endif

