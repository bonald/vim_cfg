/**
 @file ctc_usw_nexthop.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-18

 @version v2.0



Nexthop is a wider concept, which is originated from IP route's nexthop concept. In Centec SDK, Nexthop
contain a set of multiple table, and these table may contain DsFwd,DsMet,DsNexthop,DsL2Edit,DsL3Edit,
and so on. These table determine the packet sent to which a port, and the packet editing before send it
out, and in the different applications, Nexthop contains the different table.
\b
\p
\d  L2UC: DsFwd->DsNexthop
\d  L2MC: DsMet->DsNexthop
\d  IPUC: DsFwd(opt)->DsNexthop
\d  IPMC: DsMet->DsNexthop
\d  MPLS Push Nexthop: DsFwd->DsNexthop->DsL2EditEth ->(DsL3EditMpls, opt)
\d  MPLS PHP Nexthop: DsFwd->DsNexthop->DsL2EditEth
\d  IP tunnel: DsFwd->DsNexthop->DsL2EditEth& DsL3EditTunnel
\d  TRILL: DsFwd->DsNexthop->DsL2EditEth& DsL3EditTrill
\d  ECMP: N * DsFwd  (ECMP member :IPUC/MPLS/IP tunnel)
\d  ILoop: DsFwd
\d  Drop: DsFwd
\d  BypassAll: DsFwd->DsNexthop
\d  STP tunnel nexthop: DsFwd->DsNexthop->DsL2EditEth
\b
\p
There are two types of Nexthop: one is external and created by the user of SDK. A NHID is assigned by
the user when creating the nexthop, the IPUC/MPLS/IP tunnel Nexthop belong to the type, the other is
internal and created by the SDK itself ,and L2UC/L2MC/IPMC Nexthop belong to the type.
\b
\p
Nexthop ID is an important concepts are related to Nexthop. As described above, the Nexthop is a set of
multiple table. The Nexthop ID is used to find the specific set information of the Nexthop. Other than
that, the other important concepts is dsnh_offset, it indicates the offset of the corresponding
DsNexthop entry. The dsnh_offset allocated by SDK internal.

\S ctc_nexthop.h:ctc_nh_reserved_nhid_t
\S ctc_nexthop.h:ctc_nh_upd_type_t
\S ctc_nexthop.h:ctc_nh_ip_tunnel_type_t
\S ctc_nexthop.h:ctc_nh_dscp_select_mode_t
\S ctc_nexthop.h:ctc_nh_flow_label_mode_t
\S ctc_nexthop.h:ctc_mpls_nh_op_t
\S ctc_nexthop.h:ctc_nh_exp_select_mode_t
\S ctc_nexthop.h:ctc_nh_nexthop_mac_param_t
\S ctc_nexthop.h:ctc_ip_nh_param_t
\S ctc_nexthop.h:ctc_ip_tunnel_nh_param_t
\S ctc_nexthop.h:ctc_mpls_nexthop_tunnel_param_t
\S ctc_nexthop.h:ctc_mpls_nexthop_push_param_t
\S ctc_nexthop.h:ctc_mpls_nexthop_pop_param_t
\S ctc_nexthop.h:ctc_mpls_nexthop_param_t
\S ctc_nexthop.h:ctc_loopback_nexthop_param_t
\S ctc_nexthop.h:ctc_rspan_nexthop_param_t
\S ctc_nexthop.h:ctc_misc_nh_param_t
\S ctc_nexthop.h:ctc_mcast_nh_param_group_t
\S ctc_nexthop.h:ctc_vlan_egress_edit_info_t
\S ctc_nexthop.h:ctc_vlan_edit_nh_param_t
\S ctc_nexthop.h:ctc_nh_trill_param_t

*/

#ifndef _CTC_USW_NH_H
#define _CTC_USW_NH_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_stats.h"

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
 @addtogroup nexthop NEXTHOP
 @{
*/

/**
 @brief Initialize the nexthop module

 @param[in] lchip    local chip id

 @param[in] nh_cfg  nexthop module global config

 @remark[D2.TM] Initialize tables and registers structures for Nexthop management functions.
           when nh_cfg is NULL,nexthop will use default global config to initialize sdk.
           By default:
\p
            1) nexthop support 16k extenal nexthop, use nh_cfg.max_external_nhid to change the number of
                extenal nexthop;
\p
            2) dsl2edit & dsL3edtit  & dsnexthop,dsMet will share ram according to FTM profile,you can
              change the configuration according to the following variable :
              nh_cfg.local_met_num/nh_cfg.local_nh_4w_num/nh_cfg.l2_edit_4w_num/nh_cfg.l3_edit_4w_num;
\p
            3) dsnh_offset will be allocated in SDK .you can change nh_cfg.alloc_nh_offset_in_sdk to 0
               and it will be allocated by high-level application;

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nexthop_init(uint8 lchip, ctc_nh_global_cfg_t* nh_cfg);

/**
 @brief De-Initialize nexthop module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the nexthop configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nexthop_deinit(uint8 lchip);

/**********************************************************************************
                      Define l2 ucast nexthop functions
***********************************************************************************/
/**
 @brief This function is to create normal ucast bridge nexthop entry

 @param[in] lchip    local chip id

 @param[in] gport   global port id

 @param[in] nh_type   bridge unicast nexthop sub type to create

 @remark[D2.TM] Add L2UC nexthop with the given gport and nh_type, by default,
            For regular port,the L2UC nexthop have been created in the Port initialization;
            For Linkagg port,the L2UC nexthop have been created when sdk create the linkagg.
            please refer to ctc_linkagg_create() and ctc_port_init(),ctc_port_set_port_en().
 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_l2uc(uint8 lchip, uint32 gport,  ctc_nh_param_brguc_sub_type_t nh_type);

/**
 @brief This function is to delete normal ucast bridge nexthop entry

 @param[in] lchip    local chip id

 @param[in] gport   global port id

 @remark[D2.TM] Delete L2UC nexthop with the given gport,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_l2uc(uint8 lchip, uint32 gport);

/**
 @brief Get ucast nhid by type

 @param[in] lchip    local chip id

 @param[in] gport global port of the system
 @param[in] nh_type   bridge unicast nexthop sub type to create
 @param[out] nhid nexthop ID to get

 @remark[D2.TM] Retrieves L2UC nexthop's NHID of the given gport and subtype.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_get_l2uc(uint8 lchip, uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type, uint32* nhid);

/**********************************************************************************
                      Define nexthop Mac functions
***********************************************************************************/
/**
 @brief Add Next Hop Router Mac

 @param[in] lchip    local chip id

 @param[in] arp_id   ARP id

 @param[in] p_param nexthop MAC information

 @remark[D2.TM]  Add Next Hop Router Mac
 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param);

/**
 @brief Remove Next Hop Router Mac

 @param[in] lchip    local chip id

 @param[in] arp_id   ARP id

 @remark[D2.TM]  Remove Next Hop Router Mac
 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_nexthop_mac(uint8 lchip, uint16 arp_id);

/**
 @brief Update Next Hop Router Mac

 @param[in] lchip    local chip id

 @param[in] arp_id   ARP id

 @param[in] p_new_param   New nexthop MAC information

 @remark[D2.TM]  Update Next Hop Router Mac
 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param);

/**********************************************************************************
                      Define ipuc nexthop functions
***********************************************************************************/
/**
 @brief Create IPUC nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param nexthop parameter used to create this ipuc nexthop

 @remark[D2.TM]  Add IPUC nexthop with the given nhid and Next Hop parameter.
          The Next Hop parameter contains output interface(p_nh_param->oif) and Next Hop Router
          Mac(p_nh_param->mac).Use the CTC_IP_NH_FLAG_UNROV flag ,you can create unresolved nexthop.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_ipuc(uint8 lchip, uint32 nhid, ctc_ip_nh_param_t* p_nh_param);

/**
 @brief Remove IPUC nexthop

 @param[in] lchip    local chip id

 @param[in] nhid Nexthop ID to be removed

 @remark[D2.TM]  Delete IPUC nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_ipuc(uint8 lchip, uint32 nhid);

/**
 @brief Update IPUC nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] p_nh_param nexthop parameter used to update this ipuc nexthop

 @remark[D2.TM]  Delete IPUC nexthop with the given nhid and Next Hop parameter,
          the update type :
\pp
          1) CTC_NH_UPD_UNRSV_TO_FWD update unresolved(drop) nexthop to forwarding nexthop,you can
              update output interface and Next Hop Router Mac.
\pp
          2) CTC_NH_UPD_FWD_TO_UNRSV update forwarding nexthop to unresolved nexthop
\pp
          3) CTC_NH_UPD_FWD_ATTR updated forwarding nexthop attribute, you can update
              output interface and Next Hop Router Mac.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_ipuc(uint8 lchip, uint32 nhid, ctc_ip_nh_param_t* p_nh_param);

/**********************************************************************************
                      Define ip tunnel nexthop functions
***********************************************************************************/
/**
 @brief Create ip tunnel nexthop use to encapsulate the packet to a ip tunnel packet

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param nexthop parameter used to create this ip tunnel nexthop

 @remark[D2.TM]  Add IP Tunnel nexthop with the given nhid and Next Hop parameter.
          The Next Hop parameter contains output interface(p_nh_param->oif) ,Next Hop Router Mac
          (p_nh_param->mac) and  the tunnel information.
          The tunnel types supported IPV4_IN4,IPV6_IN4,IPV4_IN6,IPV6_IN6,GRE_IN4,GRE_IN6,ISATAP,6TO4 ,
          6RD tunnel, and so on.
\b
\p
          After L3 tunnel editing, supports in two ways:
\pp
          1) the encapsulated packet can be forwarded out of the port directly;
\pp
          2) the encapsulated packet should be looped back via egress loopback path to the loopback
             interface where the tunnel packet re-enter the forwarding engine and the outgoing interface
             or port will be decided based on the tunnel header (te new Ip header)in the case,the flag
             CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR is set.
\b
\p
         Use the CTC_IP_NH_FLAG_UNROV flag ,you can create unresolved nexthop.

 @return CTC_E_XXX


*/
extern int32
ctc_usw_nh_add_ip_tunnel(uint8 lchip, uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param);

/**
 @brief Remove ip tunnel nexthop

 @param[in] lchip    local chip id

 @param[in] nhid Nexthop ID to be removed

 @remark[D2.TM]  Delete IP tunnel nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.


 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_ip_tunnel(uint8 lchip, uint32 nhid);

/**
 @brief Update ip tunnel nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] p_nh_param nexthop parameter used to update this ip tunnel nexthop

 @remark[D2.TM]  Update ip tunnel nexthop

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_ip_tunnel(uint8 lchip, uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param);

/**********************************************************************************
                      Define wlan tunnel nexthop functions
***********************************************************************************/
/**
 @brief Create wlan tunnel nexthop use to encapsulate the packet to a wlan tunnel packet

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param nexthop parameter used to create this wlan tunnel nexthop

 @remark[D2.TM]  Add WLAN Tunnel nexthop with the given nhid and Next Hop parameter.
          The Next Hop parameter contains, IP information and  the tunnel information.
\b
\p
          After L3 tunnel editing, supports in two ways:
\pp
          1) the encapsulated packet can be forwarded out of the port directly;
\pp
          2) the encapsulated packet should be looped back via egress loopback path to the loopback
             interface where the tunnel packet re-enter the forwarding engine and the outgoing interface
             or port will be decided based on the tunnel header (te new Ip header)in the case,the flag
             CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR is set.
\b
\p
         Use the CTC_WLAN_NH_FLAG_UNROV flag ,you can create unresolved nexthop.

 @return CTC_E_XXX


*/
extern int32
ctc_usw_nh_add_wlan_tunnel(uint8 lchip, uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param);

/**
 @brief Remove wlan tunnel nexthop

 @param[in] lchip    local chip id

 @param[in] nhid Nexthop ID to be removed

 @remark[D2.TM]  Delete WLAN tunnel nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.


 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_wlan_tunnel(uint8 lchip, uint32 nhid);

/**
 @brief Update wlan tunnel nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] p_nh_param nexthop parameter used to update this wlan tunnel nexthop

 @remark[D2.TM]  Update wlan tunnel nexthop

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_wlan_tunnel(uint8 lchip, uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param);

/**********************************************************************************
                      Define mpls nexthop functions
***********************************************************************************/
/**
 @brief Create a mpls tunnel label

 @param[in] lchip    local chip id
 @param[in] tunnel_id    the id of mpls tunnel
 @param[in] p_nh_param   tunnel label parameter

 @remark[D2.TM] This API is called to establish an mpls tunnel.
    To Create MPLS nexthop ,firstly you must call the function ctc_nh_add_mpls_tunnel_label() to create
       an MPLS Tunnel, and then call the function ctc_nh_add_mpls() to add an mpls Nexthop.
 @return CTC_E_XXX

*/


extern int32
ctc_usw_nh_add_mpls_tunnel_label(uint8 lchip, uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_nh_param);

/**
 @brief remove a mpls tunnel label

 @param[in] lchip    local chip id

 @param[in] tunnel_id    the id of mpls tunnel

 @remark[D2.TM] This API is called to delete an mpls tunnel,
          and you need delete all use the tunnel's MPLS Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_mpls_tunnel_label(uint8 lchip, uint16 tunnel_id);

/**
 @brief update a mpls tunnel label

 @param[in] lchip    local chip id

 @param[in] tunnel_id    the id of mpls tunnel
 @param[in] p_new_param  tunnel label parameter

 @remark[D2.TM] This API is called to update an mpls tunnel.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_mpls_tunnel_label(uint8 lchip, uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_new_param);

/*
 @brief swap two mpls tunnel

 @param[in] lchip    local chip id

 @param[in] old_tunnel_id    the id of old mpls tunnel
 @param[in] new_tunnel_id     the id of new mpls tunnel

 @remark[D2.TM] This API is called to swap two mpls tunnel.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_swap_mpls_tunnel_label(uint8 lchip, uint16 old_tunnel_id,uint16 new_tunnel_id);

/**
 @brief Create a mpls nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param   nexthop parameter used to create this nexthop

 @remark[D2.TM] This API is called to add an mpls nexthop,the nexthop contains the outgoing interface,
            Next Hop router mac and label information (Tunnel label and VC label).
\b
\p
            The mpls nexthop type maybe 3 type:
\p
\d          CTC_MPLS_NH_PUSH_TYPE    indicate it is a mpls push nexthop
\d          CTC_MPLS_NH_POP_TYPE     indicate it is a mpls pop nexthop
\d          CTC_MPLS_NH_SWITCH_TYPE  indicate it is a mpls pop nexthop[HB]
\b
\p
  When aps_en is set,indicate the nexthop's APS is enable, indicate working and protection path's use different VC label)
\p

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_mpls(uint8 lchip, uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param);

/**
 @brief Remove mpls nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be removed

 @remark[D2.TM]  Delete MPLS nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_mpls(uint8 lchip, uint32 nhid);

/**
 @brief Update a mpls unresolved nexthop to forwarded mpls push nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] p_nh_param nexthop parameter used to update this nexthop

 @remark[D2.TM]  Update MPLS nexthop with the given nhid.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_mpls(uint8 lchip, uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param);

/**********************************************************************************
                      Define iloop nexthop functions
***********************************************************************************/
/**
 @brief Create a ipe loopback nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param loopback nexthop parameters

 @remark[D2.TM]  Add ILoop nexthop with the given nhid and parameter.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_iloop(uint8 lchip, uint32 nhid, ctc_loopback_nexthop_param_t* p_nh_param);

/**
 @brief Remove ipe loopback nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be removed

 @remark[D2.TM]  Delete ILoop nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_iloop(uint8 lchip, uint32 nhid);

/**********************************************************************************
                      Define rspan(remote mirror) nexthop functions
***********************************************************************************/
/**
 @brief Create a rspan(remote mirror) nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created
 @param[in] p_nh_param remote mirror nexthop parameters

 @remark[D2.TM]  Add a RSPSN nexthop with the given nhid and parameters.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_rspan(uint8 lchip, uint32 nhid, ctc_rspan_nexthop_param_t* p_nh_param);

/**
 @brief Remove rspan(remote mirror) nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be removed

 @remark[D2.TM]  Delete RSPSN nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_rspan(uint8 lchip, uint32 nhid);

/**********************************************************************************
                      Define ECMP nexthop functions
***********************************************************************************/
/**
 @brief Create a ECMP nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param ecmp nexthop parameters

 @remark[D2.TM]  Add a ECMP nexthop and create a ecmp group.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_ecmp(uint8 lchip, uint32 nhid, ctc_nh_ecmp_nh_param_t* p_nh_param);

/**
 @brief Delete a ECMP nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID of ECMP to be removed

 @remark[D2.TM]  Delete ECMP nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_remove_ecmp(uint8 lchip, uint32 nhid);

/**
 @brief Update a ECMP nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated
 @param[in] p_nh_param ecmp nexthop parameters

 @remark[D2.TM]  Add or Delete ECMP member.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_ecmp(uint8 lchip, uint32 nhid, ctc_nh_ecmp_nh_param_t* p_nh_param);

/**
 @brief Set the maximum ECMP paths allowed for a route

 @param[in] lchip    local chip id

 @param[out] max_ecmp the maximum ECMP paths allowed for a route

 @remark[D2.TM]  Set the maximum ECMP paths allowed for a route ,the default value is CTC_DEFAULT_MAX_ECMP_NUM.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_set_max_ecmp(uint8 lchip, uint16 max_ecmp);

/**
 @brief Get the maximum ECMP paths allowed for a route

 @param[in] lchip    local chip id

 @param[out] max_ecmp the maximum ECMP paths allowed for a route

 @remark[D2.TM]  Get the maximum ECMP paths allowed for a route

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_get_max_ecmp(uint8 lchip, uint16* max_ecmp);

/**********************************************************************************
                      Define advanced vlan/APS  nexthop functions
***********************************************************************************/

/**
 @brief Create Egress Vlan Editing nexthop or APS Egress Vlan Editing nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param  nexthop parameter used to create this nexthop

 @remark[D2.TM]  Add Xlate nexthop with the given nhid and xlate parameter
          it be used to do egress vlan edit for using nexthop.
          generally, it is used to L2 APS Nexthop

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_xlate(uint8 lchip, uint32 nhid, ctc_vlan_edit_nh_param_t* p_nh_param);

/**
 @brief Remove Egress Vlan Editing nexthop or APS Egress Vlan Editing nexthop

 @param[in] lchip    local chip id

 @param[in] nhid      Egress vlan Editing nexthop id or APS Egress vlan Editing nexthop id

 @remark[D2.TM]  Delete Xlate nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_xlate(uint8 lchip, uint32 nhid);

/**********************************************************************************
                      Define stats functions in nexthop
***********************************************************************************/


/**
 @brief Get mcast nexthop id

 @param[in] lchip    local chip id

 @param[in] group_id   mcast group id

 @param[out] p_nhid   get nhid

 @remark[D2.TM]  get mcast nexthop id by mcast group id

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_get_mcast_nh(uint8 lchip, uint32 group_id, uint32* p_nhid);


/**
 @brief Create mcast nexthop

 @param[in] lchip    local chip id

 @param[in] nhid   nexthop ID to be created

 @param[in] p_nh_mcast_group   nexthop parameter used to create this mcast nexthop

 @remark[D2.TM]  Create mcast nexthop

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_add_mcast(uint8 lchip, uint32 nhid, ctc_mcast_nh_param_group_t* p_nh_mcast_group);

/**
 @brief Delete mcast nexthop

 @param[in] lchip    local chip id

 @param[in] nhid   nexthopid

 @remark[D2.TM]  Delete mcast nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_remove_mcast(uint8 lchip, uint32 nhid);

/**
 @brief Add/remove mcast member

 @param[in] lchip    local chip id

 @param[in] nhid              nexthop ID to be updated
 @param[in] p_nh_mcast_group  nexthop parameter used to add/remove  mcast member


 @remark[D2.TM]  Add/remove mcast member

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_update_mcast(uint8 lchip, uint32 nhid, ctc_mcast_nh_param_group_t* p_nh_mcast_group);

/**
 @brief Create misc nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param  nexthop parameter used to create this nexthop

 @remark[D2.TM]  Add some miscellaneous nexthop with the given nhid and miscellaneous parameter.


 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_misc(uint8 lchip, uint32 nhid, ctc_misc_nh_param_t* p_nh_param);

/**
 @brief Remove misc nexthop

 @param[in] lchip    local chip id

 @param[in] nhid     nexthop ID to be removed

 @remark[D2.TM]  Delete misc nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_remove_misc(uint8 lchip, uint32 nhid);

/**
 @brief Update misc nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] p_nh_param nexthop parameter used to update this misc nexthop

 @remark[D2.TM]  Update some miscellaneous nexthop with the given nhid and miscellaneous paramer.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_misc(uint8 lchip, uint32 nhid, ctc_misc_nh_param_t* p_nh_param);

/**
 @brief Get resolved dsnh_offset  by type

 @param[in] lchip    local chip id

 @param[in] type  the type of reserved dsnh_offset

 @param[out] p_offset  Retrieve reserved dsnh_offset

 @remark[D2.TM] Retrieve resolved dsnh_offset  by type

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_get_resolved_dsnh_offset(uint8 lchip, ctc_nh_reserved_dsnh_offset_type_t type, uint32* p_offset);

/**
 @brief Get nexthop information by nhid

 @param[in] lchip    local chip id

 @param[in] nhid  nexthop ID to be get information

 @param[out] p_nh_info  Retrieve Nexthop information

 @remark[D2.TM] Retrieve Nexthop information by nhid,include dsnh_offset & destination gport

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_get_nh_info(uint8 lchip, uint32 nhid, ctc_nh_info_t* p_nh_info);

/**
 @brief Set global config max_ecmp and ecmp_dynamic_mode

 @param[in] lchip    local chip id

 @param[in] p_glb_cfg global config refer to ctc_nh_global_cfg_t

 @remark[D2.TM] Set Nexthop global config,include ecmp loadblance mode, mpls tunnel number etc

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_set_global_config(uint8 lchip, ctc_nh_global_cfg_t* p_glb_cfg);

/**
 @brief This function is to create TRILL tunnel nexthop entry

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be created

 @param[in] p_nh_param nexthop parameter used to create this trill nexthop

 @remark[D2.TM] This API is called to add a trill nexthop, the nexthop contains the outgoing port, nickname etc

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_add_trill(uint8 lchip, uint32 nhid, ctc_nh_trill_param_t* p_nh_param);

/**
 @brief Remove TRILL nexthop

 @param[in] lchip    local chip id

 @param[in] nhid Nexthop ID to be removed

 @remark[D2.TM]  Delete TRILL nexthop with the given nhid,
          and you need to delete all the forwarding tables use this Nexthop before deleting it.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_remove_trill(uint8 lchip, uint32 nhid);

/**
 @brief Update TRILL nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] p_nh_param nexthop parameter used to update this TRILL nexthop

 @remark[D2.TM]  Update TRILL nexthop

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_trill(uint8 lchip, uint32 nhid, ctc_nh_trill_param_t* p_nh_param);

/**
 @brief Add nexthop udf profile

 @param[in] lchip    local chip id

 @param[in] profile parameter

 @remark[TM]  Add udf edit profile

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_add_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit);

/**
 @brief Remove nexthop udf profile

 @param[in] lchip    local chip id

 @param[in] profile ID

 @remark[TM]  Remove udf edit profile

 @return CTC_E_XXX
*/
extern int32
ctc_usw_nh_remove_udf_profile(uint8 lchip, uint8 profile_id);

/**
 @brief Update misc nexthop

 @param[in] lchip    local chip id

 @param[in] nhid nexthop ID to be updated

 @param[in] nexthop parameter used to update this misc nexthop

 @remark[D2.TM]  Update misc nexthop

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_update_misc(uint8 lchip, uint32 nhid, ctc_misc_nh_param_t* p_nh_param);

/**
 @brief traverse mcast nexthop to get mcast member information

 @param[in] fn callback function to deal with all mcast member

 @param[in] p_data data used by the callback function

 @remark[D2.TM]  Traverse mcast nexthop

 @return CTC_E_XXX

*/
extern int32
ctc_usw_nh_traverse_mcast(uint8 lchip, ctc_nh_mcast_traverse_fn fn, ctc_nh_mcast_traverse_t* p_data);

/**@} end of @addtogroup nexthop NEXTHOP*/

#ifdef __cplusplus
}
#endif

#endif /*_CTC_USW_NH_H*/

