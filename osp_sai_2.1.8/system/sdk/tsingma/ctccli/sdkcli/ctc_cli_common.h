/**
 @file ctc_cli_common.h

 @date 2013-8-20

 @version v2.0

  The file defines Macro, stored data structure for ctc sdk cli
*/
#ifndef _CTC_SDK_CLI_H
#define _CTC_SDK_CLI_H

#ifdef __cplusplus
extern "C" {
#endif


#define CTC_CLI_ERROR_RETURN(op) \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            ctc_cli_out("%% ret = %d, %s \n", rv, ctc_get_error_desc(rv));\
            return CLI_ERROR;\
        } \
    }

extern uint8 g_api_lchip;


#define CTC_CLI_UINT32_VAR_STR  "Uint32 variable"
#define CTC_CLI_UINT16_VAR_STR  "Uint16 variable"
#define CTC_CLI_UINT8_VAR_STR  "Uint8 variable"

#define CTC_CLI_PERF_TEST_STR "Performance test"

/*module*/
#define CTC_CLI_L2_M_STR "Layer 2 module"
#define CTC_CLI_MIRROR_M_STR "Mirror module"
#define CTC_CLI_STP_M_STR "Spanning Tree Protocol (STP)"
#define CTC_CLI_PORT_M_STR "Port module"
#define CTC_CLI_VLAN_M_STR "VLAN module"
#define CTC_CLI_LINKAGG_M_STR "Linkagg module"
#define CTC_CLI_CFM_M_STR "Connectivity Fault Management Protocol (CFM)"
#define CTC_CLI_MPLS_M_STR "Multi-Protocol Label Switching (MPLS)"
#define CTC_CLI_LACP_M_STR "Link Aggregation Control Protocol (LACP)"
#define CTC_CLI_MEM_ALLOCM_STR "Memory Allocation module"
#define CTC_CLI_NH_M_STR "Nexthop module"
#define CTC_CLI_USERID_STR "Userid module"
#define CTC_CLI_L3IF_STR "L3 interface module"
#define CTC_CLI_IPUC_M_STR "IPUC module"
#define CTC_CLI_IPMC_M_STR "IPMC module"
#define CTC_CLI_MCAST_M_STR "MCAST module"
#define CTC_CLI_IPV4_STR "IPV4"
#define CTC_CLI_IPV6_STR "IPV6"
#define CTC_CLI_PARSER_STR "Parser module"
#define CTC_CLI_INTR_M_STR "Interrupt module"
#define CTC_CLI_IPFIX_M_STR "IPFIX module"
#define CTC_CLI_BPE_M_STR "BPE module"
#define CTC_CLI_APP_M_STR "APP module"
#define CTC_CLI_EFD_M_STR "EFD module"
#define CTC_CLI_TRILL_M_STR "TRILL module"
#define CTC_CLI_FCOE_M_STR "FCOE module"
#define CTC_CLI_DOT1AE_M_STR "DOT1AE module"
#define CTC_CLI_WLAN_M_STR "WLAN module"
#define CTC_CLI_DIAG_M_STR "Diag module"

/* ACL -BEGIN-*/
#define CTC_CLI_ACL_STR              "ACL module"
#define CTC_CLI_ACL_GROUP_ID_STR     "ACL group id"
#define CTC_CLI_ACL_S_GROUP_ID_VALUE "ACL normal group id value"
#define CTC_CLI_ACL_GROUP_ID_VALUE   "<0-0xFFFFFFFF>"
#define CTC_CLI_ACL_GROUP_PRIO_STR   "ACL group priority"
#define CTC_CLI_ACL_GROUP_PRIO_VALUE "Group priority"
#define CTC_CLI_ACL_ENTRY_ID_STR     "ACL entry id"
#define CTC_CLI_ACL_ENTRY_ID_VALUE   "ACL entry id value"
#define CTC_CLI_ACL_ENTRY_PRIO_STR   "ACL entry priority"
#define CTC_CLI_ACL_ENTRY_PRIO_VALUE "<0-0xFFFFFFFF> :0 is lowest"
/* SCL -BEGIN-*/
#define CTC_CLI_SCL_GROUP_PRIO_STR   "SCL group priority"
#define CTC_CLI_SCL_GROUP_PRIO_VALUE "Group priority"
#define CTC_CLI_SCL_STR              "SCL module"
#define CTC_CLI_SCL_GROUP_ID_STR     "SCL group id"
#define CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE   "SCL normal group id value"
#define CTC_CLI_SCL_HASH_GROUP_ID_VALUE   "SCL hash group id value"
#define CTC_CLI_SCL_ENTRY_ID_STR     "SCL entry id"
#define CTC_CLI_SCL_ENTRY_ID_VALUE   "SCL entry id value"
#define CTC_CLI_SCL_ENTRY_PRIO_STR   "SCL entry priority"
#define CTC_CLI_SCL_ENTRY_PRIO_VALUE "<0-0xFFFFFFFF> :0 is lowest"
#define CTC_CLI_SCL_SERVICE_ID_VALUE "<1-0xFFFF>"
#define CTC_CLI_VNID_VALUE "<0-0xFFFFFF>"
#define CTC_CLI_VNID_MASK "<0-0xFFFFFF>"

#define CTC_CLI_BPE_DESC              "Bridge port extension"
#define CTC_CLI_BPE_MUX_DEMUX         "Centec mux/demux port extension"
#define CTC_CLI_8021QBG_M_VEPA        "802.1Qbg multi-channel VEPA"
#define CTC_CLI_8021QBR_CB_CASCADE    "802.1Qbr cascade port of control bridge"
#define CTC_CLI_8021QBR_PE_CASCADE    "802.1Qbr cascade port of port extender"
#define CTC_CLI_8021QBR_EXTENDED      "802.1Qbr extended port of port extender"
#define CTC_CLI_8021QBR_UPSTREAM      "802.1Qbr upstream port of port extender"

#define CTC_CLI_MPLS_LABLE_VALUE    "<0-0xFFFFF>"
#define CTC_CLI_MPLS_S_VALUE        "<0-1>"
#define CTC_CLI_MPLS_EXP_VALUE      "<0-7>"
#define CTC_CLI_MPLS_TTL_VALUE      "<0-255>"
#define CTC_CLI_PARSER_L2_TYPE_VALUE "<0-15>"
#define CTC_CLI_PARSER_L2_TYPE_MASK  "<0-0xF>"
#define CTC_CLI_PARSER_L3_TYPE_VALUE "<0-15>"
#define CTC_CLI_PARSER_L3_TYPE_MASK  "<0-0xF>"
#define CTC_CLI_PARSER_L4_TYPE_VALUE "<0-15>"
#define CTC_CLI_PARSER_L4_TYPE_MASK  "<0-0xF>"
#define CTC_CLI_PARSER_HASH_MODE     "<0-0xFF>, 0:outer, 1:merge, 2:inner, other value be regard as 0"

#define CTC_CLI_ETHER_TYPE_VALUE     "<0-0xFFFF>"
#define CTC_CLI_ETHER_TYPE_MASK      "<0-0xFFFF>"
#define CTC_CLI_PRIORITY_VALUE       "Priority value"
#define CTC_CLI_COLOR_VALUE          "<1-3> 0 is invalid, 1:red 2:yellow 3:green"
#define CTC_CLI_MICRO_FLOW_POLICER_ID_VALUE "<0-0xFFFF>"
#define CTC_CLI_MACRO_FLOW_POLICER_ID_VALUE "<0-0xFFFF>"
#define CTC_CLI_SESSION_ID_VALUE    "<0-3>"
#define CTC_CLI_LOG_PERCENT_VALUE   "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% "
#define CTC_CLI_DSCP_VALUE          "<0-0x3F>"
#define CTC_CLI_ECN_VALUE           "<0-0x3>"
#define CTC_CLI_DSCP_MASK           "<0-0x3F>"
#define CTC_CLI_IP_PREC_VALUE       "<0-0x7>"
#define CTC_CLI_IP_PREC_MASK        "<0-0x7>"
#define CTC_CLI_L4_PROTOCOL_VALUE   "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17   STCP:132"
#define CTC_CLI_L4_PROTOCOL_MASK    "<0-0xFF>"
#define CTC_CLI_FRAG_VALUE          "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment. 5:fragment"
#define CTC_CLI_ICMP_TYPE_VALUE     "<0-0xFF>"
#define CTC_CLI_ICMP_TYPE_MASK      "<0-0xFF>"
#define CTC_CLI_ICMP_CODE_VALUE     "<0-0xFF>"
#define CTC_CLI_ICMP_CODE_MASK      "<0-0xFF>"
#define CTC_CLI_IGMP_TYPE_VALUE     "<0-0xFF>"
#define CTC_CLI_IGMP_TYPE_MASK      "<0-0xFF>"
#define CTC_CLI_GRE_KEY_VALUE       "<0-0xFFFFFFFF>"
#define CTC_CLI_GRE_KEY_MASK        "<0-0xFFFFFFFF>"
#define CTC_CLI_IP_OPTION           "<0-1>"
#define CTC_CLI_IP_HDR_ERROR        "<0-1>"
#define CTC_CLI_ROUTED_PACKET       "<0-1>"
#define CTC_CLI_IPV4_PACKET_VALUE   "<0-1>"
#define CTC_CLI_TCP_FLAGS           "<0-0x3F>"
#define CTC_CLI_IPV6_EXT_HDR_VALUE  "<0-0xFF>"
#define CTC_CLI_IPV6_EXT_HDR_MASK   "<0-0xFF>"
#define CTC_CLI_IPV6_FLOW_LABEL_VALUE "<0-0xFFFFF>"
#define CTC_CLI_IPV6_FLOW_LABEL_MASK  "<0-0xFFFFF>"
#define CTC_CLI_MPLS_LABEL_VALUE    "<0-0xFFFFFFFF>"
#define CTC_CLI_MPLS_LABEL_MASK     "<0-0xFFFFFFFF>"
#define CTC_CLI_MPLS_LABEL_NUMBER_VALUE    "<0-9>"
#define CTC_CLI_MPLS_LABEL_NUMBER_MASK     "<0-0xF>"
#define CTC_CLI_ARP_PACKET_VALUE    "<0-1>"
#define CTC_CLI_ARP_OP_CODE_VALUE  "<0-0xFFFF>"
#define CTC_CLI_ARP_OP_CODE_MASK   "<0-0xFFFF>"
#define CTC_CLI_L4_PORT_VALUE      "<0-0xFFFF>"
#define CTC_CLI_ACL_VLAN_CLASS_ID_VALUE  "value"
#define CTC_CLI_ACL_PORT_CLASS_ID_VALUE  "value"
#define CTC_CLI_ACL_SERVICE_ID_VALUE     "value"
#define CTC_CLI_ACL_PBR_CLASS_ID_VALUE   "<0-63>"

#define CTC_CLI_STATS_COLOR_STR          "Stats color "
#define CTC_CLI_STATS_COLOR_VALUE        "<0-1> 0 green, 1 not green. "
/*ACL -END-*/

/*OAM -BEGIN-*/
#define CTC_CLI_OAM_M_STR "OAM module"
#define CTC_CLI_OAM_CFM_M_STR "CFM"
#define CTC_CLI_OAM_MPLS_TP_Y1731_M_STR "MPLS TP Y.1731"
#define CTC_CLI_OAM_MPLS_TP_BFD_M_STR "MPLS TP BFD"
#define CTC_CLI_OAM_EFM_M_STR "EFM"
#define CTC_CLI_TRPT_M_STR "ThroughPut"
#define CTC_CLI_OAM_COMMON_M_STR "OAM Common"
/*OAM -END-*/

#define CTC_CLI_SCL_ID_VALUE        "Scl lookup ID Value"

/* QOS -BEGIN-*/
#define CTC_CLI_QOS_STR "QoS module"
#define CTC_CLI_QOS_RESRC_STR "QOS resource management"
#define CTC_CLI_QOS_PLC_STR "QoS policer"
#define CTC_CLI_QOS_DOMAIN_STR "QoS domain mapping"
#define CTC_CLI_QOS_TABLE_MAP_STR "QoS table mapping"
#define CTC_CLI_QOS_SHAPE_STR "QoS shaping"
#define CTC_CLI_QOS_SCHED_STR "QoS schedule"
#define CTC_CLI_QOS_STATS_STR "QoS statstics"
#define CTC_CLI_QOS_QUEUE_STR "Queue id"
#define CTC_CLI_QOS_QUEUE_VAL "Queue ID"
#define CTC_CLI_QOS_SERVICE_STR "Service identification"
#define CTC_CLI_QOS_SERVICE_VAL "Service id value"
#define CTC_CLI_QOS_FLOW_PLC_STR "QoS flow policer"
#define CTC_CLI_QOS_FLOW_PLC_VAL "<1-0xFFFF>"
#define CTC_CLI_QOS_VLAN_PLC_STR "QoS vlan policer"
#define CTC_CLI_QOS_VLAN_PLC_VAL "Vlan id"
#define CTC_CLI_QOS_COPP "Control Plane Policing"
#define CTC_CLI_QOS_PLC_CIR_STR "Config CIR (Commit Information Rate)"

#define CTC_CLI_QOS_PLC_EIR_STR "Config EIR (Excess Information Rate)"

#define CTC_CLI_QOS_PLC_PIR_STR "Config PIR (Peak Information Rate)"

#define CTC_CLI_QOS_PLC_CBS_STR "Config CBS (Commit Burst Size)"
#define CTC_CLI_QOS_PLC_EBS_STR "Config EBS (Excess Burst Size)"
#define CTC_CLI_QOS_PLC_PBS_STR "Config PBS (Peak Burst Size)"
#define CTC_CLI_QOS_SHAPE_CBS_STR "CBS value unit bytes"
#define CTC_CLI_QOS_SHAPE_PBS_STR "PBS value unit bytes"

/* QOS -END-*/

/* Stats -BEGIN- */
#define CTC_CLI_STATS_STR      "Statistics"
#define CTC_CLI_STATS_ID_DESC  "Stats id"
#define CTC_CLI_STATS_ID_VAL   "<0-0xFFFFFFFF>, 0 means invalid"
/* Stats -END- */

#define CTC_CLI_CPU_TRAFFIC_STR "CPU Traffic module"
#define CTC_CLI_SYNC_ETHER_M_STR "SyncE module"
#define CTC_CLI_PDU_STR "Pdu module"
#define CTC_CLI_LEARNING_AGING_STR "Learning and Aging module"
#define CTC_CLI_LEARNING_STR "Learning module"
#define CTC_CLI_AGING_STR "Aging module"
#define CTC_CLI_SECURITY_STR "Security module"
#define CTC_CLI_DOT1AE_STR "dot1ae module"
#define CTC_CLI_APS_M_STR "Aps Module"
#define CTC_CLI_MONITOR_M_STR "Monitor Module"
#define CTC_CLI_OVERLAY_M_STR "Overlay_tunnel Module"

#define CTC_CLI_CPU_SAMPLE_M_STR "Cpu sample module"
#define CTC_CLI_DMA_M_STR "Dma module"
#define CTC_CLI_CHIP_AGT_STR "Chip Agent module"
#define CTC_CLI_PTP_M_STR "PTP module"
#define CTC_CLI_PACKET_M_STR "Packet module"
#define CTC_CLI_PACKET_SAMPLE_M_STR "Sample Packet module"

#define CTC_CLI_NH_ADD_STR "Add nexthop"
#define CTC_CLI_NH_DEL_STR "Delete nexthop"
#define CTC_CLI_NH_UPDATE_STR "Update nexthop"
#define CTC_CLI_DEBUG_LEVEL_STR "Set the debugging output level "
#define CTC_CLI_DEBUG_LEVEL_FUNC "Output enter the functions with parameters "
#define CTC_CLI_DEBUG_LEVEL_PARAM "Output the parameters information when enter the functions or invoke other functions "
#define CTC_CLI_DEBUG_LEVEL_INFO    "Output the general information "
#define CTC_CLI_DEBUG_LEVEL_ERROR "Output the error  information "
#define CTC_CLI_DEBUG_LEVEL_EXPORT "Output the export information "
#define CTC_CLI_DEBUG_LEVEL_NONE   "Output none information "
#define CTC_CLI_DEBUG_MODE_LOG   "Log debug information by log "
#define CTC_CLI_LOG_FILE   "Log file name "

/*parameter*/
#define CTC_CLI_L2_HDR_STR "Layer 2 header of packet"
#define CTC_CLI_L3_HDR_STR "Layer 3 header of packet"
#define CTC_CLI_L4_HDR_STR "Layer 4 header of packet"
#define CTC_CLI_SHOW_MEMORY_STR  "Show Allocation Information"
#define CTC_CLI_SHOW_BRIDGE_STR  "Bridge information"
#define CTC_CLI_FDB_DESC "FDB table"

#define CTC_CLI_L2_PDU_STR "Layer2 PDU"
#define CTC_CLI_L3_PDU_STR "Layer3 PDU"

#define CTC_CLI_ADD "Add operation"
#define CTC_CLI_REMOVE "Remove operation"
#define CTC_CLI_MAC_DESC "MAC address"
#define CTC_CLI_MACDA_DESC "MAC DA address"
#define CTC_CLI_MACSA_DESC "MAC SA address"
#define CTC_CLI_MAC_FORMAT "MAC address in HHHH.HHHH.HHHH format"
#define CTC_CLI_MAC_MASK_FORMAT "MAC mask in HHHH.HHHH.HHHH format"
#define CTC_CLI_IPV4_FORMAT "IPv4 address in A.B.C.D format"
#define CTC_CLI_IPV4_MASK_FORMAT "IPv4 mask in A.B.C.D format"
#define CTC_CLI_IPV4_MASK_LEN_FORMAT "The length of Ipv4 mask <0-32>"
#define CTC_CLI_IPV6_FORMAT "IPv6 address in X:X::X:X format"
#define CTC_CLI_IPV6_MASK_FORMAT "IPv6 mask in X:X::X:X format"
#define CTC_CLI_IPV6_MASK_LEN_FORMAT "The length of Ipv6 mask <0-128>"
#define CTC_CLI_ETHTYPE_DESC          "EthType"
#define CTC_CLI_DEST_FCID_DESC "Dest FC ID address"
#define CTC_CLI_SRC_FCID_DESC "Src FC ID address"
#define CTC_CLI_FCID_RANGE_DESC "<0-0xFFFFFF>"
#define CTC_CLI_VRFID_DESC "VRF id of route"

#define CTC_CLI_VLAN_DESC "VLAN"
#define CTC_CLI_VLAN_RANGE_DESC "<1-4095>"
#define CTC_CLI_VLAN_RANGE_MASK "<0-0xFFF>"
#define CTC_CLI_COS_RANGE_DESC  "<0-7>"
#define CTC_CLI_COS_RANGE_MASK  "<0-7>"
#define CTC_CLI_CFI_RANGE_DESC  "<0-1>"
#define CTC_CLI_EXP_RANGE_DESC  "<0-7>"
#define CTC_CLI_FID_DESC "Forwarding ID"
#define CTC_CLI_EGRESS_NICKNAME "<0-0xFFFF>"
#define CTC_CLI_GLOBAL_MCASTGRP_ID_DESC "Mcast Group ID,it is allocated in platform adaption Layer"
#define CTC_CLI_NH_DSNH_OFFSET_STR "If it isn't be selected,the dsnh_offset will be internally allocated in SDK"
#define CTC_CLI_NH_DSNH_OFFSET_VALUE_STR  "DsNexthop Table's Offset"
#define CTC_CLI_NH_ID_STR "Nexthop ID, Drop nhid is 0x1, ToCpu nexthop is 0x2, external nhid's range is 0x3 ~ 0x7FFFFFFF (the max external nhid can be configed before initialize, default is 0x7FFFFFFF), internal nhid's range is 0x80000000 ~ 0xFFFFFFFF"
#define CTC_CLI_APS_BRIDGE_ID_STR      "Aps bridge id"
#define CTC_CLI_APS_BRIDGE_ID_DESC     "<0-1023>"

#define CTC_CLI_GPORT_DESC "Global port"
#define CTC_CLI_PORT_BITMAP_DESC "Bitmap of ports"
#define CTC_CLI_PORT_BITMAP_VALUE_DESC "Bitmap value"
/*Append pbmp2-7 for TM*/
#define CTC_CLI_PORT_BITMAP_STR     "{pbmp0 PORT_BITMAP_0 | pbmp1 PORT_BITMAP_1 | \
                                                            pbmp2 PORT_BITMAP_2 | pbmp3 PORT_BITMAP_3 | \
                                                            pbmp4 PORT_BITMAP_4 | pbmp5 PORT_BITMAP_5 | \
                                                            pbmp6 PORT_BITMAP_6 | pbmp7 PORT_BITMAP_7 | \
                                                            pbmp8 PORT_BITMAP_8 | pbmp9 PORT_BITMAP_9 }"
/*Append pbmp2-7 for TM*/
#define CTC_CLI_PORT_BITMAP_GET(port_bmp)\
{\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp0");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 0", ((uint32*)port_bmp)[0], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp1");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 1", ((uint32*)port_bmp)[1], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp2");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 2", ((uint32*)port_bmp)[2], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp3");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 3", ((uint32*)port_bmp)[3], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp4");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 4", ((uint32*)port_bmp)[4], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp5");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 5", ((uint32*)port_bmp)[5], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp6");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 6", ((uint32*)port_bmp)[6], argv[index + 1]);\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbmp7");\
    if (index != 0xFF)\
    {\
        CTC_CLI_GET_UINT32("port bitmap 7", ((uint32*)port_bmp)[7], argv[index + 1]);\
    }\
    if (MAX_PORT_NUM_PER_CHIP > 256)\
    {\
        index = CTC_CLI_GET_ARGC_INDEX("pbmp8");\
        if (index != 0xFF)\
        {\
            CTC_CLI_GET_UINT32("port bitmap 8", ((uint32*)port_bmp)[8], argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("pbmp9");\
        if (index != 0xFF)\
        {\
            CTC_CLI_GET_UINT32("port bitmap 9", ((uint32*)port_bmp)[9], argv[index + 1]);\
        }\
    }\
}

#define CTC_CLI_LCHIP_ID_STR      "Local chip id"
#define CTC_CLI_LCHIP_ID_VALUE    "<0~chip_num-1> ,the lchip_num can be changed by ctc_chip_init() when you initialize SDK ,lchip_num default is 1."
#define CTC_CLI_CHIP_DESC "Chip id on linecard"
#define CTC_CLI_CHIP_ID_DESC "<0-CTC_MAX_LOCAL_CHIP_NUM>"
#define CTC_CLI_GCHIP_DESC "Global chip id"
#define CTC_CLI_GCHIP_ID_DESC "<0-CTC_MAX_GCHIP_CHIP_ID>"
#define CTC_CLI_LINKAGG_DESC "Linkagg"
#define CTC_CLI_L3IF_ID_DESC "L3if ID"
#define CTC_CLI_ECMP_L3IF_ID_DESC "Ecmp l3if id"
#define CTC_CLI_ACLQOS_LABEL_ID_DESC    "<1-4294967295>"
#define CTC_CLI_IPMC_GROUP_DESC         "IP multicast group"
#define CTC_CLI_IPMC_MEMBER_DESC        "Member of the ipmc group"
#define CTC_CLI_SERVICE_ID_DESC "<1-65535>"
#define CTC_CLI_PFM_RANGE_DESC "<0-2>"
#define CTC_CLI_TAG_OP_DESC "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged), 4:REPLACE(Replace for tagged, do nothing for untagged.), 5:DO NOTHING"
#define CTC_CLI_TAG_SL_DESC "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)"
#define CTC_CLI_ACL_VLAN_TAG_OP_DESC "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged), 4:REP_OR_ADD (Replace for tagged, add for untagged)."
#define CTC_CLI_ACL_VLAN_TAG_SL_DESC "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)"
#define CTC_CLI_ACL_VLAN_TAG_SCOS_SL_DESC "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned), 3:MAP (Select mapped cos from QoS table map)"
#define CTC_CLI_ETHER_TYPE_RANGE    "<0x0000-0xFFFF>"
#define CTC_CLI_INTR_TYPE_STR "Interrupt sup-level type"
#define CTC_CLI_INTR_SUB_TYPE_STR "Interrupt sub-level type"
#define CTC_CLI_INTR_TYPE_VALUE "<0-0xFFFFFFFF>"
#define CTC_CLI_QOS_DOMAIN_VALUE    "Domain value"
#define CTC_CLI_QOS_TABLE_MAP_VALUE    "Qos table value"
#define CTC_CLI_L3PDU_IPDA_ENTRY_INDEX "<0-3>"

#define CTC_CLI_ACL_PORT_BITMAP_VALUE    "<0-0xFFFFFFFF>"
#define CTC_CLI_PARSER_UDF               "User define format"

#define CTC_CLI_FIELD_SEL_ID_DESC             "Field Select Id"
#define CTC_CLI_FIELD_SEL_ID_VALUE            "Field Select Id Value. like <0-15>"
#define CTC_CLI_ECMP_L3IF_ID_RANGE_DESC "<1-1022>"
#define CTC_CLI_HA_FORMAT                 "Hardware address in HHHH.HHHH.HHHH format"
#define CTC_CLI_L2PDU_MACDA_48BIT_ENTRY_INDEX "<0-7>"
#define CTC_CLI_L3PDU_L3IF_ACTION_INDEX "<0-15>"
#define CTC_CLI_MACDA_24BIT_MASK_DESC "Macda mask, the mask works only if entry index set 0 or 1."
#define CTC_CLI_ACL_PRIORITY_ID_DESC "ACL Priority"
#define CTC_CLI_PARSER_UDF_OFFSET         "<0-63>"
#define CTC_CLI_VLAN_BASE_DESC            "vlan base"
#define CTC_CLI_PORT_ISOLATION_GROUP_DESC "<0-31>, 0 means disable"
#define CTC_CLI_VLAN_DOMAIN_DESC "<0-2>, 0: svlan, 1: cvlan, 2: unchange"
#define CTC_CLI_ACL_HASH_TYPE_VALUE "<0-3>, 0:Disable, 1:Only L2 Field, 2:Only L3 Field, 3:L2+L3 Field"
#define CTC_CLI_ACL_TCAM_TYPE_VALUE "<0-3>, 0:Only L2 Field, 1:L2+L3 Field, 2:Only L3 Field, 3:Only Vlan Field"
#define CTC_CLI_METADATA_TYPE_VALUE "<0-3>, 0:Disable, 1:Metadata, 2:FID, 3:VRFID"
#define CTC_CLI_METADATA_VALUE "<0-0x3FFF>"
#define CTC_CLI_METADATA_MASK "<0-0x3FFF>"
#define CTC_CLI_OAM_TRPT_PKT_NUM_VAL "<1-0xFFFFFFFF>"
#define CTC_CLI_GPHYPORT_ID_DESC                "type(4bit)|reserve(10bit)|lport_ext(3bit) | gchip(7bit, 0x1F for linkagg) | lport/linkagg_id (8bit)"
#define CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC   "type(4bit)|reserve(10bit)|lport_ext(3bit) | gchip(7bit) | lport (8bit)"
#define CTC_CLI_GPORT_ID_DESC                    CTC_CLI_GPHYPORT_ID_DESC
#define CTC_CLI_EDGE_PORT_ID_DESC                CTC_CLI_GPHYPORT_ID_DESC
#define CTC_CLI_APS_GROUP_ID_DESC "Aps group id value"

#define CTC_CLI_ACL_PORT_BITMAP "Port bitmap"
#define CTC_CLI_ACL_PORT_BITMAP_HIGH_VALUE "<0-0xFFFFFFFF>  port32~ port63"
#define CTC_CLI_ACL_PORT_BITMAP_LOW_VALUE  "<0-0xFFFFFFFF>  port0~ port31"
#define CTC_CLI_TRUST_VALUE  "<0-6> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence, 5=trust stag-cos,6=trust ctag-cos"
#define CTC_CLI_USER_VLAN_PTR  "<0-0x1FFF>"
#define CTC_CLI_MAPPING_L3TYPE_INDEX  "Index Value <0-3>"
#define CTC_CLI_MAPPING_L4TYPE_INDEX  "Index value <0-3>"
#define CTC_CLI_L2FLEX_BASIC_OFFSET  "Offset value <0x0-0x3F>"
#define CTC_CLI_PORT_ISOLATION_ID_DESC "<0-31>(default value 0 indicate not an isolated port)"
#define CTC_CLI_L3IF_ID_RANGE_DESC "L3if ID value"
#define CTC_CLI_VLAN_RANGE_GRP_ID_VALUE "<0-63>"
#define CTC_CLI_FID_ID_DESC "Defined by user, default max value is 5k,4k vlan + 1k vsi<vpls>"
#define CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX "<0-9>"
#define CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX "<0-31>"
#define CTC_CLI_L3PDU_ENTRY_INDEX "<0-7>"
#define CTC_CLI_L3PDU_GLOBAL_ACTION_INDEX "<0-31>"
#define CTC_CLI_L2PDU_L3TYPE_ACTION_INDEX "<32-53>"
#define CTC_CLI_LINKAGG_ID_DESC "Linkagg id value"
#define CTC_CLI_VRFID_ID_DESC "Vrfid Value"
#define CTC_CLI_MAX_ECMP_DESC "Max_ecmp the maximum ECMP paths allowed for a route."
#define CTC_CLI_ACL_ENABLE_BITMAP "Port acl enable (bitmap <0-0xF>, acl-en0-bmp:1, acl-en1-bmp:2, acl-en2-bmp:4, acl-en3-bmp:8)"
#define CTC_CLI_ACL_B_GROUP_ID_VALUE   "Acl group id value"
#define CTC_CLI_SCL_PORT_CLASS_ID_VALUE "Scl port class id value"
#define CTC_CLI_SCL_MAX_GROUP_ID_VALUE   "Scl max group id value"
#define CTC_CLI_QOS_CLASS_VAL "Class value"
#define CTC_CLI_QOS_DROP_PRECEDENCE     "<0-3>"
#define CTC_CLI_QOS_DROP_THRESHOLD "Qos drop threshold value"
#define CTC_CLI_QOS_DROP_PROB_VAL "Qos drop prob value"
#define CTC_CLI_QOS_WDRR_WEIGHT_VAL "Weight value"
#define CTC_CLI_QOS_MAX_CPU_REASON_GRP_ID     "<0-15>"
#define CTC_CLI_STATS_MTU1_DESC "<1024 - 16383> (default:1518)"
#define CTC_CLI_STATS_MTU2_DESC "<1024 - 16383> (default:1536)"
#define CTC_CLI_PORT_MAX_FRAME_SIZE "<128-16127>"
#define CTC_CLI_PORT_MIN_FRAME_SIZE "<18-127>"


#define CTC_CLI_PORT_INTERNAL_PORT_START  "lport base of the extend port, and the extend ports must be in the same slice"
#define CTC_CLI_PORT_BASE_DESC            "extension port start base"
#define CTC_CLI_PORT_EXTENDER_NUM         "Port extender num value"
#define CTC_CLI_PORT_PREAMBLE_VAL         "Port preamble value"
#define CTC_CLI_PDU_RIP_DEST_PORT         "Pdu rip dest port"
#define CTC_CLI_LEARNING_CACHE_NUM        "Learning cache num value"
#define CTC_CLI_LPORT_CPU                 "0x10000000"
#define CTC_CLI_PTP_INDEX_RANGE           "<0-7>"
#define CTC_CLI_TOD_EPOCH_RANGE           "<0-0x7FFF>"
#define CTC_CLI_PTP_LPORT_RANGE           "Ptp lport range value"

#define CTC_CLI_QOS_PLC_CIR_VAL "Rate in kbps/pps"
#define CTC_CLI_QOS_PLC_EIR_VAL "Rate in kbps/pps"
#define CTC_CLI_QOS_PLC_PIR_VAL "Rate in kbps/pps"
#define CTC_CLI_QOS_PLC_CBS_VAL "Burst size in kb/pkt, default is 64"
#define CTC_CLI_QOS_PLC_EBS_VAL "Burst size in kb/pkt, default is 64"
#define CTC_CLI_QOS_PLC_PBS_VAL "Burst size in kb/pkt, default is 64"
#define CTC_CLI_NH_DSNH_TUNNEL_DSCP_DESC "Dscp"
#define CTC_CLI_NH_DSNH_TUNNEL_DSCP_VAL "Dscp value"
#define CTC_CLI_INTERNAL_PORT_RANGE  "Internal port range value"



#define CTC_CLI_ACL_KEY_FIELED_GREP_NUM 8
#define CTC_CLI_ACL_KEY_ARG_CHECK(arg_cnt,arg_num,p_field) \
        arg_cnt++; \
        if (p_field) p_field++;\
        if((arg_cnt >= arg_num) && (arg_cnt != CTC_CLI_ACL_KEY_FIELED_GREP_NUM) && (arg_cnt != CTC_CLI_ACL_KEY_FIELED_GREP_NUM+1)) break;\
        if(arg_cnt>CTC_CLI_ACL_KEY_FIELED_GREP_NUM) return CTC_E_INTR_INVALID_PARAM;

#define CTC_CLI_ACL_KEY_FIELD_TYPE_GREP_STR "\
| mac-sa (VALUE (MASK|)|) \
| mac-da (VALUE (MASK|)|) \
| svlan-id (VALUE (MASK|)|) \
| cvlan-id (VALUE (MASK|)|) \
| ip-sa (VALUE (MASK|)|) \
| ip-da (VALUE (MASK|)|) \
| ipv6-sa (VALUE (MASK|)|) \
| ipv6-da (VALUE (MASK|)|) \
| l4-dst-port (VALUE (MASK|)|)  \
| l4-src-port (VALUE (MASK|)|)  \
| gre-key (VALUE (MASK|)|)  \
| vn-id (VALUE (MASK|)|)  \
| mpls-label0 (VALUE (MASK|)|)  \
| mpls-label1 (VALUE (MASK|)|)  \
| mpls-label2 (VALUE (MASK|)|)  \
"
#define CTC_CLI_ACL_KEY_FIELD_TYPE_GREP_DESC \
"source mac address ","value" , "mask", \
"destination mac address ","value" , "mask", \
"s-vlan id ","value" , "mask", \
"c-vlan id ","value" , "mask", \
"source ipv4 address ","value" , "mask", \
"destination ipv4 address ","value" , "mask", \
"source ipv6 address ","value" , "mask", \
"destination ipv6 address ","value" , "mask", \
"layer 4 dest port ","value" , "mask", \
"layer 4 src port ","value" , "mask", \
"gre key ","value" , "mask", \
"Vxlan Network Id", "value", "mask", \
"mpls label0","value" , "mask", \
"mpls label1","value" , "mask", \
"mpls label2","value" , "mask"

#define CTC_CLI_ACL_KEY_FIELD_GREP_SET(key_field, arg)\
do{\
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_MAC_SA;\
        if(is_add)\
        {\
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", mac_sa_addr, argv[index + 1]);\
            key_field.ext_data = mac_sa_addr; \
            sal_memset(&mac_sa_addr_mask, 0xFF, sizeof(mac_addr_t));\
            if(index+2<argc){\
            CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", mac_sa_addr_mask, argv[index + 2]);\
            key_field.ext_mask = mac_sa_addr_mask;} \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_MAC_DA;\
        if(is_add)\
        {\
            key_field.type = CTC_FIELD_KEY_MAC_DA;\
            CTC_CLI_GET_MAC_ADDRESS("mac-da", mac_da_addr, argv[index + 1]);\
            key_field.ext_data = mac_da_addr; \
            sal_memset(&mac_da_addr_mask, 0xFF, sizeof(mac_addr_t));\
            if(index+2<argc){\
            CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", mac_da_addr_mask, argv[index + 2]);\
            key_field.ext_mask = mac_da_addr_mask;} \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-id");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_SVLAN_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if (index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]); \
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cvlan-id");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_CVLAN_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_IP_SA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", key_field.data, argv[index + 1]);\
            key_field.mask = 0xFFFFFFFF;\
            if(index+2<argc) CTC_CLI_GET_IPV4_ADDRESS("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_IP_DA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV4_ADDRESS("ipda", key_field.data, argv[index + 1]);\
            key_field.mask = 0xFFFFFFFF;\
            if(index+2<argc) CTC_CLI_GET_IPV4_ADDRESS("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_IPV6_SA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV6_ADDRESS("ip-sa", ipv6_sa_addr, argv[index + 1]);               \
            ipv6_sa_addr[0] = sal_htonl(ipv6_sa_addr[0]);                                 \
            ipv6_sa_addr[1] = sal_htonl(ipv6_sa_addr[1]);                                 \
            ipv6_sa_addr[2] = sal_htonl(ipv6_sa_addr[2]);                                 \
            ipv6_sa_addr[3] = sal_htonl(ipv6_sa_addr[3]);                                 \
            key_field.ext_data = ipv6_sa_addr;    \
            if(index+2<argc)\
            {\
                if CLI_CLI_STR_EQUAL("HOST", index + 2)                                          \
                {                                                                             \
                    ipv6_sa_addr_mask[0] = 0xFFFFFFFF;                                            \
                    ipv6_sa_addr_mask[1] = 0xFFFFFFFF;                                            \
                    ipv6_sa_addr_mask[2] = 0xFFFFFFFF;                                            \
                    ipv6_sa_addr_mask[3] = 0xFFFFFFFF;                                            \
                    key_field.ext_mask = ipv6_sa_addr_mask;    \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    CTC_CLI_GET_IPV6_ADDRESS("ip-sa-mask", ipv6_sa_addr_mask, argv[index + 2]);          \
                    ipv6_sa_addr_mask[0] = sal_htonl(ipv6_sa_addr_mask[0]);                                 \
                    ipv6_sa_addr_mask[1] = sal_htonl(ipv6_sa_addr_mask[1]);                                 \
                    ipv6_sa_addr_mask[2] = sal_htonl(ipv6_sa_addr_mask[2]);                                 \
                    ipv6_sa_addr_mask[3] = sal_htonl(ipv6_sa_addr_mask[3]);                                 \
                    key_field.ext_mask = ipv6_sa_addr_mask;    \
                }\
            }\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_IPV6_DA;\
        if(is_add)\
        {\
            CTC_CLI_GET_IPV6_ADDRESS("ip-da", ipv6_da_addr, argv[index + 1]);               \
            ipv6_da_addr[0] = sal_htonl(ipv6_da_addr[0]);                                 \
            ipv6_da_addr[1] = sal_htonl(ipv6_da_addr[1]);                                 \
            ipv6_da_addr[2] = sal_htonl(ipv6_da_addr[2]);                                 \
            ipv6_da_addr[3] = sal_htonl(ipv6_da_addr[3]);                                 \
            key_field.ext_data = ipv6_da_addr;    \
            if(index+2<argc)\
            {\
                if CLI_CLI_STR_EQUAL("HOST", index + 2)                                          \
                {                                                                             \
                    ipv6_da_addr_mask[0] = 0xFFFFFFFF;                                            \
                    ipv6_da_addr_mask[1] = 0xFFFFFFFF;                                            \
                    ipv6_da_addr_mask[2] = 0xFFFFFFFF;                                            \
                    ipv6_da_addr_mask[3] = 0xFFFFFFFF;                                            \
                    key_field.ext_mask = ipv6_da_addr_mask;    \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    CTC_CLI_GET_IPV6_ADDRESS("ip-da-mask", ipv6_da_addr_mask, argv[index + 2]);          \
                    ipv6_da_addr_mask[0] = sal_htonl(ipv6_da_addr_mask[0]);                                 \
                    ipv6_da_addr_mask[1] = sal_htonl(ipv6_da_addr_mask[1]);                                 \
                    ipv6_da_addr_mask[2] = sal_htonl(ipv6_da_addr_mask[2]);                                 \
                    ipv6_da_addr_mask[3] = sal_htonl(ipv6_da_addr_mask[3]);                                 \
                    key_field.ext_mask = ipv6_da_addr_mask;    \
                } \
            }\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_L4_DST_PORT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_GRE_KEY;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vn-id");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_VN_ID;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label0");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label1");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label2");\
    if (0xFF != index) \
    {\
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;\
        if(is_add)\
        {\
            CTC_CLI_GET_UINT32("value", key_field.data, argv[index + 1]);\
            if(index+2<argc) CTC_CLI_GET_UINT32("mask", key_field.mask, argv[index + 2]);\
        }\
        arg;\
    }\
}while(0)



struct ctc_chip_special_callback_fun_s
{
    int32(*chip_special_cli_init)(void);
    int32(*chip_special_cli_deinit)(void);
};
typedef struct ctc_chip_special_callback_fun_s ctc_chip_special_callback_fun_t;

#ifdef __cplusplus
}
#endif
#endif /* _CTC_SDK_CLI_H */

