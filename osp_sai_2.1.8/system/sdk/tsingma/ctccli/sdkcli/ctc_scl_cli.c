/**
 @date 2009-12-22

 @version v2.0

---file comments----
*/

/****************************************************************************
 *
 * Header files
 *
 *****************************************************************************/
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_scl_cli.h"

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

#define CTC_CLI_SCL_ADD_ENTRY_STR  "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|)"
#define CTC_CLI_SCL_ADD_HASH_ENTRY_STR  "scl add (group GROUP_ID|) (entry ENTRY_ID)"
#define CTC_CLI_SCL_ADD_ENTRY_DESC \
    CTC_CLI_SCL_STR, \
    "Add one entry to software table", \
    CTC_CLI_SCL_GROUP_ID_STR, \
    CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE, \
    CTC_CLI_SCL_ENTRY_ID_STR, \
    CTC_CLI_SCL_ENTRY_ID_VALUE, \
    CTC_CLI_SCL_ENTRY_PRIO_STR, \
    CTC_CLI_SCL_ENTRY_PRIO_VALUE

#define CTC_CLI_SCL_ADD_ENTRY_SET()  \
    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));             \
    CTC_CLI_GET_UINT32("group id", gid, argv[0]);                   \
    CTC_CLI_GET_UINT32("entry id", scl_entry->entry_id, argv[1]);    \
    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");                     \
    if (INDEX_VALID(index))                                         \
    {                                                               \
       scl_entry->priority_valid = 1;                                \
       CTC_CLI_GET_UINT32("entry priority", scl_entry->priority, argv[index + 1]); \
    }                                                                \

#define CTC_CLI_SCL_ADD_HASH_ENTRY_DESC \
    CTC_CLI_SCL_STR, \
    "Add one entry to software table", \
    CTC_CLI_SCL_GROUP_ID_STR, \
    CTC_CLI_SCL_HASH_GROUP_ID_VALUE, \
    CTC_CLI_SCL_ENTRY_ID_STR,          \
    CTC_CLI_SCL_ENTRY_ID_VALUE

#define CTC_CLI_SCL_ADD_HASH_ENTRY_SET()  \
    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));             \
    index = CTC_CLI_GET_ARGC_INDEX("group");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
       CTC_CLI_GET_UINT32("group id", gid, argv[index + 1]);                       \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("entry");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
       CTC_CLI_GET_UINT32("entry id", scl_entry->entry_id, argv[index + 1]);                \
    }

/****************************************************************
 *           scl string (parameter)   -begin-                   *
 ****************************************************************/
#define CTC_CLI_SCL_KEY_MAC_STR  \
    " | mac-sa MAC MASK | mac-da MAC MASK | \
      layer2-type L2_TYPE MASK | layer3-type L3_TYPE MASK "

#define CTC_CLI_SCL_KEY_MAC_DESC \
    "MAC source address",         \
    CTC_CLI_MAC_FORMAT,           \
    CTC_CLI_MAC_FORMAT,           \
    "MAC destination address",    \
    CTC_CLI_MAC_FORMAT,           \
    CTC_CLI_MAC_FORMAT,           \
    "Layer 2 Type",               \
    CTC_CLI_PARSER_L2_TYPE_VALUE, \
    CTC_CLI_PARSER_L2_TYPE_MASK,  \
    "Layer 3 Type",               \
    CTC_CLI_PARSER_L3_TYPE_VALUE, \
    CTC_CLI_PARSER_L3_TYPE_MASK

#define CTC_CLI_SCL_KEY_MAC_SET(TYPE)  \
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");                                      \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);         \
        CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", p_key->mac_da_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_MAC_DA);                        \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");                                      \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);         \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", p_key->mac_sa_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_MAC_SA);                        \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("layer2-type");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("layer2-type", p_key->l2_type, argv[index + 1]);              \
        CTC_CLI_GET_UINT8("layer2-type mask", p_key->l2_type_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_L2_TYPE);                       \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("layer3-type");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("layer3-type", p_key->l3_type, argv[index + 1]);              \
        CTC_CLI_GET_UINT8("layer3-type mask", p_key->l3_type_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_L3_TYPE);                       \
    }                                                                                   \
    CTC_CLI_SCL_KEY_VLAN_SET(MAC)

#define CTC_CLI_SCL_KEY_PORT_STR      \
    " (gport GPORT  GPORT_MASK  | key-logic-port LOGIC_PORT LOGIC_PORT_MASK | port-class CLASS_ID CLASS_ID_MASK) | "

#define CTC_CLI_SCL_KEY_PORT_DESC \
    CTC_CLI_GPORT_DESC,                     \
    CTC_CLI_GPORT_ID_DESC,                  \
    "Global Port Mask",                     \
    "Logic Port",                           \
    "Logic Port Value",                     \
    "Logic Port Mask",                      \
    "Port Class SCL",                       \
    CTC_CLI_SCL_PORT_CLASS_ID_VALUE,        \
    "Port Class Mask"

#define CTC_CLI_SCL_KEY_PORT_SET()\
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                                       \
    if (INDEX_VALID(index))                                                                        \
    {                                                                                              \
        CTC_CLI_GET_UINT32("gport", p_key->port_data.gport, argv[index + 1]);                      \
        CTC_CLI_GET_UINT32("gport mask", p_key->port_mask.gport, argv[index + 2]);                 \
        p_key->port_data.type = CTC_FIELD_PORT_TYPE_GPORT;                                         \
    }                                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("key-logic-port");                                                  \
    if (INDEX_VALID(index))                                                                        \
    {                                                                                              \
        CTC_CLI_GET_UINT16("logic port", p_key->port_data.logic_port, argv[index + 1]);            \
        CTC_CLI_GET_UINT16("logic port mask", p_key->port_mask.logic_port, argv[index + 2]);       \
        p_key->port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;                                    \
    }                                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("port-class");                                                  \
    if (INDEX_VALID(index))                                                                        \
    {                                                                                              \
        CTC_CLI_GET_UINT16("port class", p_key->port_data.port_class_id, argv[index + 1]);         \
        CTC_CLI_GET_UINT16("port class mask", p_key->port_mask.port_class_id, argv[index + 2]);    \
        p_key->port_data.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;                                    \
    }

#define CTC_CLI_SCL_KEY_VLAN_STR \
    " cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI | \
      svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI "

#define CTC_CLI_SCL_KEY_VLAN_DESC \
    "Customer VLAN",              \
    CTC_CLI_VLAN_RANGE_DESC,      \
    CTC_CLI_VLAN_RANGE_MASK,      \
    "Customer VLAN CoS",          \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Customer VLAN CFI",          \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Service VLAN",               \
    CTC_CLI_VLAN_RANGE_DESC,      \
    CTC_CLI_VLAN_RANGE_MASK,      \
    "Service VLAN CoS",           \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Service VLAN CFI",           \
    CTC_CLI_CFI_RANGE_DESC

#define CTC_CLI_SCL_KEY_VLAN_SET(TYPE)  \
    index = CTC_CLI_GET_ARGC_INDEX("cvlan");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("cvlan", p_key->cvlan, argv[index + 1]);                     \
        CTC_CLI_GET_UINT16("cvlan mask", p_key->cvlan_mask, argv[index + 2]);           \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_CVLAN);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-cos", p_key->ctag_cos, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("ctag-cos mask", p_key->ctag_cos_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_CTAG_COS);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);                \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_CTAG_CFI);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("svlan");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("svlan", p_key->svlan, argv[index + 1]);                     \
        CTC_CLI_GET_UINT16("svlan mask", p_key->svlan_mask, argv[index + 2]);           \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_SVLAN);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-cos", p_key->stag_cos, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("stag-cos mask", p_key->stag_cos_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_STAG_COS);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-cfi", p_key->stag_cfi, argv[index + 1]);                \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_STAG_CFI);                      \
    }


#define CTC_CLI_SCL_KEY_IP_COM_STR \
    "|dscp DSCP MASK | eth-type ETH_TYPE MASK | l4-type L4_TYPE MASK | ip-option IP_OPTION | ip-frag IP_FRAG | prec IP_PREC MASK "
#define CTC_CLI_SCL_KEY_IPV4_COM_STR  \
    "|l4-protocol L4_PROTOCOL MASK {l4-src-port L4_SRC_PORT MASK|l4-dst-port L4_DST_PORT MASK| \
    icmp-type ICMP_TYPE MASK| icmp-code ICMP_CODE MASK | gre-key GRE_KEY MASK (is-nvgre|) | igmp-type IGMP_TYPE  MASK | vnid VNID MASK |} "

#define CTC_CLI_SCL_KEY_IPV6_COM_STR  \
    "|l4-protocol L4_PROTOCOL MASK {l4-src-port L4_SRC_PORT MASK|l4-dst-port L4_DST_PORT MASK| \
    icmp-type ICMP_TYPE MASK| icmp-code ICMP_CODE MASK | gre-key GRE_KEY MASK (is-nvgre|) | vnid VNID MASK |} "

#define CTC_CLI_SCL_KEY_IPV4_STR  \
    "|ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D| \
     sender-ip A.B.C.D A.B.C.D | target-ip A.B.C.D A.B.C.D " \
      CTC_CLI_SCL_KEY_IPV4_COM_STR CTC_CLI_SCL_KEY_IP_COM_STR

#define CTC_CLI_SCL_KEY_IPV6_STR  \
    "|ip-sa X:X::X:X X:X::X:X | ip-da X:X::X:X X:X::X:X | flow-label FLOW_LABEL MASK " \
    CTC_CLI_SCL_KEY_IPV6_COM_STR CTC_CLI_SCL_KEY_IP_COM_STR


#define CTC_CLI_SCL_KEY_IP_COM_DESC \
    "Dscp",                       \
    CTC_CLI_DSCP_VALUE,           \
    CTC_CLI_DSCP_MASK,            \
    "Ether type",                 \
    CTC_CLI_ETHER_TYPE_VALUE,     \
    CTC_CLI_ETHER_TYPE_MASK,      \
    "Layer 4 Type",               \
    CTC_CLI_PARSER_L4_TYPE_VALUE, \
    CTC_CLI_PARSER_L4_TYPE_MASK,  \
    "Ip option",                  \
    CTC_CLI_IP_OPTION,            \
    "Ip fragment information",    \
    CTC_CLI_FRAG_VALUE,           \
    "Ip precedence",              \
    CTC_CLI_IP_PREC_VALUE,        \
    CTC_CLI_IP_PREC_MASK

#define CTC_CLI_SCL_KEY_IPV4_COM_DESC \
    "L4 protocol ",               \
    CTC_CLI_L4_PROTOCOL_VALUE,    \
    CTC_CLI_L4_PROTOCOL_MASK,     \
    "Layer4 source port",         \
    CTC_CLI_L4_PORT_VALUE,        \
    CTC_CLI_L4_PORT_VALUE,        \
    "Layer4 destination port",    \
    CTC_CLI_L4_PORT_VALUE,        \
    CTC_CLI_L4_PORT_VALUE,        \
    "Icmp type",                  \
    CTC_CLI_ICMP_TYPE_VALUE,      \
    CTC_CLI_ICMP_TYPE_MASK,       \
    "Icmp code",                  \
    CTC_CLI_ICMP_CODE_VALUE,      \
    CTC_CLI_ICMP_CODE_MASK,       \
    "Gre key",                    \
    CTC_CLI_GRE_KEY_VALUE,        \
    CTC_CLI_GRE_KEY_MASK,         \
    "is nvgre",                    \
    "Igmp type",                    \
    CTC_CLI_IGMP_TYPE_VALUE,        \
    CTC_CLI_IGMP_TYPE_MASK,         \
    "Vnid",                    \
    CTC_CLI_VNID_VALUE,        \
    CTC_CLI_VNID_MASK

#define CTC_CLI_SCL_KEY_IPV6_COM_DESC \
    "L4 protocol ",               \
    CTC_CLI_L4_PROTOCOL_VALUE,    \
    CTC_CLI_L4_PROTOCOL_MASK,     \
    "Layer4 source port",         \
    CTC_CLI_L4_PORT_VALUE,        \
    CTC_CLI_L4_PORT_VALUE,        \
    "Layer4 destination port",    \
    CTC_CLI_L4_PORT_VALUE,        \
    CTC_CLI_L4_PORT_VALUE,        \
    "Icmp type",                  \
    CTC_CLI_ICMP_TYPE_VALUE,      \
    CTC_CLI_ICMP_TYPE_MASK,       \
    "Icmp code",                  \
    CTC_CLI_ICMP_CODE_VALUE,      \
    CTC_CLI_ICMP_CODE_MASK,       \
    "Gre key",                    \
    CTC_CLI_GRE_KEY_VALUE,        \
    CTC_CLI_GRE_KEY_MASK,         \
    "is nvgre",                 \
    "Vnid",                    \
    CTC_CLI_VNID_VALUE,        \
    CTC_CLI_VNID_MASK


#define CTC_CLI_SCL_KEY_IPV4_DESC   \
    "IPv4 source address",          \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "IPv4 destination address",     \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "Sender Ip address",            \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "Target Ip address",            \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    CTC_CLI_SCL_KEY_IPV4_COM_DESC,  \
    CTC_CLI_SCL_KEY_IP_COM_DESC


#define CTC_CLI_SCL_KEY_IP_COM_SET(TYPE) \
    index = CTC_CLI_GET_ARGC_INDEX("dscp");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("dscp", p_key->dscp, argv[index + 1]);                       \
        CTC_CLI_GET_UINT8("dscp mask", p_key->dscp_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_DSCP);                         \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");                                         \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-frag", p_key->ip_frag, argv[index + 1]);                   \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_IP_FRAG);                      \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-option");                                       \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-option", p_key->ip_option, argv[index + 1]);               \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_IP_OPTION);                    \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("eth-type");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("eth-type", p_key->eth_type, argv[index + 1]);               \
        CTC_CLI_GET_UINT16("eth-type-mask", p_key->eth_type_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_ETH_TYPE);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("l4-type");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("l4-type", p_key->l4_type, argv[index + 1]);               \
        CTC_CLI_GET_UINT16("l4-type-mask", p_key->l4_type_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_L4_TYPE);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("prec");                                            \
    if (INDEX_VALID(index))                                                                \
    {                                                                                      \
        CTC_CLI_GET_UINT8("ip precedence", p_key->dscp, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("ip precedence mask", p_key->dscp_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_PRECEDENCE);                      \
    }


#define CTC_CLI_SCL_KEY_IPV4_COM_SET(TYPE)       \
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");                                     \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("l4-protocol", p_key->l4_protocol, argv[index + 1]);           \
        CTC_CLI_GET_UINT8("l4-protocol-mask", p_key->l4_protocol_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_L4_PROTOCOL);                  \
        index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");                                 \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT16("l4-src-port", p_key->l4_src_port, argv[index + 1]);              \
            CTC_CLI_GET_UINT16("l4-src-port-mask", p_key->l4_src_port_mask, argv[index + 2]);    \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_L4_SRC_PORT);      \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");                                 \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT16("l4-dst-port", p_key->l4_dst_port, argv[index + 1]);              \
            CTC_CLI_GET_UINT16("l4-dst-port-mask", p_key->l4_dst_port_mask, argv[index + 2]);    \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_L4_DST_PORT);      \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("icmp-type");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("icmp-type", p_key->icmp_type, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("icmp-type-mask", p_key->icmp_type_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_ICMP_TYPE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("icmp-code");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("icmp-code", p_key->icmp_code, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("icmp-code-mask", p_key->icmp_code_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_ICMP_CODE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("igmp-type");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("igmp-type", p_key->igmp_type, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("igmp-type-mask", p_key->igmp_type_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_IGMP_TYPE);        \
        }                                                                           \
        index = CTC_CLI_GET_ARGC_INDEX("vnid");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT32("vnid", p_key->vni, argv[index + 1]);           \
            CTC_CLI_GET_UINT32("vnid-mask", p_key->vni_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_VNI);        \
        }                                                                               \
        index =  CTC_CLI_GET_ARGC_INDEX("gre-key");                                     \
        if (INDEX_VALID(index))                                                        \
        {                                                                               \
            CTC_CLI_GET_UINT32("gre-key", p_key->gre_key, argv[index + 1]);              \
            CTC_CLI_GET_UINT32("gre-key mask", p_key->gre_key_mask, argv[index + 2]);    \
            index = CTC_CLI_GET_ARGC_INDEX("is-nvgre");                                 \
            if (INDEX_VALID(index))                                                     \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_NVGRE_KEY);   \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_GRE_KEY);     \
            }                                                                        \
        }                                                                            \
    }

#define CTC_CLI_SCL_KEY_IPV6_COM_SET(TYPE)       \
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");                                     \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("l4-protocol", p_key->l4_protocol, argv[index + 1]);           \
        CTC_CLI_GET_UINT8("l4-protocol-mask", p_key->l4_protocol_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_##TYPE##_KEY_FLAG_L4_PROTOCOL);                  \
        index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");                                 \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT16("l4-src-port", p_key->l4_src_port, argv[index + 1]);              \
            CTC_CLI_GET_UINT16("l4-src-port-mask", p_key->l4_src_port_mask, argv[index + 2]);    \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_L4_SRC_PORT);      \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");                                 \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT16("l4-dst-port", p_key->l4_dst_port, argv[index + 1]);              \
            CTC_CLI_GET_UINT16("l4-dst-port-mask", p_key->l4_dst_port_mask, argv[index + 2]);    \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_L4_DST_PORT);      \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("icmp-type");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("icmp-type", p_key->icmp_type, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("icmp-type mask", p_key->icmp_type_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_ICMP_TYPE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("icmp-code");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("icmp-code", p_key->icmp_code, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("icmp-code mask", p_key->icmp_code_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_ICMP_CODE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("vnid");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT32("vnid", p_key->vni, argv[index + 1]);           \
            CTC_CLI_GET_UINT32("vnid-mask", p_key->vni_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_VNI);        \
        }                                                                           \
        index =  CTC_CLI_GET_ARGC_INDEX("gre-key");                                     \
        if (INDEX_VALID(index))                                                        \
        {                                                                               \
            CTC_CLI_GET_UINT32("gre-key", p_key->gre_key, argv[index + 1]);              \
            CTC_CLI_GET_UINT32("gre-key mask", p_key->gre_key_mask, argv[index + 2]);    \
            index = CTC_CLI_GET_ARGC_INDEX("is-nvgre");                                 \
            if (INDEX_VALID(index))                                                     \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_NVGRE_KEY);   \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_##TYPE##_KEY_SUB_FLAG_GRE_KEY);     \
            }                                                                        \
        }                                                                            \
    }


#define CTC_CLI_SCL_KEY_IPV4_SET()   \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[index + 1]);               \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa-mask", p_key->ip_sa_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_SA);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da, argv[index + 1]);               \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da-mask", p_key->ip_da_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_DA);                             \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("sender-ip");                                        \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_IPV4_ADDRESS("sender-ip", p_key->sender_ip, argv[index + 1]);         \
        CTC_CLI_GET_IPV4_ADDRESS("sender-ip-mask", p_key->sender_ip_mask, argv[index + 2]); \
        SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP);                             \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("target-ip");                                        \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_IPV4_ADDRESS("target-ip", p_key->target_ip, argv[index + 1]);          \
        CTC_CLI_GET_IPV4_ADDRESS("target-ip-mask", p_key->target_ip_mask, argv[index + 2]); \
        SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP);                             \
    }                                                                                   \
    CTC_CLI_SCL_KEY_IPV4_COM_SET(IPV4) \
    CTC_CLI_SCL_KEY_IP_COM_SET(IPV4)


#define CTC_CLI_SCL_KEY_IPV6_DESC \
    "IPv6 source address",          \
    CTC_CLI_IPV6_FORMAT,            \
    CTC_CLI_IPV6_MASK_FORMAT,       \
    "IPv6 destination address",     \
    CTC_CLI_IPV6_FORMAT,            \
    CTC_CLI_IPV6_MASK_FORMAT,       \
    "Ipv6 Flow label",              \
    CTC_CLI_IPV6_FLOW_LABEL_VALUE,  \
    CTC_CLI_IPV6_FLOW_LABEL_MASK,   \
    CTC_CLI_SCL_KEY_IPV6_COM_DESC,  \
    CTC_CLI_SCL_KEY_IP_COM_DESC

#define CTC_CLI_SCL_KEY_IPV6_SET()         \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                          \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-sa", ipv6_address, argv[index + 1]);               \
        p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);                                 \
        p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);                                 \
        p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);                                 \
        p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-sa-mask", ipv6_address, argv[index + 2]);          \
        p_key->ip_sa_mask[0] = sal_htonl(ipv6_address[0]);                            \
        p_key->ip_sa_mask[1] = sal_htonl(ipv6_address[1]);                            \
        p_key->ip_sa_mask[2] = sal_htonl(ipv6_address[2]);                            \
        p_key->ip_sa_mask[3] = sal_htonl(ipv6_address[3]);                            \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_SA);                       \
    }                                                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                          \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-da", ipv6_address, argv[index + 1]);             \
        p_key->ip_da[0] = sal_htonl(ipv6_address[0]);                                 \
        p_key->ip_da[1] = sal_htonl(ipv6_address[1]);                                 \
        p_key->ip_da[2] = sal_htonl(ipv6_address[2]);                                 \
        p_key->ip_da[3] = sal_htonl(ipv6_address[3]);                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-da-mask", ipv6_address, argv[index + 2]);        \
        p_key->ip_da_mask[0] = sal_htonl(ipv6_address[0]);                            \
        p_key->ip_da_mask[1] = sal_htonl(ipv6_address[1]);                            \
        p_key->ip_da_mask[2] = sal_htonl(ipv6_address[2]);                            \
        p_key->ip_da_mask[3] = sal_htonl(ipv6_address[3]);                            \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_DA);                       \
    }                                                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("flow-label");                                   \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("flow-label", p_key->flow_label, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("flow-label-mask", p_key->flow_label_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_FLOW_LABEL);        \
    }                                                                               \
    CTC_CLI_SCL_KEY_IPV6_COM_SET(IPV6) \
    CTC_CLI_SCL_KEY_IP_COM_SET(IPV6)

#define CTC_CLI_SCL_KEY_HASH_COMMON_0_STR   \
    "(gport GPORT_ID | class-id CLASS_ID | logic-port LOGIC_PORT_VALUE) "

#define CTC_CLI_SCL_KEY_HASH_COMMON_0_DESC  \
    CTC_CLI_GPORT_DESC,                     \
    CTC_CLI_GPORT_ID_DESC,                  \
    "Class id",                \
    CTC_CLI_SCL_PORT_CLASS_ID_VALUE,         \
    "Logic Port",                           \
    "Logic Port Value"

#define CTC_CLI_SCL_KEY_HASH_COMMON_0_SET()   \
    index = CTC_CLI_GET_ARGC_INDEX("entry");                                         \
    if (CLI_CLI_STR_EQUAL("gport", index + 2))                                                           \
    {                                                                                 \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 3]);             \
        p_key->gport_type = 0;                                                  \
    }                                                                                 \
    if (CLI_CLI_STR_EQUAL("class-id", index + 2))                                                           \
    {                                                                                  \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 3]);             \
        p_key->gport_type = 1;                                                  \
    }                                                                                   \
    if (CLI_CLI_STR_EQUAL("logic-port", index + 2))                                                           \
    {                                                                                  \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 3]);             \
        p_key->gport_type = 2;                                                  \
    }                                                                           \

#define CTC_CLI_SCL_KEY_HASH_COMMON_1_STR   \
    "(gport GPORT_ID | class-id CLASS_ID | logic-port LOGIC_PORT_VALUE) direction (ingress|egress) "

#define CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC  \
    CTC_CLI_SCL_KEY_HASH_COMMON_0_DESC,     \
    "Direction",                            \
    "Ingress direction",                    \
    "Egress direction"

#define CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()   \
    index = CTC_CLI_GET_ARGC_INDEX("entry");                                           \
    if (CLI_CLI_STR_EQUAL("gport", index + 2))                                                           \
    {                                                                                  \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 3]);                    \
        p_key->gport_type = 0;                                                         \
        if (CLI_CLI_STR_EQUAL("ingress", index + 4))                                  \
        {                                                                              \
            p_key->dir = CTC_INGRESS;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            p_key->dir = CTC_EGRESS;                                                   \
        }                                                                              \
    }                                                                                  \
    if (CLI_CLI_STR_EQUAL("class-id", index + 2))                                                           \
    {                                                                                  \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 3]);                    \
        p_key->gport_type = 1;                                                         \
        if (CLI_CLI_STR_EQUAL("ingress", index + 4))                                  \
        {                                                                              \
            p_key->dir = CTC_INGRESS;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            p_key->dir = CTC_EGRESS;                                                   \
        }                                                                              \
    }                                                                                  \
    if (CLI_CLI_STR_EQUAL("logic-port", index + 2))                                                           \
    {                                                                                  \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 3]);                    \
        p_key->gport_type = 2;                                                         \
        if (CLI_CLI_STR_EQUAL("ingress", index + 4))                                  \
        {                                                                              \
            p_key->dir = CTC_INGRESS;                                                  \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            p_key->dir = CTC_EGRESS;                                                   \
        }                                                                              \
    }                                                                                  \

#define CTC_CLI_SCL_KEY_HASH_SVLAN_STR  \
    " (svlan VLAN_ID) "

#define CTC_CLI_SCL_KEY_HASH_SVLAN_DESC \
    "Service VLAN",                     \
    CTC_CLI_VLAN_RANGE_DESC

#define CTC_CLI_SCL_KEY_HASH_SVLAN_SET()  \
    index = CTC_CLI_GET_ARGC_INDEX("svlan");                                \
    if (INDEX_VALID(index))                                                \
    {                                                                       \
        CTC_CLI_GET_UINT16("svlan", p_key->svlan, argv[index + 1]);         \
    }

#define CTC_CLI_SCL_KEY_HASH_CVLAN_STR  \
    " (cvlan VLAN_ID) "

#define CTC_CLI_SCL_KEY_HASH_CVLAN_DESC \
    "Customer VLAN",                     \
    CTC_CLI_VLAN_RANGE_DESC

#define CTC_CLI_SCL_KEY_HASH_CVLAN_SET()  \
    index = CTC_CLI_GET_ARGC_INDEX("cvlan");                                     \
    if (INDEX_VALID(index))                                                      \
    {                                                                            \
        CTC_CLI_GET_UINT16("cvlan", p_key->cvlan, argv[index + 1]);              \
    }

#define CTC_CLI_SCL_HASH_FIELD_L2_KEY_STR                                        \
    "{mac-da | mac-sa | eth-type | class-id | vlan | cos | tag-valid | cfi |}"

#define CTC_CLI_SCL_HASH_FIELD_MAC_KEY_DESC \
    "MAC destination address",              \
    "MAC source address",                   \
    "Ether type",                           \
    "Class id",                             \
    "Vlan id",                              \
    "Cos",                                  \
    "Tag Valid",                            \
    "CFI"

#define CTC_CLI_SCL_HASH_FIELD_L2_KEY_SET()                                       \
    pf->mac_da      = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da"));     \
    pf->mac_sa      = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa"));     \
    pf->eth_type    = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("eth-type"));   \
    pf->class_id    = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("class-id"));   \
    pf->vlan_id     = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan"));       \
    pf->cos         = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("cos"));        \
    pf->tag_valid   = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("tag-valid"));  \
    pf->cfi         = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("cfi"));        \

/*action*/
#define CTC_CLI_SCL_ACTION_COMMON_STR   \
    " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|) \
                (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|) \
                (tpid-index TPID_INDEX |) \
    | discard | stats STATS_ID | aging "


#define CTC_CLI_SCL_ACTION_COMMON_DESC  \
    "Vlan Edit",                            \
    "Stag operation type",                  \
    CTC_CLI_TAG_OP_DESC,           \
    "Svid select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New stag vlan id",                     \
    CTC_CLI_VLAN_RANGE_DESC,                \
    "Scos select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New stag cos",                         \
    CTC_CLI_COS_RANGE_DESC,                 \
    "Scfi select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New stag cfi",                         \
    "<0-1>",                                  \
    "Ctag operation type",                  \
    CTC_CLI_TAG_OP_DESC,           \
    "Cvid select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New ctag vlan id",                     \
    CTC_CLI_VLAN_RANGE_DESC,                \
    "Ccos select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New ctag cos",                         \
    CTC_CLI_COS_RANGE_DESC,                 \
    "Ccfi select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New ctag cfi",                         \
    "<0-1>",                                  \
    "Set svlan tpid index",                 \
    "Index, the corresponding svlan tpid is in EpeL2TpidCtl" \
    "Discard the packet",                   \
    "Statistic",                            \
    "0~0xFFFFFFFF",                            \
    "Aging"

#define CTC_CLI_SCL_ACTION_COMMON_SET(p_dir_action,TYPE)   \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit");                                              \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        index = CTC_CLI_GET_ARGC_INDEX("stag-op");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            SET_FLAG(p_dir_action->flag, CTC_SCL_##TYPE##_ACTION_FLAG_VLAN_EDIT);                   \
            CTC_CLI_GET_UINT8("stag-op", p_dir_action->vlan_edit.stag_op, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("svid-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("svid-sl", p_dir_action->vlan_edit.svid_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-svid");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-svid", p_dir_action->vlan_edit.svid_new, argv[index + 1]);      \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("scos-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scos-sl", p_dir_action->vlan_edit.scos_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-scos");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scos", p_dir_action->vlan_edit.scos_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("scfi-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scfi-sl", p_dir_action->vlan_edit.scfi_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-scfi");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scfi", p_dir_action->vlan_edit.scfi_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ctag-op");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            SET_FLAG(p_dir_action->flag, CTC_SCL_##TYPE##_ACTION_FLAG_VLAN_EDIT);                   \
            CTC_CLI_GET_UINT8("ctag-op", p_dir_action->vlan_edit.ctag_op, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("cvid-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("cvid-sl", p_dir_action->vlan_edit.cvid_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-cvid");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-cvid", p_dir_action->vlan_edit.cvid_new, argv[index + 1]);      \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ccos-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccos-sl", p_dir_action->vlan_edit.ccos_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-ccos");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccos", p_dir_action->vlan_edit.ccos_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ccfi-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccfi-sl", p_dir_action->vlan_edit.ccfi_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-ccfi");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccfi", p_dir_action->vlan_edit.ccfi_new, argv[index + 1]);         \
        }                                                                                        \
        index = CTC_CLI_GET_ARGC_INDEX("tpid-index");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("tpid-index", p_dir_action->vlan_edit.tpid_index, argv[index + 1]);         \
        }                                                                                        \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("discard");                                              \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                       \
        SET_FLAG(p_dir_action->flag, CTC_SCL_##TYPE##_ACTION_FLAG_DISCARD);                     \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("stats");                                              \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                       \
        SET_FLAG(p_dir_action->flag, CTC_SCL_##TYPE##_ACTION_FLAG_STATS);                     \
        CTC_CLI_GET_UINT32("stats-id", p_dir_action->stats_id, argv[index + 1]);         \
    }



#define CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
    "ingress-action { priority PRIORITY | color COLOR | priority-and-color PRIORITY COLOR | (service-id SERVICEID | policer-id POLICERID) (service-acl-en|) \
    (service-queue-en|) (service-policer-en|) | copy-to-cpu| fid FID | vrf VRFID | stp-id STPID | dscp DSCP | redirect NHID | vpls (learning-en |) (mac-limit-en |) | \
    logic-port GPHYPORT_ID (logic-port-type |) | etree-leaf | \
    user-vlanptr VLAN_PTR | bind TYPE (mac-sa MAC|) (ipv4-sa A.B.C.D|) (gport GPHYPORT_ID|) (vlan-id VLAN_PTR|) | \
    aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) |" \
    CTC_CLI_SCL_ACTION_COMMON_STR "|}"

#define CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC \
    "Ingress action",              \
    "Priority",                           \
    CTC_CLI_PRIORITY_VALUE,               \
    "Color",                              \
    CTC_CLI_COLOR_VALUE,                  \
    "Priority and color",                 \
    CTC_CLI_PRIORITY_VALUE,               \
    CTC_CLI_COLOR_VALUE,                  \
    "Service Id",                         \
    CTC_CLI_SCL_SERVICE_ID_VALUE,         \
    "Policer id",                           \
    CTC_CLI_QOS_FLOW_PLC_VAL,        \
    "Enable service acl",                 \
    "Enable service queue",               \
    "Enable service policer",             \
    "Copy to cpu",                        \
    CTC_CLI_FID_DESC,                     \
    CTC_CLI_FID_ID_DESC,                  \
    CTC_CLI_VRFID_DESC,           \
    CTC_CLI_VRFID_ID_DESC,     \
    "Stp Id",                         \
    "<0-127>",                                  \
    "Dscp",                         \
    CTC_CLI_DSCP_VALUE,                                  \
    "Redirect to",                        \
    CTC_CLI_NH_ID_STR,                    \
    "Vpls",                               \
    "Enable vsi learning",                \
    "Enable vsi mac security",            \
    "Logic port",                         \
    CTC_CLI_GPHYPORT_ID_DESC,             \
    "Logic-port-type",                    \
    "It is leaf node in E-Tree networks", \
    "User vlanptr",                       \
    CTC_CLI_USER_VLAN_PTR,                \
    "Bind",                               \
    "0-4: 0 bind port; 1 bind vlan; 2 bind ipv4-sa; 3 bind ipv4-sa and vlan; 4 bind mac-sa", \
    "MAC source address",                 \
    CTC_CLI_MAC_FORMAT,                   \
    "IPv4 source address",                \
    CTC_CLI_IPV4_FORMAT,                  \
    CTC_CLI_GPORT_DESC,                   \
    CTC_CLI_GPORT_ID_DESC,                \
    "Vlan id",                            \
    CTC_CLI_VLAN_RANGE_DESC,              \
    "Aps selector",                       \
    CTC_CLI_APS_GROUP_ID_DESC,            \
    "Protected vlan, maybe working vlan", \
    CTC_CLI_VLAN_RANGE_DESC,              \
    "This is working path",               \
    "This is protection path",            \
    CTC_CLI_SCL_ACTION_COMMON_DESC

#define CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR           \
    "egress-action { " CTC_CLI_SCL_ACTION_COMMON_STR " | priority-and-color PRIORITY COLOR |}"

#define CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC          \
    "Egress action",               \
    CTC_CLI_SCL_ACTION_COMMON_DESC,         \
    "Priority and color",                 \
    CTC_CLI_PRIORITY_VALUE,               \
    CTC_CLI_COLOR_VALUE

#define CTC_CLI_SCL_EGRESS_ACTION_SET(p_dir_action)    \
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");                         \
    if (INDEX_VALID(index))                                                  \
    {                                                                        \
        SET_FLAG(p_action->type, CTC_SCL_ACTION_EGRESS);                     \
        CTC_CLI_SCL_ACTION_COMMON_SET(p_dir_action,EGS)                      \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("priority-and-color");                    \
    if (INDEX_VALID(index))                                                  \
    {                                                                        \
        SET_FLAG(p_dir_action->flag, CTC_SCL_EGS_ACTION_FLAG_PRIORITY_AND_COLOR);    \
        CTC_CLI_GET_UINT8("color_id", p_dir_action->color, argv[index + 2]);         \
        CTC_CLI_GET_UINT8("priority_id", p_dir_action->priority, argv[index + 1]);   \
    }                                                                                \

#define CTC_CLI_SCL_INGRESS_ACTION_SET(p_dir_action)                         \
{ \
    uint8 start = 0; \
    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");                        \
    if (INDEX_VALID(index))                                                 \
    {                                                                        \
        SET_FLAG(p_action->type, CTC_SCL_ACTION_INGRESS);                    \
        CTC_CLI_SCL_ACTION_COMMON_SET(p_dir_action,IGS)                      \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("priority-and-color");                    \
    if (INDEX_VALID(index))                                                  \
    {                                                                        \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_PRIORITY_AND_COLOR);    \
        CTC_CLI_GET_UINT8("color_id", p_dir_action->color, argv[index + 2]);         \
        CTC_CLI_GET_UINT8("priority_id", p_dir_action->priority, argv[index + 1]);   \
    }                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");                              \
    if (INDEX_VALID(index))                                                          \
    {                                                                                \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_PRIORITY);              \
        CTC_CLI_GET_UINT8("priority_id", p_dir_action->priority, argv[index + 1]);   \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("color");                          \
    if (INDEX_VALID(index))                                                    \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_COLOR); \
        CTC_CLI_GET_UINT8("color_id", p_dir_action->color, argv[index + 1]);   \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("service-id");                             \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_SERVICE_ID);     \
        CTC_CLI_GET_UINT16("service_id",p_dir_action->service_id, argv[index + 1]);   \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("policer-id");                             \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        CTC_CLI_GET_UINT16("service_id",p_dir_action->policer_id, argv[index + 1]);   \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");                                \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_ACL_EN);        \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("service-policer-en");                        \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_POLICER_EN);\
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("service-queue-en");                        \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->sub_flag, CTC_SCL_IGS_ACTION_SUB_FLAG_SERVICE_QUEUE_EN);\
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");                            \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_COPY_TO_CPU);        \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("redirect");                                \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_REDIRECT);           \
        CTC_CLI_GET_UINT32("redirect", p_dir_action->nh_id, argv[index + 1]);      \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("fid");                                    \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_FID);                \
        CTC_CLI_GET_UINT16("fid", p_dir_action->fid, argv[index + 1]);            \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("vrf");                                    \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_VRFID);                \
        CTC_CLI_GET_UINT16("vrf", p_dir_action->vrf_id, argv[index + 1]);            \
    }                                                                            \
    index = CTC_CLI_GET_ARGC_INDEX("stp-id");                                    \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_STP_ID);                \
        CTC_CLI_GET_UINT16("stp-id", p_dir_action->stp_id, argv[index + 1]);            \
    }                      \
     index = CTC_CLI_GET_ARGC_INDEX("dscp");                                    \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_DSCP);                \
        CTC_CLI_GET_UINT16("dscp", p_dir_action->dscp, argv[index + 1]);            \
    }                      \
    index = CTC_CLI_GET_ARGC_INDEX("vpls");                           \
    if (0xFF != index)                                                    \
    {                                                                     \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_VPLS);           \
    }                                                                     \
    index = CTC_CLI_GET_ARGC_INDEX("learning-en");                  \
    if (0xFF != index)                                                    \
    {                                                                     \
        p_dir_action->vpls.learning_en = 1;                              \
    }                                                                     \
    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-en");                        \
    if (0xFF != index)                                                    \
    {                                                                     \
        p_dir_action->vpls.mac_limit_en = 1;                              \
    }                                                                     \
    index = CTC_CLI_GET_ARGC_INDEX("etree-leaf");                                \
    if (0xFF != index)                                                        \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_ETREE_LEAF);     \
    }                                                                         \
    start = CTC_CLI_GET_ARGC_INDEX("ingress-action");                        \
    index = CTC_CLI_GET_SPECIFIC_INDEX("logic-port",start);                \
    if (INDEX_VALID(index))                                                   \
    {                                                                         \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_LOGIC_PORT);         \
        CTC_CLI_GET_UINT16("logic-port", p_dir_action->logic_port.logic_port, argv[index + start + 1]);     \
        index = CTC_CLI_GET_ARGC_INDEX("logic-port-type");                    \
        if (0xFF != index)                                                    \
        {                                                                     \
            p_dir_action->logic_port.logic_port_type = 1;                         \
        }                                                                     \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");                            \
    if (0xFF != index)                                                         \
    {                                                                          \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);              \
        CTC_CLI_GET_UINT16("user-vlanptr", p_dir_action->user_vlanptr, argv[index + 1]); \
    }                                                                          \
    index = CTC_CLI_GET_ARGC_INDEX("bind");                                    \
    if (0xFF != index)                                                         \
    {                                                                          \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_BINDING_EN);          \
        CTC_CLI_GET_UINT16("bind-type", p_dir_action->bind.type, argv[index + 1]); \
        index = CTC_CLI_GET_ARGC_INDEX("mac-sa");                              \
        if (0xFF != index)                                                     \
        {                                                                      \
           CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_dir_action->bind.mac_sa, argv[index + 1]);\
        }                                                                      \
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-sa");                             \
        if (0xFF != index)                                                     \
        {                                                                      \
           CTC_CLI_GET_IPV4_ADDRESS("ipv4-sa", p_dir_action->bind.ipv4_sa, argv[index + 1]);\
        }                                                                      \
        index = CTC_CLI_GET_ARGC_INDEX("gport");                               \
        if (0xFF != index)                                                     \
        {                                                                      \
           CTC_CLI_GET_UINT16("gport", p_dir_action->bind.gport, argv[index + 1]);    \
        }                                                                      \
        index = CTC_CLI_GET_ARGC_INDEX("vlan-id");                             \
        if (0xFF != index)                                                     \
        {                                                                      \
           CTC_CLI_GET_UINT16("vlan-id", p_dir_action->bind.vlan_id, argv[index + 1]);\
        }                                                                      \
    }                                                                          \
    index = CTC_CLI_GET_ARGC_INDEX("aps-select");                              \
    if (0xFF != index)                                                         \
    {                                                                          \
        CTC_CLI_GET_UINT16("aps select group", p_dir_action->aps.aps_select_group_id, argv[index + 1]); \
        p_dir_action->aps.protected_vlan_valid = 0;                            \
        index = CTC_CLI_GET_ARGC_INDEX("protected-vlan");                      \
        if (0xFF != index)                                                     \
        {                                                                      \
            CTC_CLI_GET_UINT16("aps protected vlan", p_dir_action->aps.protected_vlan, argv[index + 1]); \
            p_dir_action->aps.protected_vlan_valid = 1;                        \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            p_dir_action->aps.protected_vlan = 0;                              \
        }                                                                      \
        index = CTC_CLI_GET_ARGC_INDEX("working-path");                        \
        if (0xFF != index)                                                     \
        {                                                                      \
            p_dir_action->aps.is_working_path = 1;                             \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            p_dir_action->aps.is_working_path = 0;                             \
        }                                                                      \
        SET_FLAG(p_dir_action->flag, CTC_SCL_IGS_ACTION_FLAG_APS);             \
    }                                                                          \
}

#define CTC_CLI_SCL_ACTION_FLOW_ACTION_STR  \
        "flow-action {stats STATS_ID | discard | deny-bridge| deny-learning | deny-route        \
         | force-bridge | force-learning | force-route | priority PRIORITY | color COLOR         \
         | priority-and-color PRIORITY COLOR | trust TRUST | micro-flow-policer MICRO_POLICER_ID \
         | macro-flow-policer MACRO_POLICER_ID | copy-to-cpu | postcard-en | redirect NH_ID      \
         | acl-tcam-en {tcam-type TCAM_TYPE | tcam-label TCAM_LABEL |}                           \
         | acl-hash-en {hash-type HASH_TYPE | sel-id FIELD_SEL_ID|} | overwrite-acl-tcam   \
         | overwrite-acl-hash | metadata METADATA |fid FID |vrf VRFID | overwrite-port-ipfix (lkup-type TYPE field-sel-id ID) \
         |force-decap offset-base-type (l2 | l3 | l4) ext-offset OFFSET payload-type TYPE |}"

#define CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC  \
        "Flow action",                       \
        CTC_CLI_STATS_ID_DESC,               \
        CTC_CLI_STATS_ID_VAL,                \
        "Discard",                           \
        "Deny bridge",                       \
        "Deny learning",                     \
        "Deny route",                        \
        "Force bridge",                      \
        "Force learning",                    \
        "Force route",                       \
        "Priority",                          \
        CTC_CLI_PRIORITY_VALUE,              \
        "Color",                             \
        CTC_CLI_COLOR_VALUE,                 \
        "Priority and color",                \
        CTC_CLI_PRIORITY_VALUE,              \
        CTC_CLI_COLOR_VALUE,                 \
        "Trust",                             \
        CTC_CLI_TRUST_VALUE,                 \
        "Micro flow policer",                \
        CTC_CLI_MICRO_FLOW_POLICER_ID_VALUE, \
        "Macro flow policer",                \
        CTC_CLI_MACRO_FLOW_POLICER_ID_VALUE, \
        "Copy to cpu",                       \
        "postcard-en",                       \
        "redirect nh id",                    \
        CTC_CLI_NH_ID_STR,                   \
        "Acl tcam en",                       \
        "Tcam type",                         \
        CTC_CLI_ACL_TCAM_TYPE_VALUE,                             \
        "Tcam label",                        \
        "<0-0xFF>",                          \
        "Acl hash en",                       \
        "Hash type",                         \
        CTC_CLI_ACL_HASH_TYPE_VALUE,                             \
        CTC_CLI_FIELD_SEL_ID_DESC,   \
        CTC_CLI_FIELD_SEL_ID_VALUE,  \
        "Overwrite acl tcam",                \
        "Overwrite acl hash",                \
        "Metadata",                 \
        "metadata: <0-0x3FFF>",              \
        CTC_CLI_FID_DESC,                \
        CTC_CLI_FID_ID_DESC,          \
        CTC_CLI_VRFID_DESC,           \
        CTC_CLI_VRFID_ID_DESC,     \
        "Overwrite port ipfix",                \
        "Ipfix lookup packet type",        \
        "0-Disable, 1-Only L2 Field, 2-Only L3 Field, 3 - L2+ L3 Field",  \
        CTC_CLI_FIELD_SEL_ID_DESC,       \
        CTC_CLI_FIELD_SEL_ID_VALUE,    \
        "Force decap", \
        "Offset base type", \
        "Layer2 offset, refer to ctc_scl_offset_base_type_t", \
        "Layer3 offset, refer to ctc_scl_offset_base_type_t", \
        "Layer4 offset, refer to ctc_scl_offset_base_type_t", \
        "Extend offset", \
        "Extend offset value", \
        "Payload type", \
        "0: Ethernet; 1: IPv4; 3: IPv6, refer to packet_type_t"

#define CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action)                                            \
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");                                                   \
    if (INDEX_VALID(index))                                                                         \
    {                                                                                                \
        SET_FLAG(p_action->type, CTC_SCL_ACTION_FLOW);                                               \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");                                                 \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("stats id", p_flow_action->stats_id, argv[index + 1]);                    \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_STATS);                           \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("discard");                                               \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DISCARD);                         \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny-bridge");                                           \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_BRIDGE);                     \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny-learning");                                         \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_LEARNING);                   \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny-route");                                            \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_ROUTE);                      \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("force-bridge");                                          \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_BRIDGE);                    \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("force-learning");                                        \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_LEARNING);                  \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("force-route");                                           \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_ROUTE);                     \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority-and-color");                                    \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("priority", p_flow_action->priority, argv[index + 1]);                    \
        CTC_CLI_GET_UINT32("color", p_flow_action->color, argv[index + 2]);                          \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_PRIORITY);                        \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_COLOR);                           \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");                                              \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("priority", p_flow_action->priority, argv[index + 1]);                    \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_PRIORITY);                        \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("color");                                                 \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("color", p_flow_action->color, argv[index + 1]);                          \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_COLOR);                           \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("trust");                                                 \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("trust", p_flow_action->trust, argv[index + 1]);                          \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_TRUST);                           \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("micro-flow-policer");                                    \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("micro-flow-policer", p_flow_action->micro_policer_id, argv[index + 1]);  \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_MICRO_FLOW_POLICER);              \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("macro-flow-policer");                                    \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("macro-flow-policer", p_flow_action->macro_policer_id, argv[index + 1]);  \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_MACRO_FLOW_POLICER);              \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("copy-to-cpu");                                           \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_COPY_TO_CPU);                     \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("postcard-en");                                           \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_POSTCARD_EN);                     \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("redirect");                                              \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("nexthop-id", p_flow_action->nh_id, argv[index + 1]);  \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_REDIRECT);                        \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-tcam-en");                                           \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_ACL_FLOW_TCAM_EN);                \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcam-type");                                             \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("tcam-type", p_flow_action->acl.acl_tcam_lkup_type, argv[index + 1]);     \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcam-label");                                            \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("tcam-label", p_flow_action->acl.acl_tcam_lkup_class_id, argv[index + 1]);   \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-hash-en");                                           \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_ACL_FLOW_HASH_EN);                \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("hash-type");                                             \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("hash-type", p_flow_action->acl.acl_hash_lkup_type, argv[index + 1]);     \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("sel-id");                                          \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("field-sel-id", p_flow_action->acl.acl_hash_field_sel_id, argv[index + 1]);  \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-acl-tcam");                                    \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_OVERWRITE_ACL_FLOW_TCAM);         \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-acl-hash");                                    \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_OVERWRITE_ACL_FLOW_HASH);         \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("metadata");                                     \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("metadata", p_flow_action->metadata, argv[index + 1]);                    \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_METADATA);                        \
    }                                                                      \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("fid");                                     \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT16("fid", p_flow_action->fid, argv[index + 1]);                    \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FID);                        \
    }                                                                      \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vrf");                                     \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT16("vrf id", p_flow_action->vrf_id, argv[index + 1]);                    \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_VRFID);                        \
    }                                                                      \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-port-ipfix");                                    \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_OVERWRITE_PORT_IPFIX);         \
    }                                                                                               \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lkup-type");                                             \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("hash-type", p_flow_action->ipfix_lkup_type, argv[index + 1]);         \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("field-sel-id");                                          \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("field-sel-id", p_flow_action->ipfix_field_sel_id, argv[index + 1]);      \
    }                                                                                                \
    index = CTC_CLI_GET_ARGC_INDEX("force-decap");\
    if (0xFF != index) \
    {\
        CTC_SET_FLAG(p_flow_action->flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_DECAP);         \
        p_flow_action->force_decap.force_decap_en = 1;\
        index = CTC_CLI_GET_ARGC_INDEX("offset-base-type");\
        if (0xFF != index) \
        {\
            if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))\
            {\
                p_flow_action->force_decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L2;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))\
            {\
                p_flow_action->force_decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L3;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l4",index + 1))\
            {\
                p_flow_action->force_decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L4;\
            }\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ext-offset");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("ext-offset", p_flow_action->force_decap.ext_offset, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("payload-type");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("payload-type", p_flow_action->force_decap.payload_type, argv[index + 1]); \
        }\
    }

/*******************************************\*********************
 *           scl set (code)   -begin-                           *
 ****************************************************************/
static ctc_scl_entry_t scl_entry_tmp;

STATIC void
cli_scl_dump_group_info(ctc_scl_group_info_t* ginfo)
{
    char* type = NULL;

    if ((ginfo->type >= CTC_SCL_GROUP_TYPE_MAX))
    {
        ctc_cli_out("get scl group info wrong!\n");
    }

    switch (ginfo->type)
    {

    case CTC_SCL_GROUP_TYPE_NONE:
        type = "NONE";
        break;


    case CTC_SCL_GROUP_TYPE_GLOBAL:
        type = "GLOBAL";
        break;

    case CTC_SCL_GROUP_TYPE_PORT:
        type = "PORT";
        break;

    case CTC_SCL_GROUP_TYPE_LOGIC_PORT:
        type = "LOGIC_PORT";
        break;

    case CTC_SCL_GROUP_TYPE_HASH:
        type = "HASH";
        break;

    case CTC_SCL_GROUP_TYPE_PORT_CLASS:
        ctc_cli_out("port-class  : %d\n", ginfo->un.port_class_id);
        ctc_cli_out("lchip       : %d\n", ginfo->lchip);

        type = "PORT_CLASS";
        break;

    default:
        type = "XXXX";
        break;

    }

    ctc_cli_out("group-type  : %s\n", type);

    return;

}

CTC_CLI(ctc_cli_scl_create_group,
        ctc_cli_scl_create_group_cmd,
        "scl create group GROUP_ID (priority PRIORITY |) (type\
        (port GPORT| global | port-class CLASS_ID (lchip LCHIP|) | logic-port LOGIC_PORT | hash | none)|)",
        CTC_CLI_SCL_STR,
        "Create SCL group",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE,
        CTC_CLI_SCL_GROUP_PRIO_STR,
        CTC_CLI_SCL_GROUP_PRIO_VALUE,
        "Group type",
        "Port scl, care gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Global scl, mask ports",
        "Port class scl, against a class(group) of port",
        CTC_CLI_SCL_PORT_CLASS_ID_VALUE,
        "Lchip",
        CTC_CLI_LCHIP_ID_VALUE,
        "Logic Port scl, care logic port",
        "Logic port value",
        "Care port info by entry")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 group_id = 0;
    ctc_scl_group_info_t ginfo;

    sal_memset(&ginfo, 0, sizeof(ctc_scl_group_info_t));

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

    ginfo.type = CTC_SCL_GROUP_TYPE_GLOBAL;
    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("group prioirty", ginfo.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("gport", ginfo.un.gport, argv[index + 1]);
        ginfo.type = CTC_SCL_GROUP_TYPE_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("logic-port", ginfo.un.logic_port, argv[index + 1]);
        ginfo.type = CTC_SCL_GROUP_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("lchip id", ginfo.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("global");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_SCL_GROUP_TYPE_GLOBAL;

    }

    index = CTC_CLI_GET_ARGC_INDEX("hash");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_SCL_GROUP_TYPE_HASH;

    }


    index = CTC_CLI_GET_ARGC_INDEX("none");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_SCL_GROUP_TYPE_NONE;
    }


    index = CTC_CLI_GET_ARGC_INDEX("port-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("port class id", ginfo.un.port_class_id, argv[index + 1]);
        ginfo.type  = CTC_SCL_GROUP_TYPE_PORT_CLASS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_create_group(g_api_lchip, group_id, &ginfo);
    }
    else
    {
        ret = ctc_scl_create_group(group_id, &ginfo);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_destroy_group,
        ctc_cli_scl_destroy_group_cmd,
        "scl destroy group GROUP_ID",
        CTC_CLI_SCL_STR,
        "Destroy SCL group",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 group_id = 0;

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_destroy_group(g_api_lchip, group_id);
    }
    else
    {
        ret = ctc_scl_destroy_group(group_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/****************************************************************
 *           scl description    -end-                           *
 ****************************************************************/
 CTC_CLI(ctc_cli_scl_set_default_action,
        ctc_cli_scl_set_default_action_cmd,
        "scl default-action (gport GPORT_ID) "\
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " ) {scl-id SCL_ID|}",
        CTC_CLI_SCL_STR,
        "Set default entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        "Scl id", "SCL ID")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_default_action_t def_action;
    ctc_scl_action_t*     p_action =&def_action.action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;

    sal_memset(&def_action, 0, sizeof(ctc_scl_default_action_t));

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("gport", def_action.gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(def_action.action.u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(def_action.action.u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("scl id", def_action.scl_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_set_default_action(g_api_lchip, &def_action);
    }
    else
    {
        ret = ctc_scl_set_default_action(&def_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}



 CTC_CLI(ctc_cli_scl_add_port_hash_entry,
        ctc_cli_scl_add_port_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR         \
        " port-hash-entry "                    \
        CTC_CLI_SCL_KEY_HASH_COMMON_1_STR      \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_key_t* p_key = &(scl_entry->key.u.hash_port_key);
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT;

    CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

 CTC_CLI(ctc_cli_scl_add_port_cvlan_hash_entry,
        ctc_cli_scl_add_port_cvlan_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR         \
        " port-cvlan-hash-entry "              \
        CTC_CLI_SCL_KEY_HASH_COMMON_1_STR      \
        CTC_CLI_SCL_KEY_HASH_CVLAN_STR         \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port cvlan hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC,
        CTC_CLI_SCL_KEY_HASH_CVLAN_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_cvlan_key_t* p_key = &scl_entry->key.u.hash_port_cvlan_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_CVLAN;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_CVLAN;

    CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()
    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("cvlan", p_key->cvlan, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

 CTC_CLI(ctc_cli_scl_add_port_svlan_hash_entry,
        ctc_cli_scl_add_port_svlan_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-svlan-hash-entry "              \
        CTC_CLI_SCL_KEY_HASH_COMMON_1_STR      \
        CTC_CLI_SCL_KEY_HASH_SVLAN_STR         \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port svlan hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC,
        CTC_CLI_SCL_KEY_HASH_SVLAN_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_svlan_key_t* p_key = &scl_entry->key.u.hash_port_svlan_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_SVLAN;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_SVLAN;
    CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()
    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("svlan", p_key->svlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_port_dvlan_hash_entry,
        ctc_cli_scl_add_port_dvlan_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-double-vlan-hash-entry "              \
        CTC_CLI_SCL_KEY_HASH_COMMON_1_STR      \
        CTC_CLI_SCL_KEY_HASH_SVLAN_STR         \
        CTC_CLI_SCL_KEY_HASH_CVLAN_STR         \
        " ( "                          \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port double vlan hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC,
        CTC_CLI_SCL_KEY_HASH_SVLAN_DESC,
        CTC_CLI_SCL_KEY_HASH_CVLAN_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_2vlan_key_t* p_key = &scl_entry->key.u.hash_port_2vlan_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_2VLAN;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
    CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()
    CTC_CLI_SCL_KEY_HASH_SVLAN_SET()
    CTC_CLI_SCL_KEY_HASH_CVLAN_SET()

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_port_cvlan_cos_hash_entry,
        ctc_cli_scl_add_port_cvlan_cos_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-cvlan-cos-hash-entry "              \
        CTC_CLI_SCL_KEY_HASH_COMMON_1_STR      \
        CTC_CLI_SCL_KEY_HASH_CVLAN_STR         \
        " (ctag-cos COS) "                     \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port cvlan cos hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC,
        CTC_CLI_SCL_KEY_HASH_CVLAN_DESC,
        "Customer VLAN CoS",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_cvlan_cos_key_t* p_key = &scl_entry->key.u.hash_port_cvlan_cos_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_CVLAN_COS;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_CVLAN_COS;
    CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()
    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("cvlan", p_key->cvlan, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("ctag-cos", p_key->ctag_cos, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_scl_add_port_svlan_cos_hash_entry,
        ctc_cli_scl_add_port_svlan_cos_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-svlan-cos-hash-entry "              \
        CTC_CLI_SCL_KEY_HASH_COMMON_1_STR      \
        CTC_CLI_SCL_KEY_HASH_SVLAN_STR         \
        " (stag-cos COS) "                     \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port svlan cos hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_1_DESC,
        CTC_CLI_SCL_KEY_HASH_SVLAN_DESC,
        "Service VLAN CoS",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_svlan_cos_key_t* p_key = &(scl_entry->key.u.hash_port_svlan_cos_key);
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_SVLAN_COS;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_SVLAN_COS;
    CTC_CLI_SCL_KEY_HASH_COMMON_1_SET()
    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("svlan", p_key->svlan, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("stag-cos", p_key->stag_cos, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_mac_hash_entry,
        ctc_cli_scl_add_mac_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " mac-hash-entry "                  \
        " mac MAC "                       \
        " mac-is-da MAC_IS_DA "             \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Mac hash entry",
        "MAC address",
        CTC_CLI_MAC_FORMAT,
        "Mac is destination address or not ",
        "<0-1>",
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_mac_key_t* p_key = &scl_entry->key.u.hash_mac_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_MAC;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_MAC;
    CTC_CLI_GET_MAC_ADDRESS("mac", p_key->mac, argv[index+2]);
    CTC_CLI_GET_UINT8("mac-is-da", p_key->mac_is_da, argv[index+3]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_port_mac_hash_entry,
        ctc_cli_scl_add_port_mac_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-mac-hash-entry "                  \
        CTC_CLI_SCL_KEY_HASH_COMMON_0_STR   \
        " (mac MAC) "                            \
        " (mac-is-da MAC_IS_DA) "                \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port mac hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_0_DESC,
        "MAC address",
        CTC_CLI_MAC_FORMAT,
        "Mac is destination address or not ",
        "<0-1>",
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_mac_key_t* p_key = &scl_entry->key.u.hash_port_mac_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_MAC;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_MAC;
    CTC_CLI_SCL_KEY_HASH_COMMON_0_SET()

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_MAC_ADDRESS("mac", p_key->mac, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac-is-da");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("mac-is-da", p_key->mac_is_da, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_ipv4_hash_entry,
        ctc_cli_scl_add_ipv4_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " ipv4-hash-entry "                 \
        " ip-sa A.B.C.D "                   \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Ipv4 hash entry",
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_ipv4_key_t* p_key = &scl_entry->key.u.hash_ipv4_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_IPV4;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_IPV4;

    CTC_CLI_GET_IPV4_ADDRESS("ip_sa", p_key->ip_sa, argv[index+2]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_port_ipv4_hash_entry,
        ctc_cli_scl_add_port_ipv4_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-ipv4-hash-entry "            \
        CTC_CLI_SCL_KEY_HASH_COMMON_0_STR   \
        " (ip-sa A.B.C.D) "                   \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port ipv4 hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_0_DESC,
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_ipv4_key_t* p_key = &scl_entry->key.u.hash_port_ipv4_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_IPV4;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_IPV4;

    CTC_CLI_SCL_KEY_HASH_COMMON_0_SET()
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_IPV4_ADDRESS("ip_sa", p_key->ip_sa, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_ipv6_hash_entry,
        ctc_cli_scl_add_ipv6_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR       \
        " ipv6-hash-entry "                  \
        " ip-sa X:X::X:X "                   \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Ipv6 hash entry",
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_ipv6_key_t* p_key = &scl_entry->key.u.hash_ipv6_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    ipv6_addr_t ipv6_address;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_IPV6;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_IPV6;

    CTC_CLI_GET_IPV6_ADDRESS("ip_sa", ipv6_address, argv[index+2]);
    p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);
    p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);
    p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);
    p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_port_ipv6_hash_entry,
        ctc_cli_scl_add_port_ipv6_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR      \
        " port-ipv6-hash-entry "            \
        CTC_CLI_SCL_KEY_HASH_COMMON_0_STR   \
       " (ip-sa X:X::X:X) "                   \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "Port ipv6 hash entry",
        CTC_CLI_SCL_KEY_HASH_COMMON_0_DESC,
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_port_ipv6_key_t* p_key = &scl_entry->key.u.hash_port_ipv6_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    ipv6_addr_t ipv6_address;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_PORT_IPV6;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_PORT_IPV6;

    CTC_CLI_SCL_KEY_HASH_COMMON_0_SET()

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_IPV6_ADDRESS("ip_sa", ipv6_address, argv[index+1]);
        p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);
        p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);
        p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);
        p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_l2_hash_entry,
        ctc_cli_scl_add_l2_hash_entry_cmd,
        CTC_CLI_SCL_ADD_HASH_ENTRY_STR         \
        " l2-hash-entry "                      \
        "{mac-sa MAC | mac-da MAC | (gport GPORT_ID | class-id CLASS_ID) | vlan-id VLAN_ID | field-sel-id SEL_ID | eth-type ETH_TYPE | tag-valid VALID | cos COS | cfi CFI |}" \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_HASH_ENTRY_DESC,
        "L2 hash entry",
        "MAC source address",
        CTC_CLI_MAC_FORMAT,
        "MAC destination address",
        CTC_CLI_MAC_FORMAT,
        "Global port",
        CTC_CLI_GPORT_ID_DESC,
        "Class id",
        CTC_CLI_SCL_PORT_CLASS_ID_VALUE,
        "Vlan id",
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        "<0-3>",
        CTC_CLI_ETHTYPE_DESC,
        "<0-0xFFFF>",
        "Tag valid",
        "<0-1>",
        "Cos",
        CTC_CLI_COS_RANGE_DESC,
        "Cfi",
        CTC_CLI_CFI_RANGE_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_hash_l2_key_t* p_key = &scl_entry->key.u.hash_l2_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    uint32 gid = CTC_SCL_GROUP_ID_HASH_L2;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_HASH_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_HASH_L2;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("class-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("class-id", p_key->gport, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("vlan-id", p_key->vlan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cos");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("cos", p_key->cos, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("field-sel-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("field-sel-id", p_key->field_sel_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("eth-type");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("eth-type", p_key->eth_type, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tag-valid");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("tag-valid", p_key->tag_valid, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cfi");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("cfi", p_key->cfi, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_mac_entry,
        ctc_cli_scl_add_mac_entry_cmd,
        CTC_CLI_SCL_ADD_ENTRY_STR      \
        " mac-entry {"                  \
        CTC_CLI_SCL_KEY_PORT_STR       \
        CTC_CLI_SCL_KEY_VLAN_STR       \
        " | stag-valid | ctag-valid | ether-type VALUE MASK |" \
        CTC_CLI_SCL_KEY_MAC_STR        \
        " |}"                          \
        " ( "                          \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_ENTRY_DESC,
        "Mac tcam entry",
        CTC_CLI_SCL_KEY_PORT_DESC,
        CTC_CLI_SCL_KEY_VLAN_DESC,
        "Stag valid", "Ctag valid", "ether type ","value" , "mask",
        CTC_CLI_SCL_KEY_MAC_DESC,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_tcam_mac_key_t* p_key = &scl_entry->key.u.tcam_mac_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_TCAM_MAC;
    CTC_CLI_SCL_KEY_PORT_SET()
    CTC_CLI_SCL_KEY_MAC_SET(MAC)
    CTC_CLI_SCL_KEY_VLAN_SET(MAC)
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");
    if (INDEX_VALID(index))
    {
        p_key->stag_valid = 1;
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_MAC_KEY_FLAG_STAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");
    if (INDEX_VALID(index))
    {
        p_key->ctag_valid = 1;
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_MAC_KEY_FLAG_CTAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ether-type");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("value", p_key->eth_type, argv[index + 1]);
        CTC_CLI_GET_UINT16("mask", p_key->eth_type_mask, argv[index + 1]);
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_MAC_KEY_FLAG_ETH_TYPE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_scl_add_ipv4_entry,
        ctc_cli_scl_add_ipv4_entry_cmd,
        CTC_CLI_SCL_ADD_ENTRY_STR       \
        " ipv4-entry (key-size (single|double)|) {" \
        CTC_CLI_SCL_KEY_PORT_STR       \
        CTC_CLI_SCL_KEY_VLAN_STR        \
        "| stag-valid | ctag-valid |" \
        CTC_CLI_SCL_KEY_MAC_STR         \
        CTC_CLI_SCL_KEY_IPV4_STR        \
        " | arp-op-code VALUE MASK | arp-protocol-type VALUE MASK | ecn VALUE MASK |}"                          \
        " ( "                           \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_ENTRY_DESC,
        "Ipv4 tcam entry",
        "Key size",
        "Single width",
        "Double width",
        CTC_CLI_SCL_KEY_PORT_DESC,
        CTC_CLI_SCL_KEY_VLAN_DESC,
        "Stag valid", "Ctag valid",
        CTC_CLI_SCL_KEY_MAC_DESC,
        CTC_CLI_SCL_KEY_IPV4_DESC,
        "arp op code", "value" , "mask", "arp protocol type", "value" , "mask", "Ecn", "Ecn Value", "Ecn Mask",
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_tcam_ipv4_key_t* p_key = &scl_entry->key.u.tcam_ipv4_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_TCAM_IPV4;

    index = CTC_CLI_GET_ARGC_INDEX("key-size");
    p_key->key_size = CTC_SCL_KEY_SIZE_DOUBLE;
    if (INDEX_VALID(index))
    {
        if (CLI_CLI_STR_EQUAL("double", (index+1)))
        {
            p_key->key_size = CTC_SCL_KEY_SIZE_DOUBLE;
        }
        else if (CLI_CLI_STR_EQUAL("single", (index+1)))
        {
            p_key->key_size = CTC_SCL_KEY_SIZE_SINGLE;
        }
    }
    CTC_CLI_SCL_KEY_PORT_SET()

    CTC_CLI_SCL_KEY_IPV4_SET()
    CTC_CLI_SCL_KEY_MAC_SET(IPV4)
    CTC_CLI_SCL_KEY_VLAN_SET(IPV4)
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");
    if (INDEX_VALID(index))
    {
        p_key->stag_valid = 1;
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_STAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");
    if (INDEX_VALID(index))
    {
        p_key->ctag_valid = 1;
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CTAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("arp-op-code");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("value", p_key->arp_op_code, argv[index + 1]);
        CTC_CLI_GET_UINT16("mask", p_key->arp_op_code_mask, argv[index + 2]);
        SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_OP_CODE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("arp-protocol-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("value", p_key->arp_protocol_type, argv[index + 1]);
        CTC_CLI_GET_UINT16("mask", p_key->arp_protocol_type_mask, argv[index + 2]);
        SET_FLAG(p_key->sub_flag, CTC_SCL_TCAM_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("ecn-value", p_key->ecn, argv[index + 1]);
        CTC_CLI_GET_UINT8("ecn-mask", p_key->ecn_mask, argv[index + 2]);
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_ECN);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_scl_add_ipv6_entry,
        ctc_cli_scl_add_ipv6_entry_cmd,
        CTC_CLI_SCL_ADD_ENTRY_STR              \
        " ipv6-entry {"                       \
        CTC_CLI_SCL_KEY_PORT_STR       \
        CTC_CLI_SCL_KEY_VLAN_STR               \
        " | stag-valid | ctag-valid |" \
        CTC_CLI_SCL_KEY_MAC_STR                \
        CTC_CLI_SCL_KEY_IPV6_STR               \
        " | ecn VALUE MASK |}"                                 \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_ADD_ENTRY_DESC,
        "Ipv6 tcam entry",
        CTC_CLI_SCL_KEY_PORT_DESC,
        CTC_CLI_SCL_KEY_VLAN_DESC,
        "Stag valid", "Ctag valid",
        CTC_CLI_SCL_KEY_MAC_DESC,
        CTC_CLI_SCL_KEY_IPV6_DESC,
        "Ecn", "Ecn Value", "Ecn Mask",
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_tcam_ipv6_key_t* p_key = &scl_entry->key.u.tcam_ipv6_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;
    ipv6_addr_t ipv6_address;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_TCAM_IPV6;
    CTC_CLI_SCL_KEY_PORT_SET()
    CTC_CLI_SCL_KEY_IPV6_SET()
    CTC_CLI_SCL_KEY_MAC_SET(IPV6)
    CTC_CLI_SCL_KEY_VLAN_SET(IPV6)
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");
    if (INDEX_VALID(index))
    {
        p_key->stag_valid = 1;
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_STAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");
    if (INDEX_VALID(index))
    {
        p_key->ctag_valid = 1;
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CTAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("ecn-value", p_key->ecn, argv[index + 1]);
        CTC_CLI_GET_UINT8("ecn-mask", p_key->ecn_mask, argv[index + 2]);
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_ECN);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_add_vlan_entry,
        ctc_cli_scl_add_vlan_entry_cmd,
        CTC_CLI_SCL_ADD_ENTRY_STR      \
        " vlan-entry {" \
        CTC_CLI_SCL_KEY_PORT_STR       \
        CTC_CLI_SCL_KEY_VLAN_STR       \
        "| stag-valid STAG_VALID | ctag-valid CTAG_VALID | customer-id CUSTOMER_ID MASK"  \
        " |}"                                         \
        " ( "                                          \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR          \
        " | "                                          \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR           \
        " | "                                          \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR             \
        " ) ",
        CTC_CLI_SCL_ADD_ENTRY_DESC,
        "Vlan tcam entry",
        CTC_CLI_SCL_KEY_PORT_DESC,
        CTC_CLI_SCL_KEY_VLAN_DESC,
        "Stag valid",
        "<0-1>",
        "Ctag valid",
        "<0-1>",
        "Customer id",
        "<0-0xFFFFFFFF>",
        "<0-0xFFFFFFFF>",
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_scl_entry_t* scl_entry = &scl_entry_tmp;
    ctc_scl_tcam_vlan_key_t* p_key = &scl_entry->key.u.tcam_vlan_key;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    CTC_CLI_SCL_ADD_ENTRY_SET()
    scl_entry->key.type = CTC_SCL_KEY_TCAM_VLAN;
    CTC_CLI_SCL_KEY_PORT_SET()
    CTC_CLI_SCL_KEY_VLAN_SET(VLAN)
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("stag-valid", p_key->stag_valid, argv[index + 1]);
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_VLAN_KEY_FLAG_STAG_VALID);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("ctag-valid", p_key->ctag_valid, argv[index + 1]);
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_VLAN_KEY_FLAG_CTAG_VALID);
    }

    index = CTC_CLI_GET_ARGC_INDEX("customer-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT32("customer-id", p_key->customer_id, argv[index + 1]);
        CTC_CLI_GET_UINT32("customer-id mask", p_key->customer_id_mask, argv[index + 2]);
        SET_FLAG(p_key->flag, CTC_SCL_TCAM_VLAN_KEY_FLAG_CUSTOMER_ID);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_scl_remove_entry,
        ctc_cli_scl_remove_entry_cmd,
        "scl remove entry ENTRY_ID",
        CTC_CLI_SCL_STR,
        "Remove one entry from software table",
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_remove_entry(g_api_lchip, eid);
    }
    else
    {
        ret = ctc_scl_remove_entry(eid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_remove_all_entry,
        ctc_cli_scl_remove_all_entry_cmd,
        "scl remove-all group GROUP_ID",
        CTC_CLI_SCL_STR,
        "Remove all entries of a specified group from software table",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 gid = 0;

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_remove_all_entry(g_api_lchip, gid);
    }
    else
    {
        ret = ctc_scl_remove_all_entry(gid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_set_entry_priority,
        ctc_cli_scl_set_entry_priority_cmd,
        "scl entry ENTRY_ID priority PRIORITY",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        CTC_CLI_SCL_ENTRY_PRIO_STR,
        CTC_CLI_SCL_ENTRY_PRIO_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;
    uint32 prio = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);
    CTC_CLI_GET_UINT32("entry priority", prio, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_set_entry_priority(g_api_lchip, eid, prio);
    }
    else
    {
        ret = ctc_scl_set_entry_priority(eid, prio);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_update_action,
        ctc_cli_scl_update_action_cmd,
        "scl update entry ENTRY_ID " \
        " ( "                                  \
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_STR  \
        " | "                                  \
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_STR   \
        " | "                                  \
        CTC_CLI_SCL_ACTION_FLOW_ACTION_STR     \
        " ) ",
        CTC_CLI_SCL_STR,
        "Update action",
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        CTC_CLI_SCL_ACTION_INGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_EGRESS_ACTION_DESC,
        CTC_CLI_SCL_ACTION_FLOW_ACTION_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 eid = 0;
    ctc_scl_action_t   action;
    ctc_scl_action_t*  p_action = &action;
    ctc_scl_igs_action_t* p_ingress_action;
    ctc_scl_egs_action_t* p_egress_action;
    ctc_scl_flow_action_t* p_flow_action;

    sal_memset(&action, 0, sizeof(ctc_scl_action_t));

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        p_ingress_action = &(p_action->u.igs_action);
        CTC_CLI_SCL_INGRESS_ACTION_SET(p_ingress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        p_egress_action = &(p_action->u.egs_action);
        CTC_CLI_SCL_EGRESS_ACTION_SET(p_egress_action)
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        p_flow_action = &(p_action->u.flow_action);
        CTC_CLI_SCL_ACTION_FLOW_ACTION_SET(p_flow_action);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_update_action(g_api_lchip, eid, p_action);
    }
    else
    {
        ret = ctc_scl_update_action(eid, p_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_get_multi_entry,
        ctc_cli_scl_get_multi_entry_cmd,
        "show scl group GROUP_ID size ENTRY_SIZE ",
        "Show",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE,
        "Entry size",
        "0: show entry count in this group, > 0: show entry IDs in that number")
{

    int32 ret = CLI_SUCCESS;
    uint32 idx = 0;
    ctc_scl_query_t query;
    uint32* p_array = NULL;

    sal_memset(&query, 0, sizeof(query));
    CTC_CLI_GET_UINT32("group id", query.group_id, argv[0]);
    CTC_CLI_GET_UINT32("entry size", query.entry_size, argv[1]);

    p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * query.entry_size);
    query.entry_array = p_array;

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_get_multi_entry(g_api_lchip, &query);
    }
    else
    {
        ret = ctc_scl_get_multi_entry(&query);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (query.entry_size == 0)
    {
        ctc_cli_out("entry count of group 0x%-8X : %u\n", query.group_id, query.entry_count);
    }
    else
    {
        ctc_cli_out("get %u entry-ids of group 0x%-8X\n", query.entry_count, query.group_id);
        ctc_cli_out("%-3s   %-17s\n", "No.", "ENTRY_ID");
        ctc_cli_out("-------------------\n");
        for (idx = 0; idx < query.entry_count; idx++)
        {
            ctc_cli_out("%-3u   0x%-8X \n", idx + 1, query.entry_array[idx]);
        }
    }

    mem_free(p_array);

    return ret;
}

CTC_CLI(ctc_cli_scl_copy_entry,
        ctc_cli_scl_copy_entry_cmd,
        "scl entry ENTRY_ID copy-to group GROUP_ID entry ENTRY_ID ",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "Copy to",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    ctc_scl_copy_entry_t copy_entry;

    sal_memset(&copy_entry, 0, sizeof(copy_entry));

    CTC_CLI_GET_UINT32("source entry id", copy_entry.src_entry_id, argv[0]);
    CTC_CLI_GET_UINT32("dest group id", copy_entry.dst_group_id, argv[1]);
    CTC_CLI_GET_UINT32("dest entry id", copy_entry.dst_entry_id, argv[2]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_copy_entry(g_api_lchip, &copy_entry);
    }
    else
    {
        ret = ctc_scl_copy_entry(&copy_entry);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_install_group,
        ctc_cli_scl_install_group_cmd,
        "scl install group GROUP_ID (type (port GPORT| global | port-class CLASS_ID (lchip LCHIP|) |"\
        "logic-port LOGIC_PORT)|)",
        CTC_CLI_SCL_STR,
        "Install to hardware",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE,
        "Group type",
        "Port scl, care gport",
        CTC_CLI_GPORT_ID_DESC,
        "Global scl, mask ports",
        "Port class scl, against a class(group) of port",
        CTC_CLI_SCL_PORT_CLASS_ID_VALUE,
        "Lchip",
        CTC_CLI_LCHIP_ID_VALUE,
        "Logic Port scl, care logic port",
        "Logic port value")
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_scl_group_info_t ginfo;
    ctc_scl_group_info_t* pinfo = NULL;

    sal_memset(&ginfo, 0, sizeof(ctc_scl_group_info_t));

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    ginfo.type = CTC_SCL_GROUP_TYPE_GLOBAL;
    pinfo = &ginfo;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("gport", ginfo.un.gport, argv[index + 1]);
        ginfo.type = CTC_SCL_GROUP_TYPE_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("logic-port", ginfo.un.logic_port, argv[index + 1]);
        ginfo.type = CTC_SCL_GROUP_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("lchip id", ginfo.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("global");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_SCL_GROUP_TYPE_GLOBAL;

    }

    index = CTC_CLI_GET_ARGC_INDEX("port-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("port class id", ginfo.un.port_class_id, argv[index + 1]);
        ginfo.type  = CTC_SCL_GROUP_TYPE_PORT_CLASS;

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_install_group(g_api_lchip, gid, pinfo);
    }
    else
    {
        ret = ctc_scl_install_group(gid, pinfo);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_uninstall_group,
        ctc_cli_scl_uninstall_group_cmd,
        "scl uninstall group GROUP_ID",
        CTC_CLI_SCL_STR,
        "Uninstall from hardware",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 gid = 0;

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_uninstall_group(g_api_lchip, gid);
    }
    else
    {
        ret = ctc_scl_uninstall_group(gid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_install_entry,
        ctc_cli_scl_install_entry_cmd,
        "scl install entry ENTRY_ID",
        CTC_CLI_SCL_STR,
        "Install to hardware",
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_install_entry(g_api_lchip, eid);
    }
    else
    {
        ret = ctc_scl_install_entry(eid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_uninstall_entry,
        ctc_cli_scl_uninstall_entry_cmd,
        "scl uninstall entry ENTRY_ID",
        CTC_CLI_SCL_STR,
        "Uninstall from hardware",
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_uninstall_entry(g_api_lchip, eid);
    }
    else
    {
        ret = ctc_scl_uninstall_entry(eid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_scl_show_group_info,
        ctc_cli_scl_show_group_info_cmd,
        "show scl group-info group GROUP_ID (INDEX  (INDEX_END |) |)",
        "Show",
        CTC_CLI_SCL_STR,
        "Group info",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE,
        "Show entry start from that number",
        "Show entry end at that number")
{
    int32 ret = CLI_SUCCESS;
    uint32 gid = 0;
    ctc_scl_group_info_t ginfo;
    uint32 idx = 0;
    ctc_scl_query_t query;
    uint32* p_array = NULL;
    uint32 s_index = 0;
    uint32 e_index = 0xFFFFFFFF;
    uint32 entry_cnt = 0;

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);
    sal_memset(&ginfo, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&query, 0, sizeof(query));
    query.group_id = gid;

    p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * query.entry_size);
    query.entry_array = p_array;

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_get_group_info(g_api_lchip, gid, &ginfo);
    }
    else
    {
        ret = ctc_scl_get_group_info(gid, &ginfo);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        mem_free(p_array);
        return CLI_ERROR;
    }
    else
    {
        cli_scl_dump_group_info(&ginfo);
        if(g_ctcs_api_en)
    {
        ret = ctcs_scl_get_multi_entry(g_api_lchip, &query);
    }
    else
    {
        ret = ctc_scl_get_multi_entry(&query);
    }

        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            mem_free(p_array);
            return CLI_ERROR;
        }

        entry_cnt = query.entry_count;
        ctc_cli_out("entry count : %u\n", query.entry_count);
        mem_free(p_array);
        if (1 == argc )
        {
            return ret;
        }
    }

    if (2 <= argc )
    {
        CTC_CLI_GET_UINT32("start index", s_index, argv[1]);
    }

    if (3 <= argc)
    {
        CTC_CLI_GET_UINT32("end index", e_index, argv[2]);
    }

    sal_memset(&query, 0, sizeof(query));
    query.group_id = gid;
    query.entry_size  = entry_cnt;
    p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * query.entry_size);
    query.entry_array = p_array;

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_get_multi_entry(g_api_lchip, &query);
    }
    else
    {
        ret = ctc_scl_get_multi_entry(&query);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        mem_free(p_array);
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-8s%s\n", "No.", "Member-Entry");
    ctc_cli_out("--------------------\n");
    entry_cnt = 0;
    for (idx = 0; idx < query.entry_count; idx++)
    {
        if ((s_index <= query.entry_array[idx])
            && (e_index >= query.entry_array[idx]) )
        {
            ctc_cli_out("%-8u%u\n", entry_cnt, query.entry_array[idx]);
            entry_cnt ++;
        }
    }

    mem_free(p_array);

    return ret;
}

CTC_CLI(ctc_cli_scl_debug_on,
        ctc_cli_scl_debug_on_cmd,
        "debug scl (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SCL_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint8 level = CTC_DEBUG_LEVEL_INFO;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        ctc_debug_set_flag("scl", "scl", SCL_CTC, level, TRUE);

    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        ctc_debug_set_flag("scl", "scl", SCL_SYS, level, TRUE);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_scl_debug_off,
        ctc_cli_scl_debug_off_cmd,
        "no debug scl (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SCL_STR,
        "Ctc layer",
        "Sys layer",
        "SCL Label",
        "SCL entry")
{
    uint8 level = CTC_DEBUG_LEVEL_INFO;

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        ctc_debug_set_flag("scl", "scl", SCL_CTC, level, FALSE);

    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        ctc_debug_set_flag("scl", "scl", SCL_SYS, level, FALSE);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_scl_debug_show,
        ctc_cli_scl_debug_show_cmd,
        "show debug scl",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SCL_STR)
{
    uint8 level = CTC_DEBUG_LEVEL_NONE;
    uint8 en   = 0;

    en = ctc_debug_get_flag("scl", "scl", SCL_CTC, &level);

    ctc_cli_out(" scl %s debug %s level:%s\n", "CTC", en ? "on" : "off", ctc_cli_get_debug_desc(level));

    en = ctc_debug_get_flag("scl", "scl", SCL_SYS, &level);
    ctc_cli_out(" scl %s debug %s level:%s\n", "SYS", en ? "on" : "off", ctc_cli_get_debug_desc(level));

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_scl_set_hash_l2_field_sel,
        ctc_cli_scl_set_hash_l2_field_sel_cmd,
        "scl set hash-key-type l2 field-sel-id SEL_ID" \
        CTC_CLI_SCL_HASH_FIELD_L2_KEY_STR,
        CTC_CLI_SCL_STR,
        "Set",
        "hash Key type",
        "L2 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        "<0-3>",
        CTC_CLI_SCL_HASH_FIELD_MAC_KEY_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_scl_hash_field_sel_t sel;
    ctc_scl_hash_l2_field_sel_t* pf = NULL;

    sal_memset(&sel, 0, sizeof(sel));
    sel.key_type = CTC_SCL_KEY_HASH_L2;
    pf = &sel.u.l2;

    CTC_CLI_GET_UINT32("field-sel-id", sel.field_sel_id, argv[0]);

    CTC_CLI_SCL_HASH_FIELD_L2_KEY_SET();

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_set_hash_field_sel(g_api_lchip, &sel);
    }
    else
    {
        ret = ctc_scl_set_hash_field_sel(&sel);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_scl_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_scl_create_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_destroy_group_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_set_default_action_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_mac_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_ipv4_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_ipv6_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_vlan_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_cvlan_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_svlan_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_dvlan_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_cvlan_cos_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_svlan_cos_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_mac_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_mac_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_ipv4_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_ipv4_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_port_ipv6_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_ipv6_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_add_l2_hash_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_set_hash_l2_field_sel_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_remove_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_remove_all_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_set_entry_priority_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_update_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_get_multi_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_copy_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_install_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_uninstall_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_install_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_uninstall_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_show_group_info_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_scl_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_scl_debug_show_cmd);
   /*  install_element(CTC_SDK_MODE, &ctc_cli_scl_test_sort_performance_cmd);*/


    return 0;
}


