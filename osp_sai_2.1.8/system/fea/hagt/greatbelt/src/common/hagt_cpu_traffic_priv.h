#ifndef __HAGT_CPU_TRAFFIC_PRIV_H__
#define __HAGT_CPU_TRAFFIC_PRIV_H__
/*********************************************************************
 * hagt_cpu_traffic.h:  Hal agent cpu traffic process
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
*********************************************************************/

/*********************************************************************
*
* Defines and Macros
*
*********************************************************************/

#define HAGT_L2PDU_KEY_INDEX_MACDA_MAX          4
#define HAGT_L2PDU_KEY_INDEX_MACDA_LOW24_MAX    8
#define HAGT_L2PDU_KEY_INDEX_L2HDR_PROTO_MAX    16

#define HAGT_FLEX_L2PDU_KEY_EXIST   1

#define HAGT_PDU_SPECIAL_ACTION_INDEX   0xff
#define HAGT_PDU_SPECIAL_PORT_INDEX     0xff

#define BIT_2_BYTE_PER_SECOND(rate_bps) ((rate_bps) / 8)
#define BPS_2_KBPS(rate_bps)  ((rate_bps) / 1000)

#define HAGT_CPU_TRAFFIC_FLEX_L2PDU_NORMAL_LIMIT_RATE (200 * 1000)
#define HAGT_CPU_TRAFFIC_FLEX_L2PDU_L2PROTO_MAC_LIMIT_RATE (64 * 1000)
#define HAGT_CPU_TRAFFIC_FLEX_L2PDU_L2PROTO_TUNNEL_DMAC_LIMIT_RATE (600 * 1000)
#define HAGT_CPU_TRAFFIC_SPECIAL_SHAPE_BURST  0xFFFFFFFF
#define HAGT_CPU_TRAFFIC_DEFAULT_SHAPE_BURST  12
#define HAGT_CPU_TRAFFIC_SPECIAL_CPU_TRAFFIC_RATE 0xFFFFFFFF
#define HAGT_CPU_TRAFFIC_DEFAULT_FWD_CPU_CLASS 0

/* Added by yanxa, 8 = GLB_L2PDU_ACTION_INDEX_DEFAULT - GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC0 */
#define HAGT_PDU_FLEX_ACTION_INDEX_MAX_NUM 8

#define HAGT_DEBUG_CPU_TRAFFIC_FUNC()                                                   \
    HAGT_CPU_TRAFFIC_DEBUG("Enter into %s.", __FUNCTION__)

#define HAGT_DEBUG_CPU_PDU_FUNC()                                                   \
    HAGT_CPU_PDU_DEBUG("Enter into %s.", __FUNCTION__)

#define HAGT_DEBUG_FLEX_L2PDU_KEY(glb_flex_l2pdu_key)                                   \
do {                                                                                    \
    if (GLB_PDU_L2PDU_KEY_TYPE_L2HDR_PROTO == (glb_flex_l2pdu_key).key_type)            \
    {                                                                                   \
        HAGT_CPU_PDU_DEBUG(" flex-l2pdu = %d, key-type = L2HDR-PROTO, ether-type = %04x\n",\
                           (glb_flex_l2pdu_key).flex_l2pdu,                             \
                           (glb_flex_l2pdu_key).key.ether_type);                        \
    }                                                                                   \
    else if(GLB_PDU_L2PDU_KEY_TYPE_MACDA == (glb_flex_l2pdu_key).key_type)\
    {                                                                                   \
        HAGT_CPU_PDU_DEBUG(" flex-l2pdu = %d, key-type = MAC-FULL, macda = %02X%02X.%02X%02X.%02X%02X,"\
                           " macda-mask = %02X%02X.%02X%02X.%02X%02X\n",                \
                           (glb_flex_l2pdu_key).flex_l2pdu,                             \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[0],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[1],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[2],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[3],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[4],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[5],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[0],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[1],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[2],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[3],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[4],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[5]);        \
    }                                                                                   \
    else if (GLB_PDU_L2PDU_KEY_TYPE_MACDA_LOW24 == (glb_flex_l2pdu_key).key_type)       \
    {                                                                                   \
        HAGT_CPU_PDU_DEBUG(" flex-l2pdu = %d, key-type = MAC-LOW24, macda = 0180.c2%02X.%02X%02X, "\
                           "macda-mask = FFFF.FF%02X.%02X%02X\n",                       \
                           (glb_flex_l2pdu_key).flex_l2pdu,                             \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[3],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[4],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda[5],              \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[3],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[4],         \
                           (glb_flex_l2pdu_key).key.l2pdu_by_mac.macda_mask[5]);        \
    }                                                                                   \
}while(0)

#define HAGT_CPU_TRAFFIC_SET_MAC(mac, byte0, byte1, byte2, byte3, byte4, byte5)         \
do{                                                                                     \
    mac[0] = byte0;                                                                     \
    mac[1] = byte1;                                                                     \
    mac[2] = byte2;                                                                     \
    mac[3] = byte3;                                                                     \
    mac[4] = byte4;                                                                     \
    mac[5] = byte5;                                                                     \
}while(0)

/*********************************************************************
 *
 * Data Structures Definition
 *
*********************************************************************/

/* Ingress security action index */
enum hagt_security_action_index_e
{
    HAGT_SECURITY_ACTION_INDEX_MACSA_DISCARD = 0,
    HAGT_SECURITY_ACTION_INDEX_MACSA_MISMATCH = 1,
    HAGT_SECURITY_ACTION_INDEX_PORT_SECURITY_DISCARD = 2,
    HAGT_SECURITY_ACTION_INDEX_VLAN_SECURITY_DISCARD = 3,

    HAGT_SECURITY_ACTION_INDEX_MAX
};



/* L3_TYPE based L2-PDU key index */
enum hagt_l2pdu_key_index_by_l3type_e
{
#if 0 /*Removed by xgu, GB will classify EPAOL and Slow protocol in chip, no need to classify EAPOL here*/
    HAGT_L2PDU_KEY_INDEX_EAPOL      = CTC_PARSER_L3_TYPE_EAPOL,
    HAGT_L2PDU_KEY_INDEX_SLOW_PROTO = CTC_PARSER_L3_TYPE_SLOW_PROTO,
#endif        
    HAGT_L2PDU_KEY_INDEX_MAX
};
typedef enum hagt_l2pdu_key_index_by_l3type_e hagt_l2pdu_key_index_by_l3type_t;

/* BPDU key index */
#define HAGT_L2PDU_KEY_INDEX_BPDU     0

#define HAGT_L3PDU_KEY_INDEX_EXPLICT_PDU       0

enum hagt_l2pdu_key_index_by_mac_da_e
{
    HAGT_L2PDU_KEY_INDEX_ERPS_COMMON         = 0,
    HAGT_L2PDU_KEY_INDEX_ERPS_FLUSH          = 1,
    HAGT_L2PDU_KEY_INDEX_L2PROTO_TUNNEL_DMAC = 2,

    HAGT_L2PDU_KEY_INDEX_L2PROTO_FULL_MAC0 = 3     /*add by zhangjx for bug 20488 , 20120904*/

    //HAGT_L2PDU_KEY_INDEX_MAC_FLEX_BEGIN = 3,      /*delete by zhangjx for bug 20488 , 20120904*/
    //HAGT_L2PDU_KEY_INDEX_MAC_FLEX_END   = 3         /*delete by zhangjx for bug 20488 , 20120904*/
};
typedef enum hagt_l2pdu_key_index_by_mac_da_e hagt_l2pdu_key_index_by_mac_da_t;

enum hagt_l2pdu_key_index_by_mac_da_low24_e
{
    /*Commented by xgu, low24 bit with mask, index from 0~1*/
    /*Commented by xgu, low24 bit without mask, index from 2~9*/
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MASK_MAC0 = 0,/*used for 0180.c200.0000~0180.c200.003F, with mask*/
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MAC_DFT = 1,/*used for 0180.c200.0000~0180.c2FF.FFFF, with mask*/
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MAC0 = 2,
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MAC1 = 3,
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MAC2 = 4,
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MAC3 = 5,
    HAGT_L2PDU_KEY_INDEX_L2PROTO_MAC4 = 6,
    HAGT_L2PDU_KEY_INDEX_MAC_LOW24_FLEX_BEGIN = 7,
    HAGT_L2PDU_KEY_INDEX_MAC_LOW24_FLEX_END   = 9
};
typedef enum hagt_l2pdu_key_index_by_mac_da_low24_e hagt_l2pdu_key_index_by_mac_da_low24_t;


enum hagt_l2pdu_key_index_by_l2hdr_proto_s
{
    HAGT_L2PDU_KEY_INDEX_SMARK_LINK = 0,
    HAGT_L2PDU_KEY_INDEX_L2PING     = 1,
    HAGT_L2PDU_KEY_INDEX_UDLD       = 2,/* modified by yaom for udld, 2011.7.6 */
    HAGT_L2PDU_KEY_INDEX_CFM        = 3,

    HAGT_L2PDU_KEY_INDEX_L2HDR_PROTO_FLEX_BEGIN = 4,
    HAGT_L2PDU_KEY_INDEX_L2HDR_PROTO_FLEX_END   = 15
};
typedef enum hagt_l2pdu_key_index_by_l2hdr_proto_s hagt_l2pdu_key_index_by_l2hdr_proto_t;

/* L3 header protocol based L3-PDU key index */
enum hagt_l3pdu_key_index_by_l3hdr_proto_e
{
/*Modified by xgu, for GB will classify OSPF/PIM/VRRP/RSVP in chip, no need to consume key index*/
#if 0
    HAGT_L3PDU_KEY_INDEX_OSPF   = 0,
    HAGT_L3PDU_KEY_INDEX_PIM    = 1,
    HAGT_L3PDU_KEY_INDEX_VRRP   = 2,
    HAGT_L3PDU_KEY_INDEX_RSVP   = 3,
#endif    
    HAGT_L3PDU_KEY_INDEX_ICMPV6 = 0,
    HAGT_L3PDU_KEY_INDEX_IGMP   = 1,    
};
typedef enum hagt_l3pdu_key_index_by_l3hdr_proto_e hagt_l3pdu_key_index_by_l3hdr_proto_t;

/* TCP/UDP port based L3-PDU key index */
enum hagt_l3pdu_key_index_by_l4port_e
{
/*Modified by xgu, for GB will classify RIP/RIPng/LDP in chip, no need to consume key index*/
#if 0
    HAGT_L3PDU_KEY_INDEX_RIP   = 0,
    HAGT_L3PDU_KEY_INDEX_RIPng = 1,
    HAGT_L3PDU_KEY_INDEX_LDP   = 2,
#endif    
    HAGT_L3PDU_KEY_INDEX_MPLS_OAM = 3,// TODO: MPLS OAM should consider here for GB system, commented by xgu, 2012-12-31
    HAGT_L3PDU_KEY_INDEX_BFD = 4,/* modified by huangxt for BFD develop, 2011-03-04 */// TODO: BFD should consider here for GB system, commented by xgu, 2012-12-31
};
typedef enum hagt_l3pdu_key_index_by_l4port_e hagt_l3pdu_key_index_by_l4port_t;

/*flexible l2-pdu key*/
struct hagt_pdu_flex_l2pdu_index_node_s
{
    ctclib_list_node_t node;

    glb_pdu_flex_l2pdu_key_t key;

    uint8  key_index;
    uint8  action_index;
    uint8  reserved[2];
};
typedef struct hagt_pdu_flex_l2pdu_index_node_s hagt_pdu_flex_l2pdu_index_node_t;

/*hal agent layer cpu traffic master*/
struct hagt_cpu_traffic_master_s
{
    uint32 l2pdu_action_index_bmp[2];
    uint16 l2pdu_eth_type_key_index_bmp;
    uint16 l2pdu_mac_low24_key_index_bmp;
    uint8  l2pdu_mac_full_key_index_bmp;
    uint8  reserved;

    ctclib_list_t flex_l2pdu_list;     /* list of hagt_pdu_flex_l2pdu_index_t */
};
typedef struct hagt_cpu_traffic_master_s hagt_cpu_traffic_master_t;

/**********************************************************************
 *
 * Functions Declaration
 *
***********************************************************************/

#endif /*_HAGT_CPU_TRAFFIC_PRIV_H_*/
