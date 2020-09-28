/**
 @file ctc_pdu.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-30

 @version v2.0

 This file contains all pdu related data structure, enum, macro and proto.
*/

#ifndef _CTC_PDU_H_
#define _CTC_PDU_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 @defgroup pdu PDU
 @{
*/

/**
 @defgroup l2pdu L2PDU
 @{
*/

/**
 @brief pdu l2 pdu action based flags
*/
enum ctc_pdu_l2pdu_type_e
{
    CTC_PDU_L2PDU_TYPE_BPDU,                /**< [GB.GG.D2.TM] Macda = 01:80:c2:00:00:00, static classify by mac.*/
    CTC_PDU_L2PDU_TYPE_SLOW_PROTO,          /**< [GB.GG.D2.TM] Ether type = 0x8809, static classify by ether type.*/
    CTC_PDU_L2PDU_TYPE_EAPOL,               /**< [GB.GG.D2.TM] Ether type = 0x888E, static classify by ether type.*/
    CTC_PDU_L2PDU_TYPE_LLDP,                /**< [GB.GG.D2.TM] Ether type = 0x88CC, static classify by ether type.*/
    CTC_PDU_L2PDU_TYPE_ISIS,                /**< [GB.GG.D2.TM] Ether type = 0x22F4, static classify by ether type. Trill ISIS.*/
    CTC_PDU_L2PDU_TYPE_EFM_OAM,             /**< [GG.D2.TM] Ether type = 0x8809, Macda = 0180.C200.0002, static classify by ether type and mac .*/

    CTC_PDU_L2PDU_TYPE_L3TYPE,              /**< [GB.GG.D2.TM] Layer2 pdu action based on layer3 type.*/
    CTC_PDU_L2PDU_TYPE_L2HDR_PROTO,         /**< [GB.GG.D2.TM] Layer2 pdu action based on ether type.*/
    CTC_PDU_L2PDU_TYPE_MACDA,               /**< [GB.GG.D2.TM] Layer2 pdu action based on macda.*/
    CTC_PDU_L2PDU_TYPE_MACDA_LOW24,         /**< [GB.GG.D2.TM] Layer2 pdu action based on macda low24 bit, the most 24bit is 01:80:C2.*/
    CTC_PDU_L2PDU_TYPE_FIP,                 /**< [GG.D2.TM] FIP(FCoE Initialization Protocol),Ether type = 0x8914 */

    MAX_CTC_PDU_L2PDU_TYPE
};
typedef enum ctc_pdu_l2pdu_type_e ctc_pdu_l2pdu_type_t;

/**
 @brief pdu layer2 pdu entry fileds
*/
union ctc_pdu_l2pdu_key_u
{
    /*CTC_PDU_L2PDU_TYPE_MACDA and CTC_PDU_L2PDU_TYPE_MACDA_LOW24*/
    struct
    {
        mac_addr_t mac;                   /**< [GB.GG.D2.TM] Macda*/
        mac_addr_t mac_mask;              /**< [GB.GG.D2.TM] Macda mask*/
    } l2pdu_by_mac;
    uint16 l2hdr_proto;                   /**< [GB.GG.D2.TM] Layer2 header protocol: CTC_PDU_L2PDU_TYPE_L2HDR_PROTO.*/
};
typedef union ctc_pdu_l2pdu_key_u ctc_pdu_l2pdu_key_t;

/**
 @brief pdu layer2 pdu global action types
*/
struct ctc_pdu_global_l2pdu_action_s
{
    uint8 entry_valid;          /**< [GB.GG.D2.TM] Control the added protocol entry is valid or not.*/
    uint8 action_index;         /**< [GB.GG.D2.TM] Entry for per-port action index,
                                                the index range is 0-14 in humber, 0-31 in gb.*/
    uint8 copy_to_cpu;          /**< [GB] Bpdu exception operation.*/
};
typedef struct ctc_pdu_global_l2pdu_action_s ctc_pdu_global_l2pdu_action_t;

/**
 @brief  layer2 pdu action types
*/
enum ctc_pdu_port_l2pdu_action_e
{
    CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU,         /**< [GB.GG.D2.TM] Layer2 pdu action type: redirect to cpu.*/
    CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU,             /**< [GB.GG.D2.TM] Layer2 pdu action type: copy to cpu.*/
    CTC_PDU_L2PDU_ACTION_TYPE_FWD,                     /**< [GB.GG.D2.TM] Layer2 pdu action type: normal forwarding.*/
    CTC_PDU_L2PDU_ACTION_TYPE_DISCARD,                 /**< [GB.GG.D2.TM] Layer2 pdu action type: discard the pdu.*/
    MAX_CTC_PDU_L2PDU_ACTION_TYPE
};
typedef enum ctc_pdu_port_l2pdu_action_e ctc_pdu_port_l2pdu_action_t;

/**@} end of @defgroup  l2pdu L2PDU */

/**
 @defgroup l3pdu L3PDU
 @{
*/

/**
 @brief  layer3 pdu action based flags
*/
enum ctc_pdu_l3pdu_type_e
{
    CTC_PDU_L3PDU_TYPE_OSPF,              /**< [GB.GG.D2.TM] Regardless ipv4 or ipv6, layer3 protocol = 89, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_PIM,               /**< [GB.GG.D2.TM] Regardless ipv4 or ipv6, layer3 protocol = 103, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_VRRP,              /**< [GB.GG.D2.TM] Regardless ipv4 or ipv6, layer3 protocol = 112, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_RSVP,              /**< [GB.GG.D2.TM] Layer3 protocol = 46, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_RIP,               /**< [GB.GG.D2.TM] UDP, for GB DestPort = 520 or DestPort = 521, for GG DestPort = 520, static classify by udp port.*/
    CTC_PDU_L3PDU_TYPE_BGP,               /**< [GB.GG.D2.TM] Regardless of ipv4 or ipv6, TCP, DestPort = 179, other bgp destport pdu classify
                                                       by CTC_PDU_L3PDU_TYPE_LAYER4_PORT or forwarding to cpu.*/
    CTC_PDU_L3PDU_TYPE_MSDP,              /**< [GB.GG.D2.TM] TCP, DestPort = 639, static classify by udp port.*/
    CTC_PDU_L3PDU_TYPE_LDP,               /**< [GB.GG.D2.TM] TCP or UDP, DestPort = 646, other ldp destport pdu classify
                                                       by CTC_PDU_L3PDU_TYPE_LAYER4_PORT or forwarding to cpu.*/
    CTC_PDU_L3PDU_TYPE_IPV4OSPF,          /**< [GG.D2.TM] IPv4, layer3 protocol = 89, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV6OSPF,          /**< [GG.D2.TM] IPv6, layer3 protocol = 89, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV4PIM,           /**< [GG.D2.TM] IPv4, layer3 protocol = 103, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV6PIM,           /**< [GG.D2.TM] IPv6, layer3 protocol = 103, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV4VRRP,          /**< [GG.D2.TM] IPv4, layer3 protocol = 112, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV6VRRP,          /**< [GG.D2.TM] IPv6, layer3 protocol = 112, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV4BGP,           /**< [GG.D2.TM] IPv4, TCP, DestPort = 179, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IPV6BGP,           /**< [GG.D2.TM] IPv6, TCP, DestPort = 179, static classify by protocol.*/

    CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS,       /**< [GG.D2.TM] IPv6, layer3 protocol = 58, layer4 source port = 133, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA,       /**< [GG.D2.TM] IPv6, layer3 protocol = 58, layer4 source port = 134, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS,       /**< [GG.D2.TM] IPv6, layer3 protocol = 58, layer4 source port = 135, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA,       /**< [GG.D2.TM] IPv6, layer3 protocol = 58, layer4 source port = 136, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT, /**< [GG.D2.TM] IPv6, layer3 protocol = 58, layer4 source port = 137, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_RIPNG,             /**< [GG.D2.TM] UDP, DestPort = 521, static classify by udp port.*/
    CTC_PDU_L3PDU_TYPE_ISIS,              /**< [TM] Macda = 01:80:C2:00:00:14 or 01:80:C2:00:00:15£¬static classify by mac.*/
    CTC_PDU_L3PDU_TYPE_IGMP_QUERY,        /**< [TM] IPv4, layer4 protocol = 2 , IgmpType = 0x11, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE, /**< [TM] IPv4, layer4 protocol = 2 , IgmpType = 0x12 or 0x16 or 0x17 or 0x22, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN,      /**< [TM] IPv4, layer4 protocol = 2 , IgmpType != 0x11 & 0x12 & 0x16 & 0x22 & 0x17, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_MLD_QUERY,         /**< [TM] IPv6, layer4 protocol = 58 ,  MldType = 130, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE,   /**< [TM] IPv6, layer4 protocol = 58 ,  MldType = 131 or 132 or 143, static classify by protocol.*/
    CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN,       /**< [TM] IPv6, layer4 protocol = 58 ,  MldType != 130 & 131 & 132 & 143, static classify by protocol.*/

    CTC_PDU_L3PDU_TYPE_L3HDR_PROTO,       /**< [GB.GG.D2.TM] Layer3 pdu action based on layer3 header protocol.*/
    CTC_PDU_L3PDU_TYPE_LAYER3_IPDA,       /**< [GG.D2.TM] Layer3 pdu action based on layer3 ip address.*/
    CTC_PDU_L3PDU_TYPE_LAYER4_PORT,       /**< [GB.GG.D2.TM] Layer3 pdu action based on layer4 destport.*/
    CTC_PDU_L3PDU_TYPE_LAYER4_TYPE,       /**< [GB.GG.D2.TM] Layer3 pdu action based on layer4 type.*/

    CTC_PDU_L3PDU_TYPE_ARP,               /**< [GG.D2.TM] Ether type = 0x0806*/
    CTC_PDU_L3PDU_TYPE_DHCP,              /**< [GG.D2.TM] IPv4 UDP l4DestPort = 67 or 68, IPv6 UDP l4DestPort = 546 or 547.*/
    CTC_PDU_L3PDU_TYPE_CXP ,              /**< [D2.TM] CXP (CID Exchange Protocol) action index*/
    MAX_CTC_PDU_L3PDU_TYPE
};
typedef enum ctc_pdu_l3pdu_type_e ctc_pdu_l3pdu_type_t;

/**
 @brief  layer3 pdu entry fields
*/
union ctc_pdu_l3pdu_key_u
{
    uint8 l3hdr_proto;       /**< [GB.GG.D2.TM] Layer3 header protocol*/

    struct
    {
        uint16 dest_port;    /**< [GB.GG.D2.TM] Layer4 dest port*/
        uint8 is_tcp;        /**< [GB.GG.D2.TM] Field dest_port is tcp value.*/
        uint8 is_udp;        /**< [GB.GG.D2.TM] Field dest_port is udp value.*/
    }l3pdu_by_port;

    struct
    {
        uint16 ipda;         /**< [GG.D2.TM] for ipv4 ipda value apply to IPv4DaAddr(20,31),
                                                IPv4DaAddr(0,19) value is fixed to 0xE0000.
                                       for ipv6 ipda(4,7)  value apply to IPv6DaAddr(12,15),
                                                ipda(8,15) value apply to IPv6DaAddr(120,127),
                                                IPv6DaAddr(0,7) value is fixed to 0xFF, other bits are 0.*/
        uint8  is_ipv4;      /**< [GG.D2.TM] Field ipda is ipv4 value.*/
        uint8  is_ipv6;      /**< [GG.D2.TM] Field ipda is ipv6 value.*/
    }l3pdu_by_ipda;
};
typedef union ctc_pdu_l3pdu_key_u ctc_pdu_l3pdu_key_t;

/**
 @brief  layer3 pdu entry fields
*/
struct ctc_pdu_global_l3pdu_action_s
{
    uint8 action_index;      /**< [GB.GG.D2.TM] Entry for per-l3if action index, the index range is 0-12 in hb, 0-63 in gb.*/
    uint8 entry_valid;       /**< [GB.GG.D2.TM] Layer3 protocol action entry is valid or not.*/
};
typedef struct ctc_pdu_global_l3pdu_action_s ctc_pdu_global_l3pdu_action_t;

/**
 @brief  layer2 pdu action types
*/
enum ctc_pdu_l3if_l3pdu_action_e
{
    CTC_PDU_L3PDU_ACTION_TYPE_FWD,           /**< [GG.D2.TM] Layer3 pdu action type: normal forwarding.*/
    CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU,   /**< [GG.D2.TM] Layer3 pdu action type: copy to cpu.*/
    MAX_CTC_PDU_L3PDU_ACTION_TYPE
};
typedef enum ctc_pdu_l3if_l3pdu_action_e ctc_pdu_l3if_l3pdu_action_t;

/**
 @brief  default l2-pdu action index
*/
enum ctc_l2pdu_action_index_e
{
    /**< default assigned,user can changed action index except EFM OAM PDU */
    CTC_L2PDU_ACTION_INDEX_BPDU       = 0,   /**< [GB.GG.D2.TM]  BPDU action index, For GB, 0 is reserve for EFM.*/
    CTC_L2PDU_ACTION_INDEX_SLOW_PROTO = 1,   /**< [GB.GG.D2.TM]  Slow protocol action index.*/
    CTC_L2PDU_ACTION_INDEX_EAPOL      = 2,   /**< [GB.GG.D2.TM]  EAPOL action index*/
    CTC_L2PDU_ACTION_INDEX_LLDP       = 3,   /**< [GB.GG.D2.TM]  LLDP action index*/
    CTC_L2PDU_ACTION_INDEX_ISIS       = 4,   /**< [GB.GG.D2.TM]  ISIS action index*/
    CTC_L2PDU_ACTION_INDEX_EFM_OAM    = 5,   /**< [GG.D2.TM]  Efm oam action index*/
    CTC_L2PDU_ACTION_INDEX_FIP        = 6,   /**< [GG.D2.TM]  FIP(FCoE Initialization Protocol) action index*/

    /*SYSTEM MODIFIED, Modified by System to suport erps */
    CTC_L2PDU_ACTION_INDEX_ERPS       = 7,   /**< [GG]  ERPS */
    CTC_L2PDU_ACTION_INDEX_RSV_BEGIN = 8,    /**< [HB] The begin of reserve action index*/
    CTC_L2PDU_ACTION_INDEX_l2PRO_PROTOCOL_MAC = 9, /**< [HB.GB.GG] L2protocol protocol-mac action index*/
    CTC_L2PDU_ACTION_INDEX_l2PRO_GROUP_MAC = 10,  /**< [HB.GB.GG] L2protocol group-mac action index*/
    CTC_L2PDU_ACTION_INDEX_PAUSE_DMAC = 11,   /** SYSTEM MODIFY BY yejl for pause packet, 2018-12-20 */
    CTC_L2PDU_ACTION_INDEX_RSV_END   = 14,   /**< [HB] The end of reserve action index*/
    CTC_L2PDU_ACTION_INDEX_MACDA     = 63    /**< [GB.GG.D2.TM] MACDA exception to cpu action index*/
};
typedef enum ctc_l2pdu_action_index_e ctc_l2pdu_action_index_t;

/**
 @brief  default l3-pdu action index
*/
enum ctc_l3pdu_action_index_e
{
    CTC_L3PDU_ACTION_INDEX_RIP       = 0,    /**< [GB.GG.D2.TM] RIP action index*/
    CTC_L3PDU_ACTION_INDEX_RIPNG     = 0,    /**< [GG.D2.TM] RIPng action index*/
    CTC_L3PDU_ACTION_INDEX_LDP       = 1,    /**< [GB.GG.D2.TM] LDP action index*/
    CTC_L3PDU_ACTION_INDEX_OSPF      = 2,    /**< [GB.GG.D2.TM] Regardless of IPv4 or IPv6, OSPF action index, default cfg.*/
    CTC_L3PDU_ACTION_INDEX_PIM       = 3,    /**< [GB.GG.D2.TM] Regardless of IPv4 or IPv6, PIM action index, default cfg.*/
    CTC_L3PDU_ACTION_INDEX_BGP       = 4,    /**< [GB.GG.D2.TM] Regardless of IPv4 or IPv6, BGP action index, default cfg.*/
    CTC_L3PDU_ACTION_INDEX_RSVP      = 5,    /**< [GB.GG.D2.TM] RSVP action index*/
    CTC_L3PDU_ACTION_INDEX_ICMPV6    = 6,    /**< [GG.D2.TM] ICMPV6 action index*/
    CTC_L3PDU_ACTION_INDEX_MSDP      = 6,    /**< [GB.GG.D2.TM] MSDP action index*/
    CTC_L3PDU_ACTION_INDEX_IGMP      = 7,    /**< [HB.TM] IGMP action index*/
    CTC_L3PDU_ACTION_INDEX_MLD       = 7,    /**< [TM] MLD action index*/
    CTC_L3PDU_ACTION_INDEX_VRRP      = 8,    /**< [GB.GG.D2.TM] VRRP action index*/
    CTC_L3PDU_ACTION_INDEX_ISIS      = 9,    /**< [TM] ISIS action index*/
    CTC_L3PDU_ACTION_INDEX_ARP_V2    = 13,   /**< [GB] ARP action index*/
    CTC_L3PDU_ACTION_INDEX_DHCP_V2   = 14,   /**< [GB] DHCP action index*/

    CTC_L3PDU_ACTION_INDEX_ARP       = 16,   /**< [GG.D2.TM] ARP action index*/
    CTC_L3PDU_ACTION_INDEX_DHCP      = 17,   /**< [GG.D2.TM] DHCP action index*/
    CTC_L3PDU_ACTION_INDEX_CXP        = 31,  /**< [D2.TM] CXP (CID Exchange Protocol) action index*/
    CTC_L3PDU_ACTION_INDEX_IPDA      = 63    /**< [GG.D2.TM] IPDA exception to cpu action index*/
};
typedef enum ctc_l3pdu_action_index_e ctc_l3pdu_action_index_t;

/**@} end of @defgroup  l3pdu L3PDU*/

/**@} end of @defgroup  pdu PDU */

#ifdef __cplusplus
}
#endif

#endif

