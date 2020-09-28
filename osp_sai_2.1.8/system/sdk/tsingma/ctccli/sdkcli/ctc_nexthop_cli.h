/**
 @file ctc_nexthop_cli.h

 @date 2009-11-13

 @version v2.0

*/

#ifndef _CTC_NH_CLI_H
#define _CTC_NH_CLI_H

#include "sal.h"
#include "ctc_cli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CTC_CLI_NH_FLEX_STR "Flex nexthop"
#define CTC_CLI_NH_BRGUC_STR  "Bridge unicast nexthop"
#define CTC_CLI_NH_BRGMC_STR  "Bridge multicast nexthop"
#define CTC_CLI_NH_IPMC_STR  "IP multicast nexthop"
#define CTC_CLI_NH_UPMEP_STR  "UP MEP nexthop"
#define CTC_CLI_NH_MPLS_STR  "MPLS nexthop"

#define CTC_CLI_NH_EGS_VLANTRANS_STR "Egress Vlan Editing nexthop"
#define CTC_CLI_NH_APS_EGS_VLANTRANS_STR "APS Egress Vlan Editing nexthop"
#define CTC_CLI_NH_IPUC_STR  "IP unicast nexthop"
#define CTC_CLI_NH_ILOOP_STR  "IPE Loopback nexthop"
#define CTC_CLI_NH_RSPAN_STR  "RSPAN nexthop"
#define CTC_CLI_NH_MCAST_STR  "Mcast nexthop"
#define CTC_CLI_NH_IP_TUNNEL_STR  "IP Tunnel nexthop"
#define CTC_CLI_NH_MISC_STR  "Misc nexthop"
#define CTC_CLI_NH_TRILL_STR  "TRILL nexthop"

#define CTC_CLI_NH_ADD_STR "Add nexthop"
#define CTC_CLI_NH_DEL_STR "Delete nexthop"
#define CTC_CLI_NH_UPDATE_STR "Update nexthop"

#define CTC_CLI_NH_APS_WORKING_PATH_DESC     "APS working path parameter"
#define CTC_CLI_NH_APS_PROTECTION_PATH_DESC  "APS protection path parameter"
#define CTC_CLI_NH_MPLS_TUNNEL_ID  "The ID of MPLS Tunnel,the max Tunnel id can be configed before initialize, default is 1023"
#define CTC_CLI_NH_MPLS_TUNNEL  "MPLS Tunnel"
#define CTC_CLI_NH_MAC_STR      "Mac address and vlan id nexthop"

#define CTC_CLI_L3IF_ALL_STR "(sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID (vlan-port|) | routed-port GPORT_ID | ecmp-if IFID |)"

#define CTC_CLI_L3IF_ALL_DESC \
    "Sub interface", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, "VLAN port/interface", CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, CTC_CLI_GPORT_DESC,  \
    CTC_CLI_GPORT_ID_DESC,"Vlan port", "Routed Port interface", CTC_CLI_GPORT_ID_DESC, CTC_CLI_ECMP_L3IF_ID_DESC, CTC_CLI_ECMP_L3IF_ID_RANGE_DESC

#define CTC_CLI_MAC_ARP_STR "(mac MAC | arp-id ARP_ID )"

#define CTC_CLI_MAC_ARP_DESC \
    CTC_CLI_MAC_DESC, CTC_CLI_MAC_FORMAT, "Using ARP to create nexthop,parameter needn't care MACDA and outgoing Interface", "ARP-ID,if ARP-ID equal to 0,indicate that it will not use ARP"

#define CTC_CLI_IPMC_L3IF_ALL_DESC \
    "Sub interface", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,\
    CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, \
    "VLAN interface", CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, \
    CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,"Vlan port" , \
    "Physical interface/remote chip id", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC
#define CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR "(sub-if (gport GPORT_ID |("CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|))) vlanid VLAN_ID | vlan-if vlanid VLAN_ID (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|))) (vlan-port|) | phy-if (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|))))"


#define CTC_CLI_NH_MISC_MAC_EDIT_STR "{swap-mac | replace-mac-da MAC | replace-mac-sa MAC}"
#define CTC_CLI_NH_EGS_VLAN_EDIT_COS "((swap-vlan (swap-tpid|) (swap-cos|)) | (replace-svlan-cos VALUE ) | (map-svlan-cos ))"

#define CTC_CLI_NH_MISC_MPLS_EDIT_STR "mpls-hdr (replace-mpls-label (label1 LABLE ttl1 TTL exp1 EXP exp1-type EXP_TYPE (exp1-domain DOMAIN |)(map-ttl1|) (is-mcast1|)|) \
(label2 LABLE ttl2 TTL exp2 EXP exp2-type EXP_TYPE (exp2-domain DOMAIN |)(map-ttl2|) (is-mcast2|)|) |)"
#define CTC_CLI_NH_MISC_MPLS_EDIT_DESC                              \
    "MPLS hdr",                                                     \
    "MPLS replace label",                                           \
    "MPLS label1",                                                  \
    "MPLS label1 valule",                                           \
    "MPLS label1's ttl",                                            \
    "MPLS label1's ttl value(0~255)",                               \
    "MPLS EXP1",                                                    \
    "MPLS EXP1 value(0-7)",                                              \
    "MPLS EXP1 type(0-2)",                                               \
    "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP1 domain",                                                                              \
    "MPLS EXP1 domain ID",                                                                           \
    "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label1 is mcast label",                                   \
    "MPLS label2",                                                  \
    "MPLS label2 valule",                                           \
    "MPLS label2's ttl",                                            \
    "MPLS label2's ttl value(0~255)",                               \
    "MPLS EXP2",                                                    \
    "MPLS EXP2 value(0-7)",                                         \
    "MPLS EXP2 type(0-2)",                                          \
    "MPLS EXP2 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP2 domain",                                                                              \
    "MPLS EXP2 domain ID",                                                                           \
    "MPLS label2's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label2 is mcast label"

#define CTC_CLI_NH_EGS_VLAN_EDIT_STR "cvlan-edit-type VALUE svlan-edit-type VALUE output-cvid VALUE \
        output-svid VALUE {("CTC_CLI_NH_EGS_VLAN_EDIT_COS")| svlan-tpid-index TPID_INDEX|cvlan-valid|svlan-valid|user-vlanptr VLAN_PTR|is-leaf|svlan-aware|pass-through|}"
#define CTC_CLI_NH_EGS_VLAN_EDIT_DESC \
    "CVLAN Editing Type",                     \
    "0 for no egress editing, keep ingress editing; 1 for keep unchange, "  \
    "will recover ingress editing although ingress editing have done; "     \
    "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan",            \
    "SVLAN Editing Type",                                                   \
    "0 for no egress editing, keep ingress editing; 1 for keep unchange, "  \
    "will recover ingress editing although ingress editing have done; "     \
    "2 for insert a vlan; 3 for replace vlan; 4 for strip vlan",            \
    "Output CVLANID",                                                       \
    "Vlan id value",                                                \
    "Output SVLANID",                                                       \
    "Vlan id value",                                                \
    "Enable swaping SVLAN with CVLAN, if enable vlan swapping, will not do any vlan edit", \
    "Enable swaping SVLAN's TPID with CVLAN's TPID",                        \
    "Enable swaping SVLAN's Cos with CVLAN's Cos",                          \
    "Replace SVLAN's Cos",                                                  \
    "0-7",                                                                  \
    "Map SVLAN's Cos",                                                      \
    "Set svlan tpid index",                                                 \
    "index <0-3>, the corresponding svlan tpid is in EpeL2TpidCtl",         \
    "Output CVLANID is valid",                                              \
    "Output SVLANID is valid",                                               \
    "User vlanptr",                                                         \
    CTC_CLI_USER_VLAN_PTR,                                                    \
    "The nexthop is leaf node in E-Tree networks",                           \
    "Svlan edit will depend on original svlan",\
    "Ignore dot1q and untag default vlan configuration on port, only for XLATE nexthop"

#define CTC_CLI_NH_FLEX_VLAN_EDIT_STR   \
        "vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|)|) \
        (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|)|) \
        {replace-stpid stag-tpid-index STPID_INDEX|user-vlanptr VLAN_PTR|}"

#define CTC_CLI_NH_FLEX_VLAN_EDIT_DESC   \
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
    "Replace svlan tpid enable or not",     \
    "Svlan tpid index",                     \
    "0-3",                                     \
    "User vlanptr",                                                         \
    CTC_CLI_USER_VLAN_PTR

#define CTC_CLI_NH_FLEX_VLAN_EDIT_SET(flex_edit)                                           \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-edit");                                             \
    if (INDEX_VALID(tmp_argi))                                                                  \
    {                                                                                        \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG;                               \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("stag-op");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("stag-op", flex_edit.stag_op, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("svid-sl");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("svid-sl", flex_edit.svid_sl, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-svid");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-svid", flex_edit.new_svid, argv[tmp_argi + 1]);      \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("scos-sl");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scos-sl", flex_edit.scos_sl, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-scos");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scos", flex_edit.new_scos, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ctag-op");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ctag-op", flex_edit.ctag_op, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("cvid-sl");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("cvid-sl", flex_edit.cvid_sl, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-cvid");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-cvid", flex_edit.new_cvid, argv[tmp_argi + 1]);      \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ccos-sl");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccos-sl", flex_edit.ccos_sl, argv[tmp_argi + 1]);         \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-ccos");                                              \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccos", flex_edit.new_ccos, argv[tmp_argi + 1]);         \
        }                                                                                       \
		tmp_argi = CTC_CLI_GET_ARGC_INDEX("stag-tpid-index");                                    \
        if (INDEX_VALID(tmp_argi))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("stag-tpid-index", flex_edit.new_stpid_idx, argv[tmp_argi + 1]);         \
			flex_edit.new_stpid_en = 1;                      \
        }                                                                                       \
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");           \
        if (INDEX_VALID(tmp_argi))                                                              \
        {                                                                                     \
            CTC_CLI_GET_UINT16_RANGE("user-vlanptr", flex_edit.user_vlanptr, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);  \
        }                                                                                    \
    }                                                                                        \

#define CTC_CLI_NH_FLEX_ARPHDR_STR   \
    "arp-hdr (replace-ht ARP_HT|)(replace-halen ARP_HLEN|)(replace-op ARP_OP|)(replace-pt ARP_PT|)(replace-palen ARP_PLEN|)(replace-sha ARP_SHA|)(replace-spa ARP_SPA|)(replace-tha ARP_THA|)(replace-tpa ARP_TPA|)"

#define CTC_CLI_NH_FLEX_ARPHDR_SET(flex_edit)                                           \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("arp-hdr");                                       \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR;                        \
        flex_edit.packet_type = CTC_MISC_NH_PACKET_TYPE_ARP;                            \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ht");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HT;                         \
        CTC_CLI_GET_UINT16("replace-ht", flex_edit.arp_ht, argv[tmp_argi + 1]);         \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-halen");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HLEN;                         \
        CTC_CLI_GET_UINT8("replace-halen", flex_edit.arp_halen, argv[tmp_argi + 1]);         \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-op");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_OP;                         \
        CTC_CLI_GET_UINT16("replace-op", flex_edit.arp_op, argv[tmp_argi + 1]);         \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-pt");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_PT;                         \
        CTC_CLI_GET_UINT16("replace-pt", flex_edit.arp_pt, argv[tmp_argi + 1]);         \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-palen");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_PLEN;                         \
        CTC_CLI_GET_UINT8("replace-palen", flex_edit.arp_palen, argv[tmp_argi + 1]);         \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-sha");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_SHA;                         \
        CTC_CLI_GET_MAC_ADDRESS("replace-sha", flex_edit.arp_sha, argv[tmp_argi + 1]);   \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-spa");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_SPA;                         \
        CTC_CLI_GET_UINT32("replace-spa", flex_edit.arp_spa, argv[tmp_argi + 1]);         \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-tha");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_THA;                         \
        CTC_CLI_GET_MAC_ADDRESS("replace-tha", flex_edit.arp_tha, argv[tmp_argi + 1]);   \
    }                                                                                   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-tpa");                                    \
    if (tmp_argi != 0xFF)                                                               \
    {                                                                                   \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_TPA;                         \
        CTC_CLI_GET_UINT32("replace-tpa", flex_edit.arp_tpa, argv[tmp_argi + 1]);         \
    }                                                                                   \

#define CTC_CLI_NH_FLEX_ARPHDR_DESC   \
    "Arp packet header edit",               \
    "Replace hardware type",                \
    "0-0xFFFF",                             \
    "Replace length of hardware address",   \
    "0-0xFF",                               \
    "Replace ARP/RARP operation",           \
    "0-0xFFFF",                             \
    "Replace protocol type",                \
    "0-0xFFFF",                             \
    "Replace length of protocol address",   \
    "0-0xFF",                               \
    "Replace sender hardware address",      \
    CTC_CLI_HA_FORMAT,                      \
    "Replace sender protocol address",      \
    "0-0xFFFFFFFF",                         \
    "Replace target hardware address",      \
    CTC_CLI_HA_FORMAT,                      \
    "Replace target protocol address",      \
    "0-0xFFFFFFFF"

#define CTC_CLI_NH_FLEX_L3HDR_STR    \
    "(replace-ttl | decrease-ttl) TTL| (replace-ecn ECN |map-ecn) | (replace-dscp DSCP|map-dscp)  "

#define CTC_CLI_NH_FLEX_L3HDR_SET(flex_edit)   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ttl");              \
    if (tmp_argi != 0xFF)                                          \
    {                                                              \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_TTL;       \
        CTC_CLI_GET_UINT8("replace-ttl", flex_edit.ttl, argv[tmp_argi + 1]);         \
    }                                                              \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("decrease-ttl");             \
    if (tmp_argi != 0xFF)                                          \
    {                                                              \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_DECREASE_TTL;      \
        CTC_CLI_GET_UINT8("decrease-ttl", flex_edit.ttl, argv[tmp_argi + 1]);         \
    }                                                              \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ecn");              \
    if (tmp_argi != 0xFF)                                          \
    {                                                              \
        flex_edit.ecn_select = CTC_NH_ECN_SELECT_ASSIGN;              \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ECN;       \
        CTC_CLI_GET_UINT8("replace-ecn", flex_edit.ecn, argv[tmp_argi + 1]);         \
    }                                                              \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-dscp");              \
    if (tmp_argi != 0xFF)                                          \
    {                                                              \
        flex_edit.dscp_select = CTC_NH_DSCP_SELECT_ASSIGN;       \
        CTC_CLI_GET_UINT8("replace-dscp", flex_edit.dscp_or_tos, argv[tmp_argi + 1]);         \
    }                                                              \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("map-dscp");                 \
    if (tmp_argi != 0xFF)                                          \
    {                                                              \
        flex_edit.dscp_select = CTC_NH_DSCP_SELECT_MAP;            \
    }                                                              \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("map-ecn");                 \
    if (tmp_argi != 0xFF)                                          \
    {                                                              \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ECN;       \
        flex_edit.ecn_select = CTC_NH_ECN_SELECT_MAP;            \
    }
#define CTC_CLI_NH_FLEX_L3HDR_DESC   \
    "Replace ttl",                   \
    "Decrease ttl, packet's ttl - specify's ttl",                  \
    "0-255",                         \
    "Replace ecn",                   \
    "0-3",                           \
    "Map ecn",                       \
    "Replace dscp",                  \
    "0-63",                          \
    "Map dscp"

#define CTC_CLI_NH_FLEX_L4HDR_STR    \
    "l4-hdr (icmp (replace-icmp-code ICMP_CODE|)(replace-icmp-type ICMP_TYPE|)|gre (gre-key GRE_KEY|)|udp-tcp { (swap-l4port | {replace-l4destport L4_DST_PORT| replace-l4sourceport L4_SRC_PORT} ) | (replace-tcpport|replace-udpport) |}  |)"

#define CTC_CLI_NH_FLEX_L4HDR_SET(flex_edit)   \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-hdr");                                     \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR;  \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("icmp");                                       \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.packet_type = CTC_MISC_NH_PACKET_TYPE_ICMP;    \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-icmp-code");                          \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_CODE;    \
        CTC_CLI_GET_UINT8("replace-icmp-code", flex_edit.icmp_code, argv[tmp_argi + 1]);  \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-icmp-type");                          \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_TYPE;    \
        CTC_CLI_GET_UINT8("replace-icmp-type", flex_edit.icmp_type, argv[tmp_argi + 1]);  \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("gre");                                        \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.packet_type = CTC_MISC_NH_PACKET_TYPE_GRE;     \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("gre-key");                                    \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_GRE_KEY;    \
        CTC_CLI_GET_UINT32("gre-key", flex_edit.gre_key, argv[tmp_argi + 1]);        \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("udp-tcp");                                    \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.packet_type = CTC_MISC_NH_PACKET_TYPE_UDPORTCP;\
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("swap-l4port");                         \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_SWAP_L4_PORT;    \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-l4destport");                         \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_DST_PORT;    \
        CTC_CLI_GET_UINT32("replace-l4destport", flex_edit.l4_dst_port, argv[tmp_argi + 1]);\
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-l4sourceport");                       \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_SRC_PORT;    \
        CTC_CLI_GET_UINT32("replace-l4sourceport", flex_edit.l4_src_port, argv[tmp_argi + 1]);\
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-tcpport");                            \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_TCP_PORT;       \
    }                                                                                \
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-udpport");                            \
    if (tmp_argi != 0xFF)                                                            \
    {                                                                                \
        flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_UDP_PORT;       \
    }                                                                                \

#define CTC_CLI_NH_FLEX_L4HDR_DESC   \
    "L4 header",                     \
    "Icmp packet",                   \
    "Replace icmp code",             \
    "0-255",                         \
    "Replace icmp type",             \
    "0-255",                         \
    "Gre packet",                    \
    "Gre key",                       \
    "0-0xFFFFFFFF",                  \
    "Udp or tcp packet",             \
    "Swap packet l4 port",           \
    "Replace l4 dest port",          \
    "0-0xFFFF",                      \
    "Replace l4 src port",           \
    "0-0xFFFF",                      \
    "Replace tcp port",              \
    "Replace udp port"

#define CTC_CLI_NH_PARSE_L3IF(first_arg_pos, gport, vid) \
  do {\
  	  uint8 start_index = first_arg_pos;\
	   tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("vlan-if", start_index);\
	   if(tmp_argi != 0xFF)\
	   	{\
	      	CTC_CLI_GET_UINT16("VLAN ID", vid, argv[(start_index + tmp_argi + 2)]);\
            CTC_CLI_GET_UINT32("gport", gport, argv[(start_index + tmp_argi + 4)]);\
	   	}\
	   tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("sub-if", start_index);\
	   if(tmp_argi != 0xFF)\
	   	{\
	        CTC_CLI_GET_UINT32("gport", gport, argv[(start_index + tmp_argi + 2)]);\
            CTC_CLI_GET_UINT16("VLAN ID", vid, argv[(start_index + tmp_argi + 4)]);\
	   	}\
	   tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("routed-port", start_index);\
	   if(tmp_argi != 0xFF)\
	   	{\
	        CTC_CLI_GET_UINT32("gport", gport, argv[(start_index + tmp_argi + 1)]);\
	   	}\
	    tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("ecmp-if", start_index);\
	    if(tmp_argi != 0xFF)\
	   	{\
	        CTC_CLI_GET_UINT16("ecmp l3if id", gport, argv[(start_index + tmp_argi + 1)]);\
	   	}\
    } while (0)


#define CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC   "Alloc dsnh_offset by APP-index"
extern int32
ctc_nexthop_cli_init(void);

extern int32
_ctc_vlan_parser_egress_vlan_edit(ctc_vlan_egress_edit_info_t* vlan_edit, char** argv, uint16 argc);

#ifdef __cplusplus
}
#endif

#endif

