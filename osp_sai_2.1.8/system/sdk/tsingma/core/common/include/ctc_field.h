/**
 @file ctc_field.h

 @date 2015-12-19

 @version v1.1

 The file define api struct used in ACL/SCL/Ipfix.
*/

#ifndef _CTC_FIELD_H_
#define _CTC_FIELD_H_

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/


/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define CTC_UDF_WORD_NUM    4
typedef uint32 ctc_udf_t[CTC_UDF_WORD_NUM];


/** @brief  key field */
enum ctc_field_key_type_e
{
    /*Packet type field*/
    CTC_FIELD_KEY_L2_TYPE,                  /**< [D2.TM] Layer 2 type (ctc_parser_l2_type_t). */
    CTC_FIELD_KEY_L3_TYPE,                  /**< [D2.TM] Layer 3 type (ctc_parser_l3_type_t). */
    CTC_FIELD_KEY_L4_TYPE,                  /**< [D2.TM] Layer 4 type (ctc_parser_l4_type_t). */
    CTC_FIELD_KEY_L4_USER_TYPE,             /**< [D2.TM] Layer 4 Usertype (ctc_parser_l4_usertype_t). */

    /*Ether  field*/
    CTC_FIELD_KEY_MAC_SA,                   /**< [D2.TM] Source MAC Address. */
    CTC_FIELD_KEY_MAC_DA,                   /**< [D2.TM] Destination MAC Address. */
    CTC_FIELD_KEY_STAG_VALID,               /**< [D2.TM] S-Vlan Exist. */
    CTC_FIELD_KEY_PKT_STAG_VALID,       /**< [TM] Packet S-Vlan Exist. */
    CTC_FIELD_KEY_PKT_CTAG_VALID,       /**< [TM] Packet C-Vlan Exist. */
    CTC_FIELD_KEY_SVLAN_ID,                 /**< [D2.TM] S-Vlan ID. */
    CTC_FIELD_KEY_STAG_COS,                 /**< [D2.TM] Stag Cos. */
    CTC_FIELD_KEY_STAG_CFI,                 /**< [D2.TM] Stag Cfi. */
    CTC_FIELD_KEY_CTAG_VALID,               /**< [D2.TM] C-Vlan Exist. [TM] C-Vlan Exist(ParserResult Vlan or Vlan Range or Default Vlan) */
    CTC_FIELD_KEY_CVLAN_ID,                 /**< [D2.TM] C-Vlan ID. */
    CTC_FIELD_KEY_CTAG_COS,                 /**< [D2.TM] Ctag Cos. */
    CTC_FIELD_KEY_CTAG_CFI,                 /**< [D2.TM] Ctag Cfi. */
    CTC_FIELD_KEY_SVLAN_RANGE,              /**< [D2.TM] Svlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
    CTC_FIELD_KEY_CVLAN_RANGE,              /**< [D2.TM] Cvlan Range; data: min, mask: max, ext_data: (uint32*)vrange_gid. */
    CTC_FIELD_KEY_ETHER_TYPE,               /**< [D2.TM] Ether type. */
    CTC_FIELD_KEY_VLAN_NUM,                 /**< [D2.TM] Vlan Tag Number, Share with L2 Type. */

    CTC_FIELD_KEY_STP_STATE,                /**< [D2.TM] Stp Status. 0:forward, 1:blocking, 2:learning and 3: no forwarding (include learning or blocking)*/

    /*IP packet field*/
    CTC_FIELD_KEY_IP_SA,                    /**< [D2.TM] Source IPv4 Address. */
    CTC_FIELD_KEY_IP_DA,                    /**< [D2.TM] Destination IPv4 Address. */
    CTC_FIELD_KEY_IPV6_SA,                  /**< [D2.TM] Source IPv6 Address. ext_data */
    CTC_FIELD_KEY_IPV6_DA,                  /**< [D2.TM] Destination IPv6 Address. ext_data */
    CTC_FIELD_KEY_IPV6_FLOW_LABEL,          /**< [D2.TM] Ipv6 Flow label*/
    CTC_FIELD_KEY_IP_PROTOCOL,              /**< [D2.TM] Ip Protocol. */
    CTC_FIELD_KEY_IP_DSCP,                  /**< [D2.TM] DSCP. */
    CTC_FIELD_KEY_IP_PRECEDENCE,            /**< [D2.TM] Precedence. */
    CTC_FIELD_KEY_IP_ECN,                   /**< [D2.TM] Ecn. */
    CTC_FIELD_KEY_IP_FRAG,                  /**< [D2.TM] IP Fragment Information. */
    CTC_FIELD_KEY_IP_HDR_ERROR,             /**< [D2.TM] Ip Header Error. */
    CTC_FIELD_KEY_IP_OPTIONS,               /**< [D2.TM] Ip Options. */
    CTC_FIELD_KEY_IP_PKT_LEN_RANGE,         /**< [D2.TM] Ip Packet Length Range. */
    CTC_FIELD_KEY_IP_TTL,                   /**< [D2.TM] Ttl. */

    /*ARP packet field*/
    CTC_FIELD_KEY_ARP_OP_CODE,                  /**< [D2.TM] Opcode Field of ARP Header. */
    CTC_FIELD_KEY_ARP_PROTOCOL_TYPE,            /**< [D2.TM] Protocol Type of ARP Header. */
    CTC_FIELD_KEY_ARP_SENDER_IP,                /**< [D2.TM] Sender IPv4 Field of ARP Header. */
    CTC_FIELD_KEY_ARP_TARGET_IP,                /**< [D2.TM] Target IPv4 Field of ARP Header */
    CTC_FIELD_KEY_ARP_MAC_DA_CHK_FAIL,           /**< [D2.TM] Destination MAC Address of ARP Header Check Fail */
    CTC_FIELD_KEY_ARP_MAC_SA_CHK_FAIL,           /**< [D2.TM] Source MAC Address of ARP Header Check Fail */
    CTC_FIELD_KEY_ARP_SENDERIP_CHK_FAIL,         /**< [D2.TM] IP Address of sender of ARP Header Check Fail */
    CTC_FIELD_KEY_ARP_TARGETIP_CHK_FAIL,         /**< [D2.TM] IP Address of destination of ARP Header Check Fail */
    CTC_FIELD_KEY_GARP,                         /**< [D2.TM] Gratuitous ARP */
    CTC_FIELD_KEY_SENDER_MAC,                   /**< [D2.TM] Ethernet Address of sender of ARP Header */
    CTC_FIELD_KEY_TARGET_MAC,                   /**< [D2.TM] Ethernet Address of destination of ARP Header */
    CTC_FIELD_KEY_RARP,                               /**< [TM] If set and layer3 type is ARP, indicate packet is RARP packet */

    /*TCP/UDP packet field*/
    CTC_FIELD_KEY_L4_DST_PORT,              /**< [D2.TM] Layer 4 Dest Port. */
    CTC_FIELD_KEY_L4_SRC_PORT,              /**< [D2.TM] Layer 4 Src Port. */
    CTC_FIELD_KEY_L4_SRC_PORT_RANGE,        /**< [D2.TM] Layer 4 Src Port Range; data: min, mask: max. */
    CTC_FIELD_KEY_L4_DST_PORT_RANGE,        /**< [D2.TM] Layer 4 Dest Port Range; data: min, mask: max. */
    CTC_FIELD_KEY_TCP_ECN,                  /**< [D2.TM] TCP Ecn. */
    CTC_FIELD_KEY_TCP_FLAGS,                /**< [D2.TM] TCP Flags (ctc_acl_tcp_flag_flag_t). */
    CTC_FIELD_KEY_TCP_OPTIONS,               /**< [D2.TM] Ip Options,used to classify UDF key */

    /*GRE packet field*/
    CTC_FIELD_KEY_GRE_KEY,                  /**< [D2.TM] Gre Key. */
    CTC_FIELD_KEY_GRE_FLAGS,                 /**< [D2.TM] Gre Flag,used to classify UDF key  */
    CTC_FIELD_KEY_GRE_PROTOCOL_TYPE,        /**< [D2.TM] Gre PROTOCOL_TYPE,used to classify UDF key y*/

    /*NVGRE packet field*/
    CTC_FIELD_KEY_NVGRE_KEY,                  /**< [D2.TM] NVGre Key. */

    /*VXLAN packet field*/
    CTC_FIELD_KEY_VN_ID,                    /**< [D2.TM] Vxlan Network Id. */
    CTC_FIELD_KEY_VXLAN_FLAGS,        /**<[TM] Vxlan flags: data0 */
    CTC_FIELD_KEY_VXLAN_RSV1,          /**< [TM] Vxlan packet reserved1 after flags(24 bits), data0: vxlan reserved1 */
    CTC_FIELD_KEY_VXLAN_RSV2,          /**< [TM] Vxlan packet reserved2 after vni(8 bits), data0: vxlan reserved2 */

    /*ICMP packet field*/
    CTC_FIELD_KEY_ICMP_CODE,                /**< [D2.TM] ICMP Code.*/
    CTC_FIELD_KEY_ICMP_TYPE,                /**< [D2.TM] ICMP Type.*/

    /*IGMP packet field*/
    CTC_FIELD_KEY_IGMP_TYPE,                /**< [D2.TM] IGMP Type.*/

    /*MPLS packet field*/
    CTC_FIELD_KEY_LABEL_NUM,                /**< [D2.TM] MPLS Label Number. */
    CTC_FIELD_KEY_MPLS_LABEL0,              /**< [D2.TM] Label Field of the MPLS Label 0. */
    CTC_FIELD_KEY_MPLS_EXP0,                /**< [D2.TM] Exp Field of the MPLS Label 0.*/
    CTC_FIELD_KEY_MPLS_SBIT0,               /**< [D2.TM] S-bit Field of the MPLS Label 0.*/
    CTC_FIELD_KEY_MPLS_TTL0,                /**< [D2.TM] Ttl Field of the MPLS Label 0.*/
    CTC_FIELD_KEY_MPLS_LABEL1,              /**< [D2.TM] Label Field of the MPLS Label 1. */
    CTC_FIELD_KEY_MPLS_EXP1,                /**< [D2.TM] Exp Field of the MPLS Label 1.*/
    CTC_FIELD_KEY_MPLS_SBIT1,               /**< [D2.TM] S-bit Field of the MPLS Label 1.*/
    CTC_FIELD_KEY_MPLS_TTL1,                /**< [D2.TM] Ttl Field of the MPLS Label 1.*/
    CTC_FIELD_KEY_MPLS_LABEL2,              /**< [D2.TM] Label Field of the MPLS Label 2. */
    CTC_FIELD_KEY_MPLS_EXP2,                /**< [D2.TM] Exp Field of the MPLS Label 2.*/
    CTC_FIELD_KEY_MPLS_SBIT2,               /**< [D2.TM] S-bit Field of the MPLS Label 2.*/
    CTC_FIELD_KEY_MPLS_TTL2,                /**< [D2.TM] Ttl Field of the MPLS Label 2.*/
     CTC_FIELD_KEY_INTERFACE_ID,            /**< [D2.TM] L3 Interface Id,used to interface space */

    /*Network Service Header (NSH)*/
    CTC_FIELD_KEY_NSH_CBIT,                 /**< [D2] C-bit of the Network Service Header. */
    CTC_FIELD_KEY_NSH_OBIT,                 /**< [D2] O-bit of the Network Service Header. */
    CTC_FIELD_KEY_NSH_NEXT_PROTOCOL,        /**< [D2] Next Protocol of the Network Service Header. */
    CTC_FIELD_KEY_NSH_SI,                   /**< [D2] Service Index of the Network Service Header. */
    CTC_FIELD_KEY_NSH_SPI,                  /**< [D2] Service Path ID of the Network Service Header. */

    /*Ether OAM packet field*/
    CTC_FIELD_KEY_IS_Y1731_OAM,             /**< [D2.TM] Is Y1731 Oam. */
    CTC_FIELD_KEY_ETHER_OAM_LEVEL,          /**< [D2.TM] Oam Level. */
    CTC_FIELD_KEY_ETHER_OAM_OP_CODE,        /**< [D2.TM] Oam Opcode. */
    CTC_FIELD_KEY_ETHER_OAM_VERSION,        /**< [D2.TM] Oam Version. */

    /*Slow Protocol packet field*/
    CTC_FIELD_KEY_SLOW_PROTOCOL_CODE,       /**< [D2.TM] Slow Protocol Code. */
    CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS,      /**< [D2.TM] Slow Protocol Flags. */
    CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE,   /**< [D2.TM] Slow Protocol Sub Type. */

    /*PTP packet field*/
    CTC_FIELD_KEY_PTP_MESSAGE_TYPE,         /**< [D2.TM] PTP Message Type. */
    CTC_FIELD_KEY_PTP_VERSION,              /**< [D2.TM] PTP Message Version. */

    /*FCoE packet field*/
    CTC_FIELD_KEY_FCOE_DST_FCID,            /**< [D2.TM] FCoE Destination FCID.*/
    CTC_FIELD_KEY_FCOE_SRC_FCID,            /**< [D2.TM] FCoE Source FCID.*/

    /*Wlan packet field*/
    CTC_FIELD_KEY_WLAN_RADIO_MAC,         /**< [D2.TM] Wlan Radio Mac.*/
    CTC_FIELD_KEY_WLAN_RADIO_ID,          /**< [D2.TM] Wlan Radio Id.*/
    CTC_FIELD_KEY_WLAN_CTL_PKT,           /**< [D2.TM] Wlan Control Packet.*/

    /*SATPDU packet field*/
    CTC_FIELD_KEY_SATPDU_MEF_OUI,           /**< [D2.TM] Satpdu Metro Ethernet Forum Oui.*/
    CTC_FIELD_KEY_SATPDU_OUI_SUB_TYPE,      /**< [D2.TM] Satpdu Oui Sub type.*/
    CTC_FIELD_KEY_SATPDU_PDU_BYTE,          /**< [D2.TM] Satpdu Byte. ext_data: uint8 value[8], refer to network byte order*/

    /*TRILL packet field*/
    CTC_FIELD_KEY_INGRESS_NICKNAME,         /**< [D2.TM] Ingress Nick Name. */
    CTC_FIELD_KEY_EGRESS_NICKNAME,          /**< [D2.TM] Egress Nick Name. */
    CTC_FIELD_KEY_IS_ESADI,                 /**< [D2.TM] Is Esadi. */
    CTC_FIELD_KEY_IS_TRILL_CHANNEL,         /**< [D2.TM] Is Trill Channel. */
    CTC_FIELD_KEY_TRILL_INNER_VLANID,       /**< [D2.TM] Trill Inner Vlan Id. */
    CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID, /**< [D2.TM] Trill Inner Vlan Id Valid. */
    CTC_FIELD_KEY_TRILL_KEY_TYPE,           /**< [D2.TM] Trill Key Type. */
    CTC_FIELD_KEY_TRILL_LENGTH,             /**< [D2.TM] Trill Length. */
    CTC_FIELD_KEY_TRILL_MULTIHOP,           /**< [D2.TM] Trill Multi-Hop. */
    CTC_FIELD_KEY_TRILL_MULTICAST,          /**< [D2.TM] Trill MultiCast. */
    CTC_FIELD_KEY_TRILL_VERSION,            /**< [D2.TM] Trill Version. */
    CTC_FIELD_KEY_TRILL_TTL,                /**< [D2.TM] Trill Ttl. */

    /*802.1AE packet field*/
    CTC_FIELD_KEY_DOT1AE_UNKNOW_PKT,        /**< [D2.TM] Unknow 802.1AE packet, SecTag error or Mode not support. */
    CTC_FIELD_KEY_DOT1AE_ES,                /**< [D2.TM] End Station (ES) bit. */
    CTC_FIELD_KEY_DOT1AE_SC,                /**< [D2.TM] Secure Channel (SC) bit. */
    CTC_FIELD_KEY_DOT1AE_AN,                /**< [D2.TM] Association Number (AN). */
    CTC_FIELD_KEY_DOT1AE_SL,                /**< [D2.TM] Short Length (SL). */
    CTC_FIELD_KEY_DOT1AE_PN,                /**< [D2.TM] Packet Number. */
    CTC_FIELD_KEY_DOT1AE_SCI,               /**< [D2.TM] Secure Channel Identifier. ext_data: uint8 value[8], refer to network byte order*/
    CTC_FIELD_KEY_DOT1AE_CBIT,        /**< [TM] Determination of whether confidentiality or integrity alone are in use, data0: set use 1 and reset use 0 */
    CTC_FIELD_KEY_DOT1AE_EBIT,        /**< [TM] Encryption protected, data0: set use 1 and reset use 0 */
    CTC_FIELD_KEY_DOT1AE_SCB,         /**< [TM] EPON Single Copy Broadcast capability, data0: set use 1 and reset use 0 */
    CTC_FIELD_KEY_DOT1AE_VER,         /**< [TM] Version number, data0: set use 1 and reset use 0 */

    /*UDF, user defined field*/
    CTC_FIELD_KEY_UDF,                      /**< [D2.TM] UDF; ext_data: ctc_acl_udf_t */

    /*Match port type*/
    CTC_FIELD_KEY_PORT,                     /**< [D2.TM] Port; ext_data: refer to ctc_field_port_t */

    /*Forwarding information*/
    CTC_FIELD_KEY_DECAP,                    /**< [D2.TM] Decapsulation occurred. */
    CTC_FIELD_KEY_ELEPHANT_PKT,             /**< [D2.TM] Is Elephant. */
    CTC_FIELD_KEY_VXLAN_PKT,                /**< [D2.TM] Is Vxlan Packet. */
    CTC_FIELD_KEY_ROUTED_PKT,               /**< [D2.TM] Is Routed packet. */
    CTC_FIELD_KEY_PKT_FWD_TYPE,             /**< [D2.TM] Packet Forwarding Type. Refer to ctc_acl_pkt_fwd_type_t */
    CTC_FIELD_KEY_MACSA_LKUP,               /**< [D2.TM] Mac-Sa Lookup Enable. */
    CTC_FIELD_KEY_MACSA_HIT,                /**< [D2.TM] Mac-Sa Lookup Hit. */
    CTC_FIELD_KEY_MACDA_LKUP,               /**< [D2.TM] Mac-Da Lookup Enable. */
    CTC_FIELD_KEY_MACDA_HIT,                /**< [D2.TM] Mac-Da Lookup Hit. */
    CTC_FIELD_KEY_IPSA_LKUP,                /**< [D2.TM] L3-Sa Lookup Enable. */
    CTC_FIELD_KEY_IPSA_HIT,                 /**< [D2.TM] L3-Sa Lookup Hit. */
    CTC_FIELD_KEY_IPDA_LKUP,                /**< [D2.TM] L3-Da Lookup Enable. */
    CTC_FIELD_KEY_IPDA_HIT,                 /**< [D2.TM] L3-Da Lookup Hit. */
    CTC_FIELD_KEY_L2_STATION_MOVE,          /**< [D2.TM] L2 Station Move. */
    CTC_FIELD_KEY_MAC_SECURITY_DISCARD,     /**< [D2.TM] Mac Security Discard only for Black Hole MAC. */

    CTC_FIELD_KEY_DST_CID,                  /**< [D2.TM] Destination Category ID. */
    CTC_FIELD_KEY_SRC_CID,                  /**< [D2.TM] Source Category ID. */

    CTC_FIELD_KEY_DISCARD,                  /**< [D2.TM] Packet Is Flagged to be Discarded. Two mode: 1 only match discard by key.data; 2 match detail discard type
                                                                    by (uint32*) key.ext_data and the discard type refer to IPEDISCARDTYPE_XXX defined in sys_usw_register.c */
    CTC_FIELD_KEY_CPU_REASON_ID,            /**< [D2.TM] CPU Reason Id. */
    CTC_FIELD_KEY_DST_GPORT,                /**< [D2.TM] Fwd Destination Port. */
    CTC_FIELD_KEY_DST_NHID,                 /**< [D2.TM] Nexthop Id. */

    CTC_FIELD_KEY_MCAST_RPF_CHECK_FAIL,     /**< [D2.TM] Mcast Rpf Check Fail. */

    CTC_FIELD_KEY_VRFID,                    /**< [D2.TM] Vrfid. */
    CTC_FIELD_KEY_METADATA,                 /**< [D2.TM] Metadata. */
    CTC_FIELD_KEY_CUSTOMER_ID,              /**< [D2.TM] Customer Id*/

    /*Merge Key */
    CTC_FIELD_KEY_AWARE_TUNNEL_INFO,        /**< [D2.TM] Aware Tunnel Info. ext_data: refer to ctc_acl_tunnel_data_t.*/

    /*HASH key field*/
    CTC_FIELD_KEY_HASH_VALID,               /**< [D2.TM] Hash Valid Key. */
    CTC_FIELD_KEY_UDF_ENTRY_VALID,  /**< [D2.TM] classify udf entry Valid Key. */

    CTC_FIELD_KEY_IS_ROUTER_MAC,            /**< [D2.TM] MacDa is Router Mac. */
    CTC_FIELD_KEY_HASH_SEL_ID,              /**< [D2.TM] delete */

    /*Other*/
    CTC_FIELD_KEY_IS_LOG_PKT,               /**< [D2.TM] Log to Cpu Packet. */
    CTC_FIELD_KEY_CLASS_ID,                 /**< [D2.TM] Class ID*/
    CTC_FIELD_KEY_VLAN_XLATE_HIT,        /**< [TM] If Vlan xlate not hit, do egress acl lookup. data0: enable use 1 and disable use 0 */
    CTC_FIELD_KEY_GEM_PORT,                  /**< [TM] Gem port for xgpon: data0 */

    CTC_FIELD_KEY_NUM
};
typedef enum ctc_field_key_type_e ctc_field_key_type_t;



struct ctc_field_key_s
{
    uint16 type;                    /**< [D2.TM] Key Field type, ctc_field_key_type_t. */

    uint32 data;                    /**< [D2.TM] Key Field data (uint32). */
    uint32 mask;                    /**< [D2.TM] Key Field mask (uint32). */
    void*  ext_data;                /**< [D2.TM] Key Field extend data (void*). */
    void*  ext_mask;                /**< [D2.TM] Key Field extend mask (void*). */
};
typedef struct ctc_field_key_s ctc_field_key_t;


enum ctc_field_port_type_e
{
    CTC_FIELD_PORT_TYPE_NONE,         /**< [GB.GG.D2.TM] don't care port type */
    CTC_FIELD_PORT_TYPE_PORT_BITMAP,  /**< [GB.GG.D2.TM] valid Port bitmap */
    CTC_FIELD_PORT_TYPE_GPORT,        /**< [GG.D2.TM] valid GlobalPort */
    CTC_FIELD_PORT_TYPE_LOGIC_PORT,   /**< [GB.GG.D2.TM] valid logic port */
    CTC_FIELD_PORT_TYPE_PORT_CLASS,   /**< [GB.GG.D2.TM] valid port class */
    CTC_FIELD_PORT_TYPE_VLAN_CLASS,   /**< [GB.GG.D2.TM] valid vlan class */
    CTC_FIELD_PORT_TYPE_L3IF_CLASS,   /**< [D2.TM] valid l3if class */
    CTC_FIELD_PORT_TYPE_PBR_CLASS,    /**< [GB.GG.D2.TM] valid pbr class */

    CTC_FIELD_PORT_TYPE_MAX
};
typedef enum ctc_field_port_type_e ctc_field_port_type_t;

struct ctc_field_port_s
{
    uint8  type;                  /**< [GB.GG.D2.TM] Refer to ctc_field_port_type_t */
    uint16 port_class_id;         /**< [GB.GG.D2.TM] port class id */
    uint16 logic_port;            /**< [GB.GG.D2.TM] logic port.  */
    uint32 gport;                 /**< [GG.D2.TM] gport. */

    /*Only used for ACL key field  */
    uint8  lchip;                 /**< [GB.GG.D2.TM] Local chip id, only for type= PORT_BITMAP, Other type ignore it*/
    ctc_port_bitmap_t port_bitmap;/**< [GB.GG.D2.TM] bitmap*/
    uint16 vlan_class_id;         /**< [GB.GG.D2.TM] vlan class id */
    uint16 l3if_class_id;         /**< [GB.GG.D2.TM] vlan class id */
    uint8  pbr_class_id;          /**< [GB.GG.D2.TM] pbr class id. */
};
typedef struct ctc_field_port_s ctc_field_port_t;

#ifdef __cplusplus
}
#endif

#endif


