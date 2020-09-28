/**
    @file ctc_usw_acl.h

    @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

    @date 2016-01-19

    @version v0.0

    The CTC ACL module provides access to the Access control list features. ACL
    can be used to classify packets based on predefined protocol fields from Layer 2
    through Layer 4.
\b
    Based on these classifications, various actions can be taken, such as:
\p
\d        Discarding the packet
\d        Sending the packet to the CPU
\d        Sending the packet to a mirror port
\d        Sending the packet on a different CoS queue
\d        Changing the ToS, VLAN or other fields
\b
    There are 9 lookup type for TCAM and 3 for HASH. User must set the acl lookup key
    type in gport, vlan profile or l3 interface profile.
\p
    TCAM: L2, L2-L3, L3, VLAN, L3-EXT, L2-L3-EXT, SGT, Interface, Forward, Forward-EXT.
\p
    HASH: L2, L3, L2-L3.
\b
\p
\t   |          Key Type             |   Description                                                    |
\t   |-------------------------------|------------------------------------------------------------------|
\t   |    CTC_ACL_KEY_MAC            |   DsAclQosMacKey160, Support L2 Header of the Packet             |
\t   |    CTC_ACL_KEY_IPV4           |   DsAclQosL3Key160, Support L3/L4 Header of IPv4 Packet          |
\t   |    CTC_ACL_KEY_MPLS           |   Use CTC_ACL_KEY_IPV4 Instead                                   |
\t   |    CTC_ACL_KEY_IPV6           |   Mode 0: DsAclQosMacIpv6Key640, Support L2/L3/L4 Header of IPv6 Packet; Mode 1: DsAclQosIpv6Key320, Support L3/L4 Header of IPv6 Packet |
\t   |    CTC_ACL_KEY_HASH_MAC       |   DsFlowL2HashKey, Support L2 Header of the packet               |
\t   |    CTC_ACL_KEY_HASH_IPV4      |   DsFlowL3Ipv4HashKey, Support L3/L4 Header of IPv4 packet       |
\t   |    CTC_ACL_KEY_HASH_L2_L3     |   DsFlowL2L3HashKey, Support L2/L3/L4 Header of IPv4 packet      |
\t   |    CTC_ACL_KEY_HASH_MPLS      |   DsFlowL3MplsHashKey, Support Mpls Label of Mpls packet         |
\t   |    CTC_ACL_KEY_HASH_IPV6      |   DsFlowL3Ipv6HashKey, Support L3/L4 Header of IPv6 packet       |
\t   |    CTC_ACL_KEY_IPV4_EXT       |   DsAclQosL3Key320, Support L3/L4 Header of IPv4 Packet          |
\t   |    CTC_ACL_KEY_MAC_IPV4       |   DsAclQosMacL3Key320, Support L2/L3/L4 Header of IPv4 Packet    |
\t   |    CTC_ACL_KEY_MAC_IPV4_EXT   |   DsAclQosMacL3Key640, Support L2/L3/L4 Header of IPv4 Packet    |
\t   |    CTC_ACL_KEY_IPV6_EXT       |   DsAclQosIpv6Key640, Support L3/L4 Header of IPv6 Packet        |
\t   |    CTC_ACL_KEY_MAC_IPV6       |   DsAclQosMacIpv6Key640, Support L2/L3/L4 Header of IPv6 Packet  |
\t   |    CTC_ACL_KEY_CID            |   DsAclQosCidKey160, 160 bits Key for Security Group             |
\t   |    CTC_ACL_KEY_INTERFACE      |   DsAclQosShortKey80, 80 bits Key for Ipfix                      |
\t   |    CTC_ACL_KEY_FWD            |   DsAclQosForwardKey320, Support L2/L3/L4 Header of IPv4/IPv6 Packet and Forwarding Information |
\t   |    CTC_ACL_KEY_FWD_EXT        |   DsAclQosForwardKey640, Support L2/L3/L4 Header of IPv4/IPv6 Packet and Forwarding Information |
\t   |    CTC_ACL_KEY_COPP           |   DsAclQosCoppKey320, Support L2/L3/L4 Header of IPv4/IPv6 Packet and Lookup Information |
\t   |    CTC_ACL_KEY_COPP_EXT       |   DsAclQosCoppKey640, Support L2/L3/L4 Header of IPv4/IPv6 Packet and Lookup Information |
\t   |-------------------------------|------------------------------------------------------------------|
\p
\b
    Here are the details of keys:
\p
\d  DsAclQosMacKey160:
    Field: ETHER_TYPE, L2_TYPE, MAC_DA, MAC_SA, VLAN_ID, COS, CFI, VLAN_VALID;
    Note: Default use s-vlan, unless set C-vlan Domain on port. If need support double vlan,
    c-vlan will share MAC_DA with TCAM lookup type setting CTC_ACL_TCAM_LKUP_TYPE_VLAN.
\b
\d  DsAclQosL3Key160:
    The key can be divided into two parts. One can match fixed fields, and the other part may match
    different fields based on L3_TYPE.
    Fixed fields: DECAP, ELEPHANT_PKT, L3_TYPE, L4_TYPE, ROUTED_PKT;
    Depending on the L3_TYPE, can match different fields:
\p
\t   |    L3_TYPE         |   Fields                                                               |
\t   |--------------------|------------------------------------------------------------------------|
\t   |    IPv4            | IPDA, IPSA, DSCP, IP_HDR_ERROR, L4_DST_PORT, L4_SRC_PORT, L4_DST_PORT_RANGE, L4_SRC_PORT_RANGE|
\t   |    ARP             | OP_CODE, PROTOCOL_TYPE, SENDER_IP, TARGER_IP                           |
\t   |    MPLS            | LABEL_NUM, LABEL0, EXP0, SBIT0, TTL0, LABEL1, EXP1, SBIT1, TTL1, LABEL2, EXP2, SBIT2, TTL2|
\t   |    FCoE            | DST_FCID, SRC_FCID                                                     |
\t   |    Trill           | INGRESS_NICKNAME, EGRESS_NICKNAME, IS_ESADI, IS_TRILL_CHANNEL, TRILL_INNER_VLANID, TRILL_INNER_VLANID_VALID, TRILL_KEY_TYPE, TRILL_LENGTH, TRILL_MULTIHOP, TRILL_MULTICAST, TRILL_VERSION, TRILL_TTL|
\t   |    EtherOam        | IS_Y1731_OAM, ETHER_OAM_LEVEL, ETHER_OAM_OP_CODE, ETHER_OAM_VERSION    |
\t   |    SlowProtocol    | SLOW_PROTOCOL_CODE, SLOW_PROTOCOL_FLAGS, SLOW_PROTOCOL_SUB_TYPE        |
\t   |    PTP             | PTP_MESSAGE_TYPE, PTP_VERSION                                          |
\t   |    SatPdu          | MEF_OUI, OUI_SUB_TYPE                                                  |
\t   |    Unknow packet   | ETHER_TYPE                                                             |
\t   |--------------------|------------------------------------------------------------------------|
\p
\b
\d  DsAclQosL3Key320:
    Fixed fields: DECAP, ELEPHANT_PKT, IP_PROTOCOL, L2_TYPE, L3_TYPE, L4_TYPE, ROUTED_PKT,
                  NSH_CBIT, NSH_OBIT, NSH_NEXT_PROTOCOL, NSH_SI, NSH_SPI;
    Depending on the L3_TYPE, can match different fields:
\p
\t   |    L3_TYPE         |   Field                                                                |
\t   |--------------------|------------------------------------------------------------------------|
\t   |    IPv4            | IPDA, IPSA, DSCP, ECN, FRAG_INFO, IP_OPTIONS, IP_PKT_LEM_RANGE, IP_HDR_ERROR, L4_DST_PORT, L4_SRC_PORT, L4_PORT_RANGE, TCP_FLAGS, TCP_ECN, TTL|
\t   |    ARP             | OP_CODE, PROTOCOL_TYPE, SENDER_IP, TARGER_IP                           |
\t   |    MPLS            | LABEL_NUM, LABEL0, EXP0, SBIT0, TTL0, LABEL1, EXP1, SBIT1, TTL1, LABEL2, EXP2, SBIT2, TTL2|
\t   |    FCoE            | DST_FCID, SRC_FCID                                                     |
\t   |    Trill           | INGRESS_NICKNAME, EGRESS_NICKNAME, IS_ESADI, IS_TRILL_CHANNEL, TRILL_INNER_VLANID, TRILL_INNER_VLANID_VALID, TRILL_KEY_TYPE, TRILL_LENGTH, TRILL_MULTIHOP, TRILL_MULTICAST, TRILL_VERSION, TRILL_TTL|
\t   |    EtherOam        | IS_Y1731_OAM, ETHER_OAM_LEVEL, ETHER_OAM_OP_CODE, ETHER_OAM_VERSION    |
\t   |    SlowProtocol    | SLOW_PROTOCOL_CODE, SLOW_PROTOCOL_FLAGS, SLOW_PROTOCOL_SUB_TYPE        |
\t   |    PTP             | PTP_MESSAGE_TYPE, PTP_VERSION                                          |
\t   |    SatPdu          | MEF_OUI, OUI_SUB_TYPE                                                  |
\t   |    Unknow packet   | ETHER_TYPE                                                             |
\t   |--------------------|------------------------------------------------------------------------|
\p
\b
\d  DsAclQosMacL3Key320:
    Fixed fields: DECAP, ELEPHANT_PKT, IP_PROTOCOL, L2_TYPE, L3_TYPE, ROUTED_PKT, MACDA, MACSA,
                  SVLAN_ID, STAG_COS, STAG_CFI, STAG_VALID, CVLAN_ID, CTAG_COS, CTAG_CFI, CTAG_VALID;
    Depending on the L3_TYPE, can match different fields:
\p
\t   |    L3_TYPE         |   Field                                                                |
\t   |--------------------|------------------------------------------------------------------------|
\t   |    IPv4            | IPDA, IPSA, DSCP, ECN, FRAG_INFO, IP_OPTIONS, IP_PKT_LEM_RANGE, IP_HDR_ERROR, L4_DST_PORT, L4_SRC_PORT, L4_PORT_RANGE, TCP_FLAGS, TCP_ECN, TTL|
\t   |    ARP             | OP_CODE, PROTOCOL_TYPE, SENDER_IP, TARGER_IP                           |
\t   |    MPLS            | LABEL_NUM, LABEL0, EXP0, SBIT0, TTL0, LABEL1, EXP1, SBIT1, TTL1, LABEL2, EXP2, SBIT2, TTL2|
\t   |    FCoE            | DST_FCID, SRC_FCID                                                     |
\t   |    Trill           | INGRESS_NICKNAME, EGRESS_NICKNAME, IS_ESADI, IS_TRILL_CHANNEL, TRILL_INNER_VLANID, TRILL_INNER_VLANID_VALID, TRILL_KEY_TYPE, TRILL_LENGTH, TRILL_MULTIHOP, TRILL_MULTICAST, TRILL_VERSION, TRILL_TTL|
\t   |    EtherOam        | IS_Y1731_OAM, ETHER_OAM_LEVEL, ETHER_OAM_OP_CODE, ETHER_OAM_VERSION    |
\t   |    SlowProtocol    | SLOW_PROTOCOL_CODE, SLOW_PROTOCOL_FLAGS, SLOW_PROTOCOL_SUB_TYPE        |
\t   |    PTP             | PTP_MESSAGE_TYPE, PTP_VERSION                                          |
\t   |    SatPdu          | MEF_OUI, OUI_SUB_TYPE                                                  |
\t   |    Unknow packet   | ETHER_TYPE                                                             |
\t   |--------------------|------------------------------------------------------------------------|
\p
\b
\d  DsAclQosMacL3Key640:
    Fixed fields: DECAP, ELEPHANT_PKT, IP_PROTOCOL, INTERFACE_ID, L2_TYPE, L3_TYPE, L4_TYPE, ROUTED_PKT, MACDA, MACSA,
                  SVLAN_ID, STAG_COS, STAG_CFI, STAG_VALID, CVLAN_ID, CTAG_COS, CTAG_CFI, CTAG_VALID,
                  NSH_CBIT, NSH_OBIT, NSH_NEXT_PROTOCOL, NSH_SI, NSH_SPI;
    Depending on the L3_TYPE, can match different fields:
\p
\t   |    L3_TYPE         |   Field                                                                |
\t   |--------------------|------------------------------------------------------------------------|
\t   |    IPv4            | IPDA, IPSA, DSCP, ECN, FRAG_INFO, IP_OPTIONS, IP_PKT_LEM_RANGE, IP_HDR_ERROR, L4_DST_PORT, L4_SRC_PORT, L4_PORT_RANGE, TCP_FLAGS, TCP_ECN, TTL|
\t   |    ARP             | OP_CODE, PROTOCOL_TYPE, SENDER_IP, TARGER_IP                           |
\t   |    MPLS            | LABEL_NUM, LABEL0, EXP0, SBIT0, TTL0, LABEL1, EXP1, SBIT1, TTL1, LABEL2, EXP2, SBIT2, TTL2|
\t   |    FCoE            | DST_FCID, SRC_FCID                                                     |
\t   |    Trill           | INGRESS_NICKNAME, EGRESS_NICKNAME, IS_ESADI, IS_TRILL_CHANNEL, TRILL_INNER_VLANID, TRILL_INNER_VLANID_VALID, TRILL_KEY_TYPE, TRILL_LENGTH, TRILL_MULTIHOP, TRILL_MULTICAST, TRILL_VERSION, TRILL_TTL|
\t   |    EtherOam        | IS_Y1731_OAM, ETHER_OAM_LEVEL, ETHER_OAM_OP_CODE, ETHER_OAM_VERSION    |
\t   |    SlowProtocol    | SLOW_PROTOCOL_CODE, SLOW_PROTOCOL_FLAGS, SLOW_PROTOCOL_SUB_TYPE        |
\t   |    PTP             | PTP_MESSAGE_TYPE, PTP_VERSION                                          |
\t   |    SatPdu          | MEF_OUI, OUI_SUB_TYPE                                                  |
\t   |    Unknow packet   | ETHER_TYPE                                                             |
\t   |--------------------|------------------------------------------------------------------------|
\p
\b
\d  DsAclQosIpv6Key320:
    Fields: DSCP, ECN, FRAG_INFO, IPV6_DA, IPV6_SA, IP_HDR_ERROR, IP_OPTIONS, IP_PKT_LEN_RANGE,
            IPV6_FLOW_LABEL, DECAP, ELEPHANT_PKT, L4_DST_PORT, L4_SRC_PORT, L4_DST_PORT_RANGE,
            L4_SRC_PORT_RANGE, L2_TYPE, L4_TYPE, IP_PROTOCOL, ROUTED_PKT, TCP_FLAGS, TCP_ECN;
\p
\b
\d  DsAclQosIpv6Key640:
    Fields: DSCP, ECN, FRAG_INFO, IPV6_DA, IPV6_SA, IP_HDR_ERROR, IP_OPTIONS, IP_PKT_LEN_RANGE,
            IPV6_FLOW_LABEL, DECAP, ELEPHANT_PKT, L4_DST_PORT, L4_SRC_PORT, L4_DST_PORT_RANGE,
            L4_SRC_PORT_RANGE, L2_TYPE, L4_TYPE, IP_PROTOCOL, ROUTED_PKT, TCP_FLAGS, TCP_ECN,
            IP_TTL, VRFID;
\p
\b
\d  DsAclQosMacIpv6Key640:
    Fields: CVALN_ID, CVLAN_ID_VALID, CTAG_COS, CTAG_CFI, SVALN_ID, SVLAN_ID_VALID, STAG_COS, STAG_CFI,
            DSCP, ECN, FRAG_INFO, INTERFACE_ID, IPV6_DA, IPV6_SA, IP_HDR_ERROR, IP_OPTIONS, IP_PKT_LEN_RANGE,
            IPV6_FLOW_LABEL, DECAP, ELEPHANT_PKT, L4_DST_PORT, L4_SRC_PORT, L4_DST_PORT_RANGE,
            L4_SRC_PORT_RANGE, L2_TYPE, L4_TYPE, IP_PROTOCOL, MAC_DA, MAC_SA, ROUTED_PKT, TCP_FLAGS,
            TCP_ECN, IP_TTL, VRFID;
\p



    Group-id:        This represents a acl group which has many acl entries. One group can have Mac Ipv4 Ipv6 mixed.
\p
    Group-priority:  This represents a acl block, since Mac/Ipv4(Mpls)/Ipv6 has
                     its own blocks, some priority may not be suited for some entry-type.
                     Ex. a group with priority = 2 (priority starts at 0), then
                     a Ipv6-entry ( 2 blocks at most ) can not add to this group. An error
                     will be returned.
                     The priority IS block_id, the naming comes from that the actions
                     from block_id=0 take higher priority than block_id=1 when these block
                     have conflict actions.
\p
    Entry-id:        This represents a acl entry, and is a global concept. In other words,
                     the entry-id is unique for all acl groups.
\p
    Entry-priority:  This represents a entry priority within a block.
                     The bigger the higher priority. Different entry-type has
                     nothing to compare.
\b
                     Ex. group id=0, group prio=0. There are 3 entries:
\p
\d                        entry id=0, entry prio=10, type= mac
\d                        entry id=1, entry prio=20, type= mac
\d                        entry id=2, entry prio=20, type= ipv4
\d                        The priority of eid1 > eid0.
\d                        The priority of eid2 has nothing to do with eid0.

\b
\p
The module provides APIs to support ACL:
\p
\d Create acl group by ctc_acl_create_group()
\d Destroy acl group by ctc_acl_destroy_group()
\d Install all entry in one group to hardware by ctc_acl_install_group()
\d Remove all entry in one group from hardware  by ctc_acl_uninstall_group()
\d Get group info by ctc_acl_get_group_info()
\d Add acl entry by ctc_acl_add_entry()
\d Remove acl entry by ctc_acl_remove_entry()
\d Install acl entry to hardware by ctc_acl_install_entry()
\d Remove acl entry from hardware by ctc_acl_uninstall_entry()
\d Remove all acl entry from hardware for one group by ctc_acl_remove_all_entry()
\d Set acl entry priority by ctc_acl_set_entry_priority()
\d Get multiple entry  by ctc_acl_get_multi_entry()
\d Update acl action by ctc_acl_update_action()
\d Copy acl entry by ctc_acl_copy_entry()
\d Set acl hash select key field by ctc_acl_set_hash_field_sel()

\S ctc_acl.h:ctc_acl_key_type_t
\S ctc_acl.h:ctc_acl_hash_mac_key_t
\S ctc_acl.h:ctc_acl_hash_ipv4_key_t
\S ctc_acl.h:ctc_acl_hash_mpls_key_t
\S ctc_acl.h:ctc_acl_hash_ipv6_key_t
\S ctc_acl.h:ctc_acl_hash_l2_l3_key_t
\S ctc_acl.h:ctc_acl_mac_key_t
\S ctc_acl.h:ctc_acl_ipv4_key_t
\S ctc_acl.h:ctc_acl_ipv6_key_t
\S ctc_acl.h:ctc_acl_pbr_ipv4_key_t
\S ctc_acl.h:ctc_acl_pbr_ipv6_key_t
\S ctc_acl.h:ctc_acl_key_t
\S ctc_acl.h:ctc_acl_action_flag_t
\S ctc_acl.h:ctc_acl_query_t
\S ctc_acl.h:ctc_acl_vlan_edit_t
\S ctc_acl.h:ctc_acl_action_t
\S ctc_acl.h:ctc_acl_entry_t
\S ctc_acl.h:ctc_acl_copy_entry_t
\S ctc_acl.h:ctc_acl_global_cfg_t
\S ctc_acl.h:ctc_acl_group_type_t
\S ctc_acl.h:ctc_acl_group_info_t

*/

#ifndef _CTC_USW_ACL_H
#define _CTC_USW_ACL_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_parser.h"
#include "ctc_acl.h"
#include "ctc_stats.h"
#include "ctc_field.h"

/**
 @addtogroup acl ACL
 @{
*/

/**
 @brief     Init acl module.

 @param[in] lchip    local chip id

 @param[in] acl_global_cfg      Init parameter

 @remark[D2.TM]    Init acl module.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_init(uint8 lchip, ctc_acl_global_cfg_t* acl_global_cfg);

/**
 @brief De-Initialize acl module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the acl configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_deinit(uint8 lchip);

/**
 @brief     Create a acl group.

 @param[in] lchip    local chip id

 @param[in] group_id      ACL group ID
 @param[in] group_info    ACL group info

 @remark[D2.TM]    Creates a acl group with a specified priority.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_create_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);

/**
 @brief     Destroy a acl group.

 @param[in] lchip    local chip id

 @param[in] group_id      ACL group ID

 @remark[D2.TM]    Destroy a acl group.
            All entries that uses this group must have been removed before calling this routine,
            or the call will fail.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_destroy_group(uint8 lchip, uint32 group_id);

/**
 @brief     Install all entries of a acl group into the hardware tables.

 @param[in] lchip    local chip id

 @param[in] group_id      ACL group ID
 @param[in] group_info    ACL group info

 @remark[D2.TM]    Installs a group of entries into the hardware tables.
            Will silently reinstall entries already in the hardware tables.
            If the group is empty, nothing is installed but the group_info is saved.
            If group_info is null, group_info is the same as previoust install_group or it's a hash group.
            Group Info care not type, gchip, priority.
            Only after the group_info saved, ctc_acl_install_entry() will work.



 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_install_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);

/**
 @brief     Uninstall all entries of a acl group from the hardware table.

 @param[in] lchip    local chip id

 @param[in] group_id      ACL group ID

 @remark[D2.TM]    Uninstall all entries of a group from the hardware tables.
            Will silently ignore entries that are not in the hardware tables.
            This does not destroy the group or remove its entries,
            it only uninstalls them from the hardware tables.
            Remove an entry by using  ctc_acl_remove_entry(),
            and destroy a group by using ctc_acl_destroy_group().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_uninstall_group(uint8 lchip, uint32 group_id);

/**
 @brief      Get group_info of a acl group.

 @param[in] lchip    local chip id

 @param[in]  group_id            ACL group ID
 @param[out] group_info          ACL group information

 @remark[D2.TM]     Get group_info of a acl group.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_get_group_info(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);

/**
 @brief     Add an acl entry to a specified acl group.

 @param[in] lchip    local chip id

 @param[in] group_id      ACL group ID
 @param[in] acl_entry     ACL entry info.

 @remark[D2.TM]    Add an acl entry to a specified acl group.
            The entry contains: key, action, entry_id, priority, refer to ctc_acl_entry_t.
            Remove an entry by using ctc_acl_remove_entry().
            It only add entry to the software table, install to hardware table
            by using ctc_acl_install_entry().
            PS. vlan edit is supported only for ingress acl.
            If acl entry is hash entry, you must install this entry immediately when you add a entry.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_entry(uint8 lchip, uint32 group_id, ctc_acl_entry_t* acl_entry);

/**
 @brief     Remove an acl entry.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry ID

 @remark[D2.TM]    Remove an acl entry from software table,
            uninstall from hardware table by calling ctc_acl_uninstall_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_entry(uint8 lchip, uint32 entry_id);

/**
 @brief     Installs an entry into the hardware tables.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry ID

 @remark[D2.TM]    Installs an entry into the hardware tables.
            Must add an entry to software table first, by calling ctc_acl_add_entry().

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_install_entry(uint8 lchip, uint32 entry_id);

/**
 @brief     Uninstall an acl entry from the hardware tables.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry ID

 @remark[D2.TM]    Uninstall an acl entry from the hardware tables.
            The object of this entry is still in software table, thus can install again.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_uninstall_entry(uint8 lchip, uint32 entry_id);

/**
 @brief     Remove all acl entries of a acl group.

 @param[in] lchip    local chip id

 @param[in] group_id      ACL group ID

 @remark[D2.TM]    Remove all acl entries of a acl group from software table, must uninstall all
            entries first.
            Uninstall one entry from hardware table by using ctc_acl_uninstall_entry().
            Uninstall a whole group from hardware table by using ctc_acl_uninstall_group().

 @return    CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_all_entry(uint8 lchip, uint32 group_id);

/**
 @brief     set an entry priority.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry ID

 @param[in] priority      ACL entry priority

 @remark[D2.TM]    Sets the priority of an entry.
            Entries with higher priority values take precedence over entries with lower values.
            Since TCAM lookups start at low indexes, precedence within a physical block is the
            reverse of this.
            The effect of this is that entries with the greatest priority will have the lowest TCAM index.
            Entries should have only positive priority values. If 2 entry priority set equally,
            the latter is lower than former.
            Currently there are 1 predefined cases:
            FPA_PRIORITY_LOWEST  - Lowest possible priority (ALSO is the default value)

@return CTC_E_XXX
*/
extern int32
ctc_usw_acl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority);

/**
 @brief      Get an array of entry IDs in use in a group

 @param[in] lchip    local chip id

 @param[in|out]  query     Pointer to query structure.

 @remark[D2.TM]     Fills an array with the entry IDs for the specified group. This should first be called with an  entry_size  of 0
to get the number of entries to be returned, so that an appropriately-sized array can be allocated.

@return CTC_E_XXX
*/
extern int32
ctc_usw_acl_get_multi_entry(uint8 lchip, ctc_acl_query_t* query);

/**
 @brief      Update acl action

 @param[in] lchip    local chip id

 @param[in]  entry_id     ACL entry need to update action
 @param[in]  action       New action to overwrite

 @remark[D2.TM]     Update an ace's action, this action will overwrite the old ace's action.
             If the entry is installed, new action will be installed too.
             PS. vlan edit is supported only for ingress acl.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_update_action(uint8 lchip, uint32 entry_id, ctc_acl_action_t* action);

/**
 @brief      Create a copy of an existing acl entry

 @param[in] lchip    local chip id

 @param[in]  copy_entry     Pointer to copy_entry struct

 @remark[D2.TM]     Creates a copy of an existing acl entry to a sepcific group.
             The dst entry's priority is source entry's  priority.

 @return CTC_E_XXX
*/

extern int32
ctc_usw_acl_copy_entry(uint8 lchip, ctc_acl_copy_entry_t* copy_entry);

/**
 @brief      Set acl hash field select.

 @param[in] lchip    local chip id

 @param[in]  field_sel     Pointer to ctc_acl_hash_field_sel_t struct

 @remark[D2.TM]     Set hash key's hash field select. Each key has a variety of hash field, which
             indicate the fields that hash key cares.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_set_hash_field_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel);

/**
 @brief     Add field to key.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] key_field     Pointer to ctc_field_key_t struct.

 @remark[D2.TM]    Add field to key according to field type.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field);

/**
 @brief     Remove field from key.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] key_field     Pointer to ctc_field_key_t struct.

 @remark[D2.TM]    Remove field from key according to field type.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field);

/**
 @brief     Add field to action.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] action_field     Pointer to ctc_acl_field_action_t struct.

 @remark[D2.TM]    Add field to action according to field type.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field);

/**
 @brief     Remove field from action.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] action_field     Pointer to ctc_acl_field_action_t struct.

 @remark[D2.TM]    Remove field from action according to field type.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field);

/**
 @brief     Add fields to key.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] p_field_list     Pointer to key field list.

 @param[in|out] p_field_cnt     Field number of key.

 @remark[D2.TM]    Add fields to key according to field type. If ERROR happened, p_field_cnt indicate successfully added field number of key for output.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_key_field_list(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_list, uint32* p_field_cnt);

/**
 @brief     Remove fields from key.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] p_field_list     Pointer to key field list.

 @param[in|out] p_field_cnt     Field number of key.

 @remark[D2.TM]    Remove fields from key according to field type. If ERROR happened, p_field_cnt indicate successfully removed field number of key for output.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_key_field_list(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_list, uint32* p_field_cnt);

/**
 @brief     Add fields to action.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] p_field_list     Pointer to action field list.

 @param[in|out] p_field_cnt     Field number of action.

 @remark[D2.TM]    Add fields to action according to field type. If ERROR happened, p_field_cnt indicate successfully added field number of action for output.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_action_field_list(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* p_field_list, uint32* p_field_cnt);

/**
 @brief     Remove fields from action.

 @param[in] lchip    local chip id

 @param[in] entry_id      ACL entry id

 @param[in] p_field_list     Pointer to action field list.

 @param[in|out] p_field_cnt     Field number of action.

 @remark[D2.TM]    Remove fields from action according to field type. If ERROR happened, p_field_cnt indicate successfully added field number of action for output.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_action_field_list(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* p_field_list, uint32* p_field_cnt);

/**
 @brief     Set field to hash sel.

 @param[in] lchip    local chip id

 @param[in] key_type      ACL hash key type

 @param[in] field_sel_id  Hash field select id

 @param[in] sel_field     Pointer to ctc_field_key_t struct, data = 1 £ºset , data £º0 unset.

 @remark[D2.TM]    Set field to hash select according to field type.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_set_field_to_hash_field_sel(uint8 lchip, uint8 key_type, uint8 field_sel_id, ctc_field_key_t* sel_field);

/**
 @brief     Add category id pair.

 @param[in] lchip    local chip id

 @param[in] cid_pair      src category id, dest category id and action

 @remark[D2.TM]    add category id and action to hardware.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);

/**
 @brief     Remove category id pair.

 @param[in] lchip    local chip id

 @param[in] cid_pair      src category id, dest category id

 @remark[D2.TM]    remove category id from hardware.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
/**
 @brief     Add udf entry.

 @param[in] lchip    local chip id

 @param[in] udf_entry      Pointer to ctc_acl_classify_udf_t struct.

 @remark[D2.TM]    add udf entry to hardware.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry);
/**
 @brief     Remove udf entry.

 @param[in] lchip    local chip id

 @param[in] udf_entry      Pointer to ctc_acl_classify_udf_t struct.

 @remark[D2.TM]    remove udf entry from hardware.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry);
/**
 @brief     Get udf entry By UDF ID or priority.

 @param[in] lchip    local chip id

 @param[in] udf_entry   UDF Entry information

 @remark[D2.TM]     Get udf entry By priority.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_get_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry);
/**
 @brief     Add udf entry key field.

 @param[in] lchip    local chip id

 @param[in] udf_id    udf entry id

 @param[in] key_field      Pointer to ctc_field_key_t struct.

 @remark[D2.TM]    add udf entry key field to hardware.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_add_udf_entry_key_field(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field);
/**
 @brief     Remove udf entry key field.

 @param[in] lchip    local chip id

 @param[in] udf_id    udf entry id

 @param[in] key_field      Pointer to ctc_field_key_t struct.

 @remark[D2.TM]    remove udf entry key field to hardware.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_remove_udf_entry_key_field(uint8 lchip, uint32 udf_id, ctc_field_key_t* key_field);

/**
 @brief     Set acl lookup level league mode.

 @param[in] lchip    local chip id

 @param[in] league   lookup level league information

 @remark[D2.TM]     Set acl lookup level league mode.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_set_league_mode(uint8 lchip, ctc_acl_league_t* league);

/**
 @brief     Get acl lookup level league mode.

 @param[in] lchip    local chip id

 @param[in/out] league   lookup level league information

 @remark[D2.TM]     Get acl lookup level league mode.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_get_league_mode(uint8 lchip, ctc_acl_league_t* league);

/**
 @brief     Reorder entries by lookup level.

 @param[in] lchip    local chip id

 @param[in/out] reorder   reorder information

 @remark[D2.TM]     Reorder entries by lookup level.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_acl_reorder_entry(uint8 lchip, ctc_acl_reorder_t* reorder);


/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

