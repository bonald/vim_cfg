#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file ctc_usw_ipuc.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-07

 @version v2.0

 The Module supports wire speed IPv4/IPv6 unicast routing under any situation. According to the description
 in l3if module, the l3 source interface should be configured. For both IPv4/IPv6 unicast routing, the source
 interface could be any type among the three L3 interface types(phy-if/vlan-if/sub-if).
 The v4UcastEn/v6UcastEn must be set as 1 to enable IPv4/IPv6 Unicast routing on source interface by api
 ctc_l3if_set_property(). And the router MAC should be configured correctly by ctc_l3if_set_router_mac().
\p
\b
 Routing table
\pp
 Routing table is configured by api ctc_ipuc_add(), and the lookup key is IPDA + VRF ID. A longest prefix match
 searching algorithm is used to determine the longest subnet match for the given IPDA address. For the host
 route, it use HASH table to store the route entry, and the HASH table is shared with l2. For the other
 route, it use TCAM and LPM hash to store the route entry. Also the tcam can be used to solve hash conflict.
\p
 The IPDA with different prefix will be stored on different area. The relationship is shown on the following table.
\p
\t |  Vertion  |    Prefix    |    Area    |
\t |   IPv4    |    32(host)  |    HASH    |
\t |   IPv4    |     25-32    |    TCAM    |
\t |   IPv4    |     8-24     |    LPM     |
\t |   IPv4    |     0-7      |    TCAM    |
\t |   IPv6    |   128(host)  |    HASH    |
\t |   IPv6    |     65-128   |    TCAM    |
\t |   IPv6    |     48-64    |    LPM     |
\t |   IPv6    |     0-47     |    TCAM    |
\p
\b
 LPM algorithm
\pp
 The Module support two stage LPM lookup. For IPv4, the route with prefix [8-24] will use LPM lookup.
 The LPM use a TCAM entry to store a list of entries which share a common 8bit prefix.
 The first 8 bit is stored in TCAM, and the following 16 bit is stored in two stage HASH table.
 Every stage store 8 bit. If the prefix is small than 16, there's only one stage HASH(occupy one hash entry).
 Otherwise there's two stage HASH(occupy two hash entry).
\p
\b
 Merge dsfwd
\pp
 For host route, it no need to use DsFwd table, so the nexthop info is stored in DsIpda.
\p
\b
 VRF
\pp
 All kinds of routes can support VRF.
\p
\b
 uRPF
\pp
 Unicast reverse path forwarding check is an additional security function. It helps to mitigate problems that are
 caused by the introduction of malformed or spoofed source IP addresses into a network. This is done by discarding
 IP packets that lack a verifiable source IP address. If uRPF is enabled, then only those packets whose source
 IP addresses is consistent with the IP routing table are forwarded.
 The host route which use HASH is not support uRPF. All the other routes can support URPF.
\b
\pp
 There are two uRPF modes of verification: 1) strict mode, and 2) loose mode. In strict uRPF check mode,
 checks are performed to ensure that the source IP address is present in the routing table and the incoming L3
 interface matches the source IP's L3 interface in the routing table. In loose uRPF check mode, it merely verifies
 that the source IP address is present in the routing table, and doesn't care the L3 interface match or not.
\b
\pp
 Now, uRPF also support check by port. It is global configured by ctc_ipuc_init().
\b
\pp
 The Module supports uRPF without any limitation. Based on general Unicast routing, there are some additional
 process for uRPF. To set the uRPF modes please refer to the function ctc_l3if_set_property()
\p
\b
 ECMP
\pp
 Equal cost multi-path (ECMP) is a technique for routing packets along multiple paths of equal cost. If multiple
 equal-cost routes to the same destination exist, ECMP can be used to provide load balancing among the redundant paths.
 Since the forwarding logic has several nexthops for any given destination, it must use some method to choose
 which nexthop should be used for a given data packet. Various routing protocols, including
 Open Shortest Path First (OSPF) and Intermediate System to Intermediate System (ISIS), explicitly allow ECMP routing,
 and some routers also allow equal cost multi-path usage with RIP and other routing protocols.
\p

\S ctc_ipuc.h:ctc_ipuc_flag_e
\S ctc_ipuc.h:ctc_ip_global_property_flag_e
\S ctc_ipuc.h:ctc_ipuc_tunnel_flag_e
\S ctc_ipuc.h:ctc_ipuc_tunnel_payload_type_e
\S ctc_ipuc.h:ctc_ipuc_nat_flag_e
\S ctc_ipuc.h:ctc_ipuc_global_cfg_t
\S ctc_ipuc.h:ctc_ipuc_nat_sa_param_t
\S ctc_ipuc.h:ctc_ipuc_param_t
\S ctc_ipuc.h:ctc_ipuc_tunnel_param_t
\S ctc_ipuc.h:ctc_ipuc_global_property_t
*/

#ifndef _CTC_USW_IPUC_H
#define _CTC_USW_IPUC_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_ipuc.h"
#include "ctc_nexthop.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/****************************************************************
 *
* Function
*
****************************************************************/
/**
 @addtogroup ipuc IPUC
 @{
*/

/**
 @brief Initialize the IPUC module

 @param[in] lchip    local chip id

 @param[in] ipuc_global_cfg  ipuc module global config

 @remark[D2.TM] This function initialize tables and registers structures for ipuc module and set default entry.
         The parameter ipuc_global_cfg could be NULL or a pointer of ctc_ipuc_global_cfg_t type, which defined
         the hash mode IPUC module used. By default NULL should be used which means IPv4 LPM lookup use hash 16 mode.
         If IPUC module IPv4 LPM should use hash 8 mode, ipuc_global_cfg must be set.
 \p
 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_init(uint8 lchip, void* ipuc_global_cfg);

/**
 @brief De-Initialize ipuc module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the ipuc configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_ipuc_deinit(uint8 lchip);

/**
 @brief Add a route entry

 @param[in] lchip    local chip id

 @param[in] p_ipuc_param Data of the ipuc entry

 @remark[D2.TM] This function add both IPv4 and IPv6 IP unicast entrys. Different properties can be retrived by set different flags
         in route_flag. If mask length is 0 , a new default entry will be set, it equals to
         ctc_ipuc_add_default_entry(uint8 ip_ver, uint32 nh_id).
         route priority: private route > private default route > public route > public default route > default action
\p
 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_add(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

/**
 @brief Remove a route entry

 @param[in] lchip    local chip id

 @param[in] p_ipuc_param Data of the ipuc entry

 @remark[D2.TM] This function remove both IPv4 and IPv6 IP unicast entrys. Route flag no need to specified

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_remove(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

/**
 @brief get a route entry

 @param[in] lchip    local chip id

 @param[in|out] p_ipuc_param Input data of the ipuc key, and output ipuc ad

 @remark[D2.TM] This function get both IPv4 and IPv6 IP unicast entrys ad info, include nhid and route flag.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_get(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

/**
 @brief Add a NAT sa entry

 @param[in] lchip    local chip id

 @param[in] p_ipuc_nat_sa_param Data of the NAT sa entry

 @remark[D2.TM] This function add IPv4 NAT sa entrys. For basic NAT, only need ipsa and vrfid as key. For NAPT, the l4_src_port is used as key extra.
         The new_ipsa and new_l4_src_port is the output edit info.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_add_nat_sa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param);

/**
 @brief remove a NAT sa entry

 @param[in] lchip    local chip id

 @param[in] p_ipuc_nat_sa_param Data of the NAT sa entry

 @remark[D2.TM] This function add IPv4 NAT sa entrys. For basic NAT, only need ipsa and vrfid as key. For NAPT, the l4_src_port is used as key extra.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_remove_nat_sa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param);

/**
 @brief  NAT sa entry set hit

 @param[in] lchip    local chip id

 @param[in] p_ipuc_nat_sa_param Data of the NAT sa entry

 @param[in] hit  set entry hit or not

 @remark[D2.TM] This function IPv4 NAT sa entry set hit. 

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_set_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8 hit);

/**
 @brief  NAT sa entry get hit

 @param[in] lchip    local chip id

 @param[in] p_ipuc_nat_sa_param Data of the NAT sa entry

 @param[in|out] hit  get entry hit or not

 @remark[D2.TM] This function IPv4 NAT sa entry get hit. 

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_get_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8* hit);


/**
 @brief Add the default route entrys for both ipv4 and ipv6, default routes should be installed before using ipuc function

 @param[in] lchip    local chip id

 @param[in] ip_ver ip version of the default route

 @param[in] nh_id Nexthop ID of the default route

 @remark[D2.TM] This function set a new default behavior for the vrfid 0 and the ip version which defined by the parameter ip_ver.
         The new default behavior is get from the parameter nh_id. Generally speaking the nhid should be 1 (drop) or 2(to cpu).
         This function equals to the function ctc_ipuc_add(ctc_ipuc_param_t* p_ipuc_param) with mask length 0
         and vrfid 0.
 \p
 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_add_default_entry(uint8 lchip, uint8 ip_ver, uint32 nh_id);

/**
 @brief Add an IP-Tunnel route entry use to decapsulate the ip tunnel packet

 @param[in] lchip    local chip id

 @param[in] p_ipuc_tunnel_param Data of the ip tunnel entry

 @remark[D2.TM]  IP Tunneling is a mechanism for encapsulating one protocol in another and for transport of a packet across a
          network that does not support the native protocol of the packet. The Module supports diverse IP tunneling
          feature, IPv4 tunneling, IPv6 tunneling and GRE.
\p
          There are two basic function of IP tunneling feature: 1)A packet entering an IP tunnel -- Encapsulation
          or Initiator. 2) A packet leaving an IP tunnel -- Decapsulation or Terminator.
\p
          For encapsulation please refer to the function
          ctc_nh_add_ip_tunnel(uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param).
\p
          Decapsulation is implemented by IP Unicast routing process.
\p
          A packet leaving an IP tunnel, the IP tunnel header and L2 Ethernet header should be removed.
          Then an IP Unicast routing process will be implemented based on the original IP header. And a new L2 Ethernet
          header will be attached for the next hop. It looks like there are two forwarding processes. One is for outer
          IP tunnel header, the destination is an internal port. The other is for inner or original IP tunnel header,
          the destination is the next hop of this packet.
\p
          The Module supports enable tunnel decapsulation on port. The port on which tunnel is terminated should
          enable SCL lookup, more information please refer to function
          ctc_port_set_scl_key_type(uint32 gport, ctc_direction_t dir, uint8 scl_id, ctc_port_scl_key_type_t type).
\p
          This function add IP Tunnel route entry , to terminat the ip tunnel and decapsulate the ip tunnel packet.
\p

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_add_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ipuc_tunnel_param);

/**
 @brief Remove an IP-Tunnel route entry which used to decapsulate the ip tunnel packet

 @param[in] lchip    local chip id

 @param[in] p_ipuc_tunnel_param Data of the ipuc entry, route_flag no need to specified

 @remark[D2.TM] This function delete a existing IP-Tunnel route.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_remove_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ipuc_tunnel_param);

/**
 @brief traverse all ipuc entry

 @param[in] lchip    local chip id

 @param[in] ip_ver ip version of the default route

 @param[in] fn callback function to deal with all the ipuc entry

 @param[in] data data used by the callback function

 @remark[D2.TM] This function goes through the route table and runs the callback function at each valid route, passing it the route
         information.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_traverse(uint8 lchip, uint8 ip_ver, void* fn, void* data);

/**
 @brief set global property for ipuc module

 @param[in] lchip    local chip id

 @param[in] p_global_prop the property should be set

 @remark[D2.TM] This function set the global chip ipuc configuration.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_set_global_property(uint8 lchip, ctc_ipuc_global_property_t* p_global_prop);

/**
 @brief Arrange lpm fragment

 @param[in] lchip    local chip id

 @param[in] p_arrange_info not used now

 @remark[D2.TM] When add ipuc entry and return CTC_E_TOO_MANY_FRAGMENT error, it means there's too many
         fragments for the lpm sram. Then you can call this function to arrange the fragment.
         After this operation, you can add ipuc entry success.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_arrange_fragment(uint8 lchip, void* p_arrange_info);

/**
 @brief Set aging status of the ipuc entry, means hit or not

 @param[in] lchip    local chip id

 @param[in] p_ipuc_param  Data of the ipuc entry

 @param[in] hit      entry hit status

 @remark[D2.TM]
        Set the IPUC entry hit or not by user.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_set_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8 hit);

/**
 @brief Get aging status of the ipuc entry, means hit or not

 @param[in] lchip    local chip id

 @param[in] p_ipuc_param  Data of the ipuc entry

 @param[out] hit      entry hit status

 @remark[D2.TM]
        Get the IPUC entry hit or not by user.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ipuc_get_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8* hit);
/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif

