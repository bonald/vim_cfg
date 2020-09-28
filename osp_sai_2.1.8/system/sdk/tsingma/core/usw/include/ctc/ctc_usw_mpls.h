#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_mpls.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-03-12

 @version v2.0

\p
In MPLS, the assignment of a particular packet to a particular FEC is
done just once, as the packet enters the network.  The FEC to which
the packet is assigned is encoded as a short fixed length value known
as a "label".  When a packet is forwarded to its next hop, the label
is sent along with it; that is, the packets are "labeled" before they
are forwarded.
\p
\b
At subsequent hops, there is no further analysis of the packet's
network layer header.  Rather, the label is used as an index into a
table which specifies the next hop, and a new label.  The old label
is replaced with the new label, and the packet is forwarded to its
next hop.

\p
\b
 In The Module, MPLS is treated as a L3 feature, and L3 interface
 should be retrieved before MPLS process stage. The L3 interface and
 router MAC determination is the same as routing process.

\p
\b
Basic Mpls
\pp
  The Module MPLS support the basic packet operation:
\ppp
    \ddd PUSH: When packets enter Mpls network, LER add labels to packets
    \ddd SWAP: LSR replace the outer label with a new label
    \ddd POP: Switch remove the outer label
\ppp
    ctc_mpls_add_ilm()
\p
\b
Differentiated Services
\pp
  The Module MPLS support L-Lsp and E-Lsp and three Differentiated Services
  Modes:
\ppp
    \ddd UNIFORM
    \ddd PIPE
    \ddd SHORT PIPE
\ppp
    ctc_mpls_add_ilm()
\p
\b
L2VPN
\pp
 The Module MPLS support the point-to-point Virtual Private Wire Service (VPWS)
 and point-to-multipoint Virtual Private LAN Service (VPLS) Ethernet transport
 services.
\pp
 Use API ctc_mpls_add_l2vpn_pw() to build PW and use ctc_mpls_l2vpn_pw_t.l2vpntype
 to distinguish VPLS and VPWS.
\pp
 The Module MPLS support two AC access methods:
    \ddd Ethernet Access
    \ddd VLAN Access
\pp
 And support two PW encapsulation mode:
\pp
    \ddd RAW Mode
    \ddd Tagged Mode
\pp
 And also support control word decapsulation.
\pp
 Use ctc_mpls_l2vpn_pw_t.pw_mode to distinguish PW encapsulation modes.

\b
\pp
 In VPWS, a VPN consists of two MPLS ports. Packets arriving on one MPLS
 port are sent directly out the other MPLS port. Ethernet frames are mapped
 into a MPLS pseudowire based on incoming port plus packet header information.
 At the end of a pseudowire, forwarding of Ethernet frames after MPLS
 decapsulation is based on the MPLS label lookup.

\b
\pp
 In VPLS, a VPN is similar to a VLAN, which identifies a group of physical
 ports to be included in a broadcast domain. However, instead of physical
 ports, a VPLS VPN identifies a group of ¡°virtual-ports¡± to be included
 in a broadcast domain.  The VPN ID(VSI ID) is used to qualify MAC DA lookups
 in VPLS. Ethernet frames of interest are also identified by incoming port
 plus packet header information. Ethernet frames are switched into one or
 more MPLS pseudowires based on the MAC DA lookup. At the end of a pseudowire,
 frames again are switched based on MAC DA lookup.

\p
\b
L3VPN
\pp
 L3 VPNs are used to identify a virtual routing table. When creating an L3 VPN,
 a VRF must be specified. At the end of a MPLS tunnel, the IPv4 or IPv6 DestIP
 lookup is qualified by VRF associated with the L3 VPN. L3 MPLS deals with the
 establishment of MPLS tunnels. At the MPLS tunnel initiation point, packets
 entering the tunnel are encapsulated with an MPLS header (label PUSH operation).
 IPv4 and IPv6 packets can be directed into an MPLS tunnel based on the DestIP
 lookup. Along the path of the MPLS tunnel, MPLS label SWAP, PUSH, PHP, and POP
 operations are supported. At the end of an MPLS tunnel, forwarding of IPv4 and
 IPv6 packets is based on DestIP lookup.
 Virtual routing tables are supported for the DestIP lookup.
\pp
 L3 VPNs are used to identify a virtual routing table. When creating an L3 VPN,
 a VRF must be specified. At the end of a MPLS tunnel, the IPv4 or IPv6 DestIP
 lookup is qualified by VRF associated with the L3 VPN.
\pp
 Use ctc_mpls_add_ilm() to configure the ilm, and ctc_mpls_ilm_t.flw_vrf_srv_aps.vrf_id
 to attach the VRF

 \S ctc_mpls.h:ctc_mpls_inner_pkt_type_t
 \S ctc_mpls.h:ctc_mpls_ilm_flag_t
 \S ctc_mpls.h:ctc_mpls_stats_index_t
 \S ctc_mpls.h:ctc_mpls_ilm_t
 \S ctc_mpls.h:ctc_mpls_l2vpn_pw_t
 \S ctc_mpls.h:ctc_mpls_pw_t
 \S ctc_mpls.h:ctc_mpls_space_info_t

*/

#ifndef _CTC_USW_MPLS_H
#define _CTC_USW_MPLS_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_mpls.h"
#include "ctc_nexthop.h"
#include "ctc_stats.h"
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
 @addtogroup mpls MPLS
 @{
*/

/**
 @brief Initialize the MPLS module

 @param[in] lchip    local chip id

 @param[in] p_mpls_info Data of the mpls initialization

 @remark[D2.TM] Initialize the The Module MPLS subsystem, Including global variable, interface space size, register callback
                function, etc.
                This function has to be called before any other mpls functions.
\p
                Set sizetype from ctc_mpls_label_size_type_e,
                Support 256 mpls interface space, space 0 used as global, space 1-255 for interface.
\p
                User Must set MPLS Profile, at the same time, allocate the interface space by yourself.
                e.g  MPLS_Profile=3K, space0=2K, space1=512,space2=512, allocate as follows:
\b
                p_mpls_info->space_info[0].enable = TRUE;
\p
                p_mpls_info->space_info[0].sizetype = CTC_MPLS_LABEL_NUM_2K_TYPE;
\p
                p_mpls_info->space_info[1].enable = TRUE;
\p
                p_mpls_info->space_info[1].sizetype = CTC_MPLS_LABEL_NUM_512_TYPE;
\p
                p_mpls_info->space_info[2].enable = TRUE;
\p
                p_mpls_info->space_info[2].sizetype = CTC_MPLS_LABEL_NUM_512_TYPE;

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_init(uint8 lchip, ctc_mpls_init_t* p_mpls_info);

/**
 @brief De-Initialize mpls module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the mpls configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mpls_deinit(uint8 lchip);

/**
 @brief Add a mpls entry

 @param[in] lchip    local chip id

 @param[in] p_mpls_ilm Data of the mpls entry

 @remark[D2.TM]  Add a MPLS ilm for every mpls situation.
\p
                In The Module, every ilm is used with an nexthop,the default action for the label is pop.
\b
                Set pop=TRUE when need to deal with inner label & use inner label's nexthop to edit the packet.
\p
                Set decap=TRUE when the inner packet will decap in pop mode,that means if the label is popped and we use.
                    the inner packet(maybe ipv4) to do lookup and forwarding(lookup ipda if it's an ip packet).
\p
                Set model as mpls tunnel mode, from ctc_mpls_tunnel_mode_e and the default mode is uniform.
\b
                In pop mode, if nh isn't be configured, sdk will use a drop nh for solving the attack of too many label
                in one pkt. But user also can configure a loopback nh,by this way, The Module can manage the packet
                    including more than 4 labels.
\b
                Use CTC_MPLS_LABEL_TYPE_NORMAL to add a normal mpls ilm.
\p
                Use CTC_MPLS_LABEL_TYPE_L3VPN to add a l3vpn mpls ilm.
\p
                Use CTC_MPLS_LABEL_TYPE_VPWS to add a vpws mpls ilm.
\p
                Use CTC_MPLS_LABEL_TYPE_VPLS to add a vpls mpls ilm.

  @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_add_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);

/**
 @brief Remove a mpls entry

 @param[in] lchip    local chip id

 @param[in] p_mpls_ilm Data of the mpls entry

 @remark[D2.TM]  Delete a MPLS ilm by space id and mpls label.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_del_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);

/**
 @brief Get information of a mpls ilm entry, inlucde nexthop IDs

 @param[in] lchip    local chip id

 @param[out] nh_id Nexthop id array of the mpls ilm entry and maxinum is CTC_MAX_ECPN,the entrie info of the mpls ilm entry

 @param[in] p_mpls_ilm Index of the mpls ilm entry

 @remark[D2.TM]  Get a MPLS ilm by space id and mpls label, and get the nh_id at the same time.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_get_ilm(uint8 lchip, uint32* nh_id, ctc_mpls_ilm_t* p_mpls_ilm);

/**
 @brief set a mpls entry property

 @param[in] lchip    local chip id

 @param[in] p_mpls_pro Data of the mpls entry

 @remark[D2.TM] Set MPLS properties relating to OAM, QOS or APS

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_set_ilm_property(uint8 lchip, ctc_mpls_property_t* p_mpls_pro);

/**
 @brief add mpls stats

 @param[in] lchip    local chip id

 @param[in] stats_index index of the mpls labe stats to be add

 @remark[D2.TM]  Add MPLS stats by space id and mpls label.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_add_stats(uint8 lchip, ctc_mpls_stats_index_t* stats_index);

/**
 @brief delete mpls stats

 @param[in] lchip    local chip id

 @param[in] stats_index index of the mpls labe stats to be delete

 @remark[D2.TM]  Remove MPLS stats by space id and mpls label.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_remove_stats(uint8 lchip, ctc_mpls_stats_index_t* stats_index);

/**
 @brief Add the l2vpn pw entry

 @param[in] lchip    local chip id

 @param[in] p_mpls_pw Information of l2vpn pw entry

 @remark[D2.TM]  Add a l2vpn MPLS ilm for vpls or vpws.
\p
                Set fid, logic_port & logic_port_type when add a vpls ilm.
\p
                Set pw_nh_id when add a vpws ilm.
\p
                Use CTC_MPLS_L2VPN_VPLS to add a vpls ilm.
\p
                Use CTC_MPLS_L2VPN_VPWS to add a vpws ilm.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_add_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw);

/**
 @brief Remove the l2vpn pw entry

 @param[in] lchip    local chip id

 @param[in] p_mpls_pw Information of l2vpn pw entry

 @remark[D2.TM]  Delete a l2vpn MPLS ilm for vpls or vpws.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_del_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw);

/**
@brief update a mpls entry

@param[in] lchip    local chip id

@param[in] p_mpls_ilm Data of the mpls entry

@remark[D2.TM]  Update all properties of a MPLS ilm.

@return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_update_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);


/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif

