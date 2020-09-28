#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file ctc_usw_ipmc.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-01-25

 @version v2.0

 IP Multicast (IPMC) delivers IP traffic from one source to multiple receivers without adding any additional burden on the source
 or the receivers while using the least possible network bandwidth. The source sends a single stream of packets, which are
 replicated in the network by IP Multicast-enabled routers only when needed, thus conserving bandwidth. When the packet
 reaches the LAN to which the receiver hosts are attached, the switch sends out as many copies of the packet as there are ports
 to which receiver hosts are connected. The L2 MAC address used is attained through IPMC to 802.3 address mapping.
 \p
\b
 IP Multicast is based on the concept of a group specified by a Multicast address. An arbitrary group of IP hosts interested in
 receiving a particular data stream must join the group using IGMP. In other words, a Multicast address specifies an arbitrary
 group of IP hosts that have joined the group and want to receive traffic sent to this group.
 \p
\b
 The Internet Assigned Numbers Authority (IANA) controls the assignment of IP multicast addresses. It has assigned the old
 Class D address space to be used for IP multicast. All IP multicast group addresses fall in this range of 224.0.0.0 to 239.255.255.255,
 of which 224.0.0.0 through 224.0.0.255 are reserved for network protocols. 224.0.1.0 through 238.255.255.255 are called
 globally-scoped addresses which can be used to multicast data between organizations and across the Internet. 239.0.0.0
 through 239.255.255.255 contains limited/administratively scope addresses, which are constrained to a local group or organization.
 \p

\S ctc_ipmc.h:ctc_ipmc_default_action_t
\S ctc_ipmc.h:ctc_ipmc_force_route_t
\S ctc_ipmc.h:ctc_ipmc_ipv4_addr_t
\S ctc_ipmc.h:ctc_ipmc_ipv6_addr_t
\S ctc_ipmc.h:ctc_ipmc_addr_info_t
\S ctc_ipmc.h:ctc_ipmc_member_info_t
\S ctc_ipmc.h:ctc_ipmc_group_info_t
\S ctc_ipmc.h:ctc_ipmc_rp_t

*/

#ifndef _CTC_USW_IPMC_H
#define _CTC_USW_IPMC_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 @addtogroup ipmc IPMC
 @{
*/

/**
 @brief Init ipmc module, including global variable, ipmc database, etc.

 @param[in] lchip    local chip id

 @param[in] ipmc_global_cfg   ipmc global config information, use struct ctc_ipmc_global_cfg_t to set max vrf id (ipv4, ipv6)

 @remark[D2.TM]  Initialize the IPMC module, including global variable such as max vrfid number, ipmc lookup mode, default entry, etc.
                Max vrf id can set by user according to the parameter ipmc_global_cfg, when ipmc_global_cfg is NULL,ipmc will use
                default global config to initialize sdk.
                By default:
                Ipv4 max vrf id is 256
                Ipv6 max vrf id is 128
                This function has to be called before any other IPMC functions.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_init(uint8 lchip, void* ipmc_global_cfg);

/**
 @brief De-Initialize ipmc module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the ipmc configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_ipmc_deinit(uint8 lchip);

/**
 @brief Add an ipmc multicast group

 @param[in] lchip    local chip id

 @param[in] p_group     An pointer reference to an ipmc group

 @remark[D2.TM]  Add an ip multicaset group with not member and member information not used in parameter p_group.
                \p
                For (S,G), set the parameter src_ip_mask_len to 32(ipv4) or 128(ipv6).
                \p
                For (*,G), set the parameter src_ip_mask_len to zero.
                \p
\d                Use CTC_IPMC_FLAG_TTL_CHECK to enable TTL check.
\d                Use CTC_IPMC_FLAG_RPF_CHECK to enable RPF check, support 16 source interface check according to parameter rpf_intf_valid.
\d                Use CTC_IPMC_FLAG_L2MC to enable L2MC based on IP.
\d                Use CTC_IPMC_FLAG_STATS to enable  ipmc statistic.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_add_group(uint8 lchip, ctc_ipmc_group_info_t* p_group);

/**
 @brief Remove an ipmc group

 @param[in] lchip    local chip id

 @param[in] p_group     An pointer reference to an ipmc group

 @remark[D2.TM]  Remove an existed ip multicast group address (S,G) or (*,G) with the specified VRFID.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_remove_group(uint8 lchip, ctc_ipmc_group_info_t* p_group);

/**
 @brief Add members to an ipmc group

 @param[in] lchip    local chip id

 @param[in] p_group     An pointer reference to an ipmc group

 @remark[D2.TM]  Add member ports into an existed multicast group, multiple IPMC member can share the same IPMC group ID.
                Use CTC_IPMC_FLAG_L2MC if member is L2MC based on IP.
                Add IPMC member can based on l3if(vlan-if, phy-if) or nexthop-id.
                Add L2MC based on IP member can based on gport or nexthop-id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_add_member(uint8 lchip, ctc_ipmc_group_info_t* p_group);

/**
 @brief Remove members from an ipmc group

 @param[in] lchip    local chip id

 @param[in] p_group     An pointer reference to an ipmc group

 @remark[D2.TM]  Remove member ports from an existed multicast group.
                Use CTC_IPMC_FLAG_L2MC if member is L2MC based on IP.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_remove_member(uint8 lchip, ctc_ipmc_group_info_t* p_group);

/**
 @brief Update the RPF interface of the group

 @param[in] lchip    local chip id

 @param[in] p_group     An pointer reference to an ipmc group

 @remark[D2.TM]  Update the RPF check interface of an existed multicast group by setting parameter rpf_intf and rpf_intf_valid.
                Support 16 source interface check at most.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_update_rpf(uint8 lchip, ctc_ipmc_group_info_t* p_group);

/**
 @brief Add the RP interface

 @param[in] lchip    local chip id

 @param[in] p_rp     An pointer reference to an rp group

 @remark[D2.TM]  Add the rp interfaces not block of an existed rp id by setting parameter rp_intf_count and rp_intf.
                Support 16 source interfaces check at once.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_add_rp_intf(uint8 lchip, ctc_ipmc_rp_t* p_rp);

/**
 @brief Remove the RP interface

 @param[in] lchip    local chip id

 @param[in] p_rp     An pointer reference to an rp group

 @remark[D2.TM]  Add the rp interfaces block of an existed rp id by setting parameter rp_intf_count and rp_intf.
                Support 16 source interfaces check at once.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_remove_rp_intf(uint8 lchip, ctc_ipmc_rp_t* p_rp);
/**
 @brief Get info of the group

 @param[in] lchip    local chip id

 @param[out] p_group     An pointer reference to an ipmc group

 @remark[D2.TM]  Get group info by address, src_ip_mask_len, group_ip_mask_len, ip_version. You can get group id info of ipmc group.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_get_group_info(uint8 lchip, ctc_ipmc_group_info_t* p_group);

/**
 @brief Add the default action for both ipv4 and ipv6 ipmc.

 @param[in] lchip    local chip id

 @param[in] ip_version     IPv4 or IPv6.

 @param[in] type           Indicates send to cpu or drop only.

 @remark[D2.TM]  Add the default action for both ipv4 and ipv6 ipmc. Default action type:
                \p
                Use CTC_IPMC_DEFAULT_ACTION_DROP to set default entry action is drop.
                \p
                Use CTC_IPMC_DEFAULT_ACTION_TO_CPU to set default entry action is to cpu.
                \p
                Use CTC_IPMC_DEFAULT_ACTION_FALLBACK_BRIDGE to set default entry action is fallback to bridge.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_add_default_entry(uint8 lchip, uint8 ip_version, ctc_ipmc_default_action_t type);

/**
 @brief Set ipv4 and ipv6 force route register for force mcast ucast or bridge.

 @param[in] lchip    local chip id

 @param[in] p_data   An point reference to ctc_ipmc_force_route_t.

 @remark[D2.TM]  Set force mcast ucast or bridge for both ipv4 and ipv6 in a certern ipda range.
                Both ipv4 and ipv6 can set 2 ipda address and mask by setting parameter ip_addr0, addr0_mask, ip_addr1, addr1_mask.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_set_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data);

/**
 @brief Get ipv4 and ipv6 mcast force route.

 @param[in] lchip    local chip id

 @param[out] p_data   An point reference to ctc_ipmc_v4_force_route_t.

 @remark[D2.TM]  Get force mcast ucast or bridge to parameter ip_addr0, addr0_mask, force_bridge_en, force_ucast_en.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_get_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data);

/**
 @brief Set ipv4 and ipv6 mcast Entry hit.

 @param[in] lchip    local chip id

 @param[in] p_group  An pointer reference to an ipmc group

 @param[in] hit      entry hit status

 @remark[D2.TM]  Set the IPMC entry hit or not by user.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_set_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8 hit);

/**
 @brief Get ipv4 and ipv6 mcast entry hit.

 @param[in] lchip    local chip id

 @param[in] p_group  An pointer reference to an ipmc group

 @param[out] hit      entry hit status

 @remark[D2.TM]  Get the IPMC entry hit or not by user.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipmc_get_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8* hit);

/**@} end of @addtogroup ipmc IPMC*/

#ifdef __cplusplus
}
#endif

#endif /*_CTC_USW_IPMC_H*/

#endif

