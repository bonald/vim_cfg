
/****************************************************************************
 * ctp_header_format.h Provides all CTP Supportted header protocol enumeration.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * This file is automatic generate by CTP_GEN_HEADER tools and use a script
 * input file named ctp_header_format.txt
 *
*****************************************************************************/

#ifndef _CTP_HEADER_FOMAT_H_
#define _CTP_HEADER_FOMAT_H_

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "ctp_common.h"

/****************************************************************************
 *
 * Data Structure and Defines 
 *
****************************************************************************/

/*
 * This enumeration is use for list all header types
*/
enum
{
    CPUPKT,
    WIRELESSDOT11,
    ETHERNET,
    PKT_PAYLOAD,
    MACSECTAG,
    SGTCMD,
    NSH,
    SLPR,
    TRILL,
    RBCHANNEL,
    MPLS,
    ACH,
    MPLSLM,
    MPLSDM,
    MPLSLDM,
    CSF,
    CW,
    L2H,
    CFM,
    PBB,
    ETAG,
    FIP,
    FCOE,
    CCM,
    GRE,
    EAPOL,
    IPV4,
    IPV6,
    IPV6_HBH_OPT_HEADER,
    IPV6_DEST_OPT_HEADER,
    IPV6_ROUT_HEADER,
    IPV6_FRAG_HEADER,
    IPV6_AUTHEN_HEADER,
    IPV6_ENCAP_SEUC_PAYL_HEADER,
    VRRP,
    TCP,
    UDP,
    VXLAN,
    OSPF,
    L2TP,
    DHCP,
    RIP,
    RADIUS,
    LDP,
    BFD,
    PTP,
    CAPWAP,
    DTLS,
    LLC,
    TWAMP,
    OWAMP,
    IGMP,
    ICMP,
    ICMPV6,
    ARP,
    STP,
    IPV6_ENCAP_IPV6_HEADER,
    HEADER_TYPE_NUM,
};

/*
 * This enumeration is use for list all header fields
*/
enum
{
    CPUPKT_EXIST,
    CPUPKT_MACDA,
    CPUPKT_MACSA,
    CPUPKT_VLANEXIST,
    CPUPKT_TPID,
    CPUPKT_VLAN,
    CPUPKT_TYPE,
    CPUPKT_RES,
    CPUPKT_HM_HEADER_TYPE,
    CPUPKT_HM_DESTID_DISCARD,
    CPUPKT_HM_PACKET_OFFSET1,
    CPUPKT_HM_MCAST,
    CPUPKT_HM_DEST_CHIPID,
    CPUPKT_HM_DESTID,
    CPUPKT_HM_PRIORITY,
    CPUPKT_HM_PACKET_TYPE,
    CPUPKT_HM_SOURCE_COS,
    CPUPKT_HM_SRC_QUEUE_SELECT,
    CPUPKT_HM_HEADER_HASH1,
    CPUPKT_HM_VPLS_PORT_TYPE,
    CPUPKT_HM_UNTAG_PACKET,
    CPUPKT_HM_SOURCE_PORT,
    CPUPKT_HM_SRC_VLANID,
    CPUPKT_HM_COLOR,
    CPUPKT_HM_NEXT_HOP_PTR1,
    CPUPKT_HM_LEN_ADJUST_TYPE,
    CPUPKT_HM_CRITICAL_PACKET,
    CPUPKT_HM_PACKET_LEN,
    CPUPKT_HM_TTL_PTPOFFSET,
    CPUPKT_HM_HEADER_CRC,
    CPUPKT_HM_SRCPORT_ISID,
    CPUPKT_HM_PBB_SRCPORT_TYPE,
    CPUPKT_HM_PACKET_OFFSET2,
    CPUPKT_HM_SOURCE_CFI,
    CPUPKT_HM_NEXT_HOP_EXT,
    CPUPKT_HM_NEXT_HOP_PTR2,
    CPUPKT_HM_SVLAN_TPID_INDEX,
    CPUPKT_HM_DENY_REPLACE_COS,
    CPUPKT_HM_DENY_REPLACE_DSCP,
    CPUPKT_HM_SRC_SVLANID_VALID,
    CPUPKT_HM_SRC_CVLANID_VALID,
    CPUPKT_HM_SRC_CVLAN_ID,
    CPUPKT_HM_SRC_VLAN_PTR,
    CPUPKT_HM_VRFID_DESTPORT,
    CPUPKT_HM_L4_SRCPORT,
    CPUPKT_HM_SRCPORT_BUNDLEID,
    CPUPKT_HM_SRC_TARGED,
    CPUPKT_HM_USE_OUTER_VRFID,
    CPUPKT_HM_PFM,
    CPUPKT_HM_OAM_TUNNEL_EN,
    CPUPKT_HM_MIRROR,
    CPUPKT_HM_OPERATION_TYPE,
    CPUPKT_HM_HEADER_HASH2,
    CPUPKT_HM_IP_SA,
    CPUPKT_GB_SRCPORT_15_14,
    CPUPKT_GB_PACKET_OFFSET,
    CPUPKT_GB_MCAST,
    CPUPKT_GB_DEST_CHIPID,
    CPUPKT_GB_DESTID,
    CPUPKT_GB_PRIORITY,
    CPUPKT_GB_PACKET_TYPE,
    CPUPKT_GB_SOURCE_COS,
    CPUPKT_GB_SRC_QUEUE_SELECT,
    CPUPKT_GB_HEADER_HASH2_0,
    CPUPKT_GB_LOGIC_PORT_TYPE,
    CPUPKT_GB_SRC_CTAG_OFFSET_TYPE,
    CPUPKT_GB_SRCPORT,
    CPUPKT_GB_SRC_VLANID,
    CPUPKT_GB_COLOR,
    CPUPKT_GB_NEXT_HOP_PTR,
    CPUPKT_GB_LEN_ADJUST_TYPE,
    CPUPKT_GB_CRITICAL_PACKET,
    CPUPKT_GB_RXTXFCL_22_17,
    CPUPKT_GB_DM_TS_36_29,
    CPUPKT_GB_TTL,
    CPUPKT_GB_FROM_FABRIC,
    CPUPKT_GB_BYPASS_INGRESS_EDITING,
    CPUPKT_GB_SRCPORT_EXT,
    CPUPKT_GB_LOOPBACK_DISCARD,
    CPUPKT_GB_HEADER_CRC,
    CPUPKT_GB_SRCPORT_ISID,
    CPUPKT_GB_PBB_SRCPORT_TYPE,
    CPUPKT_GB_VLAN_TAG_OPERATION_VALID,
    CPUPKT_GB_SOURCE_CFI,
    CPUPKT_GB_NEXT_HOP_EXT,
    CPUPKT_GB_NON_CRC,
    CPUPKT_GB_FROM_CPU_OR_OAM,
    CPUPKT_GB_SVLAN_TPID_INDEX,
    CPUPKT_GB_STAG_ACTION,
    CPUPKT_GB_SRC_SVLANID_VALID,
    CPUPKT_GB_SRC_CVLANID_VALID,
    CPUPKT_GB_SRC_CVLAN_ID,
    CPUPKT_GB_LM_DOWN_DISABLE,
    CPUPKT_GB_INGRESS_EDIT_EN,
    CPUPKT_GB_OUTER_VLAN_IS_CVLAN,
    CPUPKT_GB_SRC_VLAN_PTR,
    CPUPKT_GB_FID,
    CPUPKT_GB_LOGIC_SRCPORT,
    CPUPKT_GB_RXTX_FCL3,
    CPUPKT_GB_CUT_THROUGH,
    CPUPKT_GB_RXTX_FCL2_1,
    CPUPKT_GB_MUX_LENGTH_TYPE,
    CPUPKT_GB_OAM_TUNNEL_EN,
    CPUPKT_GB_RXTX_FCL0,
    CPUPKT_GB_OPERATION_TYPE,
    CPUPKT_GB_HEADER_HASH7_3,
    CPUPKT_GB_IP_SA,
    CPUPKT_GG_PACKET_OFFSET,
    CPUPKT_GG_MCAST,
    CPUPKT_GG_DEST_CHIPID,
    CPUPKT_GG_DESTID,
    CPUPKT_GG_PRIORITY,
    CPUPKT_GG_PACKET_TYPE,
    CPUPKT_GG_SOURCE_COS,
    CPUPKT_GG_HEADER_HASH,
    CPUPKT_GG_LOGIC_PORT_TYPE,
    CPUPKT_GG_SRCPORT,
    CPUPKT_GG_SRC_VLANID,
    CPUPKT_GG_COLOR,
    CPUPKT_GG_NEXT_HOP_PTR,
    CPUPKT_GG_LEN_ADJUST_TYPE,
    CPUPKT_GG_CRITICAL_PACKET,
    CPUPKT_GG_IS_LEAF,
    CPUPKT_GG_SOURCE_CFI,
    CPUPKT_GG_NEXT_HOP_EXT,
    CPUPKT_GG_PORT_MACSA_EN,
    CPUPKT_GG_FROMCPULMUPDISABLE,
    CPUPKT_GG_SVLAN_TAG_OPERATION_VALID,
    CPUPKT_GG_SOURCEPORTEXTENDER,
    CPUPKT_GG_FROM_FABRIC,
    CPUPKT_GG_BYPASS_INGRESS_EDITING,
    CPUPKT_GG_DSNEXTHOP_BYPASSALL,
    CPUPKT_GG_FROMCPULMDOWNDISABLE,
    CPUPKT_GG_INGRESS_EDIT_EN,
    CPUPKT_GG_OUTER_VLAN_IS_CVLAN,
    CPUPKT_GG_FROM_CPU_OR_OAM,
    CPUPKT_GG_SVLAN_TPID_INDEX,
    CPUPKT_GG_SRC_VLAN_PTR,
    CPUPKT_GG_MUX_LENGTH_TYPE,
    CPUPKT_GG_OAM_TUNNEL_EN,
    CPUPKT_GG_BRIDGE_OP,
    CPUPKT_GG_MAC_LEARNING_EN,
    CPUPKT_GG_DEBUG_SESSION_EN,
    CPUPKT_GG_KEEP_OLD_HEADER,
    CPUPKT_GG_STAG_ACTION,
    CPUPKT_GG_MAC_KNOWN,
    CPUPKT_GG_OAM_USE_FID,
    CPUPKT_GG_FID,
    CPUPKT_GG_LOGIC_SRCPORT,
    CPUPKT_GG_TTL,
    CPUPKT_GG_OPERATION_TYPE,
    CPUPKT_GG_PTPTS_RXTXFCL,
    CPUPKT_GG_TS_0_37_HIGH,
    CPUPKT_GG_TS_0_37_LOW,
    CPUPKT_GG_SRC_DSCP,
    CPUPKT_GG_U1_MODE,
    CPUPKT_GG_L3_OFFSET,
    CPUPKT_GG_ECMPHASH,
    CPUPKT_GG_ACLDSCPVALID,
    CPUPKT_GG_ISIPV4,
    CPUPKT_GG_ACLDSCP,
    CPUPKT_GG_ECNAWARE,
    CPUPKT_GG_ECNEN,
    CPUPKT_GG_PBBCHECKDISCARD,
    CPUPKT_GG_CTAGACTION,
    CPUPKT_GG_SRC_CTAG_COS,
    CPUPKT_GG_SRC_CTAG_CFI,
    CPUPKT_GG_SRC_CTAG_OFFSET_TYPE,
    CPUPKT_GG_PBB_SRCPORT_TYPE,
    CPUPKT_GG_SRCPORT_ISOLATEID,
    CPUPKT_GG_SRCCVLANID,
    CPUPKT_GG_ISIDVALID,
    CPUPKT_GG_CTAG_OP_VALID,
    CPUPKT_GG_CONGESTION_VALID,
    CPUPKT_GG_NORMAL_ISELOOPPKT,
    CPUPKT_GG_METADATA_VALID,
    CPUPKT_GG_METADATA,
    CPUPKT_GG_OAM_RXOAM,
    CPUPKT_GG_OAM_MEPINDEX,
    CPUPKT_GG_OAM_OAM_TYPE,
    CPUPKT_GG_OAM_LINKOAM,
    CPUPKT_GG_OAM_LOCALPHYPORT,
    CPUPKT_GG_OAM_GAL_EXIST,
    CPUPKT_GG_OAM_ENTROPY_LABEL_EXIST,
    CPUPKT_GG_OAM_ISUP,
    CPUPKT_GG_OAM_DMEN,
    CPUPKT_GG_OAM_LM_RECEICED_PKT,
    CPUPKT_GG_OAM_RXFCB,
    CPUPKT_GG_OAM_RXTXFCL,
    CPUPKT_GG_OAM_ISELOOPPKT,
    CPUPKT_GG_DM_RESERVE,
    CPUPKT_GG_DM_OFFSET,
    CPUPKT_GG_DM_ISELOOPPKT,
    CPUPKT_GG_LM_PKTTYPE,
    CPUPKT_GG_LM_TXFCB,
    CPUPKT_GG_LM_RXFCB,
    CPUPKT_GG_LM_RXTXFCL,
    CPUPKT_GG_PTP_RESERVE,
    CPUPKT_GG_PTP_SEQID,
    CPUPKT_GG_PTP_DEEP_PARSER,
    CPUPKT_GG_PTP_IS_L2PTP,
    CPUPKT_GG_PTP_IS_L4PTP,
    CPUPKT_GG_PTP_CAPTURE_TS,
    CPUPKT_GG_PTP_REPLACE_TS,
    CPUPKT_GG_PTP_APPLY_EGRESS_ASY_DELAY,
    CPUPKT_GG_PTP_UP_RES_TIME,
    CPUPKT_GG_PTP_OP_MODE,
    CPUPKT_GG_PTP_OFFSET,
    CPUPKT_GG_PTP_ISELOOPPKT,
    CPUPKT_GG_C2C_RESERVE,
    CPUPKT_GG_C2C_SGMAC_STRIP_HEADER,
    CPUPKT_GG_C2C_C2C_CHECK_DISABLE,
    CPUPKT_GG_C2C_ISELOOPPKT,
    CPUPKT_GG_C2C_STACKING_SRCPORT,
    CPUPKT_DT2_PACKET_OFFSET,
    CPUPKT_DT2_MCAST,
    CPUPKT_DT2_SPANPKT,
    CPUPKT_DT2_TOCPU,
    CPUPKT_DT2_DEST_CHIPID,
    CPUPKT_DT2_DESTID,
    CPUPKT_DT2_TERMINATE_SGT_HDR,
    CPUPKT_DT2_PRIORITY,
    CPUPKT_DT2_PACKET_TYPE,
    CPUPKT_DT2_SOURCE_STAG_COS,
    CPUPKT_DT2_HEADER_HASH,
    CPUPKT_DT2_LOGIC_PORT_TYPE,
    CPUPKT_DT2_SRCPORT,
    CPUPKT_DT2_SRC_SVLANID,
    CPUPKT_DT2_COLOR,
    CPUPKT_DT2_NEXT_HOP_PTR,
    CPUPKT_DT2_FROM_LAG,
    CPUPKT_DT2_CRITICAL_PACKET,
    CPUPKT_DT2_IS_LEAF,
    CPUPKT_DT2_SOURCE_CFI,
    CPUPKT_DT2_NEXT_HOP_EXT,
    CPUPKT_DT2_PORT_MACSA_EN,
    CPUPKT_DT2_FROMCPULMUPDISABLE,
    CPUPKT_DT2_SVLAN_TAG_OPERATION_VALID,
    CPUPKT_DT2_SOURCEPORTEXTENDER,
    CPUPKT_DT2_FROM_FABRIC,
    CPUPKT_DT2_BYPASS_INGRESS_EDITING,
    CPUPKT_DT2_DSNEXTHOP_BYPASSALL,
    CPUPKT_DT2_FROMCPULMDOWNDISABLE,
    CPUPKT_DT2_INGRESS_EDIT_EN,
    CPUPKT_DT2_OUTER_VLAN_IS_CVLAN,
    CPUPKT_DT2_FROM_CPU_OR_OAM,
    CPUPKT_DT2_SVLAN_TPID_INDEX,
    CPUPKT_DT2_SRC_VLAN_PTR,
    CPUPKT_DT2_MUX_LENGTH_TYPE,
    CPUPKT_DT2_OAM_TUNNEL_EN,
    CPUPKT_DT2_BRIDGE_OP,
    CPUPKT_DT2_MAC_LEARNING_EN,
    CPUPKT_DT2_DEBUG_SESSION_EN,
    CPUPKT_DT2_ILOOP_FROM_CPU,
    CPUPKT_DT2_STAG_ACTION,
    CPUPKT_DT2_MAC_KNOWN,
    CPUPKT_DT2_OAM_USE_FID,
    CPUPKT_DT2_FID,
    CPUPKT_DT2_LOGIC_SRCPORT,
    CPUPKT_DT2_TTL,
    CPUPKT_DT2_OPERATION_TYPE,
    CPUPKT_DT2_U3_MODE,
    CPUPKT_DT2_U3_OAM_RXTXFCL,
    CPUPKT_DT2_U3_OTHER_I2E_SRC_CID_VALID,
    CPUPKT_DT2_U3_OTHER_I2E_SRC_CID,
    CPUPKT_DT2_U3_OTHER_PKT_WITH_CID_HDR,
    CPUPKT_DT2_U3_OTHER_SRC_CID_HDR_OFFSET_TYPE,
    CPUPKT_DT2_U3_OTHER_TRUNCATE_LEN_PROFILE,
    CPUPKT_DT2_LOCAL_PHY_PORT,
    CPUPKT_DT2_TS_0_36_HIGH,
    CPUPKT_DT2_SRC_DSCP,
    CPUPKT_DT2_U1_MODE,
    CPUPKT_DT2_NORMAL_RES,
    CPUPKT_DT2_IS_SPAN_PKT,
    CPUPKT_DT2_ACL_STAG_COS_VALID,
    CPUPKT_DT2_ACL_CTAG_COS_VALID,
    CPUPKT_DT2_ACL_STAG_CFI_VALID,
    CPUPKT_DT2_ACL_CTAG_CFI_VALID,
    CPUPKT_DT2_DOT1AE_ENCRYPTED,
    CPUPKT_DT2_ECMPHASH,
    CPUPKT_DT2_ACL_DSCP_VALID,
    CPUPKT_DT2_SRC_DSCP_VALID,
    CPUPKT_DT2_CNACTION,
    CPUPKT_DT2_PBBCHECKDISCARD,
    CPUPKT_DT2_CTAGACTION,
    CPUPKT_DT2_SRC_CTAG_COS,
    CPUPKT_DT2_SRC_CTAG_CFI,
    CPUPKT_DT2_SRC_CTAG_OFFSET_TYPE,
    CPUPKT_DT2_SRCPORT_ISOLATEID,
    CPUPKT_DT2_SRCCVLANID,
    CPUPKT_DT2_ISIDVALID,
    CPUPKT_DT2_CTAG_OP_VALID,
    CPUPKT_DT2_MICRO_BURST_VALID,
    CPUPKT_DT2_METADATA_TYPE,
    CPUPKT_DT2_METADATA,
    CPUPKT_DT2_NO_DOT1AE_ENCRYPT,
    CPUPKT_DT2_NEED_DOT1AE_ENCRYPT,
    CPUPKT_DT2_OAM_RES,
    CPUPKT_DT2_OAM_RXOAM,
    CPUPKT_DT2_OAM_MEPINDEX,
    CPUPKT_DT2_OAM_OAM_TYPE,
    CPUPKT_DT2_OAM_LINKOAM,
    CPUPKT_DT2_OAM_ILOOPTOAGGUPMEP,
    CPUPKT_DT2_OAM_GAL_EXIST,
    CPUPKT_DT2_OAM_USE_OAM_TTL,
    CPUPKT_DT2_OAM_ISUP,
    CPUPKT_DT2_OAM_DMEN,
    CPUPKT_DT2_OAM_LM_RECEICED_PKT,
    CPUPKT_DT2_OAM_RXFCB,
    CPUPKT_DT2_OAM_RXTXFCL,
    CPUPKT_DT2_DM_RESERVE,
    CPUPKT_DT2_DM_OFFSET,
    CPUPKT_DT2_LM_PKTTYPE,
    CPUPKT_DT2_LM_TXFCB,
    CPUPKT_DT2_LM_RXFCB,
    CPUPKT_DT2_LM_RXTXFCL,
    CPUPKT_DT2_PTP_RESERVE,
    CPUPKT_DT2_PTP_TS_62_37,
    CPUPKT_DT2_PTP_TS_0_4,
    CPUPKT_DT2_PTP_DEEP_PARSER,
    CPUPKT_DT2_PTP_IS_L2PTP,
    CPUPKT_DT2_PTP_IS_L4PTP,
    CPUPKT_DT2_PTP_REPLACE_TS,
    CPUPKT_DT2_PTP_APPLY_EGRESS_ASY_DELAY,
    CPUPKT_DT2_PTP_UP_RES_TIME,
    CPUPKT_DT2_PTP_OFFSET,
    CPUPKT_DT2_PTP_ACL_DSCP_VALID,
    CPUPKT_DT2_PTP_SRC_DSCP_VALID,
    CPUPKT_DT2_C2C_RESERVE,
    CPUPKT_DT2_C2C_NEIGHBOR_DISCOVER_PACKET,
    CPUPKT_DT2_C2C_SGMAC_STRIP_HEADER,
    CPUPKT_DT2_C2C_C2C_CHECK_DISABLE,
    CPUPKT_DT2_C2C_STACKING_SRCPORT,
    CPUPKT_PKT_TYPE,
    CPUPKT_UDT,
    DOT11_FRAME_CTL,
    DOT11_DURA_BIT_15,
    DOT11_DURA_BIT_14,
    DOT11_DURA_BIT_0_TO_13,
    DOT11_MAX_MAC_NUM,
    DOT11_MAC_ADDR,
    DOT11_SEQ_CTL_EXIST,
    DOT11_SEQ_CTL,
    DOT11_MAC_ADDR4_EXIST,
    DOT11_MAC_ADDR4,
    DOT11_QOS_CTL_EXIST,
    DOT11_QOS_CTL,
    DOT11_HT_CTL_EXIST,
    DOT11_HT_CTL,
    DOT11_PAYLOAD_EXIST,
    DOT11_LLC_EXIST,
    DOT11_LLC_CHARACTER,
    DOT11_LLC_VLAN_EXIST,
    DOT11_LLC_VLAN_TAG,
    DOT11_LLC_TYPE,
    DOT11_UDT,
    ETH_MAC_DA,
    ETH_MAC_SA,
    ETH_MAX_VNUM,
    ETH_TPID,
    ETH_COS,
    ETH_DEI,
    ETH_VLAN_ID,
    ETH_LEN_TYPE,
    ETH_DSAP,
    ETH_SSAP,
    ETH_CTL0,
    ETH_CTL1,
    ETH_OUI,
    ETH_L2_TYPE,
    ETH_UDT,
    PKT_PAYLOAD_LEN,
    PKT_PAYLOAD_DATA,
    PKT_PAYLOAD_UDT,
    MACSEC_TCI_VER,
    MACSEC_TCI_ES,
    MACSEC_TCI_SC,
    MACSEC_TCI_SCB,
    MACSEC_TCI_E,
    MACSEC_TCI_C,
    MACSEC_AN,
    MACSEC_SL,
    MACSEC_PN,
    MACSEC_SCI,
    MACSEC_PAYLOAD,
    MACSEC_UDT,
    SGTCMD_VER,
    SGTCMD_LENGTH,
    SGTCMD_SGT_LEN,
    SGTCMD_SGT_OPT_TYPE,
    SGTCMD_SGT_VALUE,
    SGTCMD_NEXT_TYPE,
    SGTCMD_UDT,
    NSH_VER,
    NSH_FLAG_O,
    NSH_FLAG_C,
    NSH_FLAG_RES,
    NSH_LEN,
    NSH_MD_TYPE,
    NSH_NEXT_PROTOCOL,
    NSH_SERVICE_PATH_ID,
    NSH_SERVICE_INDEX,
    NSH_MANDATORY_1,
    NSH_MANDATORY_2,
    NSH_MANDATORY_3,
    NSH_MANDATORY_4,
    NSH_UDT,
    SLPR_SUBTYPE,
    SLPR_EFM_FLAGS,
    SLPR_EFM_CODE,
    SLPR_EFM_PLD_LEN,
    SLPR_EFM_PLD_DATA,
    SLPR_LACP_VER,
    SLPR_LACP_ACT_INFO,
    SLPR_LACP_ACT_INFO_LEN,
    SLPR_LACP_ACT_SYS_PRI,
    SLPR_LACP_ACT_SYS,
    SLPR_LACP_ACT_KEY,
    SLPR_LACP_ACT_PORT_PRI,
    SLPR_LACP_ACT_PORT,
    SLPR_LACP_ACT_STATE,
    SLPR_LACP_RES0,
    SLPR_LACP_PART_INFO,
    SLPR_LACP_PART_INFO_LEN,
    SLPR_LACP_PART_SYS_PRI,
    SLPR_LACP_PART_SYS,
    SLPR_LACP_PART_KEY,
    SLPR_LACP_PART_PORT_PRI,
    SLPR_LACP_PART_PORT,
    SLPR_LACP_PART_STATE,
    SLPR_LACP_RES1,
    SLPR_LACP_COLL_INFO,
    SLPR_LACP_COLL_INFO_LEN,
    SLPR_LACP_COLL_MAX_DE,
    SLPR_LACP_RES2,
    SLPR_LACP_TERM,
    SLPR_LACP_TERM_LEN,
    SLPR_LACP_RES3,
    SLPR_UDT,
    TRILL_VER,
    TRILL_RES,
    TRILL_MUL,
    TRILL_OL,
    TRILL_HC,
    TRILL_EG_RB_NAME,
    TRILL_IN_RB_NAME,
    TRILL_OP,
    TRILL_NEXTP,
    TRILL_UDT,
    CHAN_VER,
    CHAN_PROTOTOL,
    CHAN_FLAGS,
    CHAN_ERR,
    CHAN_UDT,
    LABEL,
    LAB_EXP,
    LAB_S,
    LAB_TTL,
    LAB_NEXTP,
    LAB_PAYLOAD_LEN,
    LAB_PAYLOAD_DATA,
    MPLS_UDT,
    ACH_HEADER,
    ACH_VERSION,
    ACH_RESERVE,
    ACH_CHANNELTYPE,
    ACH_UDT,
    MPLSLM_VERSION,
    MPLSLM_FLAGS,
    MPLSLM_CTL_CODE,
    MPLSLM_MSG_LENG,
    MPLSLM_DFLAGS,
    MPLSLM_OTF,
    MPLSLM_RES,
    MPLSLM_SESS_ID,
    MPLSLM_DS,
    MPLSLM_ORI_TMSTMP,
    MPLSLM_CNT1,
    MPLSLM_CNT2,
    MPLSLM_CNT3,
    MPLSLM_CNT4,
    MPLSLM_TLV_TYPE,
    MPLSLM_TLV_LEN,
    MPLSLM_TLV_VALUE,
    MPLSLM_UDT,
    MPLSDM_VERSION,
    MPLSDM_FLAGS,
    MPLSDM_CTL_CODE,
    MPLSDM_MSG_LENG,
    MPLSDM_QTF,
    MPLSDM_RTF,
    MPLSDM_RPTF,
    MPLSDM_RES,
    MPLSDM_SESS_ID,
    MPLSDM_DS,
    MPLSDM_TMSTMP1,
    MPLSDM_TMSTMP2,
    MPLSDM_TMSTMP3,
    MPLSDM_TMSTMP4,
    MPLSDM_TLV_TYPE,
    MPLSDM_TLV_LEN,
    MPLSDM_TLV_VALUE,
    MPLSDM_UDT,
    MPLSLDM_VERSION,
    MPLSLDM_FLAGS,
    MPLSLDM_CTL_CODE,
    MPLSLDM_MSG_LENG,
    MPLSLDM_DFLAGS,
    MPLSLDM_QTF,
    MPLSLDM_RTF,
    MPLSLDM_RPTF,
    MPLSLDM_RES,
    MPLSLDM_SESS_ID,
    MPLSLDM_DS,
    MPLSLDM_TMSTMP1,
    MPLSLDM_TMSTMP2,
    MPLSLDM_TMSTMP3,
    MPLSLDM_TMSTMP4,
    MPLSLDM_CNT1,
    MPLSLDM_CNT2,
    MPLSLDM_CNT3,
    MPLSLDM_CNT4,
    MPLSLDM_TLV_TYPE,
    MPLSLDM_TLV_LEN,
    MPLSLDM_TLV_VALUE,
    MPLSLDM_UDT,
    CSF_VERSION,
    CSF_RESERVE1,
    CSF_FLAG,
    CSF_RESERVE2,
    CSF_TLV_LEN,
    CSF_UDT,
    CW_HEADER,
    CW_FLAGS,
    CW_FRAG,
    CW_LENGTH,
    CW_SEQUENCE,
    CW_NEXTP,
    CW_UDT,
    L2H_MACDA,
    L2H_MACSA,
    L2H_MAXVNUM,
    L2H_TYPE_ID,
    L2H_VLAN_COS,
    L2H_VLAN_DEI,
    L2H_VLAN_ID,
    L2H_LEN_TYPE,
    L2H_UDT,
    CFM_MD_LEVEL,
    CFM_VER,
    CFM_OP_CODE,
    CFM_FLAGS,
    CFM_FIRST_TLV_OFFSET,
    CFM_PAYLOAD_LEN,
    CFM_PAYLOAD_DATA,
    CFM_CCM_SEQ_NUM,
    CFM_CCM_MAIN_ASS_END_POINT_ID,
    CFM_CCM_MAIN_DOMAIN_NAME_FORMAT,
    CFM_CCM_MAIN_DOMAIN_NAME_LEN,
    CFM_CCM_MAIN_DOMAIN_NAME_STR,
    CFM_CCM_SHORT_MA_NAME_FORMAT,
    CFM_CCM_SHORT_MA_NAME_LEN,
    CFM_CCM_SHORT_MA_NAME_STR,
    CFM_CCM_MAID_ZERO_PAD,
    CFM_CCM_TXFCF,
    CFM_CCM_RXFCB,
    CFM_CCM_TXFCB,
    CFM_CCM_RESERVED,
    CFM_LB_TP,
    CFM_LB_ID,
    CFM_LT_ID,
    CFM_LT_TTL,
    CFM_LTM_ORI_MAC,
    CFM_LTM_TAR_MAC,
    CFM_LTR_RL_ACT,
    CFM_LM_TXFCF,
    CFM_LM_RXFCF,
    CFM_LM_TXFCB,
    CFM_APS_LEN,
    CFM_APS_DATA,
    CFM_DM_TX_STPF,
    CFM_DM_RX_STPF,
    CFM_DM_TX_STPB,
    CFM_DM_RX_STPB,
    CFM_SLM_SRC_MEPID,
    CFM_SLM_RESP_MEPID,
    CFM_SLM_TESTID,
    CFM_SLM_TXFCF,
    CFM_SLM_TXFCB,
    CFM_TLV_TYPE,
    CFM_TLV_LEN,
    CFM_TLV_VALUE,
    CFM_TP_LBM_TLV_TYPE,
    CFM_TP_LBM_TLV_LEN,
    CFM_TP_LBM_TLV_SUBTYPE,
    CFM_TP_LBM_TLV_MEPID,
    CFM_TP_LBM_TLV_ZERO,
    CFM_UDT,
    PBB_ITAG_PCP,
    PBB_ITAG_DEI,
    PBB_ITAG_NCA,
    PBB_ITAG_RES1,
    PBB_ITAG_RES2,
    PBB_ITAG_ISID,
    PBB_ITAG_CMAC_DA,
    PBB_ITAG_CMAC_SA,
    PBB_STAG_TPID,
    PBB_STAG_COS,
    PBB_STAG_DEI,
    PBB_STAG_VLANID,
    PBB_CTAG_TPID,
    PBB_CTAG_COS,
    PBB_CTAG_DEI,
    PBB_CTAG_VLANID,
    PBB_LEN_TYPE,
    PBB_UDT,
    ETAG_PCP,
    ETAG_DEI,
    ETAG_ING_E_CID,
    ETAG_RES1,
    ETAG_E_CID,
    ETAG_RES2,
    ETAG_STAG_TPID,
    ETAG_STAG_COS,
    ETAG_STAG_DEI,
    ETAG_STAG_VLANID,
    ETAG_CTAG_TPID,
    ETAG_CTAG_COS,
    ETAG_CTAG_DEI,
    ETAG_CTAG_VLANID,
    ETAG_LEN_TYPE,
    ETAG_UDT,
    FIP_VER,
    FIP_RES1,
    FIP_PAYLOAD_LEN,
    FIP_PAYLOAD_DATA,
    FIP_UDT,
    FCOE_VER,
    FCOE_RES1,
    FCOE_SOF,
    FCOE_R_CTL,
    FCOE_D_ID,
    FCOE_CS_CTL,
    FCOE_S_ID,
    FCOE_TYPE,
    FCOE_F_CTL,
    FCOE_SEQ_ID,
    FCOE_DF_CTL,
    FCOE_SEQ_CNT,
    FCOE_CX_ID,
    FCOE_RX_ID,
    FCOE_PARA,
    FCOE_PAYLOAD_LEN,
    FCOE_PAYLOAD_DATA,
    FCOE_EOF,
    FCOE_RES2,
    FCOE_UDT,
    MEL,
    VER,
    OPCODE,
    FLAGS,
    TLV_OFF,
    SEQ_NUM,
    MEP_ID,
    MEG_ID,
    TX_FCF,
    RX_FCB,
    TX_FCB,
    RESERVED,
    END_TLV,
    TLV_TYPE,
    TLV_LEN,
    TLV_VALUE,
    CCM_PAYLOAD,
    CCM_UDT,
    GRE_CBIT,
    GRE_RBIT,
    GRE_KBIT,
    GRE_SBIT,
    GRE_RES0,
    GRE_VER,
    GRE_PROT,
    GRE_CHECKSUM,
    GRE_RES1,
    GRE_KEY,
    GRE_SEQ,
    GRE_UDT,
    EAPOL_VER,
    EAPOL_TYPE,
    EAPOL_LEN,
    EAP_PACKET_CODE,
    EAP_PACKET_ID,
    EAP_PACKET_LEN,
    EAP_PACKET_TYPE,
    EAP_PACKET_DATA,
    EAP_MD5_CHALL_VALUE_SIZE,
    EAP_MD5_CHALL_VALUE,
    EAP_MD5_CHALL_NAME_LEN,
    EAP_MD5_CHALL_NAME,
    EAPOL_UDT,
    IPV4_VER,
    IPV4_IHL,
    IPV4_TOS,
    IPV4_LEN,
    IPV4_ID,
    IPV4_FLG,
    IPV4_FRAGO,
    IPV4_TTL,
    IPV4_PAYLOAD_FORCE,
    IPV4_PROTO,
    IPV4_CHKSM,
    IPV4_SA,
    IPV4_DA,
    IPV4_OPT,
    IPV4_PAYLOAD_LEN,
    IPV4_PAYLOAD,
    IPV4_UDT,
    IPV6_VER,
    IPV6_TRA_CLASS,
    IPV6_FLOW_LBL,
    IPV6_PAYL_LEN,
    IPV6_NEXT_HDR,
    IPV6_HOP_LIMIT,
    IPV6_SA,
    IPV6_DA,
    IPV6_PAYLOAD_LEN,
    IPV6_PAYLOAD,
    IPV6_UDT,
    IPV6_HBH_OPT_HEADER_NEXT_HEADER,
    IPV6_HBH_OPT_HEADER_EXT_LENGTHER,
    IPV6_HBH_OPT_HEADER_OPTION_TYPE,
    IPV6_HBH_OPT_HEADER_JUM_PAYL_OPTION_LEN,
    IPV6_HBH_OPT_HEADER_JUM_PAYL_LEN,
    IPV6_HBH_OPT_HEADER_ROUT_ALT_OPTION_LEN,
    IPV6_HBH_OPT_HEADER_ROUT_ALT_VALUE,
    IPV6_HBH_OPT_HEADER_PADN_OPTION_LEN,
    IPV6_HBH_OPT_HEADER_PADN_OPTION_DATA,
    IPV6_HBH_OPT_HEADER_UDT,
    IPV6_DEST_OPT_HEADER_NEXT_HEADER,
    IPV6_DEST_OPT_HEADER_EXT_LENGTHER,
    IPV6_DEST_OPT_HEADER_OPTION_TYPE,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_LEN,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_FLAG,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_RES,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_PRE_LEN,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SEQ_NUM,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_LIFETIME,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SUB_OPTION_NUM,
    IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SUB_OPTION,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_LEN,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_STATUS,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SEQ_NUM,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_LIFETIME,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_REFRESH,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SUB_OPTION_NUM,
    IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SUB_OPTION,
    IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_LEN,
    IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_SUB_OPTION_NUM,
    IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_SUB_OPTION,
    IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_LEN,
    IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_HOME_ADDR,
    IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_SUB_OPTION_NUM,
    IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_SUB_OPTION,
    IPV6_DEST_OPT_HEADER_UDT,
    IPV6_ROUT_HEADER_NEXT_HEADER,
    IPV6_ROUT_HEADER_EXT_LENGTHER,
    IPV6_ROUT_HEADER_ROUT_TYPE,
    IPV6_ROUT_HEADER_SEG_LEFT,
    IPV6_ROUT_HEADER_ROUT_TYPE_SPEC_DATA_NUM,
    IPV6_ROUT_HEADER_ROUT_TYPE_SPEC_DATA,
    IPV6_ROUT_TYPE0_HEADER_SEG_LEFT,
    IPV6_ROUT_TYPE0_HEADER_RES,
    IPV6_ROUT_TYPE0_HEADER_ADDR_NUM,
    IPV6_ROUT_TYPE0_HEADER_ADDR,
    IPV6_ROUT_HEADER_UDT,
    IPV6_FRAG_HEADER_NEXT_HEADER,
    IPV6_FRAG_HEADER_RES_1,
    IPV6_FRAG_HEADER_FRAG_OFFSET,
    IPV6_FRAG_HEADER_RES_2,
    IPV6_FRAG_HEADER_MORE_FRAG_FLAG,
    IPV6_FRAG_HEADER_ID,
    IPV6_FRAG_HEADER_UDT,
    IPV6_AUTHEN_HEADER_NEXT_HEADER,
    IPV6_AUTHEN_HEADER_PAYL_LEN,
    IPV6_AUTHEN_HEADER_RES,
    IPV6_AUTHEN_HEADER_SEUC_PARA_INDEX,
    IPV6_AUTHEN_HEADER_SEQ_NUM,
    IPV6_AUTHEN_HEADER_AUTHEN_DATA_NUM,
    IPV6_AUTHEN_HEADER_AUTHEN_DATA,
    IPV6_AUTHEN_HEADER_UDT,
    IPV6_ENCAP_SEUC_PAYL_HEADER_SEUC_PARA_INDEX,
    IPV6_ENCAP_SEUC_PAYL_HEADER_SEQ_NUM,
    IPV6_ENCAP_SEUC_PAYL_HEADER_PAYL_DATA_NUM,
    IPV6_ENCAP_SEUC_PAYL_HEADER_PAYL_DATA,
    IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING_NUM,
    IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING,
    IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING_LEN,
    IPV6_ENCAP_SEUC_PAYL_HEADER_NEXT_HEADER,
    IPV6_ENCAP_SEUC_PAYL_HEADER_AUTHEN_DATA_NUM,
    IPV6_ENCAP_SEUC_PAYL_HEADER_AUTHEN_DATA,
    IPV6_ENCAP_SEUC_PAYL_HEADER_UDT,
    VRRP_VER,
    VRRP_TYPE,
    VRRP_VIR_RTR_ID,
    VRRP_PRIORITY,
    VRRP_COUNT_IP_ADDR,
    VRRP_AUTH_TYPE,
    VRRP_ADVER_INT,
    VRRP_CHKSM,
    VRRP_IP_ADDR,
    VRRP_AUTH_DATA1,
    VRRP_AUTH_DATA2,
    VRRP_UDT,
    TCP_USE_PAYLOAD,
    TCP_SOURCE_PORT,
    TCP_DEST_PORT,
    TCP_SEQUENCE_NUM,
    TCP_ACKNOW_NUM,
    TCP_DATA_OFFSET,
    TCP_RESERVED,
    TCP_URG,
    TCP_ACK,
    TCP_PSH,
    TCP_RST,
    TCP_SYN,
    TCP_FIN,
    TCP_WINDOW,
    TCP_CHECKSUM,
    TCP_URGENT_POINT,
    TCP_OPTION,
    TCP_UDT,
    UDP_USE_PAYLOAD,
    UDP_SOURCE_PORT,
    UDP_VXLANHEADER,
    UDP_CAPWAPHEADER,
    UDP_OTWAMPHEADER,
    UDP_DEST_PORT,
    UDP_LENGTH,
    UDP_CHECKSUM,
    UDP_UDT,
    VXLAN_BITFLAGS,
    VXLAN_RESERVED1,
    VXLAN_VNIID,
    VXLAN_PAYLOADMODE,
    VXLAN_RESERVED2,
    VXLAN_PAYLOADLEN,
    VXLAN_PAYLOAD,
    VXLAN_UDT,
    OSPF_VER,
    OSPF_TYPE,
    OSPF_LEN,
    OSPF_ROU_ID,
    OSPF_ARE_ID,
    OSPF_CHKSM,
    OSPF_AU_TYPE,
    OSPF_AU_DATA,
    OSPF_HELLO_MASK,
    OSPF_HELLO_INT,
    OSPF_HELLO_OPT,
    OSPF_HELLO_RTR_PRIO,
    OSPF_HELLO_ROUT_DEA_INT,
    OSPF_HELLO_DES_ROUT,
    OSPF_HELLO_BAK_DES_ROUT,
    OSPF_HELLO_NEIG,
    OSPF_DBDES_INT_MTU,
    OSPF_DBDES_OPT,
    OSPF_DBDES_RES,
    OSPF_DBDES_I,
    OSPF_DBDES_M,
    OSPF_DBDES_MS,
    OSPF_DBDES_MSG_SEQ,
    OSPF_DBDES_LSA,
    OSPF_LSR_LS_TYPE,
    OSPF_LSR_LS_ID,
    OSPF_LSR_ADV_ROUT,
    OSPF_LSA,
    OSPF_UDT,
    L2TP_T,
    L2TP_L,
    L2TP_X1,
    L2TP_S,
    L2TP_X2,
    L2TP_O,
    L2TP_P,
    L2TP_X3,
    L2TP_VER,
    L2TP_LENGTH,
    L2TP_TUNL_ID,
    L2TP_SESS_ID,
    L2TP_NS,
    L2TP_NR,
    L2TP_OFF_SZ,
    L2TP_OFF_PAD,
    L2TP_AVP_M,
    L2TP_AVP_H,
    L2TP_AVP_RSVD,
    L2TP_AVP_LENGTH,
    L2TP_AVP_VND_ID,
    L2TP_AVP_TYPE,
    L2TP_AVP_VALUE,
    L2TP_UDT,
    DHCP_OP,
    DHCP_HTYPE,
    DHCP_HLEN,
    DHCP_HOPS,
    DHCP_XID,
    DHCP_SECS,
    DHCP_FLAGS,
    DHCP_CIADDR,
    DHCP_YIADDR,
    DHCP_SYIADDR,
    DHCP_GIADDR,
    DHCP_CHADDR,
    DHCP_SNAME,
    DHCP_FILE,
    DHCP_MAGIC_COOK,
    DHCP_OPT_TYPE,
    DHCP_OPT_REQ_IP_ADDR_LEN,
    DHCP_OPT_REQ_IP_ADDR,
    DHCP_OPT_MESSAGE_TYPE_LEN,
    DHCP_OPT_MESSAGE_TYPE,
    DHCP_OPT_PARA_REQ_LEN,
    DHCP_OPT_PARA_REQ_DATA,
    DHCP_OPT_AGE_INFO_LEN,
    DHCP_OPT_AGE_INFO_SUB_SEQ1,
    DHCP_OPT_AGE_INFO_SUB_SEQ1_LEN,
    DHCP_OPT_AGE_INFO_SUB_SEQ1_DATA,
    DHCP_OPT_AGE_INFO_SUB_SEQ2,
    DHCP_OPT_AGE_INFO_SUB_SEQ2_LEN,
    DHCP_OPT_AGE_INFO_SUB_SEQ2_DATA,
    DHCP_OPT_TIME_LEN,
    DHCP_OPT_TIME_DATA,
    DHCP_OPT_PAYLOAD_LEN,
    DHCP_OPT_PAYLOAD,
    DHCP_UDT,
    RIP_CMD,
    RIP_VER,
    RIP_EXTEND,
    RIP_ENTRY_NUM,
    RIP_ADDR_FAMILY_ID,
    RIP_ROUT_TAG,
    RIP_IP_ADDR,
    RIP_SUBNET_MASK,
    RIP_NEXT_HOP,
    RIP_METRIC,
    RIP_ADDR_FAMILY_TYPEFF_AUT_TYPE,
    RIP_ADDR_FAMILY_TYPEFF_AUT_INFO,
    RIP_MD5_DIG_OFFSET,
    RIP_MD5_KEY_ID,
    RIP_MD5_AUTH_DATA_LEN,
    RIP_MD5_SEQ_NUM,
    RIP_MD5_ZERO_PADD,
    RIP_UDT,
    RADIUS_CODE,
    RADIUS_ID,
    RADIUS_LEN,
    RADIUS_AUTH,
    RADIUS_ATTRI_TYPE,
    RADIUS_ATTRI_LEN,
    RADIUS_ATTRI_DATA,
    RADIUS_ATTRI_NUM,
    RADIUS_UDT,
    LDP_VERSION,
    LDP_PDU_LEN,
    LDP_LSR_ID,
    LDP_LBL_SPC_ID,
    LDP_MSG_TYPE,
    LDP_MSG_LEN,
    LDP_MSG_ID,
    LDP_CMMN_TLV_TYPE,
    LDP_CMMN_TLV_LEN,
    LDP_CMMN_TLV_HLD_TIME,
    LDP_CMMN_RESV,
    LDP_IPV4_TLV_TYPE,
    LDP_IPV4_TLV_LEN,
    LDP_IPV4_TRANS_ADDR,
    LDP_UDT,
    BFD_VERSION,
    BFD_DIAG,
    BFD_STATUS,
    BFD_FLAGS,
    BFD_MULTIPLE,
    BFD_LENGTH,
    BFD_MYDISCRI,
    BFD_YOURDISCRI,
    BFD_MINTXVAL,
    BFD_MINRXVAL,
    BFD_ECHOVAL,
    BFD_UDT,
    PTP_TRANS_SPE,
    PTP_MSG_TYPE,
    PTP_RES0,
    PTP_VER,
    PTP_MSG_LEN,
    PTP_DM_NUM,
    PTP_RES1,
    PTP_FLAGS,
    PTP_CORR,
    PTP_RES2,
    PTP_SRC_PT_ID,
    PTP_SEQ_ID,
    PTP_CTRL,
    PTP_LOG_MSG_INT,
    PTP_AM_ORG_TMSTMP,
    PTP_AM_CURR_UTC_OFFSET,
    PTP_AM_RES,
    PTP_AM_GRANMST_P1,
    PTP_AM_GRANMST_CLKQLT,
    PTP_AM_GRANMST_P2,
    PTP_AM_GRANMST_ID,
    PTP_AM_STP_RM,
    PTP_AM_TIME_SRC,
    PTP_SDRM_ORG_TMSTMP,
    PTP_FUM_PRC_ORG_TMSTMP,
    PTP_DRM_REC_TMSTMP,
    PTP_DRM_REQ_PT_ID,
    PTP_PRQM_ORG_TMPSTMP,
    PTP_PRQM_RES,
    PTP_PRSM_REQ_RCPT_TMSTMP,
    PTP_PRSM_REQ_PT_ID,
    PTP_PRFUM_RES_ORG_TMSTMP,
    PTP_PRFUM_REQ_PT_ID,
    PTP_SM_TGT_PT_ID,
    PTP_SM_TLV_TYPE,
    PTP_SM_TLV_LEN,
    PTP_SM_TLV_VALUE,
    PTP_MM_TGT_PT_ID,
    PTP_MM_START_BOUND_HOP,
    PTP_MM_BOUND_HOP,
    PTP_MM_RES0,
    PTP_MM_ACTION,
    PTP_MM_RES1,
    PTP_MM_TLV_TYPE,
    PTP_MM_TLV_LEN,
    PTP_MM_TLV_VALUE,
    PTP_UDT,
    CAPWAP_PRE_VER,
    CAPWAP_PRE_TYPE,
    CAPWAP_HLEN,
    CAPWAP_RID,
    CAPWAP_WBID,
    CAPWAP_T,
    CAPWAP_F,
    CAPWAP_L,
    CAPWAP_W,
    CAPWAP_M,
    CAPWAP_K,
    CAPWAP_FLAGS,
    CAPWAP_FRAGMENT_ID,
    CAPWAP_FRAG_OFFSET,
    CAPWAP_RES,
    CAPWAP_RAD_MAC_ADDR,
    CAPWAP_WIR_SPEC_INFO,
    CAPWAP_PAYLOAD_LEN,
    CAPWAP_PAYLOAD_DATA,
    CAPWAP_ONLY_USE_PAYLOAD,
    CAPWAP_80211_PAYLOAD,
    CAPWAP_UDT,
    DTLS_RES,
    DTLS_CONTENT_TYPE,
    DTLS_VER,
    DTLS_EPOCH,
    DTLS_SEQ,
    DTLS_LENGTH,
    DTLS_PAYLOAD_LEN,
    DTLS_PAYLOAD_DATA,
    DTLS_UDT,
    LLC_VLAN_EXIST,
    LLC_VLAN_DSAP,
    LLC_VLAN_SSAP,
    LLC_VLAN_CTL0,
    LLC_VLAN_CTL1,
    LLC_VLAN_OUI,
    LLC_TPID,
    LLC_COS,
    LLC_DEI,
    LLC_VLAN_ID,
    LLC_DSAP,
    LLC_SSAP,
    LLC_CTL0,
    LLC_CTL1,
    LLC_OUI,
    LLC_L2_TYPE,
    LLC_UDT,
    TWAMP_SEQ_NUM,
    TWAMP_TS,
    TWAMP_ERR_ESTIMATE,
    TWAMP_MBZ,
    TWAMP_RX_TS,
    TWAMP_SENDER_SEQ_NUM,
    TWAMP_SENDER_TS,
    TWAMP_SENDER_ERR_ESTIMATE,
    TWAMP_SENDER_MBZ,
    TWAMP_SENDER_TTL,
    TWAMP_PAYLOAD_LEN,
    TWAMP_PAYLOAD_DATA,
    TWAMP_UDT,
    OWAMP_SEQ_NUM,
    OWAMP_TS,
    OWAMP_ERR_ESTIMATE,
    OWAMP_PAYLOAD_LEN,
    OWAMP_PAYLOAD_DATA,
    OWAMP_UDT,
    IGMP_TYPE,
    IGMP_RESP_CODE,
    IGMP_CHKSM,
    IGMP_GROUP_ADDR,
    IGMP_VERSION,
    IGMPV3_QUE_RESV,
    IGMPV3_QUE_SFLAG,
    IGMPV3_QUE_QRV,
    IGMPV3_QUE_QQIC,
    IGMPV3_QUE_NUM_OF_SRC,
    IGMPV3_QUE_SRC_ADDR,
    IGMPV3_REP_RESERVER1,
    IGMPV3_REP_CHKSM,
    IGMPV3_REP_RESERVER2,
    IGMPV3_REP_NUM_OF_REC,
    IGMPV3_REP_RECORD_TYPE,
    IGMPV3_REP_AUX_DATA_LEN,
    IGMPV3_REP_NUM_OF_SRC,
    IGMPV3_REP_MUL_ADDR,
    IGMPV3_REP_SRC_ADDR,
    IGMPV3_REP_AUX_DATA_LEN2,
    IGMPV3_REP_AUX_DATA,
    IGMP_UDT,
    ICMP_TYPE,
    ICMP_CODE,
    ICMP_CHECKSUM,
    ICMP_PPM_POINTER,
    ICMP_ECHO_IDE,
    ICMP_ECHO_SEQ_NUM,
    ICMP_ECHO_DATA_REP,
    ICMP_ECHO_DATA,
    ICMP_UDT,
    ICMPV6_TYPE,
    ICMPV6_CODE,
    ICMPV6_CHECKSUM,
    ICMPV6_ECHO_IDE,
    ICMPV6_ECHO_SEQ_NUM,
    ICMPV6_ECHO_DATA_REP,
    ICMPV6_ECHO_DATA,
    ICMPV6_RS_RESERVED,
    ICMPV6_RA_CHL,
    ICMPV6_RA_M,
    ICMPV6_RA_O,
    ICMPV6_RA_RESERVED,
    ICMPV6_RA_ROUT_LT,
    ICMPV6_RA_REA_TIME,
    ICMPV6_RA_RET_TIME,
    ICMPV6_NS_RESERVED,
    ICMPV6_NS_TAR_ADDR,
    ICMPV6_NA_R,
    ICMPV6_NA_S,
    ICMPV6_NA_O,
    ICMPV6_NA_RESERVED,
    ICMPV6_NA_TAR_ADDR,
    ICMPV6_RM_RESERVED,
    ICMPV6_RM_TAR_ADDR,
    ICMPV6_RM_DES_ADDR,
    ICMPV6_OPT_NUM,
    ICMPV6_OPT_TYPE,
    ICMPV6_OPT_LEN,
    ICMPV6_OPT_LINK_LAY_ADDR,
    ICMPV6_OPT_PRE_LEN,
    ICMPV6_OPT_PRE_L,
    ICMPV6_OPT_PRE_A,
    ICMPV6_OPT_PRE_RESERVED1,
    ICMPV6_OPT_PRE_VLD_LT,
    ICMPV6_OPT_PRE_PRE_LT,
    ICMPV6_OPT_PRE_RESERVED2,
    ICMPV6_OPT_PRE_PREFIX,
    ICMPV6_OPT_MTU_RESERVED,
    ICMPV6_OPT_MTU_MTU,
    ICMPV6_MLD_MAX_RES,
    ICMPV6_MLD_RESERVE,
    ICMPV6_MLD_MULT_ADDR,
    ICMPV6_UDT,
    ARP_HARD_TYPE,
    ARP_PROT_TYPE,
    ARP_HARD_ADDR,
    ARP_PROT_ADDR,
    ARP_OPERATION,
    ARP_SRC_MAC,
    ARP_SRC_IP,
    ARP_DEST_MAC,
    ARP_DEST_IP,
    ARP_UDT,
    STP_PROT_ID,
    STP_PROT_VER_ID,
    STP_BPDU_TYPE,
    STP_BPDU_FLAGS,
    STP_ROOT_ID,
    STP_ROOT_PATH_COST,
    STP_BRI_ID,
    STP_PORT_ID,
    STP_MSG_AGE,
    STP_MAX_AGE,
    STP_HELLO_TIME,
    STP_FWD_DELAY,
    STP_R_VER_1_LENGTH,
    STP_R_M_VER_3_LENGTH,
    STP_R_M_CFG_ID_FMT_SEL,
    STP_R_M_CFG_NAME,
    STP_R_M_CFG_REVISION,
    STP_R_M_CFG_DGST,
    STP_R_M_CIST_INTNL_RT_PTH_COST,
    STP_R_M_CIST_BRIDGE_ID,
    STP_R_M_REMAIN_HOP,
    STP_R_M_MSTI_CFG_NUM,
    STP_R_M_MSTI_FLG,
    STP_R_M_MSTI_RGN_RT_ID,
    STP_R_M_MSTI_INTNL_RT_PTH_COST,
    STP_R_M_MSTI_BRIDGE_PRI,
    STP_R_M_MSTI_PORT_PRI,
    STP_R_M_MSTI_REMAIN_HOP,
    STP_UDT,
    FIELD_NUM,
};


#endif
