
/****************************************************************************
 * ctp_header_format.c  provides all CTP Supportted header protocol infomation.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * This file is automatic generate by CTP_GEN_HEADER tools and use a script
 * input file named ctp_header_format.txt
 *
*****************************************************************************/

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include <unistd.h>
#include "ctp_common.h"

/****************************************************************************
 *
 * Global and Declarations 
 *
****************************************************************************/

/*
 * This array is use for header types ID and String inter-search
*/
static char *header_type_str[] =
{
    "CPUPKT",
    "WIRELESSDOT11",
    "ETHERNET",
    "PKT_PAYLOAD",
    "MACSECTAG",
    "SGTCMD",
    "NSH",
    "SLPR",
    "TRILL",
    "RBCHANNEL",
    "MPLS",
    "ACH",
    "MPLSLM",
    "MPLSDM",
    "MPLSLDM",
    "CSF",
    "CW",
    "L2H",
    "CFM",
    "PBB",
    "ETAG",
    "FIP",
    "FCOE",
    "CCM",
    "GRE",
    "EAPOL",
    "IPV4",
    "IPV6",
    "IPV6_HBH_OPT_HEADER",
    "IPV6_DEST_OPT_HEADER",
    "IPV6_ROUT_HEADER",
    "IPV6_FRAG_HEADER",
    "IPV6_AUTHEN_HEADER",
    "IPV6_ENCAP_SEUC_PAYL_HEADER",
    "VRRP",
    "TCP",
    "UDP",
    "VXLAN",
    "OSPF",
    "L2TP",
    "DHCP",
    "RIP",
    "RADIUS",
    "LDP",
    "BFD",
    "PTP",
    "CAPWAP",
    "DTLS",
    "LLC",
    "TWAMP",
    "OWAMP",
    "IGMP",
    "ICMP",
    "ICMPV6",
    "ARP",
    "STP",
    "IPV6_ENCAP_IPV6_HEADER",
};

/*
 * This array is use for header types start field ID
*/
static uint16 header_start_field[] =
{
    CPUPKT_EXIST,
    DOT11_FRAME_CTL,
    ETH_MAC_DA,
    PKT_PAYLOAD_LEN,
    MACSEC_TCI_VER,
    SGTCMD_VER,
    NSH_VER,
    SLPR_SUBTYPE,
    TRILL_VER,
    CHAN_VER,
    LABEL,
    ACH_HEADER,
    MPLSLM_VERSION,
    MPLSDM_VERSION,
    MPLSLDM_VERSION,
    CSF_VERSION,
    CW_HEADER,
    L2H_MACDA,
    CFM_MD_LEVEL,
    PBB_ITAG_PCP,
    ETAG_PCP,
    FIP_VER,
    FCOE_VER,
    MEL,
    GRE_CBIT,
    EAPOL_VER,
    IPV4_VER,
    IPV6_VER,
    IPV6_HBH_OPT_HEADER_NEXT_HEADER,
    IPV6_DEST_OPT_HEADER_NEXT_HEADER,
    IPV6_ROUT_HEADER_NEXT_HEADER,
    IPV6_FRAG_HEADER_NEXT_HEADER,
    IPV6_AUTHEN_HEADER_NEXT_HEADER,
    IPV6_ENCAP_SEUC_PAYL_HEADER_SEUC_PARA_INDEX,
    VRRP_VER,
    TCP_USE_PAYLOAD,
    UDP_USE_PAYLOAD,
    VXLAN_BITFLAGS,
    OSPF_VER,
    L2TP_T,
    DHCP_OP,
    RIP_CMD,
    RADIUS_CODE,
    LDP_VERSION,
    BFD_VERSION,
    PTP_TRANS_SPE,
    CAPWAP_PRE_VER,
    DTLS_RES,
    LLC_VLAN_EXIST,
    TWAMP_SEQ_NUM,
    OWAMP_SEQ_NUM,
    IGMP_TYPE,
    ICMP_TYPE,
    ICMPV6_TYPE,
    ARP_HARD_TYPE,
    STP_PROT_ID,
};

/*
 * These arraies is use for flags infomation start field ID
*/
static ctp_flags_jump_t cpupkt_exist_jump[] =
{
    { 1, 1, {CPUPKT_PKT_TYPE}, "[$CPUPKT_EXIST$] == [#0]" },
    { 1, 1, {CPUPKT_GG_PACKET_OFFSET}, "[$CPUPKT_EXIST$] == [#5]" },
    { 0, 1, {CPUPKT_DT2_PACKET_OFFSET}, "[$CPUPKT_EXIST$] == [#6]" },
};

static ctp_flags_jump_t cpupkt_vlanexist_jump[] =
{
    { 0, 1, {CPUPKT_TYPE}, "[$CPUPKT_VLANEXIST$] == [#0]" },
};

static ctp_flags_jump_t cpupkt_type_jump[] =
{
    { 0, 1, {CPUPKT_GG_PACKET_OFFSET}, "[$CPUPKT_EXIST$] == [#5]" },
};

static ctp_flags_jump_t cpupkt_res_jump[] =
{
    { 1, 1, {CPUPKT_HM_HEADER_TYPE}, "[$CPUPKT_EXIST$] == [#3]" },
    { 1, 1, {CPUPKT_GB_SRCPORT_15_14}, "[$CPUPKT_EXIST$] == [#4]" },
    { 1, 1, {CPUPKT_GG_PACKET_OFFSET}, "[$CPUPKT_EXIST$] == [#5]" },
    { 1, 1, {CPUPKT_DT2_PACKET_OFFSET}, "[$CPUPKT_EXIST$] == [#6]" },
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_gg_u1_mode_jump[] =
{
    { 1, 1, {CPUPKT_GG_L3_OFFSET}, "[$CPUPKT_GG_U1_MODE$] == [#0]" },
    { 1, 1, {CPUPKT_GG_OAM_RXOAM}, "[$CPUPKT_GG_U1_MODE$] == [#1]" },
    { 1, 1, {CPUPKT_GG_DM_RESERVE}, "[$CPUPKT_GG_U1_MODE$] == [#2]" },
    { 1, 1, {CPUPKT_GG_LM_PKTTYPE}, "[$CPUPKT_GG_U1_MODE$] == [#3]" },
    { 1, 1, {CPUPKT_GG_PTP_RESERVE}, "[$CPUPKT_GG_U1_MODE$] == [#4]" },
    { 0, 1, {CPUPKT_GG_C2C_RESERVE}, "[$CPUPKT_GG_U1_MODE$] == [#5]" },
};

static ctp_flags_jump_t cpupkt_gg_metadata_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_gg_oam_iselooppkt_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_gg_dm_iselooppkt_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_gg_lm_rxtxfcl_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_gg_ptp_iselooppkt_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_gg_c2c_stacking_srcport_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_u3_mode_jump[] =
{
    { 1, 1, {CPUPKT_DT2_U3_OTHER_I2E_SRC_CID_VALID}, "[$CPUPKT_DT2_U3_MODE$] == [#0]" },
    { 0, 1, {CPUPKT_DT2_U3_OAM_RXTXFCL}, "[$CPUPKT_DT2_U3_MODE$] == [#1]" },
};

static ctp_flags_jump_t cpupkt_dt2_u3_oam_rxtxfcl_jump[] =
{
    { 0, 1, {CPUPKT_DT2_LOCAL_PHY_PORT}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_u1_mode_jump[] =
{
    { 1, 1, {CPUPKT_DT2_NORMAL_RES}, "[$CPUPKT_DT2_U1_MODE$] == [#0]" },
    { 1, 1, {CPUPKT_DT2_OAM_RES}, "[$CPUPKT_DT2_U1_MODE$] == [#1]" },
    { 1, 1, {CPUPKT_DT2_DM_RESERVE}, "[$CPUPKT_DT2_U1_MODE$] == [#2]" },
    { 1, 1, {CPUPKT_DT2_LM_PKTTYPE}, "[$CPUPKT_DT2_U1_MODE$] == [#3]" },
    { 1, 1, {CPUPKT_DT2_PTP_RESERVE}, "[$CPUPKT_DT2_U1_MODE$] == [#4]" },
    { 0, 1, {CPUPKT_DT2_C2C_RESERVE}, "[$CPUPKT_DT2_U1_MODE$] == [#5]" },
};

static ctp_flags_jump_t cpupkt_dt2_need_dot1ae_encrypt_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_oam_rxtxfcl_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_dm_offset_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_lm_rxtxfcl_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_ptp_src_dscp_valid_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_jump_t cpupkt_dt2_c2c_stacking_srcport_jump[] =
{
    { 0, 1, {CPUPKT_PKT_TYPE}, "[null]" },
};

static ctp_flags_nextp_t cpupkt_pkt_type_nextp[] =
{
    { 1, 1, {WIRELESSDOT11}, "[$CPUPKT_PKT_TYPE$] == [#1]" },
    { 1, 1, {PKT_PAYLOAD}, "[$CPUPKT_PKT_TYPE$] == [#2]" },
    { 0, 1, {ETHERNET}, "[$CPUPKT_PKT_TYPE$] == [#0]" },
};

static ctp_flags_repeat_t dot11_max_mac_num_repeat[] =
{
    { 0, 1, {DOT11_UDT}, "[$DOT11_MAX_MAC_NUM$]" },
};

static ctp_flags_jump_t dot11_mac_addr_jump[] =
{
    { 0, 9, {DOT11_MAC_ADDR}, "[@REPEAT@] > [#1]" },
};

static ctp_flags_jump_t dot11_seq_ctl_exist_jump[] =
{
    { 0, 1, {DOT11_MAC_ADDR4_EXIST}, "[$DOT11_SEQ_CTL_EXIST$]  == [#0]" },
};

static ctp_flags_jump_t dot11_mac_addr4_exist_jump[] =
{
    { 0, 1, {DOT11_QOS_CTL_EXIST}, "[$DOT11_MAC_ADDR4_EXIST$]  == [#0]" },
};

static ctp_flags_jump_t dot11_qos_ctl_exist_jump[] =
{
    { 0, 1, {DOT11_HT_CTL_EXIST}, "[$DOT11_QOS_CTL_EXIST$]  == [#0]" },
};

static ctp_flags_jump_t dot11_ht_ctl_exist_jump[] =
{
    { 0, 1, {DOT11_PAYLOAD_EXIST}, "[$DOT11_HT_CTL_EXIST$]  == [#0]" },
};

static ctp_flags_jump_t dot11_payload_exist_jump[] =
{
    { 1, 1, {PKT_PAYLOAD_LEN}, "[$DOT11_PAYLOAD_EXIST$] == [#1]" },
    { 0, 1, {DOT11_LLC_EXIST}, "[$DOT11_PAYLOAD_EXIST$] == [#0]" },
};

static ctp_flags_jump_t dot11_llc_exist_jump[] =
{
    { 0, 1, {DOT11_UDT}, "[$DOT11_LLC_EXIST$] == [#0]" },
};

static ctp_flags_jump_t dot11_llc_vlan_exist_jump[] =
{
    { 1, 1, {DOT11_LLC_TYPE}, "[$DOT11_LLC_VLAN_EXIST$]  == [#0]" },
    { 0, 1, {DOT11_LLC_VLAN_TAG}, "[$DOT11_LLC_VLAN_EXIST$]  == [#1]" },
};

static ctp_flags_nextp_t dot11_llc_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$DOT11_LLC_TYPE$] == [#0800]" },
    { 1, 1, {ARP}, "[$DOT11_LLC_TYPE$] == [#0806]" },
    { 1, 1, {CFM}, "[$DOT11_LLC_TYPE$] == [#8902]" },
    { 1, 1, {EAPOL}, "[$DOT11_LLC_TYPE$] == [#888E]" },
    { 1, 1, {MPLS}, "[$DOT11_LLC_TYPE$] == [#8847]" },
    { 1, 1, {MPLS}, "[$DOT11_LLC_TYPE$] == [#8848]" },
    { 1, 1, {PBB}, "[$DOT11_LLC_TYPE$] == [#88e7]" },
    { 1, 1, {ETAG}, "[$DOT11_LLC_TYPE$] == [#893F]" },
    { 1, 1, {FCOE}, "[$DOT11_LLC_TYPE$] == [#8906]" },
    { 1, 1, {FIP}, "[$DOT11_LLC_TYPE$] == [#8914]" },
    { 1, 1, {IPV6}, "[$DOT11_LLC_TYPE$] == [#86DD]" },
    { 1, 1, {PKT_PAYLOAD}, "[$DOT11_LLC_TYPE$] == [#0]" },
    { 1, 1, {SLPR}, "[$DOT11_LLC_TYPE$] == [#8809]" },
    { 1, 1, {PTP}, "[$DOT11_LLC_TYPE$] == [#88f7]" },
    { 1, 1, {TRILL}, "[$DOT11_LLC_TYPE$] == [#22f3]" },
    { 1, 1, {RBCHANNEL}, "[$DOT11_LLC_TYPE$] == [#22f5]" },
    { 1, 1, {NSH}, "[$DOT11_LLC_TYPE$] == [#894F]" },
    { 1, 1, {MACSECTAG}, "[$DOT11_LLC_TYPE$] == [#88e5]" },
    { 0, 1, {STP}, "[$DOT11_LLC_TYPE$] == [#7ff]" },
};

static ctp_flags_exist_t eth_tpid_exist[] =
{
    { 1, 1, {ETH_LEN_TYPE}, "[#0] == [#1]" },
    { 1, 1, {ETH_LEN_TYPE}, "[$ETH_TPID$] == [#8100]" },
    { 1, 1, {ETH_LEN_TYPE}, "[$ETH_TPID$] == [#9100]" },
    { 0, 1, {ETH_LEN_TYPE}, "[$ETH_TPID$] == [#88a8]" },
};

static ctp_flags_jump_t eth_vlan_id_jump[] =
{
    { 0, 1, {ETH_TPID}, "[null]" },
};

static ctp_flags_nextp_t eth_len_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$ETH_LEN_TYPE$] == [#0800]" },
    { 1, 1, {ARP}, "[$ETH_LEN_TYPE$] == [#0806]" },
    { 1, 1, {CFM}, "[$ETH_LEN_TYPE$] == [#8902]" },
    { 1, 1, {EAPOL}, "[$ETH_LEN_TYPE$] == [#888E]" },
    { 1, 1, {MPLS}, "[$ETH_LEN_TYPE$] == [#8847]" },
    { 1, 1, {MPLS}, "[$ETH_LEN_TYPE$] == [#8848]" },
    { 1, 1, {PBB}, "[$ETH_LEN_TYPE$] == [#88e7]" },
    { 1, 1, {ETAG}, "[$ETH_LEN_TYPE$] == [#893F]" },
    { 1, 1, {FCOE}, "[$ETH_LEN_TYPE$] == [#8906]" },
    { 1, 1, {FIP}, "[$ETH_LEN_TYPE$] == [#8914]" },
    { 1, 1, {IPV6}, "[$ETH_LEN_TYPE$] == [#86DD]" },
    { 1, 1, {PKT_PAYLOAD}, "[$ETH_LEN_TYPE$] == [#0]" },
    { 1, 1, {SLPR}, "[$ETH_LEN_TYPE$] == [#8809]" },
    { 1, 1, {PTP}, "[$ETH_LEN_TYPE$] == [#88f7]" },
    { 1, 1, {TRILL}, "[$ETH_LEN_TYPE$] == [#22f3]" },
    { 1, 1, {RBCHANNEL}, "[$ETH_LEN_TYPE$] == [#22f5]" },
    { 1, 1, {NSH}, "[$ETH_LEN_TYPE$] == [#894F]" },
    { 1, 1, {MACSECTAG}, "[$ETH_LEN_TYPE$] == [#88e5]" },
    { 1, 1, {SGTCMD}, "[$ETH_LEN_TYPE$] == [#8909]" },
    { 0, 1, {STP}, "[$ETH_LEN_TYPE$] == [#7ff]" },
};

static ctp_flags_exist_t eth_dsap_exist[] =
{
    { 1, 2, {ETH_UDT}, "[@PKT_LEN@] <= [@MTU@]" },
    { 0, 1, {ETH_UDT}, "[@EN_8022@] == [#1]" },
};

static ctp_flags_nextp_t eth_dsap_nextp[] =
{
    { 0, 1, {STP}, "[$ETH_DSAP$] == [#42]" },
};

static ctp_flags_exist_t eth_ctl1_exist[] =
{
    { 0, 1, {ETH_OUI}, "[$ETH_TPID$ & #3] != [#3]" },
};

static ctp_flags_exist_t eth_oui_exist[] =
{
    { 1, 2, {ETH_UDT}, "[$ETH_DSAP$] == [#AA]" },
    { 1, 2, {ETH_UDT}, "[$ETH_SSAP$] == [#AA]" },
    { 0, 1, {ETH_UDT}, "[$ETH_CTL0$] == [#3]" },
};

static ctp_flags_nextp_t eth_l2_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$ETH_L2_TYPE$] == [#0800]" },
    { 0, 1, {IPV6}, "[$ETH_L2_TYPE$] == [#86DD]" },
};

static ctp_flags_repeat_t pkt_payload_len_repeat[] =
{
    { 0, 1, {PKT_PAYLOAD_UDT}, "[$PKT_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t pkt_payload_len_jump[] =
{
    { 0, 1, {PKT_PAYLOAD_UDT}, "[$PKT_PAYLOAD_LEN$]  == [#0]" },
};

static ctp_flags_jump_t pkt_payload_data_jump[] =
{
    { 0, 9, {PKT_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
};

static ctp_flags_jump_t macsec_pn_jump[] =
{
    { 1, 1, {MACSEC_SCI}, "[$MACSEC_TCI_SC$] == [#1]" },
    { 0, 1, {MACSEC_PAYLOAD}, "[$MACSEC_TCI_SC$] == [#0]" },
};

static ctp_flags_nextp_t macsec_payload_nextp[] =
{
    { 0, 1, {PKT_PAYLOAD}, "[$MACSEC_PAYLOAD$] == [#0]" },
};

static ctp_flags_nextp_t sgtcmd_next_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$SGTCMD_NEXT_TYPE$] == [#0800]" },
    { 1, 1, {ARP}, "[$SGTCMD_NEXT_TYPE$] == [#0806]" },
    { 1, 1, {CFM}, "[$SGTCMD_NEXT_TYPE$] == [#8902]" },
    { 1, 1, {EAPOL}, "[$SGTCMD_NEXT_TYPE$] == [#888E]" },
    { 1, 1, {MPLS}, "[$SGTCMD_NEXT_TYPE$] == [#8847]" },
    { 1, 1, {MPLS}, "[$SGTCMD_NEXT_TYPE$] == [#8848]" },
    { 1, 1, {PBB}, "[$SGTCMD_NEXT_TYPE$] == [#88e7]" },
    { 1, 1, {ETAG}, "[$SGTCMD_NEXT_TYPE$] == [#893F]" },
    { 1, 1, {FCOE}, "[$SGTCMD_NEXT_TYPE$] == [#8906]" },
    { 1, 1, {FIP}, "[$SGTCMD_NEXT_TYPE$] == [#8914]" },
    { 1, 1, {IPV6}, "[$SGTCMD_NEXT_TYPE$] == [#86DD]" },
    { 1, 1, {PKT_PAYLOAD}, "[$SGTCMD_NEXT_TYPE$] == [#0]" },
    { 1, 1, {SLPR}, "[$SGTCMD_NEXT_TYPE$] == [#8809]" },
    { 1, 1, {PTP}, "[$SGTCMD_NEXT_TYPE$] == [#88f7]" },
    { 1, 1, {TRILL}, "[$SGTCMD_NEXT_TYPE$] == [#22f3]" },
    { 1, 1, {RBCHANNEL}, "[$SGTCMD_NEXT_TYPE$] == [#22f5]" },
    { 1, 1, {NSH}, "[$SGTCMD_NEXT_TYPE$] == [#894F]" },
    { 1, 1, {MACSECTAG}, "[$SGTCMD_NEXT_TYPE$] == [#88e5]" },
    { 1, 1, {SGTCMD}, "[$SGTCMD_NEXT_TYPE$] == [#8909]" },
    { 0, 1, {STP}, "[$SGTCMD_NEXT_TYPE$] == [#7ff]" },
};

static ctp_flags_repeat_t trill_ol_repeat[] =
{
    { 0, 1, {TRILL_NEXTP}, "[$TRILL_OL$]" },
};

static ctp_flags_exist_t trill_op_exist[] =
{
    { 0, 1, {TRILL_NEXTP}, "[$TRILL_OL$] == 0" },
};

static ctp_flags_jump_t trill_op_jump[] =
{
    { 1, 1, {TRILL_NEXTP}, "[$TRILL_OL$] == 0" },
    { 0, 9, {TRILL_OP}, "[@REPEAT@] > [#1]" },
};

static ctp_flags_jump_t trill_nextp_jump[] =
{
    { 1, 1, {L2H_MACDA}, "[$TRILL_NEXTP$] == [#1]" },
    { 0, 1, {TRILL_UDT}, "[null]" },
};

static ctp_flags_nextp_t chan_err_nextp[] =
{
    { 1, 1, {BFD}, "[$CHAN_PROTOTOL$] == [#2]" },
    { 1, 1, {PKT_PAYLOAD}, "[$CHAN_PROTOTOL$] == [#3]" },
    { 0, 1, {PKT_PAYLOAD}, "[null]" },
};

static ctp_flags_jump_t lab_ttl_jump[] =
{
    { 0, 1, {LABEL}, "[$LAB_S$] == [#0]" },
};

static ctp_flags_repeat_t lab_payload_len_repeat[] =
{
    { 0, 1, {MPLS_UDT}, "[$LAB_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t lab_payload_len_jump[] =
{
    { 0, 1, {MPLS_UDT}, "[$LAB_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t lab_payload_data_jump[] =
{
    { 0, 9, {LAB_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
};

static ctp_flags_nextp_t lab_nextp_nextp[] =
{
    { 1, 1, {L2H}, "[$LAB_NEXTP$] == [#1]" },
    { 1, 1, {IPV6}, "[$LAB_NEXTP$] == [#2]" },
    { 1, 1, {ACH}, "[$LAB_NEXTP$] == [#3]" },
    { 1, 1, {CW}, "[$LAB_NEXTP$] == [#4]" },
    { 1, 1, {PKT_PAYLOAD}, "[$LAB_NEXTP$] == [#0]" },
    { 0, 1, {IPV4}, "[$LAB_NEXTP$] == [#5]" },
};

static ctp_flags_nextp_t cw_nextp_nextp[] =
{
    { 0, 1, {L2H}, "[$CW_NEXTP$] == [#0]" },
};

static ctp_flags_repeat_t l2h_maxvnum_repeat[] =
{
    { 0, 1, {L2H_UDT}, "[$L2H_MAXVNUM$]" },
};

static ctp_flags_exist_t l2h_type_id_exist[] =
{
    { 1, 1, {L2H_LEN_TYPE}, "[$L2H_TYPE_ID$] == [#8100]" },
    { 0, 1, {L2H_LEN_TYPE}, "[$L2H_TYPE_ID$] == [#88a8]" },
};

static ctp_flags_jump_t l2h_vlan_id_jump[] =
{
    { 0, 9, {L2H_TYPE_ID}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_nextp_t l2h_len_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$L2H_LEN_TYPE$] == [#0800]" },
    { 1, 1, {EAPOL}, "[$L2H_LEN_TYPE$] == [#888E]" },
    { 1, 1, {MPLS}, "[$L2H_LEN_TYPE$] == [#8847]" },
    { 1, 1, {PBB}, "[$L2H_LEN_TYPE$] == [#88e7]" },
    { 1, 1, {CFM}, "[$L2H_LEN_TYPE$] == [#8902]" },
    { 1, 1, {PTP}, "[$L2H_LEN_TYPE$] == [#88f7]" },
    { 1, 1, {RBCHANNEL}, "[$L2H_LEN_TYPE$] == [#22f5]" },
    { 1, 1, {PKT_PAYLOAD}, "[$L2H_LEN_TYPE$] == [#0]" },
    { 0, 1, {IPV6}, "[$L2H_LEN_TYPE$] == [#86DD]" },
};

static ctp_flags_nextp_t ach_channeltype_nextp[] =
{
    { 1, 1, {CFM}, "[$ACH_CHANNELTYPE$] == [#8902]" },
    { 1, 1, {BFD}, "[$ACH_CHANNELTYPE$] == [#0022]" },
    { 1, 1, {BFD}, "[$ACH_CHANNELTYPE$] == [#0023]" },
    { 1, 1, {BFD}, "[$ACH_CHANNELTYPE$] == [#7]" },
    { 1, 1, {CSF}, "[$ACH_CHANNELTYPE$] == [#9]" },
    { 1, 1, {MPLSLM}, "[$ACH_CHANNELTYPE$] == [#a]" },
    { 1, 1, {MPLSLM}, "[$ACH_CHANNELTYPE$] == [#b]" },
    { 1, 1, {MPLSDM}, "[$ACH_CHANNELTYPE$] == [#c]" },
    { 1, 1, {MPLSLDM}, "[$ACH_CHANNELTYPE$] == [#d]" },
    { 1, 1, {MPLSLDM}, "[$ACH_CHANNELTYPE$] == [#e]" },
    { 1, 1, {IPV4}, "[$ACH_CHANNELTYPE$] == [#0021]" },
    { 1, 1, {IPV6}, "[$ACH_CHANNELTYPE$] == [#0057]" },
    { 0, 1, {PKT_PAYLOAD}, "[$ACH_CHANNELTYPE$] == [#0]" },
};

static ctp_flags_exist_t mplslm_tlv_type_exist[] =
{
    { 0, 1, {MPLSLM_UDT}, "[#0] == [#1]" },
};

static ctp_flags_jump_t mplslm_tlv_type_jump[] =
{
    { 0, 1, {MPLSLM_UDT}, "[$MPLSLM_TLV_TYPE$] == [#0]" },
};

static ctp_flags_repeat_t mplslm_tlv_len_repeat[] =
{
    { 0, 1, {MPLSLM_UDT}, "[$MPLSLM_TLV_LEN$]" },
};

static ctp_flags_jump_t mplslm_tlv_value_jump[] =
{
    { 1, 9, {MPLSLM_TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {MPLSLM_TLV_TYPE}, "[null]" },
};

static ctp_flags_exist_t mplsdm_tlv_type_exist[] =
{
    { 0, 1, {MPLSDM_UDT}, "[#0] == [#1]" },
};

static ctp_flags_jump_t mplsdm_tlv_type_jump[] =
{
    { 0, 1, {MPLSDM_UDT}, "[$MPLSDM_TLV_TYPE$] == [#0]" },
};

static ctp_flags_repeat_t mplsdm_tlv_len_repeat[] =
{
    { 0, 1, {MPLSDM_UDT}, "[$MPLSDM_TLV_LEN$]" },
};

static ctp_flags_jump_t mplsdm_tlv_value_jump[] =
{
    { 1, 9, {MPLSDM_TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {MPLSDM_TLV_TYPE}, "[null]" },
};

static ctp_flags_exist_t mplsldm_tlv_type_exist[] =
{
    { 0, 1, {MPLSLDM_UDT}, "[#0] == [#1]" },
};

static ctp_flags_jump_t mplsldm_tlv_type_jump[] =
{
    { 0, 1, {MPLSLDM_UDT}, "[$MPLSLDM_TLV_TYPE$] == [#0]" },
};

static ctp_flags_repeat_t mplsldm_tlv_len_repeat[] =
{
    { 0, 1, {MPLSLDM_UDT}, "[$MPLSLDM_TLV_LEN$]" },
};

static ctp_flags_jump_t mplsldm_tlv_value_jump[] =
{
    { 1, 9, {MPLSLDM_TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {MPLSLDM_TLV_TYPE}, "[null]" },
};

static ctp_flags_exist_t pbb_stag_tpid_exist[] =
{
    { 1, 1, {PBB_CTAG_TPID}, "[#1] == [#0]" },
    { 1, 1, {PBB_CTAG_TPID}, "[$PBB_STAG_TPID$]  ==  [#8100]" },
    { 0, 1, {PBB_CTAG_TPID}, "[$PBB_STAG_TPID$]  ==  [#88a8]" },
};

static ctp_flags_exist_t pbb_ctag_tpid_exist[] =
{
    { 1, 1, {PBB_LEN_TYPE}, "[#1] == [#0]" },
    { 1, 1, {PBB_LEN_TYPE}, "[$PBB_CTAG_TPID$]  ==  [#8100]" },
    { 0, 1, {PBB_LEN_TYPE}, "[$PBB_CTAG_TPID$]  ==  [#88a8]" },
};

static ctp_flags_nextp_t pbb_len_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$PBB_LEN_TYPE$] == [#0800]" },
    { 1, 1, {EAPOL}, "[$PBB_LEN_TYPE$] == [#888E]" },
    { 1, 1, {MPLS}, "[$PBB_LEN_TYPE$] == [#8847]" },
    { 1, 1, {CCM}, "[$PBB_LEN_TYPE$] == [#8902]" },
    { 0, 1, {IPV6}, "[$PBB_LEN_TYPE$] == [#86DD]" },
};

static ctp_flags_exist_t etag_stag_tpid_exist[] =
{
    { 1, 1, {ETAG_CTAG_TPID}, "[#1] == [#0]" },
    { 0, 1, {ETAG_STAG_COS}, "[#1] == [#1]" },
};

static ctp_flags_exist_t etag_ctag_tpid_exist[] =
{
    { 1, 1, {ETAG_LEN_TYPE}, "[#1] == [#0]" },
    { 0, 1, {ETAG_CTAG_COS}, "[#1] == [#1]" },
};

static ctp_flags_nextp_t etag_len_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$ETAG_LEN_TYPE$] == [#0800]" },
    { 1, 1, {ARP}, "[$ETAG_LEN_TYPE$] == [#0806]" },
    { 1, 1, {EAPOL}, "[$ETAG_LEN_TYPE$] == [#888E]" },
    { 1, 1, {MPLS}, "[$ETAG_LEN_TYPE$] == [#8847]" },
    { 1, 1, {CCM}, "[$ETAG_LEN_TYPE$] == [#8902]" },
    { 0, 1, {IPV6}, "[$ETAG_LEN_TYPE$] == [#86DD]" },
};

static ctp_flags_repeat_t fip_payload_len_repeat[] =
{
    { 0, 1, {FIP_UDT}, "[$FIP_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t fip_payload_len_jump[] =
{
    { 0, 1, {FIP_UDT}, "[$FIP_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t fip_payload_data_jump[] =
{
    { 1, 9, {FIP_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {FIP_UDT}, "[null]" },
};

static ctp_flags_repeat_t fcoe_payload_len_repeat[] =
{
    { 0, 1, {FCOE_UDT}, "[$FCOE_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t fcoe_payload_len_jump[] =
{
    { 0, 1, {FCOE_EOF}, "[$FCOE_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t fcoe_payload_data_jump[] =
{
    { 1, 9, {FCOE_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {FCOE_EOF}, "[null]" },
};

static ctp_flags_jump_t end_tlv_jump[] =
{
    { 0, 1, {CCM_PAYLOAD}, "[$END_TLV$] == [#0]" },
};

static ctp_flags_repeat_t tlv_len_repeat[] =
{
    { 0, 1, {CCM_UDT}, "[$TLV_LEN$]" },
};

static ctp_flags_jump_t tlv_value_jump[] =
{
    { 1, 9, {TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {END_TLV}, "[null]" },
};

static ctp_flags_exist_t ccm_payload_exist[] =
{
    { 1, 1, {CCM_UDT}, "[$CCM_PAYLOAD$] == [#aa]" },
    { 0, 1, {CCM_UDT}, "[$CCM_PAYLOAD$] == [#bb]" },
};

static ctp_flags_jump_t ccm_payload_jump[] =
{
    { 0, 1, {CCM_PAYLOAD}, "[null]" },
};

static ctp_flags_nextp_t gre_prot_nextp[] =
{
    { 1, 1, {IPV4}, "[$GRE_PROT$] == [#0800]" },
    { 1, 1, {EAPOL}, "[$GRE_PROT$] == [#888E]" },
    { 1, 1, {L2H}, "[$GRE_PROT$] == [#6558]" },
    { 1, 1, {MPLS}, "[$GRE_PROT$] == [#8847]" },
    { 1, 1, {MPLS}, "[$GRE_PROT$] == [#8848]" },
    { 1, 1, {PBB}, "[$GRE_PROT$] == [#88e7]" },
    { 1, 1, {CCM}, "[$GRE_PROT$] == [#8902]" },
    { 1, 1, {PKT_PAYLOAD}, "[$GRE_PROT$] == [#0]" },
    { 0, 1, {IPV6}, "[$GRE_PROT$] == [#86DD]" },
};

static ctp_flags_jump_t gre_prot_jump[] =
{
    { 1, 1, {GRE_UDT}, "[$GRE_CBIT$ + $GRE_KBIT$ + $GRE_SBIT$] == [#0]" },
    { 1, 1, {GRE_CHECKSUM}, "[$GRE_CBIT$] == [#1]" },
    { 1, 1, {GRE_KEY}, "[$GRE_KBIT$] == [#1]" },
    { 0, 1, {GRE_SEQ}, "[$GRE_SBIT$] == [#1]" },
};

static ctp_flags_jump_t gre_res1_jump[] =
{
    { 1, 1, {GRE_UDT}, "[$GRE_KBIT$ + $GRE_SBIT$] == [#0]" },
    { 1, 1, {GRE_KEY}, "[$GRE_SBIT$] == [#0]" },
    { 0, 1, {GRE_SEQ}, "[$GRE_KBIT$] == [#0]" },
};

static ctp_flags_jump_t gre_key_jump[] =
{
    { 0, 1, {GRE_UDT}, "[$GRE_SBIT$] == [#0]" },
};

static ctp_flags_jump_t eapol_len_jump[] =
{
    { 0, 1, {EAPOL_UDT}, "[$EAPOL_TYPE$] != [#0]" },
};

static ctp_flags_repeat_t eap_packet_len_repeat[] =
{
    { 0, 1, {EAPOL_UDT}, "[$EAP_PACKET_LEN$]" },
};

static ctp_flags_jump_t eap_packet_len_jump[] =
{
    { 1, 1, {EAPOL_UDT}, "[$EAP_PACKET_LEN$] == [#0]" },
    { 1, 1, {EAP_PACKET_TYPE}, "[$EAP_PACKET_CODE$] == [#1]" },
    { 1, 1, {EAP_PACKET_TYPE}, "[$EAP_PACKET_CODE$] == [#2]" },
    { 0, 1, {EAPOL_UDT}, "[null]" },
};

static ctp_flags_jump_t eap_packet_type_jump[] =
{
    { 0, 1, {EAP_MD5_CHALL_VALUE_SIZE}, "[$EAP_PACKET_TYPE$] == [#4]" },
};

static ctp_flags_jump_t eap_packet_data_jump[] =
{
    { 1, 9, {EAP_PACKET_DATA}, "[@REPEAT@] > [#6]" },
    { 0, 1, {EAPOL_UDT}, "[null]" },
};

static ctp_flags_repeat_t eap_md5_chall_value_size_repeat[] =
{
    { 0, 1, {EAPOL_UDT}, "[$EAP_MD5_CHALL_VALUE_SIZE$]" },
};

static ctp_flags_jump_t eap_md5_chall_value_jump[] =
{
    { 1, 9, {EAP_MD5_CHALL_VALUE}, "[@REPEAT@] > [#1]" },
    { 1, 1, {EAPOL_UDT}, "[$EAP_PACKET_LEN$ - $EAP_MD5_CHALL_VALUE_SIZE$ - #6] <= [#0]" },
    { 0, 1, {EAP_MD5_CHALL_NAME_LEN}, "[null]" },
};

static ctp_flags_repeat_t eap_md5_chall_name_len_repeat[] =
{
    { 0, 1, {EAPOL_UDT}, "[$EAP_MD5_CHALL_NAME_LEN$]" },
};

static ctp_flags_jump_t eap_md5_chall_name_jump[] =
{
    { 1, 9, {EAP_MD5_CHALL_NAME}, "[@REPEAT@] > [#0]" },
    { 0, 1, {EAPOL_UDT}, "[null]" },
};

static ctp_flags_jump_t slpr_subtype_jump[] =
{
    { 1, 1, {SLPR_EFM_FLAGS}, "[$SLPR_SUBTYPE$] == [#3]" },
    { 0, 1, {SLPR_LACP_VER}, "[$SLPR_SUBTYPE$] == [#1]" },
};

static ctp_flags_repeat_t slpr_efm_pld_len_repeat[] =
{
    { 0, 1, {SLPR_UDT}, "[$SLPR_EFM_PLD_LEN$]" },
};

static ctp_flags_jump_t slpr_efm_pld_data_jump[] =
{
    { 1, 9, {SLPR_EFM_PLD_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {SLPR_UDT}, "[null]" },
};

static ctp_flags_jump_t cfm_first_tlv_offset_jump[] =
{
    { 1, 1, {CFM_LB_TP}, "[$CFM_OP_CODE$] == [#2]" },
    { 1, 1, {CFM_LB_TP}, "[$CFM_OP_CODE$] == [#3]" },
    { 1, 1, {CFM_LB_TP}, "[$CFM_OP_CODE$] == [#25]" },
    { 1, 1, {CFM_LT_ID}, "[$CFM_OP_CODE$] == [#4]" },
    { 1, 1, {CFM_LT_ID}, "[$CFM_OP_CODE$] == [#5]" },
    { 1, 1, {CFM_LM_TXFCF}, "[$CFM_OP_CODE$] == [#2A]" },
    { 1, 1, {CFM_LM_TXFCF}, "[$CFM_OP_CODE$] == [#2B]" },
    { 1, 1, {CFM_APS_LEN}, "[$CFM_OP_CODE$] == [#27]" },
    { 1, 1, {CFM_APS_LEN}, "[$CFM_OP_CODE$] == [#28]" },
    { 1, 1, {CFM_DM_TX_STPF}, "[$CFM_OP_CODE$] == [#2d]" },
    { 1, 1, {CFM_DM_TX_STPF}, "[$CFM_OP_CODE$] == [#2e]" },
    { 1, 1, {CFM_DM_TX_STPF}, "[$CFM_OP_CODE$] == [#2f]" },
    { 1, 1, {CFM_CCM_SEQ_NUM}, "[$CFM_OP_CODE$] == [#1]" },
    { 1, 1, {CFM_SLM_SRC_MEPID}, "[$CFM_OP_CODE$] == [#37]" },
    { 1, 1, {CFM_SLM_SRC_MEPID}, "[$CFM_OP_CODE$] == [#36]" },
    { 0, 1, {CFM_PAYLOAD_LEN}, "[null]" },
};

static ctp_flags_repeat_t cfm_payload_len_repeat[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t cfm_payload_len_jump[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t cfm_payload_data_jump[] =
{
    { 1, 9, {CFM_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {CFM_UDT}, "[null]" },
};

static ctp_flags_jump_t cfm_ccm_main_domain_name_format_jump[] =
{
    { 0, 1, {CFM_CCM_SHORT_MA_NAME_FORMAT}, "[$CFM_CCM_MAIN_DOMAIN_NAME_FORMAT$] == [#1]" },
};

static ctp_flags_repeat_t cfm_ccm_main_domain_name_len_repeat[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_CCM_MAIN_DOMAIN_NAME_LEN$]" },
};

static ctp_flags_jump_t cfm_ccm_main_domain_name_str_jump[] =
{
    { 1, 9, {CFM_CCM_MAIN_DOMAIN_NAME_STR}, "[@REPEAT@] > [#1]" },
    { 0, 1, {CFM_CCM_SHORT_MA_NAME_FORMAT}, "[null]" },
};

static ctp_flags_jump_t cfm_ccm_short_ma_name_format_jump[] =
{
    { 0, 1, {CFM_CCM_MAID_ZERO_PAD}, "[$CFM_CCM_SHORT_MA_NAME_FORMAT$] == [#1]" },
};

static ctp_flags_repeat_t cfm_ccm_short_ma_name_len_repeat[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_CCM_SHORT_MA_NAME_LEN$]" },
};

static ctp_flags_jump_t cfm_ccm_short_ma_name_len_jump[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_CCM_SHORT_MA_NAME_LEN$] == [#0]" },
};

static ctp_flags_jump_t cfm_ccm_short_ma_name_str_jump[] =
{
    { 1, 9, {CFM_CCM_SHORT_MA_NAME_STR}, "[@REPEAT@] > [#1]" },
    { 0, 1, {CFM_CCM_MAID_ZERO_PAD}, "[null]" },
};

static ctp_flags_exist_t cfm_ccm_maid_zero_pad_exist[] =
{
    { 1, 1, {CFM_CCM_TXFCF}, "[#0] == [#1]" },
    { 0, 1, {CFM_CCM_MAID_ZERO_PAD}, "[#1] == [#1]" },
};

static ctp_flags_jump_t cfm_ccm_maid_zero_pad_jump[] =
{
    { 0, 1, {CFM_CCM_MAID_ZERO_PAD}, "[null]" },
};

static ctp_flags_jump_t cfm_ccm_reserved_jump[] =
{
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_jump_t cfm_lb_id_jump[] =
{
    { 1, 1, {CFM_TP_LBM_TLV_TYPE}, "[$CFM_LB_TP$] == [#1]" },
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_jump_t cfm_lt_ttl_jump[] =
{
    { 0, 1, {CFM_LTR_RL_ACT}, "[$CFM_OP_CODE$] == [#4]" },
};

static ctp_flags_jump_t cfm_ltm_tar_mac_jump[] =
{
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_jump_t cfm_ltr_rl_act_jump[] =
{
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_jump_t cfm_lm_txfcb_jump[] =
{
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_repeat_t cfm_aps_len_repeat[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_APS_LEN$]" },
};

static ctp_flags_jump_t cfm_aps_data_jump[] =
{
    { 1, 9, {CFM_APS_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_exist_t cfm_dm_tx_stpb_exist[] =
{
    { 1, 1, {CFM_TLV_TYPE}, "[#0] == [#1]" },
    { 0, 1, {CFM_TLV_TYPE}, "[$CFM_OP_CODE$] != [#2d]" },
};

static ctp_flags_jump_t cfm_dm_rx_stpb_jump[] =
{
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_jump_t cfm_slm_txfcb_jump[] =
{
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_jump_t cfm_tlv_type_jump[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_TLV_TYPE$] == [#0]" },
};

static ctp_flags_repeat_t cfm_tlv_len_repeat[] =
{
    { 0, 1, {CFM_UDT}, "[$CFM_TLV_LEN$]" },
};

static ctp_flags_jump_t cfm_tlv_value_jump[] =
{
    { 1, 9, {CFM_TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {CFM_TLV_TYPE}, "[null]" },
};

static ctp_flags_nextp_t ipv4_proto_nextp[] =
{
    { 1, 1, {PKT_PAYLOAD}, "[$IPV4_PAYLOAD_FORCE$] == [#1]" },
    { 1, 1, {TCP}, "[$IPV4_PROTO$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV4_PROTO$] == [#11]" },
    { 1, 1, {ICMP}, "[$IPV4_PROTO$] == [#1]" },
    { 1, 1, {IGMP}, "[$IPV4_PROTO$] == [#2]" },
    { 1, 1, {VRRP}, "[$IPV4_PROTO$] == [#70]" },
    { 1, 1, {IPV6}, "[$IPV4_PROTO$] == [#29]" },
    { 1, 1, {GRE}, "[$IPV4_PROTO$] == [#2f]" },
    { 1, 1, {PKT_PAYLOAD}, "[$IPV4_PROTO$] == [#0]" },
    { 1, 1, {OSPF}, "[$IPV4_PROTO$] == [#59]" },
    { 1, 1, {PKT_PAYLOAD}, "[$IPV4_PROTO$] == [#67]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_jump_t ipv4_da_jump[] =
{
    { 1, 1, {IPV4_UDT}, "[$IPV4_PROTO$] == [#4]" },
    { 1, 1, {IPV4_UDT}, "[$IPV4_PROTO$] == [#29]" },
    { 0, 1, {IPV4_UDT}, "[$IPV4_PROTO$] == [#2f]" },
};

static ctp_flags_repeat_t ipv4_payload_len_repeat[] =
{
    { 0, 1, {IPV4_UDT}, "[$IPV4_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t ipv4_payload_len_jump[] =
{
    { 0, 1, {IPV4_UDT}, "[$IPV4_PAYLOAD_LEN$]  == [#0]" },
};

static ctp_flags_jump_t ipv4_payload_jump[] =
{
    { 1, 9, {IPV4_PAYLOAD}, "[@REPEAT@] > [#1]" },
    { 0, 1, {IPV4_UDT}, "[null]" },
};

static ctp_flags_nextp_t ipv6_next_hdr_nextp[] =
{
    { 1, 1, {IPV6_HBH_OPT_HEADER}, "[$IPV6_NEXT_HDR$] == [#0]" },
    { 1, 1, {IPV4}, "[$IPV6_NEXT_HDR$] == [#4]" },
    { 1, 1, {IPV6}, "[$IPV6_NEXT_HDR$] == [#29]" },
    { 1, 1, {GRE}, "[$IPV6_NEXT_HDR$] == [#2f]" },
    { 1, 1, {IPV6_DEST_OPT_HEADER}, "[$IPV6_NEXT_HDR$] == [#3c]" },
    { 1, 1, {IPV6_ROUT_HEADER}, "[$IPV6_NEXT_HDR$] == [#2b]" },
    { 1, 1, {IPV6_FRAG_HEADER}, "[$IPV6_NEXT_HDR$] == [#2c]" },
    { 1, 1, {IPV6_AUTHEN_HEADER}, "[$IPV6_NEXT_HDR$] == [#33]" },
    { 1, 1, {OSPF}, "[$IPV6_NEXT_HDR$] == [#59]" },
    { 1, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER}, "[$IPV6_NEXT_HDR$] == [#32]" },
    { 1, 1, {TCP}, "[$IPV6_NEXT_HDR$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_NEXT_HDR$] == [#11]" },
    { 1, 1, {ICMPV6}, "[$IPV6_NEXT_HDR$] == [#3a]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_jump_t ipv6_da_jump[] =
{
    { 1, 1, {IPV6_UDT}, "[$IPV6_NEXT_HDR$] == [#4]" },
    { 1, 1, {IPV6_UDT}, "[$IPV6_NEXT_HDR$] == [#29]" },
    { 0, 1, {IPV6_UDT}, "[$IPV6_NEXT_HDR$] == [#2f]" },
};

static ctp_flags_repeat_t ipv6_payload_len_repeat[] =
{
    { 0, 1, {IPV6_UDT}, "[$IPV6_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t ipv6_payload_len_jump[] =
{
    { 0, 1, {IPV6_UDT}, "[$IPV6_PAYLOAD_LEN$]  == [#0]" },
};

static ctp_flags_jump_t ipv6_payload_jump[] =
{
    { 1, 9, {IPV6_PAYLOAD}, "[@REPEAT@] > [#1]" },
    { 0, 1, {IPV6_UDT}, "[null]" },
};

static ctp_flags_nextp_t ipv6_hbh_opt_header_next_header_nextp[] =
{
    { 1, 1, {IPV6_DEST_OPT_HEADER}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#3c]" },
    { 1, 1, {IPV6_ROUT_HEADER}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#2b]" },
    { 1, 1, {IPV6_FRAG_HEADER}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#2c]" },
    { 1, 1, {IPV6_AUTHEN_HEADER}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#33]" },
    { 1, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#32]" },
    { 1, 1, {TCP}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#11]" },
    { 1, 1, {IPV6}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#29]" },
    { 1, 1, {ICMPV6}, "[$IPV6_HBH_OPT_HEADER_NEXT_HEADER$] == [#3a]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_jump_t ipv6_hbh_opt_header_option_type_jump[] =
{
    { 1, 1, {IPV6_HBH_OPT_HEADER_UDT}, "[$IPV6_HBH_OPT_HEADER_OPTION_TYPE$] == [#0]" },
    { 1, 1, {IPV6_HBH_OPT_HEADER_PADN_OPTION_LEN}, "[$IPV6_HBH_OPT_HEADER_OPTION_TYPE$] == [#1]" },
    { 1, 1, {IPV6_HBH_OPT_HEADER_PADN_OPTION_LEN}, "[$IPV6_HBH_OPT_HEADER_OPTION_TYPE$] == [#87]" },
    { 1, 1, {IPV6_HBH_OPT_HEADER_JUM_PAYL_OPTION_LEN}, "[$IPV6_HBH_OPT_HEADER_OPTION_TYPE$] == [#c2]" },
    { 0, 1, {IPV6_HBH_OPT_HEADER_ROUT_ALT_OPTION_LEN}, "[$IPV6_HBH_OPT_HEADER_OPTION_TYPE$] == [#5]" },
};

static ctp_flags_jump_t ipv6_hbh_opt_header_jum_payl_len_jump[] =
{
    { 0, 1, {IPV6_HBH_OPT_HEADER_UDT}, "[null]" },
};

static ctp_flags_jump_t ipv6_hbh_opt_header_rout_alt_value_jump[] =
{
    { 0, 1, {IPV6_HBH_OPT_HEADER_UDT}, "[null]" },
};

static ctp_flags_repeat_t ipv6_hbh_opt_header_padn_option_len_repeat[] =
{
    { 0, 1, {IPV6_HBH_OPT_HEADER_UDT}, "[$IPV6_HBH_OPT_HEADER_PADN_OPTION_DATA_NUM$]" },
};

static ctp_flags_jump_t ipv6_hbh_opt_header_padn_option_data_jump[] =
{
    { 0, 9, {IPV6_HBH_OPT_HEADER_PADN_OPTION_DATA}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_exist_t ipv6_hbh_opt_header_padn_option_data_exist[] =
{
    { 0, 1, {IPV6_HBH_OPT_HEADER_UDT}, "[$IPV6_HBH_OPT_HEADER_PADN_OPTION_DATA$ ] > [#0]" },
};

static ctp_flags_nextp_t ipv6_dest_opt_header_next_header_nextp[] =
{
    { 1, 1, {IPV6_ROUT_HEADER}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#2b]" },
    { 1, 1, {IPV6_FRAG_HEADER}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#2c]" },
    { 1, 1, {IPV6_AUTHEN_HEADER}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#33]" },
    { 1, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#32]" },
    { 1, 1, {TCP}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#11]" },
    { 1, 1, {IPV6}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#29]" },
    { 1, 1, {ICMPV6}, "[$IPV6_DEST_OPT_HEADER_NEXT_HEADER$] == [#3a]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_jump_t ipv6_dest_opt_header_option_type_jump[] =
{
    { 1, 1, {IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_LEN}, "[$IPV6_DEST_OPT_HEADER_OPTION_TYPE$] == [#c6]" },
    { 1, 1, {IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_LEN}, "[$IPV6_DEST_OPT_HEADER_OPTION_TYPE$] == [#7]" },
    { 1, 1, {IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_LEN}, "[$IPV6_DEST_OPT_HEADER_OPTION_TYPE$] == [#8]" },
    { 0, 1, {IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_LEN}, "[$IPV6_DEST_OPT_HEADER_OPTION_TYPE$] == [#c9]" },
};

static ctp_flags_repeat_t ipv6_dest_opt_header_bind_up_option_sub_option_num_repeat[] =
{
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[$IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SUB_OPTION_NUM$]" },
};

static ctp_flags_jump_t ipv6_dest_opt_header_bind_up_option_sub_option_jump[] =
{
    { 1, 9, {IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SUB_OPTION}, "[@REPEAT@] > [#0]" },
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[null]" },
};

static ctp_flags_repeat_t ipv6_dest_opt_header_bind_ack_option_sub_option_num_repeat[] =
{
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[$IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SUB_OPTION_NUM$]" },
};

static ctp_flags_jump_t ipv6_dest_opt_header_bind_ack_option_sub_option_jump[] =
{
    { 1, 9, {IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SUB_OPTION}, "[@REPEAT@] > [#0]" },
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[null]" },
};

static ctp_flags_repeat_t ipv6_dest_opt_header_bind_req_option_sub_option_num_repeat[] =
{
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[$IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_SUB_OPTION_NUM$]" },
};

static ctp_flags_jump_t ipv6_dest_opt_header_bind_req_option_sub_option_jump[] =
{
    { 1, 9, {IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_SUB_OPTION}, "[@REPEAT@] > [#0]" },
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[null]" },
};

static ctp_flags_repeat_t ipv6_dest_opt_header_home_addr_option_sub_option_num_repeat[] =
{
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[$IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_SUB_OPTION_NUM$]" },
};

static ctp_flags_jump_t ipv6_dest_opt_header_home_addr_option_sub_option_jump[] =
{
    { 1, 9, {IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_SUB_OPTION}, "[@REPEAT@] > [#0]" },
    { 0, 1, {IPV6_DEST_OPT_HEADER_UDT}, "[null]" },
};

static ctp_flags_nextp_t ipv6_rout_header_next_header_nextp[] =
{
    { 1, 1, {IPV6_FRAG_HEADER}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#2c]" },
    { 1, 1, {IPV6_AUTHEN_HEADER}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#33]" },
    { 1, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#32]" },
    { 1, 1, {TCP}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#11]" },
    { 1, 1, {IPV6}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#29]" },
    { 1, 1, {ICMPV6}, "[$IPV6_ROUT_HEADER_NEXT_HEADER$] == [#3a]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_jump_t ipv6_rout_header_rout_type_jump[] =
{
    { 0, 1, {IPV6_ROUT_TYPE0_HEADER_SEG_LEFT}, "[$IPV6_ROUT_HEADER_ROUT_TYPE$] == [#0]" },
};

static ctp_flags_repeat_t ipv6_rout_header_rout_type_spec_data_num_repeat[] =
{
    { 0, 1, {IPV6_ROUT_HEADER_UDT}, "[$IPV6_ROUT_HEADER_ROUT_TYPE_SPEC_DATA_NUM$]" },
};

static ctp_flags_jump_t ipv6_rout_header_rout_type_spec_data_jump[] =
{
    { 1, 9, {IPV6_ROUT_HEADER_ROUT_TYPE_SPEC_DATA}, "[@REPEAT@] > [#0]" },
    { 0, 1, {IPV6_ROUT_HEADER_UDT}, "[null]" },
};

static ctp_flags_repeat_t ipv6_rout_type0_header_addr_num_repeat[] =
{
    { 0, 1, {IPV6_ROUT_HEADER_UDT}, "[$IPV6_ROUT_TYPE0_HEADER_ADDR_NUM$]" },
};

static ctp_flags_jump_t ipv6_rout_type0_header_addr_jump[] =
{
    { 0, 9, {IPV6_ROUT_TYPE0_HEADER_ADDR}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_nextp_t ipv6_frag_header_next_header_nextp[] =
{
    { 1, 1, {IPV6_AUTHEN_HEADER}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#33]" },
    { 1, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#32]" },
    { 1, 1, {TCP}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#11]" },
    { 1, 1, {IPV6}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#29]" },
    { 1, 1, {ICMPV6}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#3a]" },
    { 1, 1, {GRE}, "[$IPV6_FRAG_HEADER_NEXT_HEADER$] == [#2f]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_nextp_t ipv6_authen_header_next_header_nextp[] =
{
    { 1, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER}, "[$IPV6_AUTHEN_HEADER_NEXT_HEADER$] == [#32]" },
    { 1, 1, {TCP}, "[$IPV6_AUTHEN_HEADER_NEXT_HEADER$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_AUTHEN_HEADER_NEXT_HEADER$] == [#11]" },
    { 1, 1, {IPV6}, "[$IPV6_AUTHEN_HEADER_NEXT_HEADER$] == [#29]" },
    { 1, 1, {ICMPV6}, "[$IPV6_AUTHEN_HEADER_NEXT_HEADER$] == [#3a]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_repeat_t ipv6_authen_header_authen_data_num_repeat[] =
{
    { 0, 1, {IPV6_AUTHEN_HEADER_UDT}, "[$IPV6_AUTHEN_HEADER_AUTHEN_DATA_NUM$]" },
};

static ctp_flags_jump_t ipv6_authen_header_authen_data_jump[] =
{
    { 0, 9, {IPV6_AUTHEN_HEADER_AUTHEN_DATA}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_nextp_t ipv6_encap_seuc_payl_header_next_header_nextp[] =
{
    { 1, 1, {TCP}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_NEXT_HEADER$] == [#6]" },
    { 1, 1, {UDP}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_NEXT_HEADER$] == [#11]" },
    { 1, 1, {IPV6}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_NEXT_HEADER$] == [#29]" },
    { 1, 1, {ICMPV6}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_NEXT_HEADER$] == [#3a]" },
    { 0, 1, {HEADER_TYPE_NUM}, "[null]" },
};

static ctp_flags_repeat_t ipv6_encap_seuc_payl_header_payl_data_num_repeat[] =
{
    { 0, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER_UDT}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_PAYL_DATA_NUM$]" },
};

static ctp_flags_jump_t ipv6_encap_seuc_payl_header_payl_data_jump[] =
{
    { 0, 9, {IPV6_ENCAP_SEUC_PAYL_HEADER_PAYL_DATA}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_repeat_t ipv6_encap_seuc_payl_header_padding_num_repeat[] =
{
    { 0, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER_UDT}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING_NUM$]" },
};

static ctp_flags_jump_t ipv6_encap_seuc_payl_header_padding_jump[] =
{
    { 0, 9, {IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_repeat_t ipv6_encap_seuc_payl_header_authen_data_num_repeat[] =
{
    { 0, 1, {IPV6_ENCAP_SEUC_PAYL_HEADER_UDT}, "[$IPV6_ENCAP_SEUC_PAYL_HEADER_AUTHEN_DATA_NUM$]" },
};

static ctp_flags_jump_t ipv6_encap_seuc_payl_header_authen_data_jump[] =
{
    { 0, 9, {IPV6_ENCAP_SEUC_PAYL_HEADER_AUTHEN_DATA}, "[@REPEAT@] > [#0]" },
};

static ctp_flags_repeat_t vrrp_count_ip_addr_repeat[] =
{
    { 0, 1, {VRRP_UDT}, "[$VRRP_COUNT_IP_ADDR$]" },
};

static ctp_flags_jump_t vrrp_ip_addr_jump[] =
{
    { 1, 9, {VRRP_IP_ADDR}, "[@REPEAT@] > [#1]" },
    { 0, 1, {VRRP_AUTH_DATA1}, "[null]" },
};

static ctp_flags_jump_t tcp_use_payload_jump[] =
{
    { 1, 1, {PKT_PAYLOAD_LEN}, "[$TCP_USE_PAYLOAD$] > [#0]" },
    { 0, 1, {TCP_SOURCE_PORT}, "[null]" },
};

static ctp_flags_nextp_t tcp_option_nextp[] =
{
    { 0, 1, {PKT_PAYLOAD}, "[null]" },
};

static ctp_flags_jump_t udp_use_payload_jump[] =
{
    { 1, 1, {PKT_PAYLOAD_LEN}, "[$UDP_USE_PAYLOAD$] > [#0]" },
    { 0, 1, {UDP_SOURCE_PORT}, "[null]" },
};

static ctp_flags_nextp_t udp_dest_port_nextp[] =
{
    { 1, 1, {DHCP}, "[$UDP_DEST_PORT$] == [#43]" },
    { 1, 1, {DHCP}, "[$UDP_DEST_PORT$] == [#44]" },
    { 1, 1, {DHCP}, "[$UDP_DEST_PORT$] == [#222]" },
    { 1, 1, {DHCP}, "[$UDP_DEST_PORT$] == [#223]" },
    { 1, 1, {L2TP}, "[$UDP_DEST_PORT$] == [#6a5]" },
    { 1, 1, {RIP}, "[$UDP_DEST_PORT$] == [#208]" },
    { 1, 1, {LDP}, "[$UDP_DEST_PORT$] == [#286]" },
    { 1, 1, {RADIUS}, "[$UDP_DEST_PORT$] == [#714]" },
    { 1, 1, {RADIUS}, "[$UDP_DEST_PORT$] == [#715]" },
    { 1, 1, {BFD}, "[$UDP_DEST_PORT$] == [#EC8]" },
    { 1, 1, {BFD}, "[$UDP_DEST_PORT$] == [#12B0]" },
    { 1, 1, {CAPWAP}, "[$UDP_DEST_PORT$] == [#147E]" },
    { 1, 1, {CAPWAP}, "[$UDP_DEST_PORT$] == [#147F]" },
    { 1, 1, {CAPWAP}, "[$UDP_SOURCE_PORT$] == [#147E]" },
    { 1, 1, {CAPWAP}, "[$UDP_SOURCE_PORT$] == [#147F]" },
    { 1, 1, {PTP}, "[$UDP_DEST_PORT$] == [#13F]" },
    { 1, 1, {PTP}, "[$UDP_DEST_PORT$] == [#140]" },
    { 1, 1, {VXLAN}, "[$UDP_VXLANHEADER$] == [#1]" },
    { 1, 1, {CAPWAP}, "[$UDP_CAPWAPHEADER$] == [#1]" },
    { 1, 1, {OWAMP}, "[$UDP_OTWAMPHEADER$] == [#1]" },
    { 1, 1, {TWAMP}, "[$UDP_OTWAMPHEADER$] == [#2]" },
    { 0, 1, {PKT_PAYLOAD}, "[null]" },
};

static ctp_flags_nextp_t vxlan_reserved2_nextp[] =
{
    { 1, 1, {L2H}, "[$VXLAN_PAYLOADMODE$] == [#1]" },
    { 1, 1, {IPV4}, "[$VXLAN_PAYLOADMODE$] == [#2]" },
    { 0, 1, {IPV6}, "[$VXLAN_PAYLOADMODE$] == [#3]" },
};

static ctp_flags_jump_t vxlan_reserved2_jump[] =
{
    { 1, 1, {VXLAN_UDT}, "[$VXLAN_PAYLOADMODE$] == [#1]" },
    { 1, 1, {VXLAN_UDT}, "[$VXLAN_PAYLOADMODE$] == [#2]" },
    { 0, 1, {VXLAN_UDT}, "[$VXLAN_PAYLOADMODE$] == [#3]" },
};

static ctp_flags_repeat_t vxlan_payloadlen_repeat[] =
{
    { 0, 1, {VXLAN_UDT}, "[$VXLAN_PAYLOADLEN$]" },
};

static ctp_flags_jump_t vxlan_payload_jump[] =
{
    { 1, 9, {VXLAN_PAYLOAD}, "[@REPEAT@] > [#0]" },
    { 0, 1, {VXLAN_UDT}, "[null]" },
};

static ctp_flags_repeat_t owamp_payload_len_repeat[] =
{
    { 0, 1, {OWAMP_UDT}, "[$OWAMP_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t owamp_payload_len_jump[] =
{
    { 0, 1, {OWAMP_UDT}, "[$OWAMP_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t owamp_payload_data_jump[] =
{
    { 1, 9, {OWAMP_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {OWAMP_UDT}, "[null]" },
};

static ctp_flags_repeat_t twamp_payload_len_repeat[] =
{
    { 0, 1, {TWAMP_UDT}, "[$TWAMP_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t twamp_payload_len_jump[] =
{
    { 0, 1, {TWAMP_UDT}, "[$TWAMP_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t twamp_payload_data_jump[] =
{
    { 1, 9, {TWAMP_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {TWAMP_UDT}, "[null]" },
};

static ctp_flags_jump_t rip_extend_jump[] =
{
    { 0, 1, {UDP_UDT}, "[$UDP_LENGTH$ - #C]  < [#1]" },
};

static ctp_flags_repeat_t rip_entry_num_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$RIP_ENTRY_NUM$]" },
};

static ctp_flags_jump_t rip_metric_jump[] =
{
    { 1, 9, {RIP_ADDR_FAMILY_ID}, "[@REPEAT@] > [#0]" },
    { 0, 1, {UDP_UDT}, "[null]" },
};

static ctp_flags_jump_t rip_addr_family_id_jump[] =
{
    { 1, 1, {RIP_ADDR_FAMILY_TYPEFF_AUT_TYPE}, "[$RIP_ADDR_FAMILY_ID$]  == [#FFFF]" },
    { 0, 1, {RIP_ROUT_TAG}, "[null]" },
};

static ctp_flags_jump_t rip_addr_family_typeff_aut_type_jump[] =
{
    { 1, 1, {RIP_MD5_DIG_OFFSET}, "[$RIP_ADDR_FAMILY_TYPEFF_AUT_TYPE$]  == [#3]" },
    { 0, 1, {RIP_ADDR_FAMILY_TYPEFF_AUT_INFO}, "[null]" },
};

static ctp_flags_jump_t rip_addr_family_typeff_aut_info_jump[] =
{
    { 1, 9, {RIP_ADDR_FAMILY_ID}, "[@REPEAT@] > [#0]" },
    { 0, 1, {UDP_UDT}, "[null]" },
};

static ctp_flags_jump_t rip_md5_zero_padd_jump[] =
{
    { 1, 9, {RIP_ADDR_FAMILY_ID}, "[@REPEAT@] > [#0]" },
    { 0, 1, {UDP_UDT}, "[null]" },
};

static ctp_flags_jump_t ospf_au_data_jump[] =
{
    { 1, 1, {OSPF_HELLO_MASK}, "[$OSPF_TYPE$] == [#1]" },
    { 1, 1, {OSPF_DBDES_INT_MTU}, "[$OSPF_TYPE$] == [#2]" },
    { 1, 1, {OSPF_LSR_LS_TYPE}, "[$OSPF_TYPE$] == [#3]" },
    { 1, 1, {OSPF_LSA}, "[$OSPF_TYPE$] == [#5]" },
    { 0, 1, {OSPF_UDT}, "[null]" },
};

static ctp_flags_exist_t ospf_hello_neig_exist[] =
{
    { 1, 1, {OSPF_UDT}, "[#0]  == [#1]" },
    { 0, 1, {OSPF_UDT}, "[#0]  == [#1]" },
};

static ctp_flags_jump_t ospf_hello_neig_jump[] =
{
    { 0, 1, {OSPF_HELLO_NEIG}, "[null]" },
};

static ctp_flags_exist_t ospf_dbdes_lsa_exist[] =
{
    { 1, 1, {OSPF_UDT}, "[#0]  == [#1]" },
    { 0, 1, {OSPF_UDT}, "[#0]  == [#1]" },
};

static ctp_flags_jump_t ospf_dbdes_lsa_jump[] =
{
    { 0, 1, {OSPF_DBDES_LSA}, "[null]" },
};

static ctp_flags_exist_t ospf_lsr_ls_type_exist[] =
{
    { 1, 1, {OSPF_UDT}, "[#0]  == [#1]" },
    { 0, 1, {OSPF_UDT}, "[#0]  == [#1]" },
};

static ctp_flags_jump_t ospf_lsr_adv_rout_jump[] =
{
    { 0, 1, {OSPF_LSR_LS_TYPE}, "[null]" },
};

static ctp_flags_jump_t l2tp_ver_jump[] =
{
    { 0, 1, {L2TP_TUNL_ID}, "[$L2TP_L$] == [#0]" },
};

static ctp_flags_jump_t l2tp_sess_id_jump[] =
{
    { 0, 1, {L2TP_OFF_SZ}, "[$L2TP_S$] == [#0]" },
};

static ctp_flags_jump_t l2tp_nr_jump[] =
{
    { 0, 1, {L2TP_AVP_M}, "[$L2TP_O$] == [#0]" },
};

static ctp_flags_repeat_t l2tp_off_sz_repeat[] =
{
    { 0, 1, {L2TP_UDT}, "[$L2TP_OFF_SZ$]" },
};

static ctp_flags_jump_t l2tp_off_pad_jump[] =
{
    { 1, 9, {L2TP_OFF_PAD}, "[@REPEAT@] > [#1]" },
    { 0, 1, {L2TP_AVP_M}, "[null]" },
};

static ctp_flags_exist_t l2tp_avp_m_exist[] =
{
    { 1, 1, {L2TP_UDT}, "[#1] == [#0]" },
    { 0, 1, {L2TP_UDT}, "[#1] == [#0]" },
};

static ctp_flags_len_t l2tp_avp_length_len[] =
{
    { 0, 1, {L2TP_AVP_VALUE}, "[$L2TP_AVP_LENGTH$ - #6  * #8]" },
};

static ctp_flags_jump_t l2tp_avp_value_jump[] =
{
    { 0, 1, {L2TP_AVP_M}, "[null]" },
};

static ctp_flags_exist_t dhcp_opt_type_exist[] =
{
    { 0, 1, {DHCP_OPT_PAYLOAD_LEN}, "[null]" },
};

static ctp_flags_jump_t dhcp_opt_type_jump[] =
{
    { 1, 1, {DHCP_OPT_REQ_IP_ADDR_LEN}, "[$DHCP_OPT_TYPE$] == [#32]" },
    { 1, 1, {DHCP_OPT_MESSAGE_TYPE_LEN}, "[$DHCP_OPT_TYPE$] == [#35]" },
    { 1, 1, {DHCP_OPT_PARA_REQ_LEN}, "[$DHCP_OPT_TYPE$] == [#37]" },
    { 1, 1, {DHCP_OPT_AGE_INFO_LEN}, "[$DHCP_OPT_TYPE$] == [#52]" },
    { 1, 1, {DHCP_OPT_TIME_LEN}, "[$DHCP_OPT_TYPE$] == [#33]" },
    { 0, 1, {DHCP_OPT_TIME_LEN}, "[$DHCP_OPT_TYPE$] == [#3A]" },
};

static ctp_flags_jump_t dhcp_opt_req_ip_addr_jump[] =
{
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_jump_t dhcp_opt_message_type_jump[] =
{
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_repeat_t dhcp_opt_para_req_len_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$DHCP_OPT_PARA_REQ_LEN$]" },
};

static ctp_flags_jump_t dhcp_opt_para_req_data_jump[] =
{
    { 1, 9, {DHCP_OPT_PARA_REQ_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_jump_t dhcp_opt_age_info_len_jump[] =
{
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_repeat_t dhcp_opt_age_info_sub_seq1_len_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$DHCP_OPT_AGE_INFO_SUB_SEQ1_LEN$]" },
};

static ctp_flags_jump_t dhcp_opt_age_info_sub_seq1_data_jump[] =
{
    { 1, 9, {DHCP_OPT_AGE_INFO_SUB_SEQ1_DATA}, "[@REPEAT@] > [#0]" },
    { 1, 1, {DHCP_OPT_AGE_INFO_SUB_SEQ2}, "[$DHCP_OPT_AGE_INFO_LEN$ - $DHCP_OPT_AGE_INFO_SUB_SEQ1_LEN$] > [#2]" },
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_repeat_t dhcp_opt_age_info_sub_seq2_len_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$DHCP_OPT_AGE_INFO_SUB_SEQ2_LEN$]" },
};

static ctp_flags_jump_t dhcp_opt_age_info_sub_seq2_data_jump[] =
{
    { 1, 9, {DHCP_OPT_AGE_INFO_SUB_SEQ2_DATA}, "[@REPEAT@] > [#0]" },
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_jump_t dhcp_opt_time_data_jump[] =
{
    { 0, 1, {DHCP_OPT_TYPE}, "[null]" },
};

static ctp_flags_repeat_t dhcp_opt_payload_len_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$DHCP_OPT_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t dhcp_opt_payload_len_jump[] =
{
    { 0, 1, {UDP_UDT}, "[$DHCP_OPT_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t dhcp_opt_payload_jump[] =
{
    { 1, 9, {DHCP_OPT_PAYLOAD}, "[@REPEAT@] > [#1]" },
    { 0, 1, {UDP_UDT}, "[null]" },
};

static ctp_flags_repeat_t radius_attri_len_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$RADIUS_ATTRI_LEN$]" },
};

static ctp_flags_jump_t radius_attri_len_jump[] =
{
    { 0, 1, {UDP_UDT}, "[$RADIUS_ATTRI_LEN$] < [#3]" },
};

static ctp_flags_jump_t radius_attri_data_jump[] =
{
    { 1, 9, {RADIUS_ATTRI_DATA}, "[@REPEAT@] > [#3]" },
    { 0, 1, {RADIUS_ATTRI_NUM}, "[null]" },
};

static ctp_flags_jump_t radius_attri_num_jump[] =
{
    { 1, 1, {RADIUS_ATTRI_TYPE}, "[$RADIUS_ATTRI_NUM$] > [#0]" },
    { 0, 1, {UDP_UDT}, "[null]" },
};

static ctp_flags_nextp_t bfd_echoval_nextp[] =
{
    { 0, 1, {PKT_PAYLOAD}, "[null]" },
};

static ctp_flags_jump_t ptp_log_msg_int_jump[] =
{
    { 1, 1, {PTP_AM_ORG_TMSTMP}, "[$PTP_MSG_TYPE$] == [#b]" },
    { 1, 1, {PTP_SDRM_ORG_TMSTMP}, "[$PTP_MSG_TYPE$] == [#0]" },
    { 1, 1, {PTP_SDRM_ORG_TMSTMP}, "[$PTP_MSG_TYPE$] == [#1]" },
    { 1, 1, {PTP_FUM_PRC_ORG_TMSTMP}, "[$PTP_MSG_TYPE$] == [#8]" },
    { 1, 1, {PTP_DRM_REC_TMSTMP}, "[$PTP_MSG_TYPE$] == [#9]" },
    { 1, 1, {PTP_PRQM_ORG_TMPSTMP}, "[$PTP_MSG_TYPE$] == [#2]" },
    { 1, 1, {PTP_PRSM_REQ_RCPT_TMSTMP}, "[$PTP_MSG_TYPE$] == [#3]" },
    { 1, 1, {PTP_PRFUM_RES_ORG_TMSTMP}, "[$PTP_MSG_TYPE$] == [#a]" },
    { 1, 1, {PTP_SM_TGT_PT_ID}, "[$PTP_MSG_TYPE$] == [#c]" },
    { 1, 1, {PTP_MM_TGT_PT_ID}, "[$PTP_MSG_TYPE$] == [#d]" },
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_am_time_src_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_sdrm_org_tmstmp_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_fum_prc_org_tmstmp_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_drm_req_pt_id_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_prqm_res_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_prsm_req_pt_id_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t ptp_prfum_req_pt_id_jump[] =
{
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_repeat_t ptp_sm_tlv_len_repeat[] =
{
    { 0, 1, {PTP_UDT}, "[$PTP_SM_TLV_LEN$]" },
};

static ctp_flags_jump_t ptp_sm_tlv_len_jump[] =
{
    { 0, 1, {PTP_UDT}, "[$PTP_SM_TLV_LEN$] == [#0]" },
};

static ctp_flags_jump_t ptp_sm_tlv_value_jump[] =
{
    { 1, 9, {PTP_SM_TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_repeat_t ptp_mm_tlv_len_repeat[] =
{
    { 0, 1, {PTP_UDT}, "[$PTP_MM_TLV_LEN$]" },
};

static ctp_flags_jump_t ptp_mm_tlv_len_jump[] =
{
    { 0, 1, {PTP_UDT}, "[$PTP_MM_TLV_LEN$] == [#0]" },
};

static ctp_flags_jump_t ptp_mm_tlv_value_jump[] =
{
    { 1, 9, {PTP_MM_TLV_VALUE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {PTP_UDT}, "[null]" },
};

static ctp_flags_jump_t capwap_pre_type_jump[] =
{
    { 1, 1, {CAPWAP_HLEN}, "[$CAPWAP_PRE_TYPE$] == [#0]" },
    { 0, 1, {DTLS_RES}, "[$CAPWAP_PRE_TYPE$] == [#1]" },
};

static ctp_flags_nextp_t capwap_wbid_nextp[] =
{
    { 0, 1, {WIRELESSDOT11}, "[$CAPWAP_WBID$] == [#1]" },
};

static ctp_flags_jump_t capwap_res_jump[] =
{
    { 1, 1, {CAPWAP_RAD_MAC_ADDR}, "[$CAPWAP_M$] == [#1]" },
    { 1, 1, {CAPWAP_WIR_SPEC_INFO}, "[$CAPWAP_W$] == [#1]" },
    { 0, 1, {CAPWAP_PAYLOAD_LEN}, "[$CAPWAP_W$ + $CAPWAP_M$] == [#0]" },
};

static ctp_flags_jump_t capwap_rad_mac_addr_jump[] =
{
    { 1, 1, {CAPWAP_WIR_SPEC_INFO}, "[$CAPWAP_W$] == [#1]" },
    { 0, 1, {CAPWAP_PAYLOAD_LEN}, "[$CAPWAP_W$] == [#0]" },
};

static ctp_flags_repeat_t capwap_payload_len_repeat[] =
{
    { 0, 1, {CAPWAP_ONLY_USE_PAYLOAD}, "[$CAPWAP_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t capwap_payload_len_jump[] =
{
    { 0, 1, {CAPWAP_ONLY_USE_PAYLOAD}, "[$CAPWAP_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t capwap_payload_data_jump[] =
{
    { 0, 9, {CAPWAP_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
};

static ctp_flags_jump_t capwap_only_use_payload_jump[] =
{
    { 1, 1, {CAPWAP_UDT}, "[$CAPWAP_ONLY_USE_PAYLOAD$] == [#1]" },
    { 0, 1, {CAPWAP_80211_PAYLOAD}, "[$CAPWAP_ONLY_USE_PAYLOAD$] == [#0]" },
};

static ctp_flags_nextp_t capwap_80211_payload_nextp[] =
{
    { 1, 1, {WIRELESSDOT11}, "[$CAPWAP_T$] == [#1]" },
    { 0, 1, {ETHERNET}, "[null]" },
};

static ctp_flags_repeat_t dtls_payload_len_repeat[] =
{
    { 0, 1, {UDP_UDT}, "[$DTLS_PAYLOAD_LEN$]" },
};

static ctp_flags_jump_t dtls_payload_len_jump[] =
{
    { 0, 1, {UDP_UDT}, "[$DTLS_PAYLOAD_LEN$] == [#0]" },
};

static ctp_flags_jump_t dtls_payload_data_jump[] =
{
    { 0, 9, {DTLS_PAYLOAD_DATA}, "[@REPEAT@] > [#1]" },
};

static ctp_flags_jump_t llc_vlan_exist_jump[] =
{
    { 1, 1, {LLC_VLAN_DSAP}, "[$LLC_VLAN_EXIST$] == [#1]" },
    { 0, 1, {LLC_DSAP}, "[$LLC_VLAN_EXIST$] == [#0]" },
};

static ctp_flags_exist_t llc_dsap_exist[] =
{
    { 1, 2, {LLC_UDT}, "[@PKT_LEN@] <= [@MTU@]" },
    { 0, 1, {LLC_UDT}, "[@EN_8022@] == [#1]" },
};

static ctp_flags_nextp_t llc_dsap_nextp[] =
{
    { 0, 1, {STP}, "[$LLC_DSAP$] == [#42]" },
};

static ctp_flags_exist_t llc_oui_exist[] =
{
    { 1, 2, {LLC_UDT}, "[$LLC_DSAP$] == [#AA]" },
    { 1, 2, {LLC_UDT}, "[$LLC_SSAP$] == [#AA]" },
    { 0, 1, {LLC_UDT}, "[$LLC_CTL0$] == [#3]" },
};

static ctp_flags_nextp_t llc_l2_type_nextp[] =
{
    { 1, 1, {IPV4}, "[$LLC_L2_TYPE$] == [#0800]" },
    { 0, 1, {IPV6}, "[$LLC_L2_TYPE$] == [#86DD]" },
};

static ctp_flags_nextp_t nsh_mandatory_4_nextp[] =
{
    { 1, 1, {IPV4}, "[$NSH_NEXT_PROTOCOL$] == [#1]" },
    { 0, 1, {L2H}, "[$NSH_NEXT_PROTOCOL$] == [#3]" },
};

static ctp_flags_jump_t igmp_type_jump[] =
{
    { 1, 1, {IGMPV3_REP_RESERVER1}, "[$IGMP_TYPE$] == [#22]" },
    { 1, 1, {IGMP_RESP_CODE}, "[$IGMP_TYPE$] == [#17]" },
    { 1, 1, {IGMP_RESP_CODE}, "[$IGMP_TYPE$] == [#16]" },
    { 1, 1, {IGMP_RESP_CODE}, "[$IGMP_TYPE$] == [#11]" },
    { 0, 1, {IGMP_UDT}, "[null]" },
};

static ctp_flags_jump_t igmp_version_jump[] =
{
    { 1, 1, {IGMPV3_QUE_RESV}, "[$IGMP_VERSION$] == [#3]" },
    { 0, 1, {IGMP_UDT}, "[null]" },
};

static ctp_flags_repeat_t igmpv3_que_num_of_src_repeat[] =
{
    { 0, 1, {IGMP_UDT}, "[$IGMPV3_QUE_NUM_OF_SRC$]" },
};

static ctp_flags_jump_t igmpv3_que_num_of_src_jump[] =
{
    { 1, 1, {IGMP_UDT}, "[$IGMPV3_QUE_NUM_OF_SRC$]  == [#0]" },
    { 0, 1, {IGMPV3_QUE_SRC_ADDR}, "[null]" },
};

static ctp_flags_jump_t igmpv3_que_src_addr_jump[] =
{
    { 1, 9, {IGMPV3_QUE_SRC_ADDR}, "[@REPEAT@] > [#1]" },
    { 0, 1, {IGMP_UDT}, "[null]" },
};

static ctp_flags_jump_t igmpv3_rep_num_of_rec_jump[] =
{
    { 1, 1, {IGMP_UDT}, "[$IGMPV3_REP_NUM_OF_REC$]  == [#0]" },
    { 0, 1, {IGMPV3_REP_RECORD_TYPE}, "[null]" },
};

static ctp_flags_exist_t igmpv3_rep_record_type_exist[] =
{
    { 0, 1, {IGMP_UDT}, "[null]" },
};

static ctp_flags_repeat_t igmpv3_rep_num_of_src_repeat[] =
{
    { 0, 1, {IGMP_UDT}, "[$IGMPV3_REP_NUM_OF_SRC$]" },
};

static ctp_flags_jump_t igmpv3_rep_src_addr_jump[] =
{
    { 1, 9, {IGMPV3_REP_SRC_ADDR}, "[@REPEAT@] > [#1]" },
    { 0, 1, {IGMPV3_REP_AUX_DATA_LEN2}, "[null]" },
};

static ctp_flags_repeat_t igmpv3_rep_aux_data_len2_repeat[] =
{
    { 0, 1, {IGMP_UDT}, "[$IGMPV3_REP_AUX_DATA_LEN2$]" },
};

static ctp_flags_jump_t igmpv3_rep_aux_data_jump[] =
{
    { 1, 9, {IGMPV3_REP_AUX_DATA}, "[@REPEAT@] > [#1]" },
    { 0, 1, {IGMPV3_REP_RECORD_TYPE}, "[null]" },
};

static ctp_flags_jump_t icmp_checksum_jump[] =
{
    { 1, 1, {ICMP_PPM_POINTER}, "[$ICMP_TYPE$] == [#c]" },
    { 1, 1, {ICMP_ECHO_IDE}, "[$ICMP_TYPE$] == [#0]" },
    { 1, 1, {ICMP_ECHO_IDE}, "[$ICMP_TYPE$] == [#8]" },
    { 0, 1, {ICMP_UDT}, "[null]" },
};

static ctp_flags_repeat_t icmp_echo_data_rep_repeat[] =
{
    { 0, 1, {ICMP_UDT}, "[$ICMP_ECHO_DATA_REP$]" },
};

static ctp_flags_jump_t icmp_echo_data_jump[] =
{
    { 1, 9, {ICMP_ECHO_DATA}, "[@REPEAT@] > [#0]" },
    { 0, 1, {ICMP_UDT}, "[null]" },
};

static ctp_flags_jump_t icmpv6_checksum_jump[] =
{
    { 1, 1, {ICMPV6_ECHO_IDE}, "[$ICMPV6_TYPE$] == [#80]" },
    { 1, 1, {ICMPV6_ECHO_IDE}, "[$ICMPV6_TYPE$] == [#81]" },
    { 1, 1, {ICMPV6_MLD_MAX_RES}, "[$ICMPV6_TYPE$] == [#82]" },
    { 1, 1, {ICMPV6_MLD_MAX_RES}, "[$ICMPV6_TYPE$] == [#83]" },
    { 1, 1, {ICMPV6_MLD_MAX_RES}, "[$ICMPV6_TYPE$] == [#84]" },
    { 1, 1, {ICMPV6_RS_RESERVED}, "[$ICMPV6_TYPE$] == [#85]" },
    { 1, 1, {ICMPV6_RA_CHL}, "[$ICMPV6_TYPE$] == [#86]" },
    { 1, 1, {ICMPV6_NS_RESERVED}, "[$ICMPV6_TYPE$] == [#87]" },
    { 1, 1, {ICMPV6_NA_R}, "[$ICMPV6_TYPE$] == [#88]" },
    { 1, 1, {ICMPV6_RM_RESERVED }, "[$ICMPV6_TYPE$] == [#89]" },
    { 0, 1, {ICMPV6_UDT}, "[null]" },
};

static ctp_flags_repeat_t icmpv6_echo_data_rep_repeat[] =
{
    { 0, 1, {ICMPV6_UDT}, "[$ICMPV6_ECHO_DATA_REP$]" },
};

static ctp_flags_jump_t icmpv6_echo_data_jump[] =
{
    { 1, 9, {ICMPV6_ECHO_DATA}, "[@REPEAT@] > [#0]" },
    { 0, 1, {ICMPV6_UDT}, "[null]" },
};

static ctp_flags_jump_t icmpv6_rs_reserved_jump[] =
{
    { 0, 1, {ICMPV6_OPT_NUM}, "[null]" },
};

static ctp_flags_jump_t icmpv6_ra_ret_time_jump[] =
{
    { 0, 1, {ICMPV6_OPT_NUM}, "[null]" },
};

static ctp_flags_jump_t icmpv6_ns_tar_addr_jump[] =
{
    { 0, 1, {ICMPV6_OPT_NUM}, "[null]" },
};

static ctp_flags_jump_t icmpv6_na_tar_addr_jump[] =
{
    { 0, 1, {ICMPV6_OPT_NUM}, "[null]" },
};

static ctp_flags_jump_t icmpv6_rm_des_addr_jump[] =
{
    { 0, 1, {ICMPV6_OPT_NUM}, "[null]" },
};

static ctp_flags_repeat_t icmpv6_opt_num_repeat[] =
{
    { 0, 1, {ICMPV6_UDT}, "[$ICMPV6_OPT_NUM$]" },
};

static ctp_flags_jump_t icmpv6_opt_num_jump[] =
{
    { 0, 1, {ICMPV6_UDT}, "[$ICMPV6_OPT_NUM$] == [#0]" },
};

static ctp_flags_jump_t icmpv6_opt_len_jump[] =
{
    { 1, 1, {ICMPV6_OPT_LINK_LAY_ADDR}, "[$ICMPV6_OPT_TYPE$] == [#1]" },
    { 1, 1, {ICMPV6_OPT_LINK_LAY_ADDR}, "[$ICMPV6_OPT_TYPE$] == [#2]" },
    { 1, 1, {ICMPV6_OPT_PRE_LEN}, "[$ICMPV6_OPT_TYPE$] == [#3]" },
    { 1, 1, {ICMPV6_OPT_MTU_RESERVED}, "[$ICMPV6_OPT_TYPE$] == [#5]" },
    { 0, 1, {ICMPV6_UDT}, "[null]" },
};

static ctp_flags_jump_t icmpv6_opt_link_lay_addr_jump[] =
{
    { 1, 9, {ICMPV6_OPT_TYPE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {ICMPV6_UDT}, "[null]" },
};

static ctp_flags_jump_t icmpv6_opt_pre_prefix_jump[] =
{
    { 1, 9, {ICMPV6_OPT_TYPE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {ICMPV6_UDT}, "[null]" },
};

static ctp_flags_jump_t icmpv6_opt_mtu_mtu_jump[] =
{
    { 1, 9, {ICMPV6_OPT_TYPE}, "[@REPEAT@] > [#1]" },
    { 0, 1, {ICMPV6_UDT}, "[null]" },
};

static ctp_flags_exist_t stp_r_ver_1_length_exist[] =
{
    { 0, 1, {STP_UDT}, "[null]" },
};

static ctp_flags_exist_t stp_r_m_ver_3_length_exist[] =
{
    { 0, 1, {STP_UDT}, "[null]" },
};

static ctp_flags_jump_t stp_r_m_msti_cfg_num_jump[] =
{
    { 0, 1, {STP_UDT}, "[$STP_R_M_MSTI_CFG_NUM$] == [#0]" },
};

static ctp_flags_repeat_t stp_r_m_msti_cfg_num_repeat[] =
{
    { 0, 1, {STP_UDT}, "[$STP_R_M_MSTI_CFG_NUM$]" },
};

static ctp_flags_jump_t stp_r_m_msti_remain_hop_jump[] =
{
    { 0, 9, {STP_R_M_MSTI_FLG}, "[@REPEAT@] > [#1]" },
};

/*
 * This array is use for header fields ID and String inter-search
*/
static ctp_field_info_t field_info[] =
{
    {"CPUPKT_EXIST", 0, 0x6, NULL, cpupkt_exist_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_MACDA", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_MACSA", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_VLANEXIST", 0, 0x6, NULL, cpupkt_vlanexist_jump, NULL, NULL, NULL, "1"},
    {"CPUPKT_TPID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "8100"},
    {"CPUPKT_VLAN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "23"},
    {"CPUPKT_TYPE", 16, 0x2, NULL, cpupkt_type_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_RES", 16, 0x6, NULL, cpupkt_res_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_HEADER_TYPE", 2, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_DESTID_DISCARD", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PACKET_OFFSET1", 7, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_MCAST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_DEST_CHIPID", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_DESTID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PRIORITY", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PACKET_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SOURCE_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_QUEUE_SELECT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_HEADER_HASH1", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_VPLS_PORT_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_UNTAG_PACKET", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SOURCE_PORT", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_COLOR", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_NEXT_HOP_PTR1", 18, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_LEN_ADJUST_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_CRITICAL_PACKET", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PACKET_LEN", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_TTL_PTPOFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_HEADER_CRC", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRCPORT_ISID", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PBB_SRCPORT_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PACKET_OFFSET2", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SOURCE_CFI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_NEXT_HOP_EXT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_NEXT_HOP_PTR2", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SVLAN_TPID_INDEX", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_DENY_REPLACE_COS", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_DENY_REPLACE_DSCP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_SVLANID_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_CVLANID_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_CVLAN_ID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_VLAN_PTR", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_VRFID_DESTPORT", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_L4_SRCPORT", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRCPORT_BUNDLEID", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_SRC_TARGED", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_USE_OUTER_VRFID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_PFM", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_OAM_TUNNEL_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_MIRROR", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_OPERATION_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_HEADER_HASH2", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_HM_IP_SA", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRCPORT_15_14", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_PACKET_OFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_MCAST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_DEST_CHIPID", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_DESTID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_PRIORITY", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_PACKET_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SOURCE_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_QUEUE_SELECT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_HEADER_HASH2_0", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_LOGIC_PORT_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_CTAG_OFFSET_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRCPORT", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_COLOR", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_NEXT_HOP_PTR", 18, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_LEN_ADJUST_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_CRITICAL_PACKET", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_RXTXFCL_22_17", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_DM_TS_36_29", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_TTL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_FROM_FABRIC", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_BYPASS_INGRESS_EDITING", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRCPORT_EXT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_LOOPBACK_DISCARD", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_HEADER_CRC", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRCPORT_ISID", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_PBB_SRCPORT_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_VLAN_TAG_OPERATION_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SOURCE_CFI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_NEXT_HOP_EXT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_NON_CRC", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_FROM_CPU_OR_OAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SVLAN_TPID_INDEX", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_STAG_ACTION", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_SVLANID_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_CVLANID_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_CVLAN_ID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_LM_DOWN_DISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_INGRESS_EDIT_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_OUTER_VLAN_IS_CVLAN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_SRC_VLAN_PTR", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_FID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_LOGIC_SRCPORT", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_RXTX_FCL3", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_CUT_THROUGH", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_RXTX_FCL2_1", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_MUX_LENGTH_TYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_OAM_TUNNEL_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_RXTX_FCL0", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_OPERATION_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_HEADER_HASH7_3", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GB_IP_SA", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PACKET_OFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_MCAST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DEST_CHIPID", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DESTID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PRIORITY", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PACKET_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SOURCE_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_HEADER_HASH", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LOGIC_PORT_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRCPORT", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRC_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_COLOR", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_NEXT_HOP_PTR", 18, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LEN_ADJUST_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_CRITICAL_PACKET", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_IS_LEAF", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SOURCE_CFI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_NEXT_HOP_EXT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PORT_MACSA_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_FROMCPULMUPDISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SVLAN_TAG_OPERATION_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SOURCEPORTEXTENDER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_FROM_FABRIC", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_BYPASS_INGRESS_EDITING", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DSNEXTHOP_BYPASSALL", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_FROMCPULMDOWNDISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_INGRESS_EDIT_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OUTER_VLAN_IS_CVLAN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_FROM_CPU_OR_OAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SVLAN_TPID_INDEX", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRC_VLAN_PTR", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_MUX_LENGTH_TYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_TUNNEL_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_BRIDGE_OP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_MAC_LEARNING_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DEBUG_SESSION_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_KEEP_OLD_HEADER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_STAG_ACTION", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_MAC_KNOWN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_USE_FID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_FID", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LOGIC_SRCPORT", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_TTL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OPERATION_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTPTS_RXTXFCL", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_TS_0_37_HIGH", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_TS_0_37_LOW", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRC_DSCP", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_U1_MODE", 0, 0x6, NULL, cpupkt_gg_u1_mode_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_L3_OFFSET", 7, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ECMPHASH", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ACLDSCPVALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ISIPV4", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ACLDSCP", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ECNAWARE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ECNEN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PBBCHECKDISCARD", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_CTAGACTION", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRC_CTAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRC_CTAG_CFI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRC_CTAG_OFFSET_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PBB_SRCPORT_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRCPORT_ISOLATEID", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_SRCCVLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_ISIDVALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_CTAG_OP_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_CONGESTION_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_NORMAL_ISELOOPPKT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_METADATA_VALID", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_METADATA", 14, 0x6, NULL, cpupkt_gg_metadata_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_RXOAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_MEPINDEX", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_OAM_TYPE", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_LINKOAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_LOCALPHYPORT", 9, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_GAL_EXIST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_ENTROPY_LABEL_EXIST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_ISUP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_DMEN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_LM_RECEICED_PKT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_RXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_RXTXFCL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_OAM_ISELOOPPKT", 1, 0x6, NULL, cpupkt_gg_oam_iselooppkt_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DM_RESERVE", 65, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DM_OFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_DM_ISELOOPPKT", 1, 0x6, NULL, cpupkt_gg_dm_iselooppkt_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LM_PKTTYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LM_TXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LM_RXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_LM_RXTXFCL", 8, 0x6, NULL, cpupkt_gg_lm_rxtxfcl_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_RESERVE", 54, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_SEQID", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_DEEP_PARSER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_IS_L2PTP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_IS_L4PTP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_CAPTURE_TS", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_REPLACE_TS", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_APPLY_EGRESS_ASY_DELAY", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_UP_RES_TIME", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_OP_MODE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_OFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_PTP_ISELOOPPKT", 1, 0x6, NULL, cpupkt_gg_ptp_iselooppkt_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_C2C_RESERVE", 57, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_C2C_SGMAC_STRIP_HEADER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_C2C_C2C_CHECK_DISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_C2C_ISELOOPPKT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_GG_C2C_STACKING_SRCPORT", 14, 0x6, NULL, cpupkt_gg_c2c_stacking_srcport_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PACKET_OFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_MCAST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SPANPKT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_TOCPU", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DEST_CHIPID", 7, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DESTID", 9, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_TERMINATE_SGT_HDR", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PRIORITY", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PACKET_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SOURCE_STAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_HEADER_HASH", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LOGIC_PORT_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRCPORT", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_SVLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_COLOR", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_NEXT_HOP_PTR", 18, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_FROM_LAG", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_CRITICAL_PACKET", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_IS_LEAF", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SOURCE_CFI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_NEXT_HOP_EXT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PORT_MACSA_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_FROMCPULMUPDISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SVLAN_TAG_OPERATION_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SOURCEPORTEXTENDER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_FROM_FABRIC", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_BYPASS_INGRESS_EDITING", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DSNEXTHOP_BYPASSALL", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_FROMCPULMDOWNDISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_INGRESS_EDIT_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OUTER_VLAN_IS_CVLAN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_FROM_CPU_OR_OAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SVLAN_TPID_INDEX", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_VLAN_PTR", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_MUX_LENGTH_TYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_TUNNEL_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_BRIDGE_OP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_MAC_LEARNING_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DEBUG_SESSION_EN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ILOOP_FROM_CPU", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_STAG_ACTION", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_MAC_KNOWN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_USE_FID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_FID", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LOGIC_SRCPORT", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_TTL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OPERATION_TYPE", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_MODE", 0, 0x6, NULL, cpupkt_dt2_u3_mode_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_OAM_RXTXFCL", 24, 0x6, NULL, cpupkt_dt2_u3_oam_rxtxfcl_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_OTHER_I2E_SRC_CID_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_OTHER_I2E_SRC_CID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_OTHER_PKT_WITH_CID_HDR", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_OTHER_SRC_CID_HDR_OFFSET_TYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U3_OTHER_TRUNCATE_LEN_PROFILE", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LOCAL_PHY_PORT", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_TS_0_36_HIGH", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_DSCP", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_U1_MODE", 0, 0x6, NULL, cpupkt_dt2_u1_mode_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_NORMAL_RES", 9, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_IS_SPAN_PKT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ACL_STAG_COS_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ACL_CTAG_COS_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ACL_STAG_CFI_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ACL_CTAG_CFI_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DOT1AE_ENCRYPTED", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ECMPHASH", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ACL_DSCP_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_DSCP_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_CNACTION", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PBBCHECKDISCARD", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_CTAGACTION", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_CTAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_CTAG_CFI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRC_CTAG_OFFSET_TYPE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRCPORT_ISOLATEID", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_SRCCVLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_ISIDVALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_CTAG_OP_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_MICRO_BURST_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_METADATA_TYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_METADATA", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_NO_DOT1AE_ENCRYPT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_NEED_DOT1AE_ENCRYPT", 1, 0x6, NULL, cpupkt_dt2_need_dot1ae_encrypt_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_RES", 9, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_RXOAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_MEPINDEX", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_OAM_TYPE", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_LINKOAM", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_ILOOPTOAGGUPMEP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_GAL_EXIST", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_USE_OAM_TTL", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_ISUP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_DMEN", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_LM_RECEICED_PKT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_RXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_OAM_RXTXFCL", 8, 0x6, NULL, cpupkt_dt2_oam_rxtxfcl_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DM_RESERVE", 66, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_DM_OFFSET", 8, 0x6, NULL, cpupkt_dt2_dm_offset_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LM_PKTTYPE", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LM_TXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LM_RXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_LM_RXTXFCL", 8, 0x6, NULL, cpupkt_dt2_lm_rxtxfcl_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_RESERVE", 28, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_TS_62_37", 25, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_TS_0_4", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_DEEP_PARSER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_IS_L2PTP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_IS_L4PTP", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_REPLACE_TS", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_APPLY_EGRESS_ASY_DELAY", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_UP_RES_TIME", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_OFFSET", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_ACL_DSCP_VALID", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_PTP_SRC_DSCP_VALID", 1, 0x6, NULL, cpupkt_dt2_ptp_src_dscp_valid_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_C2C_RESERVE", 55, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_C2C_NEIGHBOR_DISCOVER_PACKET", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_C2C_SGMAC_STRIP_HEADER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_C2C_C2C_CHECK_DISABLE", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CPUPKT_DT2_C2C_STACKING_SRCPORT", 16, 0x6, NULL, cpupkt_dt2_c2c_stacking_srcport_jump, NULL, NULL, NULL, "0"},
    {"CPUPKT_PKT_TYPE", 0, 0xc, NULL, NULL, cpupkt_pkt_type_nextp, NULL, NULL, "0"},
    {"CPUPKT_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_FRAME_CTL", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_DURA_BIT_15", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_DURA_BIT_14", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_DURA_BIT_0_TO_13", 14, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_MAX_MAC_NUM", 0, 0x14, NULL, NULL, NULL, dot11_max_mac_num_repeat, NULL, "0"},
    {"DOT11_MAC_ADDR", 48, 0x2, NULL, dot11_mac_addr_jump, NULL, NULL, NULL, "0"},
    {"DOT11_SEQ_CTL_EXIST", 0, 0x2, NULL, dot11_seq_ctl_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_SEQ_CTL", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_MAC_ADDR4_EXIST", 0, 0x6, NULL, dot11_mac_addr4_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_MAC_ADDR4", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_QOS_CTL_EXIST", 0, 0x6, NULL, dot11_qos_ctl_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_QOS_CTL", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_HT_CTL_EXIST", 0, 0x6, NULL, dot11_ht_ctl_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_HT_CTL", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_PAYLOAD_EXIST", 0, 0x6, NULL, dot11_payload_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_LLC_EXIST", 0, 0x6, NULL, dot11_llc_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_LLC_CHARACTER", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_LLC_VLAN_EXIST", 0, 0x6, NULL, dot11_llc_vlan_exist_jump, NULL, NULL, NULL, "0"},
    {"DOT11_LLC_VLAN_TAG", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DOT11_LLC_TYPE", 16, 0xc, NULL, NULL, dot11_llc_type_nextp, NULL, NULL, "0"},
    {"DOT11_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_MAC_DA", 48, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_MAC_SA", 48, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_MAX_VNUM", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_TPID", 16, 0x1, eth_tpid_exist, NULL, NULL, NULL, NULL, "0"},
    {"ETH_COS", 3, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_DEI", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_VLAN_ID", 12, 0x6, NULL, eth_vlan_id_jump, NULL, NULL, NULL, "1"},
    {"ETH_LEN_TYPE", 16, 0xc, NULL, NULL, eth_len_type_nextp, NULL, NULL, "0"},
    {"ETH_DSAP", 8, 0x9, eth_dsap_exist, NULL, eth_dsap_nextp, NULL, NULL, "0"},
    {"ETH_SSAP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_CTL0", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETH_CTL1", 8, 0x1, eth_ctl1_exist, NULL, NULL, NULL, NULL, "0"},
    {"ETH_OUI", 24, 0x1, eth_oui_exist, NULL, NULL, NULL, NULL, "0"},
    {"ETH_L2_TYPE", 16, 0x8, NULL, NULL, eth_l2_type_nextp, NULL, NULL, "0"},
    {"ETH_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PKT_PAYLOAD_LEN", 0, 0x16, NULL, pkt_payload_len_jump, NULL, pkt_payload_len_repeat, NULL, "0"},
    {"PKT_PAYLOAD_DATA", 8, 0x2, NULL, pkt_payload_data_jump, NULL, NULL, NULL, "0"},
    {"PKT_PAYLOAD_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_TCI_VER", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_TCI_ES", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_TCI_SC", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_TCI_SCB", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_TCI_E", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_TCI_C", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_AN", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_SL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_PN", 32, 0x6, NULL, macsec_pn_jump, NULL, NULL, NULL, "0"},
    {"MACSEC_SCI", 64, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MACSEC_PAYLOAD", 0, 0xc, NULL, NULL, macsec_payload_nextp, NULL, NULL, "0"},
    {"MACSEC_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SGTCMD_VER", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SGTCMD_LENGTH", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SGTCMD_SGT_LEN", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SGTCMD_SGT_OPT_TYPE", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SGTCMD_SGT_VALUE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SGTCMD_NEXT_TYPE", 16, 0xc, NULL, NULL, sgtcmd_next_type_nextp, NULL, NULL, "0"},
    {"SGTCMD_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_VER", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_FLAG_O", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_FLAG_C", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_FLAG_RES", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_LEN", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_MD_TYPE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_NEXT_PROTOCOL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_SERVICE_PATH_ID", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_SERVICE_INDEX", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_MANDATORY_1", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_MANDATORY_2", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_MANDATORY_3", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"NSH_MANDATORY_4", 32, 0xc, NULL, NULL, nsh_mandatory_4_nextp, NULL, NULL, "0"},
    {"NSH_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_SUBTYPE", 8, 0x6, NULL, slpr_subtype_jump, NULL, NULL, NULL, "3"},
    {"SLPR_EFM_FLAGS", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_EFM_CODE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_EFM_PLD_LEN", 0, 0x10, NULL, NULL, NULL, slpr_efm_pld_len_repeat, NULL, "0"},
    {"SLPR_EFM_PLD_DATA", 8, 0x6, NULL, slpr_efm_pld_data_jump, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_VER", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"SLPR_LACP_ACT_INFO", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"SLPR_LACP_ACT_INFO_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "14"},
    {"SLPR_LACP_ACT_SYS_PRI", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_ACT_SYS", 48, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_ACT_KEY", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_ACT_PORT_PRI", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_ACT_PORT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_ACT_STATE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_RES0", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_PART_INFO", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"SLPR_LACP_PART_INFO_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "14"},
    {"SLPR_LACP_PART_SYS_PRI", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_PART_SYS", 48, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_PART_KEY", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_PART_PORT_PRI", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_PART_PORT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_PART_STATE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_RES1", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_COLL_INFO", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "3"},
    {"SLPR_LACP_COLL_INFO_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "10"},
    {"SLPR_LACP_COLL_MAX_DE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_RES2", 96, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_TERM", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_TERM_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_LACP_RES3", 400, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"SLPR_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_VER", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_RES", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_MUL", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_OL", 5, 0x14, NULL, NULL, NULL, trill_ol_repeat, NULL, "0"},
    {"TRILL_HC", 6, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_EG_RB_NAME", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_IN_RB_NAME", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TRILL_OP", 32, 0x3, trill_op_exist, trill_op_jump, NULL, NULL, NULL, "0"},
    {"TRILL_NEXTP", 0, 0x6, NULL, trill_nextp_jump, NULL, NULL, NULL, "1"},
    {"TRILL_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CHAN_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CHAN_PROTOTOL", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CHAN_FLAGS", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CHAN_ERR", 4, 0xc, NULL, NULL, chan_err_nextp, NULL, NULL, "0"},
    {"CHAN_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LABEL", 20, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LAB_EXP", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LAB_S", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LAB_TTL", 8, 0x6, NULL, lab_ttl_jump, NULL, NULL, NULL, "64"},
    {"LAB_NEXTP", 0, 0xc, NULL, NULL, lab_nextp_nextp, NULL, NULL, "5"},
    {"LAB_PAYLOAD_LEN", 0, 0x16, NULL, lab_payload_len_jump, NULL, lab_payload_len_repeat, NULL, "0"},
    {"LAB_PAYLOAD_DATA", 8, 0x2, NULL, lab_payload_data_jump, NULL, NULL, NULL, "0"},
    {"MPLS_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ACH_HEADER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"ACH_VERSION", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ACH_RESERVE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ACH_CHANNELTYPE", 16, 0x8, NULL, NULL, ach_channeltype_nextp, NULL, NULL, "0"},
    {"ACH_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_VERSION", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"MPLSLM_FLAGS", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_CTL_CODE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_MSG_LENG", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_DFLAGS", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_OTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_RES", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_SESS_ID", 26, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_DS", 6, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_ORI_TMSTMP", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_CNT1", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_CNT2", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_CNT3", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_CNT4", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLM_TLV_TYPE", 8, 0x3, mplslm_tlv_type_exist, mplslm_tlv_type_jump, NULL, NULL, NULL, "0"},
    {"MPLSLM_TLV_LEN", 8, 0x14, NULL, NULL, NULL, mplslm_tlv_len_repeat, NULL, "0"},
    {"MPLSLM_TLV_VALUE", 8, 0x6, NULL, mplslm_tlv_value_jump, NULL, NULL, NULL, "0"},
    {"MPLSLM_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_VERSION", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"MPLSDM_FLAGS", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_CTL_CODE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_MSG_LENG", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_QTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_RTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_RPTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_RES", 20, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_SESS_ID", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_DS", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_TMSTMP1", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_TMSTMP2", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_TMSTMP3", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_TMSTMP4", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSDM_TLV_TYPE", 8, 0x3, mplsdm_tlv_type_exist, mplsdm_tlv_type_jump, NULL, NULL, NULL, "0"},
    {"MPLSDM_TLV_LEN", 8, 0x14, NULL, NULL, NULL, mplsdm_tlv_len_repeat, NULL, "0"},
    {"MPLSDM_TLV_VALUE", 8, 0x6, NULL, mplsdm_tlv_value_jump, NULL, NULL, NULL, "0"},
    {"MPLSDM_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_VERSION", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"MPLSLDM_FLAGS", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_CTL_CODE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_MSG_LENG", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_DFLAGS", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_QTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_RTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_RPTF", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_RES", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_SESS_ID", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_DS", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_TMSTMP1", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_TMSTMP2", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_TMSTMP3", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_TMSTMP4", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_CNT1", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_CNT2", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_CNT3", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_CNT4", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MPLSLDM_TLV_TYPE", 8, 0x3, mplsldm_tlv_type_exist, mplsldm_tlv_type_jump, NULL, NULL, NULL, "0"},
    {"MPLSLDM_TLV_LEN", 8, 0x14, NULL, NULL, NULL, mplsldm_tlv_len_repeat, NULL, "0"},
    {"MPLSLDM_TLV_VALUE", 8, 0x6, NULL, mplsldm_tlv_value_jump, NULL, NULL, NULL, "0"},
    {"MPLSLDM_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CSF_VERSION", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CSF_RESERVE1", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CSF_FLAG", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CSF_RESERVE2", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CSF_TLV_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CSF_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CW_HEADER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CW_FLAGS", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CW_FRAG", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CW_LENGTH", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CW_SEQUENCE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CW_NEXTP", 0, 0xc, NULL, NULL, cw_nextp_nextp, NULL, NULL, "0"},
    {"CW_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2H_MACDA", 48, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2H_MACSA", 48, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2H_MAXVNUM", 0, 0x14, NULL, NULL, NULL, l2h_maxvnum_repeat, NULL, "0"},
    {"L2H_TYPE_ID", 16, 0x1, l2h_type_id_exist, NULL, NULL, NULL, NULL, "0"},
    {"L2H_VLAN_COS", 3, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2H_VLAN_DEI", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2H_VLAN_ID", 12, 0x6, NULL, l2h_vlan_id_jump, NULL, NULL, NULL, "1"},
    {"L2H_LEN_TYPE", 16, 0x8, NULL, NULL, l2h_len_type_nextp, NULL, NULL, "0"},
    {"L2H_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_MD_LEVEL", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_VER", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_OP_CODE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"CFM_FLAGS", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_FIRST_TLV_OFFSET", 8, 0x6, NULL, cfm_first_tlv_offset_jump, NULL, NULL, NULL, "4"},
    {"CFM_PAYLOAD_LEN", 0, 0x16, NULL, cfm_payload_len_jump, NULL, cfm_payload_len_repeat, NULL, "0"},
    {"CFM_PAYLOAD_DATA", 8, 0x2, NULL, cfm_payload_data_jump, NULL, NULL, NULL, "0"},
    {"CFM_CCM_SEQ_NUM", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_CCM_MAIN_ASS_END_POINT_ID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "58"},
    {"CFM_CCM_MAIN_DOMAIN_NAME_FORMAT", 8, 0x6, NULL, cfm_ccm_main_domain_name_format_jump, NULL, NULL, NULL, "4"},
    {"CFM_CCM_MAIN_DOMAIN_NAME_LEN", 8, 0x14, NULL, NULL, NULL, cfm_ccm_main_domain_name_len_repeat, NULL, "8"},
    {"CFM_CCM_MAIN_DOMAIN_NAME_STR", 8, 0x6, NULL, cfm_ccm_main_domain_name_str_jump, NULL, NULL, NULL, "63"},
    {"CFM_CCM_SHORT_MA_NAME_FORMAT", 8, 0x6, NULL, cfm_ccm_short_ma_name_format_jump, NULL, NULL, NULL, "2"},
    {"CFM_CCM_SHORT_MA_NAME_LEN", 8, 0x16, NULL, cfm_ccm_short_ma_name_len_jump, NULL, cfm_ccm_short_ma_name_len_repeat, NULL, "3"},
    {"CFM_CCM_SHORT_MA_NAME_STR", 8, 0x2, NULL, cfm_ccm_short_ma_name_str_jump, NULL, NULL, NULL, "63"},
    {"CFM_CCM_MAID_ZERO_PAD", 8, 0x3, cfm_ccm_maid_zero_pad_exist, cfm_ccm_maid_zero_pad_jump, NULL, NULL, NULL, "0"},
    {"CFM_CCM_TXFCF", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_CCM_RXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_CCM_TXFCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_CCM_RESERVED", 32, 0x6, NULL, cfm_ccm_reserved_jump, NULL, NULL, NULL, "0"},
    {"CFM_LB_TP", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_LB_ID", 32, 0x6, NULL, cfm_lb_id_jump, NULL, NULL, NULL, "1"},
    {"CFM_LT_ID", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"CFM_LT_TTL", 8, 0x6, NULL, cfm_lt_ttl_jump, NULL, NULL, NULL, "3e"},
    {"CFM_LTM_ORI_MAC", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_LTM_TAR_MAC", 48, 0x6, NULL, cfm_ltm_tar_mac_jump, NULL, NULL, NULL, "0"},
    {"CFM_LTR_RL_ACT", 8, 0x6, NULL, cfm_ltr_rl_act_jump, NULL, NULL, NULL, "1"},
    {"CFM_LM_TXFCF", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_LM_RXFCF", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_LM_TXFCB", 32, 0x6, NULL, cfm_lm_txfcb_jump, NULL, NULL, NULL, "0"},
    {"CFM_APS_LEN", 0, 0x14, NULL, NULL, NULL, cfm_aps_len_repeat, NULL, "0"},
    {"CFM_APS_DATA", 8, 0x2, NULL, cfm_aps_data_jump, NULL, NULL, NULL, "8"},
    {"CFM_DM_TX_STPF", 64, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_DM_RX_STPF", 64, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_DM_TX_STPB", 64, 0x1, cfm_dm_tx_stpb_exist, NULL, NULL, NULL, NULL, "0"},
    {"CFM_DM_RX_STPB", 64, 0x2, NULL, cfm_dm_rx_stpb_jump, NULL, NULL, NULL, "0"},
    {"CFM_SLM_SRC_MEPID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_SLM_RESP_MEPID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_SLM_TESTID", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_SLM_TXFCF", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_SLM_TXFCB", 32, 0x6, NULL, cfm_slm_txfcb_jump, NULL, NULL, NULL, "0"},
    {"CFM_TLV_TYPE", 8, 0x6, NULL, cfm_tlv_type_jump, NULL, NULL, NULL, "0"},
    {"CFM_TLV_LEN", 16, 0x14, NULL, NULL, NULL, cfm_tlv_len_repeat, NULL, "0"},
    {"CFM_TLV_VALUE", 8, 0x6, NULL, cfm_tlv_value_jump, NULL, NULL, NULL, "0"},
    {"CFM_TP_LBM_TLV_TYPE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "21"},
    {"CFM_TP_LBM_TLV_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "19"},
    {"CFM_TP_LBM_TLV_SUBTYPE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"CFM_TP_LBM_TLV_MEPID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_TP_LBM_TLV_ZERO", 184, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CFM_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_PCP", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_DEI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_NCA", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_RES1", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_RES2", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_ISID", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_CMAC_DA", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_ITAG_CMAC_SA", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_STAG_TPID", 16, 0x1, pbb_stag_tpid_exist, NULL, NULL, NULL, NULL, "0"},
    {"PBB_STAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_STAG_DEI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_STAG_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_CTAG_TPID", 16, 0x1, pbb_ctag_tpid_exist, NULL, NULL, NULL, NULL, "0"},
    {"PBB_CTAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_CTAG_DEI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_CTAG_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PBB_LEN_TYPE", 16, 0x8, NULL, NULL, pbb_len_type_nextp, NULL, NULL, "0"},
    {"PBB_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_PCP", 3, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_DEI", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_ING_E_CID", 12, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_RES1", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_E_CID", 14, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_RES2", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_STAG_TPID", 16, 0x1, etag_stag_tpid_exist, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_STAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_STAG_DEI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_STAG_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_CTAG_TPID", 16, 0x1, etag_ctag_tpid_exist, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_CTAG_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_CTAG_DEI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_CTAG_VLANID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ETAG_LEN_TYPE", 16, 0x8, NULL, NULL, etag_len_type_nextp, NULL, NULL, "0"},
    {"ETAG_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FIP_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FIP_RES1", 12, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FIP_PAYLOAD_LEN", 0, 0x16, NULL, fip_payload_len_jump, NULL, fip_payload_len_repeat, NULL, "0"},
    {"FIP_PAYLOAD_DATA", 32, 0x2, NULL, fip_payload_data_jump, NULL, NULL, NULL, "0"},
    {"FIP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_RES1", 100, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_SOF", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_R_CTL", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_D_ID", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_CS_CTL", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_S_ID", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_F_CTL", 24, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_SEQ_ID", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_DF_CTL", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_SEQ_CNT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_CX_ID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_RX_ID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_PARA", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_PAYLOAD_LEN", 0, 0x16, NULL, fcoe_payload_len_jump, NULL, fcoe_payload_len_repeat, NULL, "0"},
    {"FCOE_PAYLOAD_DATA", 32, 0x2, NULL, fcoe_payload_data_jump, NULL, NULL, NULL, "0"},
    {"FCOE_EOF", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_RES2", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"FCOE_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MEL", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VER", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OPCODE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"FLAGS", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TLV_OFF", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "46"},
    {"SEQ_NUM", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MEP_ID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"MEG_ID", 384, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TX_FCF", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RX_FCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TX_FCB", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RESERVED", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"END_TLV", 8, 0x6, NULL, end_tlv_jump, NULL, NULL, NULL, "0"},
    {"TLV_TYPE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TLV_LEN", 16, 0x14, NULL, NULL, NULL, tlv_len_repeat, NULL, "1"},
    {"TLV_VALUE", 8, 0x6, NULL, tlv_value_jump, NULL, NULL, NULL, "0"},
    {"CCM_PAYLOAD", 8, 0x3, ccm_payload_exist, ccm_payload_jump, NULL, NULL, NULL, "0"},
    {"CCM_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_CBIT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_RBIT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_KBIT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_SBIT", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_RES0", 9, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_VER", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_PROT", 16, 0xe, NULL, gre_prot_jump, gre_prot_nextp, NULL, NULL, "0"},
    {"GRE_CHECKSUM", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_RES1", 16, 0x6, NULL, gre_res1_jump, NULL, NULL, NULL, "0"},
    {"GRE_KEY", 32, 0x6, NULL, gre_key_jump, NULL, NULL, NULL, "0"},
    {"GRE_SEQ", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"GRE_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"EAPOL_VER", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"EAPOL_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"EAPOL_LEN", 16, 0x2, NULL, eapol_len_jump, NULL, NULL, NULL, "0"},
    {"EAP_PACKET_CODE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"EAP_PACKET_ID", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"EAP_PACKET_LEN", 16, 0x12, NULL, eap_packet_len_jump, NULL, eap_packet_len_repeat, NULL, "0"},
    {"EAP_PACKET_TYPE", 8, 0x2, NULL, eap_packet_type_jump, NULL, NULL, NULL, "0"},
    {"EAP_PACKET_DATA", 8, 0x2, NULL, eap_packet_data_jump, NULL, NULL, NULL, "0"},
    {"EAP_MD5_CHALL_VALUE_SIZE", 8, 0x14, NULL, NULL, NULL, eap_md5_chall_value_size_repeat, NULL, "10"},
    {"EAP_MD5_CHALL_VALUE", 8, 0x2, NULL, eap_md5_chall_value_jump, NULL, NULL, NULL, "0"},
    {"EAP_MD5_CHALL_NAME_LEN", 0, 0x10, NULL, NULL, NULL, eap_md5_chall_name_len_repeat, NULL, "0"},
    {"EAP_MD5_CHALL_NAME", 8, 0x6, NULL, eap_md5_chall_name_jump, NULL, NULL, NULL, "0"},
    {"EAPOL_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "4"},
    {"IPV4_IHL", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "5"},
    {"IPV4_TOS", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_ID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_FLG", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_FRAGO", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_TTL", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "ff"},
    {"IPV4_PAYLOAD_FORCE", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_PROTO", 8, 0xc, NULL, NULL, ipv4_proto_nextp, NULL, NULL, "ff"},
    {"IPV4_CHKSM", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_SA", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_DA", 32, 0x2, NULL, ipv4_da_jump, NULL, NULL, NULL, "0"},
    {"IPV4_OPT", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV4_PAYLOAD_LEN", 0, 0x16, NULL, ipv4_payload_len_jump, NULL, ipv4_payload_len_repeat, NULL, "0"},
    {"IPV4_PAYLOAD", 8, 0x2, NULL, ipv4_payload_jump, NULL, NULL, NULL, "0"},
    {"IPV4_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "6"},
    {"IPV6_TRA_CLASS", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FLOW_LBL", 20, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"IPV6_PAYL_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_NEXT_HDR", 8, 0xc, NULL, NULL, ipv6_next_hdr_nextp, NULL, NULL, "3b"},
    {"IPV6_HOP_LIMIT", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "ff"},
    {"IPV6_SA", 128, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DA", 128, 0x2, NULL, ipv6_da_jump, NULL, NULL, NULL, "0"},
    {"IPV6_PAYLOAD_LEN", 0, 0x16, NULL, ipv6_payload_len_jump, NULL, ipv6_payload_len_repeat, NULL, "0"},
    {"IPV6_PAYLOAD", 8, 0x2, NULL, ipv6_payload_jump, NULL, NULL, NULL, "0"},
    {"IPV6_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_NEXT_HEADER", 8, 0xc, NULL, NULL, ipv6_hbh_opt_header_next_header_nextp, NULL, NULL, "3b"},
    {"IPV6_HBH_OPT_HEADER_EXT_LENGTHER", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_OPTION_TYPE", 8, 0x2, NULL, ipv6_hbh_opt_header_option_type_jump, NULL, NULL, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_JUM_PAYL_OPTION_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "4"},
    {"IPV6_HBH_OPT_HEADER_JUM_PAYL_LEN", 32, 0x2, NULL, ipv6_hbh_opt_header_jum_payl_len_jump, NULL, NULL, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_ROUT_ALT_OPTION_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"IPV6_HBH_OPT_HEADER_ROUT_ALT_VALUE", 8, 0x6, NULL, ipv6_hbh_opt_header_rout_alt_value_jump, NULL, NULL, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_PADN_OPTION_LEN", 8, 0x10, NULL, NULL, NULL, ipv6_hbh_opt_header_padn_option_len_repeat, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_PADN_OPTION_DATA", 8, 0x3, ipv6_hbh_opt_header_padn_option_data_exist, ipv6_hbh_opt_header_padn_option_data_jump, NULL, NULL, NULL, "0"},
    {"IPV6_HBH_OPT_HEADER_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_NEXT_HEADER", 8, 0xc, NULL, NULL, ipv6_dest_opt_header_next_header_nextp, NULL, NULL, "3b"},
    {"IPV6_DEST_OPT_HEADER_EXT_LENGTHER", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_OPTION_TYPE", 8, 0x2, NULL, ipv6_dest_opt_header_option_type_jump, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_FLAG", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_RES", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_PRE_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SEQ_NUM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_LIFETIME", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SUB_OPTION_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_dest_opt_header_bind_up_option_sub_option_num_repeat, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_UP_OPTION_SUB_OPTION", 32, 0x2, NULL, ipv6_dest_opt_header_bind_up_option_sub_option_jump, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_STATUS", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SEQ_NUM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_LIFETIME", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_REFRESH", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SUB_OPTION_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_dest_opt_header_bind_ack_option_sub_option_num_repeat, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_ACK_OPTION_SUB_OPTION", 32, 0x2, NULL, ipv6_dest_opt_header_bind_ack_option_sub_option_jump, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_SUB_OPTION_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_dest_opt_header_bind_req_option_sub_option_num_repeat, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_BIND_REQ_OPTION_SUB_OPTION", 32, 0x2, NULL, ipv6_dest_opt_header_bind_req_option_sub_option_jump, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_HOME_ADDR", 128, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_SUB_OPTION_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_dest_opt_header_home_addr_option_sub_option_num_repeat, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_HOME_ADDR_OPTION_SUB_OPTION", 32, 0x2, NULL, ipv6_dest_opt_header_home_addr_option_sub_option_jump, NULL, NULL, NULL, "0"},
    {"IPV6_DEST_OPT_HEADER_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_HEADER_NEXT_HEADER", 8, 0xc, NULL, NULL, ipv6_rout_header_next_header_nextp, NULL, NULL, "3b"},
    {"IPV6_ROUT_HEADER_EXT_LENGTHER", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_HEADER_ROUT_TYPE", 8, 0x2, NULL, ipv6_rout_header_rout_type_jump, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_HEADER_SEG_LEFT", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_HEADER_ROUT_TYPE_SPEC_DATA_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_rout_header_rout_type_spec_data_num_repeat, NULL, "0"},
    {"IPV6_ROUT_HEADER_ROUT_TYPE_SPEC_DATA", 8, 0x2, NULL, ipv6_rout_header_rout_type_spec_data_jump, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_TYPE0_HEADER_SEG_LEFT", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_TYPE0_HEADER_RES", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_TYPE0_HEADER_ADDR_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_rout_type0_header_addr_num_repeat, NULL, "0"},
    {"IPV6_ROUT_TYPE0_HEADER_ADDR", 128, 0x2, NULL, ipv6_rout_type0_header_addr_jump, NULL, NULL, NULL, "0"},
    {"IPV6_ROUT_HEADER_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FRAG_HEADER_NEXT_HEADER", 8, 0xc, NULL, NULL, ipv6_frag_header_next_header_nextp, NULL, NULL, "3b"},
    {"IPV6_FRAG_HEADER_RES_1", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FRAG_HEADER_FRAG_OFFSET", 13, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FRAG_HEADER_RES_2", 2, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FRAG_HEADER_MORE_FRAG_FLAG", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FRAG_HEADER_ID", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_FRAG_HEADER_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_NEXT_HEADER", 8, 0xc, NULL, NULL, ipv6_authen_header_next_header_nextp, NULL, NULL, "3b"},
    {"IPV6_AUTHEN_HEADER_PAYL_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_RES", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_SEUC_PARA_INDEX", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_SEQ_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_AUTHEN_DATA_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_authen_header_authen_data_num_repeat, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_AUTHEN_DATA", 8, 0x2, NULL, ipv6_authen_header_authen_data_jump, NULL, NULL, NULL, "0"},
    {"IPV6_AUTHEN_HEADER_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_SEUC_PARA_INDEX", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_SEQ_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_PAYL_DATA_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_encap_seuc_payl_header_payl_data_num_repeat, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_PAYL_DATA", 8, 0x2, NULL, ipv6_encap_seuc_payl_header_payl_data_jump, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_encap_seuc_payl_header_padding_num_repeat, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING", 8, 0x2, NULL, ipv6_encap_seuc_payl_header_padding_jump, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_PADDING_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_NEXT_HEADER", 8, 0xc, NULL, NULL, ipv6_encap_seuc_payl_header_next_header_nextp, NULL, NULL, "3b"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_AUTHEN_DATA_NUM", 0, 0x14, NULL, NULL, NULL, ipv6_encap_seuc_payl_header_authen_data_num_repeat, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_AUTHEN_DATA", 8, 0x2, NULL, ipv6_encap_seuc_payl_header_authen_data_jump, NULL, NULL, NULL, "0"},
    {"IPV6_ENCAP_SEUC_PAYL_HEADER_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VRRP_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"VRRP_TYPE", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"VRRP_VIR_RTR_ID", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VRRP_PRIORITY", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "64"},
    {"VRRP_COUNT_IP_ADDR", 8, 0x10, NULL, NULL, NULL, vrrp_count_ip_addr_repeat, NULL, "0"},
    {"VRRP_AUTH_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VRRP_ADVER_INT", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"VRRP_CHKSM", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VRRP_IP_ADDR", 32, 0x2, NULL, vrrp_ip_addr_jump, NULL, NULL, NULL, "0"},
    {"VRRP_AUTH_DATA1", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VRRP_AUTH_DATA2", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VRRP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_USE_PAYLOAD", 0, 0x6, NULL, tcp_use_payload_jump, NULL, NULL, NULL, "0"},
    {"TCP_SOURCE_PORT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_DEST_PORT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_SEQUENCE_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_ACKNOW_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_DATA_OFFSET", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_RESERVED", 6, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_URG", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_ACK", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_PSH", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_RST", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_SYN", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_FIN", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_WINDOW", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_CHECKSUM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_URGENT_POINT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TCP_OPTION", 32, 0x8, NULL, NULL, tcp_option_nextp, NULL, NULL, "0"},
    {"TCP_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_USE_PAYLOAD", 0, 0x6, NULL, udp_use_payload_jump, NULL, NULL, NULL, "0"},
    {"UDP_SOURCE_PORT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_VXLANHEADER", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_CAPWAPHEADER", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_OTWAMPHEADER", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_DEST_PORT", 16, 0x8, NULL, NULL, udp_dest_port_nextp, NULL, NULL, "0"},
    {"UDP_LENGTH", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_CHECKSUM", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"UDP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VXLAN_BITFLAGS", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "8"},
    {"VXLAN_RESERVED1", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VXLAN_VNIID", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"VXLAN_PAYLOADMODE", 0, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"VXLAN_RESERVED2", 8, 0xe, NULL, vxlan_reserved2_jump, vxlan_reserved2_nextp, NULL, NULL, "0"},
    {"VXLAN_PAYLOADLEN", 0, 0x14, NULL, NULL, NULL, vxlan_payloadlen_repeat, NULL, "0"},
    {"VXLAN_PAYLOAD", 8, 0x2, NULL, vxlan_payload_jump, NULL, NULL, NULL, "0"},
    {"VXLAN_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_VER", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_LEN", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_ROU_ID", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_ARE_ID", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_CHKSM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_AU_TYPE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_AU_DATA", 64, 0x2, NULL, ospf_au_data_jump, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_MASK", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_INT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_OPT", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_RTR_PRIO", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_ROUT_DEA_INT", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_DES_ROUT", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_BAK_DES_ROUT", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_HELLO_NEIG", 32, 0x3, ospf_hello_neig_exist, ospf_hello_neig_jump, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_INT_MTU", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_OPT", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_RES", 5, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_I", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_M", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_MS", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_MSG_SEQ", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_DBDES_LSA", 160, 0x3, ospf_dbdes_lsa_exist, ospf_dbdes_lsa_jump, NULL, NULL, NULL, "0"},
    {"OSPF_LSR_LS_TYPE", 32, 0x1, ospf_lsr_ls_type_exist, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_LSR_LS_ID", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_LSR_ADV_ROUT", 32, 0x2, NULL, ospf_lsr_adv_rout_jump, NULL, NULL, NULL, "0"},
    {"OSPF_LSA", 160, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OSPF_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_T", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_L", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_X1", 2, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_S", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_X2", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_O", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_P", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_X3", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_VER", 4, 0x2, NULL, l2tp_ver_jump, NULL, NULL, NULL, "0"},
    {"L2TP_LENGTH", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_TUNL_ID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_SESS_ID", 16, 0x2, NULL, l2tp_sess_id_jump, NULL, NULL, NULL, "0"},
    {"L2TP_NS", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_NR", 16, 0x2, NULL, l2tp_nr_jump, NULL, NULL, NULL, "0"},
    {"L2TP_OFF_SZ", 16, 0x10, NULL, NULL, NULL, l2tp_off_sz_repeat, NULL, "0"},
    {"L2TP_OFF_PAD", 8, 0x2, NULL, l2tp_off_pad_jump, NULL, NULL, NULL, "0"},
    {"L2TP_AVP_M", 1, 0x1, l2tp_avp_m_exist, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_AVP_H", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_AVP_RSVD", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_AVP_LENGTH", 10, 0x20, NULL, NULL, NULL, NULL, l2tp_avp_length_len, "0"},
    {"L2TP_AVP_VND_ID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_AVP_TYPE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"L2TP_AVP_VALUE", 8, 0x2, NULL, l2tp_avp_value_jump, NULL, NULL, NULL, "0"},
    {"L2TP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_OP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_HTYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_HLEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_HOPS", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_XID", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_SECS", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_FLAGS", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_CIADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_YIADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_SYIADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_GIADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_CHADDR", 128, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_SNAME", 512, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_FILE", 1024, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_MAGIC_COOK", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_TYPE", 8, 0x3, dhcp_opt_type_exist, dhcp_opt_type_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_REQ_IP_ADDR_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "4"},
    {"DHCP_OPT_REQ_IP_ADDR", 32, 0x2, NULL, dhcp_opt_req_ip_addr_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_MESSAGE_TYPE_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"DHCP_OPT_MESSAGE_TYPE", 8, 0x2, NULL, dhcp_opt_message_type_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_PARA_REQ_LEN", 8, 0x10, NULL, NULL, NULL, dhcp_opt_para_req_len_repeat, NULL, "0"},
    {"DHCP_OPT_PARA_REQ_DATA", 8, 0x2, NULL, dhcp_opt_para_req_data_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_AGE_INFO_LEN", 8, 0x6, NULL, dhcp_opt_age_info_len_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_AGE_INFO_SUB_SEQ1", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"DHCP_OPT_AGE_INFO_SUB_SEQ1_LEN", 8, 0x10, NULL, NULL, NULL, dhcp_opt_age_info_sub_seq1_len_repeat, NULL, "0"},
    {"DHCP_OPT_AGE_INFO_SUB_SEQ1_DATA", 8, 0x2, NULL, dhcp_opt_age_info_sub_seq1_data_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_AGE_INFO_SUB_SEQ2", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"DHCP_OPT_AGE_INFO_SUB_SEQ2_LEN", 8, 0x10, NULL, NULL, NULL, dhcp_opt_age_info_sub_seq2_len_repeat, NULL, "0"},
    {"DHCP_OPT_AGE_INFO_SUB_SEQ2_DATA", 8, 0x2, NULL, dhcp_opt_age_info_sub_seq2_data_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_TIME_LEN", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "4"},
    {"DHCP_OPT_TIME_DATA", 32, 0x2, NULL, dhcp_opt_time_data_jump, NULL, NULL, NULL, "0"},
    {"DHCP_OPT_PAYLOAD_LEN", 0, 0x16, NULL, dhcp_opt_payload_len_jump, NULL, dhcp_opt_payload_len_repeat, NULL, "0"},
    {"DHCP_OPT_PAYLOAD", 8, 0x2, NULL, dhcp_opt_payload_jump, NULL, NULL, NULL, "0"},
    {"DHCP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_CMD", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_VER", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_EXTEND", 16, 0x2, NULL, rip_extend_jump, NULL, NULL, NULL, "0"},
    {"RIP_ENTRY_NUM", 0, 0x14, NULL, NULL, NULL, rip_entry_num_repeat, NULL, "0"},
    {"RIP_ADDR_FAMILY_ID", 16, 0x2, NULL, rip_addr_family_id_jump, NULL, NULL, NULL, "0"},
    {"RIP_ROUT_TAG", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_IP_ADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_SUBNET_MASK", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_NEXT_HOP", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_METRIC", 32, 0x2, NULL, rip_metric_jump, NULL, NULL, NULL, "0"},
    {"RIP_ADDR_FAMILY_TYPEFF_AUT_TYPE", 16, 0x2, NULL, rip_addr_family_typeff_aut_type_jump, NULL, NULL, NULL, "0"},
    {"RIP_ADDR_FAMILY_TYPEFF_AUT_INFO", 128, 0x2, NULL, rip_addr_family_typeff_aut_info_jump, NULL, NULL, NULL, "0"},
    {"RIP_MD5_DIG_OFFSET", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_MD5_KEY_ID", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_MD5_AUTH_DATA_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_MD5_SEQ_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RIP_MD5_ZERO_PADD", 64, 0x6, NULL, rip_md5_zero_padd_jump, NULL, NULL, NULL, "0"},
    {"RIP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RADIUS_CODE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RADIUS_ID", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RADIUS_LEN", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RADIUS_AUTH", 128, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RADIUS_ATTRI_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"RADIUS_ATTRI_LEN", 8, 0x12, NULL, radius_attri_len_jump, NULL, radius_attri_len_repeat, NULL, "0"},
    {"RADIUS_ATTRI_DATA", 8, 0x2, NULL, radius_attri_data_jump, NULL, NULL, NULL, "0"},
    {"RADIUS_ATTRI_NUM", 0, 0x2, NULL, radius_attri_num_jump, NULL, NULL, NULL, "0"},
    {"RADIUS_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_VERSION", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"LDP_PDU_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_LSR_ID", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_LBL_SPC_ID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_MSG_TYPE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_MSG_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_MSG_ID", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_CMMN_TLV_TYPE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_CMMN_TLV_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_CMMN_TLV_HLD_TIME", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_CMMN_RESV", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_IPV4_TLV_TYPE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_IPV4_TLV_LEN", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_IPV4_TRANS_ADDR", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LDP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_VERSION", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"BFD_DIAG", 5, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_STATUS", 2, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"BFD_FLAGS", 6, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_MULTIPLE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "3"},
    {"BFD_LENGTH", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "18"},
    {"BFD_MYDISCRI", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_YOURDISCRI", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_MINTXVAL", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_MINRXVAL", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"BFD_ECHOVAL", 32, 0xc, NULL, NULL, bfd_echoval_nextp, NULL, NULL, "0"},
    {"BFD_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_TRANS_SPE", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MSG_TYPE", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_RES0", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"PTP_MSG_LEN", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_DM_NUM", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_RES1", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_FLAGS", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_CORR", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_RES2", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_SRC_PT_ID", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_SEQ_ID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_CTRL", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_LOG_MSG_INT", 8, 0x2, NULL, ptp_log_msg_int_jump, NULL, NULL, NULL, "0"},
    {"PTP_AM_ORG_TMSTMP", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_CURR_UTC_OFFSET", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_RES", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_GRANMST_P1", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_GRANMST_CLKQLT", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_GRANMST_P2", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_GRANMST_ID", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_STP_RM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_AM_TIME_SRC", 8, 0x2, NULL, ptp_am_time_src_jump, NULL, NULL, NULL, "0"},
    {"PTP_SDRM_ORG_TMSTMP", 80, 0x2, NULL, ptp_sdrm_org_tmstmp_jump, NULL, NULL, NULL, "0"},
    {"PTP_FUM_PRC_ORG_TMSTMP", 80, 0x2, NULL, ptp_fum_prc_org_tmstmp_jump, NULL, NULL, NULL, "0"},
    {"PTP_DRM_REC_TMSTMP", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_DRM_REQ_PT_ID", 80, 0x2, NULL, ptp_drm_req_pt_id_jump, NULL, NULL, NULL, "0"},
    {"PTP_PRQM_ORG_TMPSTMP", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_PRQM_RES", 80, 0x2, NULL, ptp_prqm_res_jump, NULL, NULL, NULL, "0"},
    {"PTP_PRSM_REQ_RCPT_TMSTMP", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_PRSM_REQ_PT_ID", 80, 0x2, NULL, ptp_prsm_req_pt_id_jump, NULL, NULL, NULL, "0"},
    {"PTP_PRFUM_RES_ORG_TMSTMP", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_PRFUM_REQ_PT_ID", 80, 0x2, NULL, ptp_prfum_req_pt_id_jump, NULL, NULL, NULL, "0"},
    {"PTP_SM_TGT_PT_ID", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_SM_TLV_TYPE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_SM_TLV_LEN", 16, 0x16, NULL, ptp_sm_tlv_len_jump, NULL, ptp_sm_tlv_len_repeat, NULL, "0"},
    {"PTP_SM_TLV_VALUE", 8, 0x6, NULL, ptp_sm_tlv_value_jump, NULL, NULL, NULL, "0"},
    {"PTP_MM_TGT_PT_ID", 80, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_START_BOUND_HOP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_BOUND_HOP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_RES0", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_ACTION", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_RES1", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_TLV_TYPE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"PTP_MM_TLV_LEN", 16, 0x16, NULL, ptp_mm_tlv_len_jump, NULL, ptp_mm_tlv_len_repeat, NULL, "0"},
    {"PTP_MM_TLV_VALUE", 8, 0x6, NULL, ptp_mm_tlv_value_jump, NULL, NULL, NULL, "0"},
    {"PTP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_PRE_VER", 4, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_PRE_TYPE", 4, 0x6, NULL, capwap_pre_type_jump, NULL, NULL, NULL, "0"},
    {"CAPWAP_HLEN", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_RID", 5, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_WBID", 5, 0xc, NULL, NULL, capwap_wbid_nextp, NULL, NULL, "0"},
    {"CAPWAP_T", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_F", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_L", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_W", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_M", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_K", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_FLAGS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_FRAGMENT_ID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_FRAG_OFFSET", 13, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_RES", 3, 0x6, NULL, capwap_res_jump, NULL, NULL, NULL, "0"},
    {"CAPWAP_RAD_MAC_ADDR", 64, 0x6, NULL, capwap_rad_mac_addr_jump, NULL, NULL, NULL, "0"},
    {"CAPWAP_WIR_SPEC_INFO", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"CAPWAP_PAYLOAD_LEN", 0, 0x16, NULL, capwap_payload_len_jump, NULL, capwap_payload_len_repeat, NULL, "0"},
    {"CAPWAP_PAYLOAD_DATA", 8, 0x2, NULL, capwap_payload_data_jump, NULL, NULL, NULL, "0"},
    {"CAPWAP_ONLY_USE_PAYLOAD", 0, 0x6, NULL, capwap_only_use_payload_jump, NULL, NULL, NULL, "0"},
    {"CAPWAP_80211_PAYLOAD", 0, 0xc, NULL, NULL, capwap_80211_payload_nextp, NULL, NULL, "0"},
    {"CAPWAP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_RES", 24, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_CONTENT_TYPE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_VER", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_EPOCH", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_SEQ", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_LENGTH", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"DTLS_PAYLOAD_LEN", 0, 0x16, NULL, dtls_payload_len_jump, NULL, dtls_payload_len_repeat, NULL, "0"},
    {"DTLS_PAYLOAD_DATA", 8, 0x2, NULL, dtls_payload_data_jump, NULL, NULL, NULL, "0"},
    {"DTLS_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_EXIST", 0, 0x2, NULL, llc_vlan_exist_jump, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_DSAP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_SSAP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_CTL0", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_CTL1", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_OUI", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_TPID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_COS", 3, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_DEI", 1, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_VLAN_ID", 12, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"LLC_DSAP", 8, 0x9, llc_dsap_exist, NULL, llc_dsap_nextp, NULL, NULL, "0"},
    {"LLC_SSAP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_CTL0", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_CTL1", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"LLC_OUI", 16, 0x1, llc_oui_exist, NULL, NULL, NULL, NULL, "0"},
    {"LLC_L2_TYPE", 16, 0x8, NULL, NULL, llc_l2_type_nextp, NULL, NULL, "0"},
    {"LLC_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_SEQ_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_TS", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_ERR_ESTIMATE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_MBZ", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_RX_TS", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_SENDER_SEQ_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_SENDER_TS", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_SENDER_ERR_ESTIMATE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_SENDER_MBZ", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_SENDER_TTL", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"TWAMP_PAYLOAD_LEN", 0, 0x16, NULL, twamp_payload_len_jump, NULL, twamp_payload_len_repeat, NULL, "0"},
    {"TWAMP_PAYLOAD_DATA", 8, 0x2, NULL, twamp_payload_data_jump, NULL, NULL, NULL, "0"},
    {"TWAMP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OWAMP_SEQ_NUM", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OWAMP_TS", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OWAMP_ERR_ESTIMATE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"OWAMP_PAYLOAD_LEN", 0, 0x16, NULL, owamp_payload_len_jump, NULL, owamp_payload_len_repeat, NULL, "0"},
    {"OWAMP_PAYLOAD_DATA", 8, 0x2, NULL, owamp_payload_data_jump, NULL, NULL, NULL, "0"},
    {"OWAMP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMP_TYPE", 8, 0x2, NULL, igmp_type_jump, NULL, NULL, NULL, "0"},
    {"IGMP_RESP_CODE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMP_CHKSM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMP_GROUP_ADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMP_VERSION", 0, 0x6, NULL, igmp_version_jump, NULL, NULL, NULL, "0"},
    {"IGMPV3_QUE_RESV", 4, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_QUE_SFLAG", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_QUE_QRV", 3, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_QUE_QQIC", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_QUE_NUM_OF_SRC", 16, 0x12, NULL, igmpv3_que_num_of_src_jump, NULL, igmpv3_que_num_of_src_repeat, NULL, "0"},
    {"IGMPV3_QUE_SRC_ADDR", 32, 0x2, NULL, igmpv3_que_src_addr_jump, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_RESERVER1", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_CHKSM", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_RESERVER2", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_NUM_OF_REC", 16, 0x2, NULL, igmpv3_rep_num_of_rec_jump, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_RECORD_TYPE", 8, 0x1, igmpv3_rep_record_type_exist, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_AUX_DATA_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_NUM_OF_SRC", 16, 0x10, NULL, NULL, NULL, igmpv3_rep_num_of_src_repeat, NULL, "0"},
    {"IGMPV3_REP_MUL_ADDR", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_SRC_ADDR", 32, 0x2, NULL, igmpv3_rep_src_addr_jump, NULL, NULL, NULL, "0"},
    {"IGMPV3_REP_AUX_DATA_LEN2", 0, 0x10, NULL, NULL, NULL, igmpv3_rep_aux_data_len2_repeat, NULL, "0"},
    {"IGMPV3_REP_AUX_DATA", 32, 0x2, NULL, igmpv3_rep_aux_data_jump, NULL, NULL, NULL, "0"},
    {"IGMP_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMP_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMP_CODE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMP_CHECKSUM", 16, 0x6, NULL, icmp_checksum_jump, NULL, NULL, NULL, "0"},
    {"ICMP_PPM_POINTER", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMP_ECHO_IDE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMP_ECHO_SEQ_NUM", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMP_ECHO_DATA_REP", 0, 0x14, NULL, NULL, NULL, icmp_echo_data_rep_repeat, NULL, "0"},
    {"ICMP_ECHO_DATA", 8, 0x6, NULL, icmp_echo_data_jump, NULL, NULL, NULL, "0"},
    {"ICMP_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_CODE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_CHECKSUM", 16, 0x6, NULL, icmpv6_checksum_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_ECHO_IDE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_ECHO_SEQ_NUM", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_ECHO_DATA_REP", 0, 0x10, NULL, NULL, NULL, icmpv6_echo_data_rep_repeat, NULL, "0"},
    {"ICMPV6_ECHO_DATA", 8, 0x6, NULL, icmpv6_echo_data_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_RS_RESERVED", 32, 0x6, NULL, icmpv6_rs_reserved_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_CHL", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_M", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_O", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_RESERVED", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_ROUT_LT", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_REA_TIME", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RA_RET_TIME", 32, 0x2, NULL, icmpv6_ra_ret_time_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_NS_RESERVED", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_NS_TAR_ADDR", 128, 0x2, NULL, icmpv6_ns_tar_addr_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_NA_R", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_NA_S", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_NA_O", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_NA_RESERVED", 29, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_NA_TAR_ADDR", 128, 0x2, NULL, icmpv6_na_tar_addr_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_RM_RESERVED", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RM_TAR_ADDR", 128, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_RM_DES_ADDR", 128, 0x2, NULL, icmpv6_rm_des_addr_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_NUM", 0, 0x12, NULL, icmpv6_opt_num_jump, NULL, icmpv6_opt_num_repeat, NULL, "0"},
    {"ICMPV6_OPT_TYPE", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_LEN", 8, 0x2, NULL, icmpv6_opt_len_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_LINK_LAY_ADDR", 48, 0x2, NULL, icmpv6_opt_link_lay_addr_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_LEN", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_L", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_A", 1, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_RESERVED1", 6, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_VLD_LT", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_PRE_LT", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_RESERVED2", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_PRE_PREFIX", 128, 0x2, NULL, icmpv6_opt_pre_prefix_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_MTU_RESERVED", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_OPT_MTU_MTU", 32, 0x2, NULL, icmpv6_opt_mtu_mtu_jump, NULL, NULL, NULL, "0"},
    {"ICMPV6_MLD_MAX_RES", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_MLD_RESERVE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_MLD_MULT_ADDR", 128, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ICMPV6_UDT", 0, 0x40, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ARP_HARD_TYPE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "1"},
    {"ARP_PROT_TYPE", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "800"},
    {"ARP_HARD_ADDR", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "6"},
    {"ARP_PROT_ADDR", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "4"},
    {"ARP_OPERATION", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ARP_SRC_MAC", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ARP_SRC_IP", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ARP_DEST_MAC", 48, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ARP_DEST_IP", 32, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"ARP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_PROT_ID", 16, 0x4, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_PROT_VER_ID", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "3"},
    {"STP_BPDU_TYPE", 8, 0x4, NULL, NULL, NULL, NULL, NULL, "2"},
    {"STP_BPDU_FLAGS", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_ROOT_ID", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_ROOT_PATH_COST", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_BRI_ID", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_PORT_ID", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_MSG_AGE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_MAX_AGE", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_HELLO_TIME", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_FWD_DELAY", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_VER_1_LENGTH", 8, 0x1, stp_r_ver_1_length_exist, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_VER_3_LENGTH", 16, 0x1, stp_r_m_ver_3_length_exist, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_CFG_ID_FMT_SEL", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_CFG_NAME", 256, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_CFG_REVISION", 16, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_CFG_DGST", 128, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_CIST_INTNL_RT_PTH_COST", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_CIST_BRIDGE_ID", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_REMAIN_HOP", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_MSTI_CFG_NUM", 0, 0x12, NULL, stp_r_m_msti_cfg_num_jump, NULL, stp_r_m_msti_cfg_num_repeat, NULL, "0"},
    {"STP_R_M_MSTI_FLG", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_MSTI_RGN_RT_ID", 64, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_MSTI_INTNL_RT_PTH_COST", 32, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_MSTI_BRIDGE_PRI", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_MSTI_PORT_PRI", 8, 0x0, NULL, NULL, NULL, NULL, NULL, "0"},
    {"STP_R_M_MSTI_REMAIN_HOP", 8, 0x6, NULL, stp_r_m_msti_remain_hop_jump, NULL, NULL, NULL, "0"},
    {"STP_UDT", 0, 0x44, NULL, NULL, NULL, NULL, NULL, "0"},
};

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/

/*******************************************************************************
 * Name   : get_header_type_string
 * Purpose: Get the pointer of stored header type string.
 * Input  : header type enum.
 * Output : None.
 * Return : stored header type string.
 * Note   : N/A
*******************************************************************************/
char *
ctc_pktxr_get_header_type_string(uint16 header_type)
{
    return header_type_str[header_type];
}

/*******************************************************************************
 * Name   : ctc_pktxr_get_header_start_field
 * Purpose: Get the first field of header type.
 * Input  : header type enum.
 * Output : None.
 * Return : stored field ID.
 * Note   : N/A
*******************************************************************************/
uint16 
ctc_pktxr_get_header_start_field(uint16 header_type)
{
    return header_start_field[header_type];
}

/*******************************************************************************
 * Name   : ctc_pktxr_get_field_info_pointer
 * Purpose: Get the pointer of stored.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
ctp_field_info_t *
ctc_pktxr_get_field_info_pointer(uint16 field_id)
{
    uint16 max_count = sizeof(field_info) / sizeof(ctp_field_info_t);

    if (field_id >= max_count) {
        assert(0);
        return NULL;
    }
    return &field_info[field_id];
}


/* return 0 for failed
 * return 1 for success
 */
ctp_field_info_t* ctc_pktxr_find_field_by_name(char *pszName)
{
   uint16 max_count = sizeof(field_info) / sizeof(ctp_field_info_t);
   int i;

   for (i = 0; i < max_count; i++) {
       if (!strcasecmp(field_info[i].descriptor, pszName)) {
           return &field_info[i];
       }
   }

   return NULL;
}

void ctc_pktxr_output_all_fields(int fd)
{
   uint16 max_count = sizeof(field_info) / sizeof(ctp_field_info_t);
   int i;

   for (i = 0; i < max_count; i++) {
       write(fd, field_info[i].descriptor, strlen(field_info[i].descriptor));
       write(fd, "\r\n", 2);
   }

   return;
}
