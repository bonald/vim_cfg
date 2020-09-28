
#ifndef __LLDP_DEFINE_H__
#define __LLDP_DEFINE_H__

#define LLDP_TX_HOLD_DEFAULT            4
#define LLDP_TX_INTERVAL_DEFAULT        30
#define LLDP_TX_DELAY_DEFAULT           2
#define LLDP_REINIT_DELAY_DEFAULT       2
#define LLDP_MANAGEMENT_ADDRESS_IPV4_TLV_LENGTH    12

#define LLDP_MANAGEMENT_ADDRESS_TLV_LENGTH  14

/* LLDP Ethertype */
#define LLDP_ETHERTYPE_1                                 0x88
#define LLDP_ETHERTYPE_2                                 0xcc

/* IEEE 802.1 OUI */
#define IEEE_8021_OUI_1                                  0x00
#define IEEE_8021_OUI_2                                  0x80
#define IEEE_8021_OUI_3                                  0xc2

/* IEEE 802.3 OUI */
#define IEEE_8023_OUI_1                                  0x00
#define IEEE_8023_OUI_2                                  0x12
#define IEEE_8023_OUI_3                                  0x0f

/* TIA OUI */
#define TIA_OUI_1                                  0x00
#define TIA_OUI_2                                  0x12
#define TIA_OUI_3                                  0xbb

/* IEEE 802.1 Organizationally Specific TLVs Subtypes */
#define PORT_VLAN_ID_TLV_SUBTYPE                         1
#define PORT_AND_PROTOCOL_VLAN_ID_TLV_SUBTYPE            2
#define VLAN_NAME_TLV_SUBTYPE                            3
#define PROTOCOL_IDENTITY_TLV_SUBTYPE                    4
#define LINK_AGGREGATION_8021_TLV_SUBTYPE                7
/* IEEE 802.3 Organizationally Specific TLVs Subtypes */
#define MAC_PHY_CONFIG_STATUS_TLV_SUBTYPE                1
#define POWER_VIA_MDI_TLV_SUBTYPE                        2
#define LINK_AGGREGATION_TLV_SUBTYPE                     3
#define MAXIMUM_FRAME_SIZE_TLV_SUBTYPE                   4

/* TIA Organizationally Specific TLVs Subtypes*/
#define MED_TYPE_CAPABILITIES_SUBTYPE                       1                             
#define MED_TYPE_NETWORK_POLICY_SUBTYPE                     2
#define MED_TYPE_LOCATION_ID_SUBTYPE                        3
#define MED_TYPE_EXT_POWER_VIA_MDI_SUBTYPE                  4 

#define MED_TYPE_CAPABILITIES_SUB_CAP               0
#define MED_TYPE_CAPABILITIES_SUB_NP                1
#define MED_TYPE_CAPABILITIES_SUB_LCI               2
#define MED_TYPE_CAPABILITIES_SUB_PSE               3
#define MED_TYPE_CAPABILITIES_SUB_PD                4
#define MED_TYPE_CAPABILITIES_SUB_IN                5
#define MED_TYPE_CAPABILITIES_SUB_RESERVED          6

#define MED_TYPE_INVENTORY_SUBTYPE_HARDWARE                   5
#define MED_TYPE_INVENTORY_SUBTYPE_FIRMWARE                   6
#define MED_TYPE_INVENTORY_SUBTYPE_SOFTWARE                   7
#define MED_TYPE_INVENTORY_SUBTYPE_SERIAL                     8
#define MED_TYPE_INVENTORY_SUBTYPE_MANUFACTURER_NAME          9
#define MED_TYPE_INVENTORY_SUBTYPE_MODEL_NAME                 10
#define MED_TYPE_INVENTORY_SUBTYPE_ASSET_ID                   11

#define MED_TYPE_POWER_FLAG_TYPE_PSE                0x00
#define MED_TYPE_POWER_FLAG_TYPE_PD                 0x01

#define MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_PRI        0x01
#define MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_BAK        0x02

#define MED_TYPE_POWER_FLAG_TYPE_PD_SRC_PSE         0x01
#define MED_TYPE_POWER_FLAG_TYPE_PD_SRC_LOCAL       0x02
#define MED_TYPE_POWER_FLAG_TYPE_PD_SRC_ALL         0x03

#define MED_TYPE_POWER_FLAG_TYPE_PRIORITY_CRI       0x01
#define MED_TYPE_POWER_FLAG_TYPE_PRIORITY_HIGH      0x02
#define MED_TYPE_POWER_FLAG_TYPE_PRIORITY_LOW       0x03

#define LLDP_MED_NP_GET_APP_TYPE(x)                 ((x) >> 24)
#define LLDP_MED_NP_GET_U_VALUE(x)                  (((x) >> 23) & 1)
#define LLDP_MED_NP_GET_T_VALUE(x)                  (((x) >> 22) & 1)
#define LLDP_MED_NP_GET_VLAN_ID(x)                  (((x) >> 9) & 0xFFF)
#define LLDP_MED_NP_GET_L2_PRIORITY(x)              (((x) >> 6) & 0x7)
#define LLDP_MED_NP_GET_DSCP(x)                     ((x) & 0x3F)
#define LLDP_MED_NP_DEFAULT_VALUE                   0x01800000

#define LLDP_MED_POW_GET_TYPE(x)                    (((x) & 0xC0) >> 6)
#define LLDP_MED_POW_GET_SRC(x)                     (((x) & 0x30) >> 4)
#define LLDP_MED_POW_GET_PRIORITY(x)                ((x) & 0x0F)

/* Network connectivity*/
#define LLDP_MED_DEV_TYPE_DEFAULT                   0X04


/* extended power-via-MID*/
#define MED_TYPE_POWER_FLAG_TYPE_PSE                0x00
#define MED_TYPE_POWER_FLAG_TYPE_PD                 0x01

#define MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_PRI        0x01
#define MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_BAK        0x02

#define MED_TYPE_POWER_FLAG_TYPE_PD_SRC_PSE         0x01
#define MED_TYPE_POWER_FLAG_TYPE_PD_SRC_LOCAL       0x02
#define MED_TYPE_POWER_FLAG_TYPE_PD_SRC_ALL         0x03

#define MED_TYPE_POWER_FLAG_TYPE_PRIORITY_CRI       0x01
#define MED_TYPE_POWER_FLAG_TYPE_PRIORITY_HIGH      0x02
#define MED_TYPE_POWER_FLAG_TYPE_PRIORITY_LOW       0x03

typedef enum
{
    LLDP_SYS_CAP_L2_OTHER       = 0,
    LLDP_SYS_CAP_L2_REPEATER    = 1,
    LLDP_SYS_CAP_L2_SWITCHING   = 2,
    LLDP_SYS_CAP_L2_WAN         = 3,
    LLDP_SYS_CAP_ROUTING        = 4,
    LLDP_SYS_CAP_TEL            = 5,
    LLDP_SYS_CAP_CABLE_DEV      = 6,
    LLDP_SYS_CAP_STATION        = 7,
    LLDP_SYS_CAP_MAX            = 8
} lldp_sys_cap_t;

#define LLDP_SYS_CAP_DEFAULT        ((1<<LLDP_SYS_CAP_L2_SWITCHING) | (1<<LLDP_SYS_CAP_ROUTING))

typedef enum
{
    LLDP_PROTO_STP              = 0,
    LLDP_PROTO_RSTP             = 1,
    LLDP_PROTO_MSTP             = 2,
    LLDP_PROTO_GMRP             = 3,
    LLDP_PROTO_MMRP             = 4,
    LLDP_PROTO_GVRP             = 5,
    LLDP_PROTO_MVRP             = 6,
    LLDP_PROTO_LACP             = 7,
    LLDP_PROTO_DOT1X            = 8,
    LLDP_PROTO_EFM_OAM          = 9,
    LLDP_PROTO_MAX              = 10
} lldp_proto_t;

#define LLDP_802_1_PROTO_ID_LEN                        8

#define LLDP_802_3_PROTO_ID_LEN                        8
#define LLDP_MRP_PROTO_ID_LEN                          2
#define LLDP_DOT1X_PROTO_ID_LEN                        2
#define LLDP_SLOW_PROTO_ID_LEN                         3
#define LLDP_UNKNOWN_PROTO_ID_LEN                      0

typedef struct lldp_protocol_identity_s
{
  uint8  *protocol_payload;
  uint8  protocol_id_len;
  uint8  protocol_id;
  char   *protocol_str; 
}lldp_protocol_identity_t;

/* 802.1AB-2005 9.4.1 TLV Type */
typedef enum
{
    LLDP_TLV_TYPE_END_OF_LLDPDU         = 0,
    LLDP_TLV_TYPE_CHASSIS_ID            = 1,    
    LLDP_TLV_TYPE_PORT_ID               = 2,
    LLDP_TLV_TYPE_TTL                   = 3,
    LLDP_TLV_TYPE_PORT_DESCRIPTION      = 4,
    LLDP_TLV_TYPE_SYSTEM_NAME           = 5,
    LLDP_TLV_TYPE_SYSTEM_DESCRIPTION    = 6,
    LLDP_TLV_TYPE_SYSTEM_CAPABILITIES   = 7,
    LLDP_TLV_TYPE_MANAGEMENT_ADDRESS    = 8,
    LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC = 127,
} lldp_tlv_type_t;


#define TTL_TLV_MIN_SIZE                                 2

#define LLDP_TLV_HDR_LEN                                 2
#define LLDP_PROTOCOL_ID_TLV_HDR_LEN                     5

#define LLDP_CONF_MGMT_IP_ADDRESS  1

#define LLDP_TX_DELAY_CONSTANT 0.25

/* 802.1AB 2005 Figure 9-2 Basic TLV format */
#define LLDP_ENCODE_TL(T, L) \
do { \
    uint16 u16 = (((T) << 9) + ((L))); \
    MSG_ENCODE_PUTW(u16); \
} while (0)

#define LLDP_DECODE_TL(T, L) \
do { \
    uint16 u16 = 0; \
    MSG_DECODE_GETW(u16); \
    (T) = (u16 >> 9); \
    (L) = (u16 & 0x01FF); \
} while (0)

#define BASIC_TLV_EN_PORT_DESCRIPTION                   0
#define BASIC_TLV_EN_SYSTEM_NAME                        1
#define BASIC_TLV_EN_SYSTEM_DESCRIPTION                 2
#define BASIC_TLV_EN_SYSTEM_CAPABILITIES                3
#define BASIC_TLV_EN_MANAGEMENT_ADDRESS                 4
#define BASIC_TLV_EN_ALL ((1 << BASIC_TLV_EN_PORT_DESCRIPTION)\
                |(1 << BASIC_TLV_EN_SYSTEM_NAME)\
                |(1 << BASIC_TLV_EN_SYSTEM_DESCRIPTION)\
                |(1 << BASIC_TLV_EN_SYSTEM_CAPABILITIES)\
                |(1 << BASIC_TLV_EN_MANAGEMENT_ADDRESS))

#define IEEE_8021_TLV_EN_PORT_VLAN_ID                   0
#define IEEE_8021_TLV_EN_PROTO_VLAN_ID                  1
#define IEEE_8021_TLV_EN_PORT_VLAN_NAME                 2
#define IEEE_8021_TLV_EN_PROTOCOL_ID                    3
#define IEEE_8021_TLV_EN_LINK_AGG_8021                  4
#define IEEE_8021_TLV_EN_DCBX                           5
#define IEEE_8021_TLV_EN_ALL ((1 << IEEE_8021_TLV_EN_PORT_VLAN_ID)\
                |(1 << IEEE_8021_TLV_EN_PROTO_VLAN_ID)\
                |(1 << IEEE_8021_TLV_EN_PORT_VLAN_NAME)\
                |(1 << IEEE_8021_TLV_EN_PROTOCOL_ID)) 
/* IEEE_8021_TLV_EN_LINK_AGG_8021 is at 802.3 by default */

#define IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS          0
#define IEEE_8023_TLV_EN_POWER_VIA_MDI                  1
#define IEEE_8023_TLV_EN_LINK_AGG_8023                  2
#define IEEE_8023_TLV_EN_MAX_FRAME_SIZE                 3
#define IEEE_8023_TLV_EN_ALL ((1 << IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS)\
                |(1 << IEEE_8023_TLV_EN_POWER_VIA_MDI)\
                |(1 << IEEE_8023_TLV_EN_LINK_AGG_8023)\
                |(1 << IEEE_8023_TLV_EN_MAX_FRAME_SIZE))

#define MED_TLV_EN_CAP                                  0
#define MED_TLV_EN_NETWORK_POLICY                       1
#define MED_TLV_EN_LOCATION_ID                          2
#define MED_TLV_EN_EXT_POWER_VIA_MDI                    3
#define MED_TLV_EN_INVENTORY                            4
#define MED_TLV_EN_ALL ((1 << MED_TLV_EN_CAP)\
                |(1 << MED_TLV_EN_NETWORK_POLICY)\
                |(1 << MED_TLV_EN_EXT_POWER_VIA_MDI)\
                |(1 << MED_TLV_EN_INVENTORY))


#define LLDP_8021_AGGREGATION_CAPABLE                       1
#define LLDP_8021_AGGREGATION_ENABLE                        2

#define LLDP_8023_AGGREGATION_CAPABLE                       1
#define LLDP_8023_AGGREGATION_ENABLE                        2


#define LLDP_MAU_AUTONEG_SUPPORTED                  1
#define LLDP_MAU_AUTONEG_ENABLED                    2


#define LLDP_PSE_POWER_PAIR_SIGNAL 1
#define LLDP_PSE_POWER_PAIR_SPARE  2

#define LLDP_PSE_POWER_FLAG_CLASS   0x01
#define LLDP_PSE_POWER_FLAG_SUPPORT 0x02
#define LLDP_PSE_POWER_FLAG_STATE   0x03
#define LLDP_PSE_POWER_FLAG_PAIRS   0x04

#define LLDP_MED_NP_TYPE_VOICE                  1
#define LLDP_MED_NP_TYPE_VOICE_SIGNALING        2
#define LLDP_MED_NP_TYPE_GUEST_VOICE            3
#define LLDP_MED_NP_TYPE_GUEST_VOICE_SIGNALING  4
#define LLDP_MED_NP_TYPE_SOFTPHONE_VOICE        5
#define LLDP_MED_NP_TYPE_VIDEO_CONFER           6
#define LLDP_MED_NP_TYPE_STREAMING_VIDEO        7
#define LLDP_MED_NP_TYPE_VIDEO_SIGNALING        8
#define LLDP_MED_NP_TYPE_MAX                    9


#endif /* __LLDP_DEFINE_H__ */

