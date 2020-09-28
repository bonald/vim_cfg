/**
 @file ctc_cli.h

 @date 2010-7-9

 @version v2.0

  The file defines Macro, stored data structure for ctc cli
*/
#ifndef _CTC_CLI_H
#define _CTC_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef SDK_IN_VXWORKS
#include "ioLib.h"
#else
#include <termios.h>
#include <unistd.h>
#endif
#include "sal_types.h"

#include "vty.h"
#include "command.h"
#include "ctc_common_cli.h"

/* CLI modes.  */

#define EXEC_MODE 0
#define CTC_SDK_MODE 1
#define CTC_CMODEL_MODE 2
#define CTC_SDK_OAM_CHAN_MODE 3
#define CTC_FECD_MODE 4
#define CTC_DEBUG_MODE 5
#define CTC_SDK_V1_MODE 6
#define CTC_DBG_TOOL_MODE 7
#define CTC_CTCCLI_MODE 8
#define CTC_INTERNAL_MODE 9


/* Return value.  */
#define CLI_SUCCESS           0
#define CLI_ERROR             1
#define CLI_AUTH_REQUIRED     2
#define CLI_EOL               3

/* Max length of each token.  */
#define MAX_TOKEN_LENGTH   256

/* Used for shield system API other than show command */
#undef SDK_INTERNAL_CLIS

/* Used for shield system API in show command */
#define HUMBER_INTERNAL_CLI_SHOW

/* Common descriptions. */
#define CTC_CLI_SHOW_STR "Show running system information"
#define CTC_CLI_NO_STR      "Negate a command or set its defaults"
#define CTC_CLI_CLEAR_STR   "Clear functions"
#define CTC_CLI_ENABLE      "Enable functions"
#define CTC_CLI_DISABLE     "Disable functions"
#define CTC_CLI_SHOW_SYS_MEM_STR  "Memory information"
#define CTC_CLI_DEBUG_STR  "Debugging functions"
#define CTC_CLI_BOOL_VAR_STR  "Boolean variable"

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

/* ACL -BEGIN-*/
#define CTC_CLI_ACL_STR              "ACL module"
#define CTC_CLI_ACL_GROUP_ID_STR     "ACL group id"
#define CTC_CLI_ACL_B_GROUP_ID_VALUE   "<0xFFFF0002>"
#define CTC_CLI_ACL_S_GROUP_ID_VALUE   "<0xFFFF0000>"
#define CTC_CLI_ACL_GROUP_ID_VALUE   "<0~0xFFFFFFFF>"
#define CTC_CLI_ACL_GROUP_PRIO_STR   "ACL group priority"
#define CTC_CLI_ACL_GROUP_PRIO_VALUE "<0~3> :0 is highest"
#define CTC_CLI_ACL_ENTRY_ID_STR     "ACL entry id"
#define CTC_CLI_ACL_ENTRY_ID_VALUE   "<0~0xFFFFFFFF>"
#define CTC_CLI_ACL_ENTRY_PRIO_STR   "ACL entry priority"
#define CTC_CLI_ACL_ENTRY_PRIO_VALUE "<0~0xFFFFFFFF> :0 is lowest"
/* SCL -BEGIN-*/
#define CTC_CLI_SCL_STR              "SCL module"
#define CTC_CLI_SCL_GROUP_ID_STR     "SCL group id"
#define CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE   "<0~0xFFFF0000>"
#define CTC_CLI_SCL_MAX_GROUP_ID_VALUE   "<0xFFFF000C>"
#define CTC_CLI_SCL_HASH_GROUP_ID_VALUE   "<0xFFFF0000~0xFFFF000C>"
#define CTC_CLI_SCL_ENTRY_ID_STR     "SCL entry id"
#define CTC_CLI_SCL_ENTRY_ID_VALUE   "<0~0x7FFFFFFF>"
#define CTC_CLI_SCL_ENTRY_PRIO_STR   "SCL entry priority"
#define CTC_CLI_SCL_ENTRY_PRIO_VALUE "<0~0xFFFFFFFF> :0 is lowest"
#define CTC_CLI_SCL_SERVICE_ID_VALUE "<1-0xFFFF>"

#define CTC_CLI_PARSER_L2_TYPE_VALUE "<0-15>"
#define CTC_CLI_PARSER_L2_TYPE_MASK  "<0-0xF>"
#define CTC_CLI_PARSER_L3_TYPE_VALUE "<0-15>"
#define CTC_CLI_PARSER_L3_TYPE_MASK  "<0-0xF>"
#define CTC_CLI_PARSER_L4_TYPE_VALUE "<0-15>"
#define CTC_CLI_PARSER_L4_TYPE_MASK  "<0-0xF>"
#define CTC_CLI_ETHER_TYPE_VALUE     "<0-0xFFFF>"
#define CTC_CLI_ETHER_TYPE_MASK      "<0-0xFFFF>"
#define CTC_CLI_PRIORITY_VALUE       "<0-0x3F>"
#define CTC_CLI_COLOR_VALUE          "<1-3> 0 is invalid, 1:red 2:yellow 3:green"
#define CTC_CLI_MICRO_FLOW_POLICER_ID_VALUE "<0-0xFFFF>"
#define CTC_CLI_MACRO_FLOW_POLICER_ID_VALUE "<0-0xFFFF>"
#define CTC_CLI_SESSION_ID_VALUE    "<0-3>"
#define CTC_CLI_LOG_PERCENT_VALUE   "<0-0xF> :0 is 2 (-15), 1 is 2 (-14), ... , 0xE is 50% ,0xF is 100% "
#define CTC_CLI_DSCP_VALUE          "<0-0x3F>"
#define CTC_CLI_DSCP_MASK           "<0-0x3F>"
#define CTC_CLI_IP_PREC_VALUE       "<0-0x7>"
#define CTC_CLI_IP_PREC_MASK        "<0-0x7>"
#define CTC_CLI_L4_PROTOCOL_VALUE   "<0-0xFF> ICMP:1(ipv4) 58(ipv6)  IGMP:2(ipv4)    TCP:6   UDP :17"
#define CTC_CLI_L4_PROTOCOL_MASK    "<0-0xFF>"
#define CTC_CLI_FRAG_VALUE          "0:non-fragment. 1:first fragment. 2:non or first. 3:small fragment. 4:not first fragment"
#define CTC_CLI_ICMP_TYPE_VALUE     "<0-0xFF>"
#define CTC_CLI_ICMP_TYPE_MASK      "<0-0xFF>"
#define CTC_CLI_ICMP_CODE_VALUE     "<0-0xFF>"
#define CTC_CLI_ICMP_CODE_MASK      "<0-0xFF>"
#define CTC_CLI_IGMP_TYPE_VALUE     "<0-0xFF>"
#define CTC_CLI_IGMP_TYPE_MASK      "<0-0xFF>"
#define CTC_CLI_IP_OPTION           "<0-1>"
#define CTC_CLI_IP_HDR_ERROR        "<0-1>"
#define CTC_CLI_ROUTED_PACKET       "<0-1>"
#define CTC_CLI_IP_OPTION           "<0-1>"
#define CTC_CLI_IPV4_PACKET_VALUE   "<0-1>"
#define CTC_CLI_TCP_FLAGS           "<0-0x3F>"
#define CTC_CLI_IPV6_EXT_HDR_VALUE  "<0-0xFF>"
#define CTC_CLI_IPV6_EXT_HDR_MASK   "<0-0xFF>"
#define CTC_CLI_IPV6_FLOW_LABEL_VALUE "<0-0xFFFFF>"
#define CTC_CLI_IPV6_FLOW_LABEL_MASK  "<0-0xFFFFF>"
#define CTC_CLI_MPLS_LABEL_VALUE    "<0-0xFFFFFFFF>"
#define CTC_CLI_MPLS_LABEL_MASK     "<0-0xFFFFFFFF>"
#define CTC_CLI_ARP_PACKET_VALUE    "<0-1>"
#define CTC_CLI_ARP_OP_CODE_VALUE  "<0-0xFFFF>"
#define CTC_CLI_ARP_OP_CODE_MASK   "<0-0xFFFF>"
#define CTC_CLI_L4_PORT_VALUE      "<0-0xFFFF>"
#define CTC_CLI_ACL_PORT_BITMAP "Port bitmap (52 ports supported)"
#define CTC_CLI_ACL_PORT_BITMAP_HIGH_VALUE "<0-0xFFFFF> each bit stands a port valid. 20 high ports: port32~ port51"
#define CTC_CLI_ACL_PORT_BITMAP_LOW_VALUE "<0-0xFFFFFFFF>, 32 low ports: port0~ port31"
#define CTC_CLI_ACL_VLAN_CLASS_ID_VALUE  "<1-1023> 0 is reserved"
#define CTC_CLI_ACL_PORT_CLASS_ID_VALUE  "<1-1023> 0 is reserved"
#define CTC_CLI_ACL_SERVICE_ID_VALUE     "<1-0xFFFF>"
#define CTC_CLI_ACL_PBR_CLASS_ID_VALUE   "<0-63>"
#define CTC_CLI_LCHIP_ID_VALUE           "<0-1>"
#define CTC_CLI_LCHIP_ID_VALUE           "<0-1>"
#define CTC_CLI_STATS_COLOR_STR          "Stats color "
#define CTC_CLI_STATS_COLOR_VALUE        "<0-1> 0 green, 1 not green. "
/*ACL -END-*/

/*OAM -BEGIN-*/
#define CTC_CLI_OAM_M_STR "OAM module"
#define CTC_CLI_OAM_CFM_M_STR "CFM"
#define CTC_CLI_OAM_MPLS_TP_Y1731_M_STR "MPLS TP Y.1731"
#define CTC_CLI_OAM_MPLS_TP_BFD_M_STR "MPLS TP BFD"
#define CTC_CLI_OAM_EFM_M_STR "EFM"
#define CTC_CLI_OAM_COMMON_M_STR "OAM Common"
#define CTC_CLI_TRPT_M_STR "ThroughPut"

/*OAM -END-*/

#define CTC_CLI_SCL_ID_VALUE        "<0-1> There are 2 scl lookup. Each has its own feature."

/* QOS -BEGIN-*/
#define CTC_CLI_QOS_STR "QoS module"
#define CTC_CLI_QOS_PLC_STR "QoS policer"
#define CTC_CLI_QOS_DOMAIN_STR "QoS domain maping"
#define CTC_CLI_QOS_SHAPE_STR "QoS shaping"
#define CTC_CLI_QOS_SCHED_STR "QoS schedule"
#define CTC_CLI_QOS_STATS_STR "QoS statstics"
#define CTC_CLI_QOS_QUEUE_STR "Queue id"
#define CTC_CLI_QOS_QUEUE_VAL "<0-7>"
#define CTC_CLI_QOS_SERVICE_STR "Service identification"
#define CTC_CLI_QOS_SERVICE_VAL "<0-0xFFFF>"
#define CTC_CLI_QOS_FLOW_PLC_STR "QoS flow policer"
#define CTC_CLI_QOS_FLOW_PLC_VAL "<1-0xFFFF>"
#define CTC_CLI_QOS_PLC_CIR_STR "Config CIR (Commit Information Rate)"
#define CTC_CLI_QOS_PLC_CIR_VAL "<0-10000000>, rate in Kbps (1000/sec)"
#define CTC_CLI_QOS_PLC_EIR_STR "Config EIR (Excess Information Rate)"
#define CTC_CLI_QOS_PLC_EIR_VAL "<0-10000000>, rate in Kbps (1000/sec)"
#define CTC_CLI_QOS_PLC_PIR_STR "Config PIR (Peak Information Rate)"
#define CTC_CLI_QOS_PLC_PIR_VAL "<0-10000000>, rate in Kbps (1000/sec)"
#define CTC_CLI_QOS_PLC_CBS_STR "Config CBS (Commit Burst Size)"
#define CTC_CLI_QOS_PLC_CBS_VAL "<0-1024>, burst size in Kbits (1000), default is 64"
#define CTC_CLI_QOS_PLC_EBS_STR "Config EBS (Excess Burst Size)"
#define CTC_CLI_QOS_PLC_EBS_VAL "<0-1024>, burst size in Kbits (1000), default is 64"
#define CTC_CLI_QOS_PLC_PBS_STR "Config PBS (Peak Burst Size)"
#define CTC_CLI_QOS_PLC_PBS_VAL "<0-1024>, burst size in Kbits (1000), default is 64"
/* QOS -END-*/

#define CTC_CLI_CPU_TRAFFIC_STR "CPU Traffic module"
#define CTC_CLI_SYNC_ETHER_M_STR "SyncE module"
#define CTC_CLI_PDU_STR "Pdu modue"
#define CTC_CLI_LEARNING_AGING_STR "Learning and Aging module"
#define CTC_CLI_LEARNING_STR "Learning module"
#define CTC_CLI_AGING_STR "Aging module"
#define CTC_CLI_SECURITY_STR "Security module"
#define CTC_CLI_APS_M_STR "Aps Module"
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
#define CTC_CLI_DEBUG_LEVEL_NONE   "Output none information "

/*parameter*/
#define CTC_CLI_L2_HDR_STR "Layer 2 header of packet"
#define CTC_CLI_L3_HDR_STR "Layer 3 header of packet"
#define CTC_CLI_L4_HDR_STR "Layer 4 header of packet"
#define CTC_CLI_STATS_STR "Statistics"
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
#define CTC_CLI_IPV4_MASK_LEN_FORMAT "The length of Ipv4 mask <0~32>"
#define CTC_CLI_IPV6_FORMAT "IPv6 address in X:X::X:X format"
#define CTC_CLI_IPV6_MASK_FORMAT "IPv6 mask in X:X::X:X format"
#define CTC_CLI_IPV6_MASK_LEN_FORMAT "The length of Ipv6 mask <0~128>"

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
#define CTC_CLI_NH_ID_STR "Nexthop ID, Drop nhid is 1, ToCpu nexthop is 2, external nhid's range is 3 ~ 16383 (the max external nhid can be configed before initialize, default is 16383), internal nhid's range is 16384 ~ 49151"
#define CTC_CLI_APS_BRIDGE_ID_STR      "Aps bridge id"
#define CTC_CLI_APS_BRIDGE_ID_DESC     "<0-1023>"

#define CTC_CLI_GPORT_DESC "Global port"

#define CTC_CLI_CHIP_DESC "Chip id on linecard"
#define CTC_CLI_CHIP_ID_DESC "<0-1>"
#define CTC_CLI_GCHIP_DESC "Global chip id"
#define CTC_CLI_GCHIP_ID_DESC "<0-29>"
#define CTC_CLI_LINKAGG_DESC "Linkagg"
#define CTC_CLI_L3IF_ID_DESC "L3if ID"
#define CTC_CLI_ACLQOS_LABEL_ID_DESC    "<1-4294967295>"
#define CTC_CLI_IPMC_GROUP_DESC         "IP multicast group"
#define CTC_CLI_IPMC_MEMBER_DESC        "Member of the ipmc group"
#define CTC_CLI_SERVICE_ID_DESC "<1-65535>"
#define CTC_CLI_PFM_RANGE_DESC "<0-2>"
#define CTC_CLI_TAG_OP_DESC "0:NONE(Do nothing), 1:REP_OR_ADD (Replace for tagged, add for untagged. Rep or add is auto-ajusted), 2:APPEND(add for untagged, even for tagged ), 3:DELETE (delete tagged)"
#define CTC_CLI_TAG_SL_DESC "0:AS_RECEIVE(Select packet's vid|cos), 1:ALTERNATE(Select the other tag's vid|cos), 2:NEW (Select the tag user assigned), 3:Default (Select the default vid|cos)"
#define CTC_CLI_ACL_VLAN_TAG_OP_DESC "0:NONE(Do nothing), 1:REP (Replace only for tagged), 2:ADD (append a new tag), 3:DELETE (delete tag only for tagged)"
#define CTC_CLI_ACL_VLAN_TAG_SL_DESC "0:NONE(Do nothing), 1:ALTERNATE(Select the other tag's vid|cos|cfi), 2:NEW (Select the tag user assigned)"
#define CTC_CLI_ETHER_TYPE_RANGE    "<0x0000-0xFFFF>"
#define CTC_CLI_INTR_TYPE_STR "Interrupt sup-level type"
#define CTC_CLI_INTR_SUB_TYPE_STR "Interrupt sub-level type"
#define CTC_CLI_INTR_TYPE_VALUE "<0-0xFFFFFFFF>"
#define CTC_CLI_QOS_DOMAIN_VALUE    "<1-7> 0 is invalid"

#if defined (GREATBELT) || defined (RIALTO) || defined (RAMA)

#define CTC_CLI_TRUST_VALUE  "<0-6> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence, 5=trust stag-cos,6=trust ctag-cos"
#define CTC_CLI_GPHYPORT_ID_DESC "<0-0x1D7F>"
#define CTC_CLI_USER_VLAN_PTR  CTC_CLI_VLAN_RANGE_DESC
#define CTC_CLI_MAPPING_L3TYPE_INDEX  "Index Value <0-3>"
#define CTC_CLI_MAPPING_L4TYPE_INDEX  "Index value <0-3>"
#define CTC_CLI_L2FLEX_BASIC_OFFSET  "Offset value <0x0-0x3F>"
#define CTC_CLI_PORT_ISOLATION_ID_DESC "<0-31>(default value 0 indicate not an isolated port)"
#define CTC_CLI_GPORT_ID_DESC                   "<0-0x1D7F, 0x1F00~0x1F3F(linkagg)>"
#define CTC_CLI_EDGE_PORT_ID_DESC     CTC_CLI_GPHYPORT_ID_DESC
#define CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC   CTC_CLI_GPHYPORT_ID_DESC
#define CTC_CLI_L3IF_ID_RANGE_DESC "<1-1022>"
#define CTC_CLI_VLAN_RANGE_GRP_ID_VALUE "<0-63>"
#define CTC_CLI_FID_ID_DESC "Defined by user, default max value is 5k,4k vlan + 1k vsi<vpls>"
#define CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX "<0-9>"
#define CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX "<0-31>"
#define CTC_CLI_L3PDU_ENTRY_INDEX "<0-7>"
#define CTC_CLI_L3PDU_GLOBAL_ACTION_INDEX "<0-31>"
#define CTC_CLI_L2PDU_L3TYPE_ACTION_INDEX "<32-53>"
#define CTC_CLI_LINKAGG_ID_DESC "<0-31>"
#define CTC_CLI_VRFID_ID_DESC "Defined by user, default is <0~255>"
#define CTC_CLI_MAX_ECMP_DESC "Max_ecmp the maximum ECMP paths allowed for a route.<2-16>"
#define CTC_CLI_ACL_ENABLE_BITMAP "Acl enable (bitmap 0~15)"
#define CTC_CLI_SCL_PORT_CLASS_ID_VALUE "<1~61>"
#define CTC_CLI_APS_GROUP_ID_DESC "<0-1023>"
#define CTC_CLI_QOS_CLASS_VAL "<0-7>"
#define CTC_CLI_QOS_DROP_THRESHOLD "<0-0x3FFF>"
#define CTC_CLI_QOS_DROP_PROB_VAL "<0-127>"
#define CTC_CLI_QOS_WDRR_WEIGHT_VAL "<1-(0x3FFFFFF/MTU) max is 0xFFFF>, Defulat MTU(3200)"
#define CTC_CLI_STATS_MTU1_DESC "<1024 - 16383> (default:1518)"
#define CTC_CLI_STATS_MTU2_DESC "<1024 - 16383> (default:1536)"
#define CTC_CLI_PORT_MAX_FRAME_SIZE "<0-16128> (default:9600)"
#define CTC_CLI_PORT_MIN_FRAME_SIZE "<18-64>"
#define CTC_CLI_PORT_INTERNAL_PORT_START "<74-127>"
#define CTC_CLI_PORT_EXTENDER_NUM "<0-54>"
#endif  /*!GREATBELT*/
#ifdef HUMBER
#define CTC_CLI_TRUST_VALUE  "<0-4> 0=trust Port, 1=copy outer, 2=trust COS, 3=trust DSCP, 4=trust IP Precedence"
#define CTC_CLI_GPHYPORT_ID_DESC "<0-0x1DFF>"
#define CTC_CLI_USER_VLAN_PTR  "not support in mode1. 1~7167 in mode2. 4288~4799 in mode3."
#define CTC_CLI_MAPPING_L3TYPE_INDEX  "Index Value <0-2>"
#define CTC_CLI_MAPPING_L4TYPE_INDEX  "Index value <0,1,2,3,6>"
#define CTC_CLI_L2FLEX_BASIC_OFFSET  "Offset value <0x0-0xFF>"
#define CTC_CLI_PORT_ISOLATION_ID_DESC  "<0-63> (default value 63 indicate not an isolated port)"
#define CTC_CLI_GPORT_ID_DESC "<0-0x1DFF, 0x1F00~0x1F7F(linkagg)>"
#define CTC_CLI_EDGE_PORT_ID_DESC     CTC_CLI_GPORT_ID_DESC
#define CTC_CLI_L3IF_ID_RANGE_DESC "<0-1022>"
#define CTC_CLI_FID_ID_DESC "<0-16383>"
#define CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX "<0-7>"
#define CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX "<0-14>"
#define CTC_CLI_L3PDU_ENTRY_INDEX "<0-15>"
#define CTC_CLI_L3PDU_GLOBAL_ACTION_INDEX "<0-14>"
#define CTC_CLI_L2PDU_L3TYPE_ACTION_INDEX "<0-14>"
#define CTC_CLI_LINKAGG_ID_DESC "<0-127>"
#define CTC_CLI_VRFID_ID_DESC "VRF id of route <0-8191>"
#define CTC_CLI_MAX_ECMP_DESC "Max_ecmp the maximum ECMP paths allowed for a route.<1-8>"
#define CTC_CLI_ACL_ENABLE_BITMAP "Acl enable (bitmap 0~3)"
#define CTC_CLI_SCL_PORT_CLASS_ID_VALUE "<0~63>"
#define CTC_CLI_APS_GROUP_ID_DESC "<0-2047>"
#define CTC_CLI_QOS_CLASS_VAL "<0-3>"
#define CTC_CLI_QOS_DROP_THRESHOLD "<0-0xFFFF>"
#define CTC_CLI_QOS_DROP_PROB_VAL "<0-15>"
#define CTC_CLI_QOS_WDRR_WEIGHT_VAL "<1-(0xFFFFFF/MTU) max is 0xFFFF>, Defulat MTU(2000)"
#define CTC_CLI_STATS_MTU1_DESC "<1024 - 16383> (default:1518)"
#define CTC_CLI_STATS_MTU2_DESC "<1024 - 8191> (default:1536)"
#define CTC_CLI_PORT_MAX_FRAME_SIZE "0:size0, 1:size1. (default:0)"
#define CTC_CLI_PORT_MIN_FRAME_SIZE "<33-64>"
#define CTC_CLI_PORT_INTERNAL_PORT_START "<64-127>"
#define CTC_CLI_PORT_EXTENDER_NUM "<0-63>"
#endif  /*!HUMBER*/

/* Multicast static route CLI strings */

#define CTC_CLI_GET_ARGC_INDEX(str) ctc_cli_get_prefix_item(&argv[0], argc, str, sal_strlen(str))
#define CTC_CLI_GET_SPECIFIC_INDEX(str, idx) ctc_cli_get_prefix_item(&argv[idx], argc - idx, str, sal_strlen(str))
#define CLI_CLI_STR_EQUAL(str, idx)  (0 == sal_strncmp((str), argv[(idx)], sal_strlen(str)))
#define CTC_CLI_STR_EQUAL_ENHANCE(str, idx)  \
    ((0 == sal_strncmp((str), argv[(idx)], sal_strlen(str))) && (sal_strlen(argv[idx]) == sal_strlen(str)))
#define INDEX_VALID(index)  (0xFF != (index))

#define CLI_MROUTE_STR      "Configure static multicast routes"

#define UINT64_STR_LEN      21

#define CTC_MAX_UINT16_VALUE 0xFFFF
#define CTC_MAX_UINT32_VALUE 0xFFFFFFFF
#define CTC_MAX_UINT8_VALUE 0xFF

#define MAX_CPU_PKT_FILE_NAME_SIZE      256

struct ctc_l2_write_info_para_s
{
    char file[MAX_CPU_PKT_FILE_NAME_SIZE];
    void* pdata;
};
typedef struct ctc_l2_write_info_para_s ctc_l2_write_info_para_t;

typedef int (* CTC_CLI_OUT_FUNC) (const char* str, ...);
int ctc_cli_out(const char* fmt, ...);

typedef uint16 (* CTC_CLI_PORT_MAP_FUNC) (uint16, uint8);
extern CTC_CLI_PORT_MAP_FUNC cli_port_map_func_ptr;

#ifdef HAVE_ISO_MACRO_VARARGS
#define CTC_CLI(func_name, cli_name, cmd_str, ...)  \
    char* cli_name ## _help[] = {__VA_ARGS__, NULL}; \
    DEFUN(func_name, cli_name, cmd_str, cli_name ## _help)
#else
#define CTC_CLI(func_name, cli_name, cmd_str, ARGS...) \
    char* cli_name ## _help[] = {ARGS, NULL}; \
    DEFUN(func_name, cli_name, cmd_str, cli_name ## _help)
#endif

extern int32
ctc_cmd_str2int(char* str, int32* ret);
extern uint32
ctc_cmd_str2uint(char* str, int32* ret);
extern void
    ctc_uint64_to_str(uint64 src, char dest[UINT64_STR_LEN]);

extern int32
ctc_cmd_judge_is_num(char* str);

extern char*
ctc_cli_get_debug_desc(unsigned char level);

#define CTC_CLI_GET_INTEGER(NAME, V, STR)                \
    {                                                  \
        int32 retv;                                      \
        (V) = ctc_cmd_str2int((STR), &retv);            \
        if (retv < 0)                                    \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_GET_INTEGER_RANGE(NAME, V, STR, MIN, MAX)  \
    {                                                  \
        int32 retv;                                      \
        (V) = ctc_cmd_str2int((STR), &retv);            \
        if (retv < 0 || (V) < (MIN) || (V) > (MAX))      \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_GET_INTEGER64(NAME, V, INT, STR, MIN, MAX)  \
    {                                                  \
        int32 retv;                                      \
        int32 val = 0;                                   \
        (V) = ctc_cmd_str2int((STR), &retv);            \
        sal_memcpy(&INT.l[0], &val, 4);                 \
        sal_memcpy(&INT.l[1], &(V), 4);                 \
        if (retv < 0 || (V) < (MIN) || (V) > (MAX))      \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_GET_UINT32(NAME, V, STR)             \
    {                                                  \
        int32 retv;                                      \
        (V) = ctc_cmd_str2uint((STR), &retv);            \
        if (retv < 0)                                    \
        {                                               \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                               \
    }
#define CTC_CLI_GET_UINT16(NAME, V, STR)             \
    {                                                  \
        int32 retv;                                      \
        uint32 tmp = 0;                                   \
        uint16 temp_global_port = 0;                                 \
        uint8 j = 0;                                     \
        char string[100] = {0};                       \
                                                      \
        sal_memcpy(string, NAME, sal_strlen(NAME)); \
        tmp = ctc_cmd_str2uint((STR), &retv);           \
        if (retv < 0 || tmp > CTC_MAX_UINT16_VALUE)       \
        {                                               \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                               \
        (V) = (uint16)tmp;                               \
        for (j = 0; j < sal_strlen(string); j++)            \
        {                                                 \
            (string[j]) = sal_tolower((string[j]));      \
        }                                                 \
        if (cli_port_map_func_ptr)\
        {\
            if ((0 == sal_strncmp((string), "gport", sal_strlen(string)))||  \
               (0 == sal_strncmp((string), "port", sal_strlen(string))))\
            {                                                         \
                temp_global_port = (*cli_port_map_func_ptr)((V),1);       \
                if (temp_global_port == 0xFFFF)                                  \
                {                                                     \
                    ctc_cli_out("%% Invalid %s value\n", "port");     \
                    return CLI_ERROR;                                 \
                }                                                     \
                V = temp_global_port;                                 \
            }                                                         \
        }\
    }
#define CTC_CLI_GET_UINT8(NAME, V, STR)              \
    {                                                  \
        int32 retv;                                      \
        uint32 tmp = 0;                                   \
        uint8 j = 0;                                     \
        uint16 temp_global_port = 0;                     \
        char string[100] = {0};                       \
        sal_memcpy(string, NAME, sal_strlen(NAME)); \
        tmp = ctc_cmd_str2uint((STR), &retv);           \
        if (retv < 0 || tmp > CTC_MAX_UINT8_VALUE)       \
        {                                               \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                               \
        (V) = (uint8)tmp;                                \
        for (j = 0; j < sal_strlen(string); j++)            \
        {                                                 \
            (string[j]) = sal_tolower((string[j]));      \
        }                                                 \
        if (cli_port_map_func_ptr)\
        {\
            if ((0 == sal_strncmp((string), "lport", sal_strlen(string)))||  \
               (0 == sal_strncmp((string), "port", sal_strlen(string))))\
            {                                                         \
                temp_global_port = (*cli_port_map_func_ptr)((V),0);       \
                if (temp_global_port == 0xFFFF)                                  \
                {                                                     \
                    return CLI_ERROR;                                 \
                }                                                     \
                V = temp_global_port;                                 \
            }                                                         \
        }\
    }

#define CTC_CLI_GET_UINT8_RANGE(NAME, V, STR, MIN, MAX) CTC_CLI_GET_UINT8(NAME, V, STR)
#define CTC_CLI_GET_UINT16_RANGE(NAME, V, STR, MIN, MAX) CTC_CLI_GET_UINT16(NAME, V, STR)
#define CTC_CLI_GET_UINT32_RANGE(NAME, V, STR, MIN, MAX)  CTC_CLI_GET_UINT32(NAME, V, STR)

#define CTC_CLI_GET_MAC_ADDRESS(NAME, V, STR)                                                                  \
    {                                                                                                        \
        int32 retv = 0;                                                                                      \
        uint8 i = 0;                                                                                         \
        while (STR[i] != '\0')                                                                                 \
        {                                                                                                    \
            if (STR[i] == '.'){ retv++; }                                                                    \
            i++;                                                                                             \
        }                                                                                                    \
        if (retv != 2)                                                                                       \
        {                                                                                                    \
            ctc_cli_out("%% invalid %s value\n", NAME);                                                     \
            return CLI_ERROR;                                                                                \
        }                                                                                                    \
        retv = sal_sscanf((STR), "%4hx.%4hx.%4hx", (uint16*)&(V[0]), (uint16*)&(V[2]), (uint16*)&(V[4])); \
        if (retv != 3)                                                                                       \
        {                                                                                                    \
            ctc_cli_out("%% invalid %s value\n", NAME);                                                     \
            return CLI_ERROR;                                                                                \
        }                                                                                                    \
        *(uint16*)&(V[0]) = sal_htons(*(uint16*)&(V[0]));                                                  \
        *(uint16*)&(V[2]) = sal_htons(*(uint16*)&(V[2]));                                                  \
        *(uint16*)&(V[4]) = sal_htons(*(uint16*)&(V[4]));                                                  \
    }

#define CTC_CLI_GET_IPV4_ADDRESS(NAME, V, STR)               \
    {                                                      \
        int32 retv;                                          \
        retv = sal_inet_pton(AF_INET, (STR), &(V));         \
        if (!retv)                                           \
        {                                                  \
            ctc_cli_out("%% Invalid %s value\n", NAME);     \
            return CLI_ERROR;                                \
        }                                                  \
        (V) = sal_htonl(V);                                  \
    }

#define CTC_CLI_GET_IPV6_ADDRESS(NAME, V, STR)           \
    {                                                  \
        int32 retv;                                      \
        retv = sal_inet_pton(AF_INET6, (STR), &(V));    \
        if (!retv)                                       \
        {                                              \
            ctc_cli_out("%% Invalid %s value\n", NAME); \
            return CLI_ERROR;                            \
        }                                              \
    }

#define CTC_CLI_IS_NUM(STR)                           \
    {                                                 \
        int32 retv;                                   \
        retv = ctc_cmd_judge_is_num((STR));          \
        if (0 != retv)                                  \
        {                                             \
            ctc_cli_out("%% Incomplete command\n");  \
            return CLI_ERROR;                         \
        }                                             \
    }

enum cmd_token_type_s
{
    cmd_token_paren_open,
    cmd_token_paren_close,
    cmd_token_cbrace_open,
    cmd_token_cbrace_close,
    cmd_token_brace_open,
    cmd_token_brace_close,
    cmd_token_separator,
    cmd_token_keyword,
    cmd_token_var,
    cmd_token_unknown
};
typedef enum cmd_token_type_s cmd_token_type;

/* APIs, user shall also call  CTC_CLI(), install_element() */
extern struct vty* vty;

extern int
vty_read (void);
extern void
ctc_cmd_init(int terminal);
extern void ctc_cli_register_print_fun(CTC_CLI_OUT_FUNC func);
extern unsigned char
ctc_cli_get_prefix_item(char** argv, unsigned char argc, char* prefix, unsigned char prefix_len);
extern int
vtysh_vty_command (struct vty *vty, char *buf);

extern void
set_terminal_raw_mode(void);

extern void
restore_terminal_mode(void);

extern void
 cli_enable_cmd_debug(int enable);

extern void
cli_enable_arg_debug(int enable);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_CLI_H */

