#if (FEATURE_MODE == 0)
/**
    @file ctc_usw_stacking.h

    @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

    @date 2012-10-19

    @version v2.0

The Module supports following stacking features:
\p

\d  Any-port stacking: each port can work as stacking port include gmac and xgmac,
    supports using uplink aggregation port as stacking port for higher bandwidth and availability.
    ctc_stacking_create_trunk(ctc_stacking_trunk_t* p_trunk) is used to crate uplink trunk group
    and member can be gmac or xgmac

\d  Local switch: supports packet local switching to save the unnecessary bandwidth consuming
    of stacking uplink. For example,  mcast replication is done on local switch

\d  Smart unicast: supports shortest path select for unicast packet, and  supports rapid failover
    after uplink fault detect without flushing FDB. ctc_stacking_add_trunk_rchip(ctc_stacking_trunk_t* p_trunk)
    is used to select one path from the uplink trunk group.

\d  Smart multicast: supports duplicating multicast packet by each node not only the ingress device to save
    the bandwidth of the stacking uplink. CTC_STK_PROP_REMOTE_CHIP_END_BITMAP is set to by api
    ctc_stacking_set_property(ctc_stacking_property_t* p_prop) stop the multicast packet from assign chip.

\d  Flexible stacking topology: supports flexible stacking topology, such as ring, tree, linear and
    full mesh per application scenario.CTC_STK_PROP_FULL_MESH_EN is set to by api
    ctc_stacking_set_property(ctc_stacking_property_t* p_prop) when topology is full meshed

\d  Flexible packet editing: supports both ingress editing and egress editing mode.
    In ingress editing mode, ingress node will edit the most fields in packet while in egress edit modes,
    egress node will edit the most fields in packet.

\d  Could Stacking: supports stacking over L2 or L3 network which enables cloud stacking with distributed devices.
    ctc_stacking_hdr_flag_t control cloud header type.

\S ctc_stacking.h:ctc_stacking_load_mode_t
\S ctc_stacking.h:ctc_stacking_hdr_flag_t
\S ctc_stacking.h:ctc_stacking_prop_type_t
\S ctc_stacking.h:ctc_stacking_hdr_glb_t
\S ctc_stacking.h:ctc_stacking_glb_cfg_t
\S ctc_stacking.h:ctc_stacking_hdr_encap_t
\S ctc_stacking.h:ctc_stacking_extend_t
\S ctc_stacking.h:ctc_stacking_trunk_t
\S ctc_stacking.h:ctc_stacking_keeplive_member_type_t
\S ctc_stacking.h:ctc_stacking_keeplive_t
\S ctc_stacking.h:ctc_stacking_stop_rchip_t
\S ctc_stacking.h:ctc_stacking_mcast_break_point_t
\S ctc_stacking.h:ctc_stacking_property_t

*/

#ifndef _CTC_USW_STK_H
#define _CTC_USW_STK_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_stacking.h"

/**
 @addtogroup stacking STK
 @{
*/

/**
 @brief Create a stacking uplink port trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk     stacking uplink trunk group property

 @remark[D2.TM]  Create a stacking uplink trunk with trunk property, this function only create trunk group, will not add port.
          CTC_STK_LOAD_STATIC: trunk load balance is static like static uplinkagg
          CTC_STK_LOAD_DYNAMIC: trunk load balance is dynamic according to traffic instance as well as Linagg DLB
          ctc_stacking_hdr_flag_t: stacking uplink can connect l2,l3 networks

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_create_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk);

/**
 @brief Destroy a stacking uplink port trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk     stacking uplink trunk group property

 @remark[D2.TM]    Destroy a stacking uplink trunk group with trunk_id

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_destroy_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk);

/**
 @brief Add stacking uplink port to trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk     stacking uplink trunk group property

 @remark[D2.TM]    Add stacking uplink port to trunk group
            gport represent stacking uplink port

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_add_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk);

/**
 @brief Remove stacking uplink port from trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk     stacking uplink trunk group property

 @remark[D2.TM]    Remove stacking uplink port from trunk group
            gport represent stacking uplink port

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_remove_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk);

/**
 @brief Replace stacking uplink ports for the trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk    stacking uplink trunk group property

 @param[in] gports     global port of the member ports which wanted to replace

 @param[in] mem_ports  number of the member ports wanted to replace

 @remark[D2.TM]    Replace stacking trunk group's member ports with the given trunk id and gports.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_replace_trunk_ports(uint8 lchip, ctc_stacking_trunk_t* p_trunk, uint32* gports, uint16 mem_ports);

/**
 @brief Get trunk member ports

 @param[in] lchip_id   local chip id

 @param[in] trunk_id    trunk id

 @param[out] p_gports   member ports array

 @param[out] cnt     member count

 @remark[D2.TM]    Get trunk member ports

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_get_member_ports(uint8 lchip_id, uint8 trunk_id, uint32* p_gports, uint8* cnt);


/**
 @brief Add remote device chipid to trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk     stacking uplink trunk group property

 @remark[D2.TM]    Add stacking remote device chipid to trunk group which used for unicast uplink select
            remote_gchip represent remote device chipid

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_add_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk);

/**
 @brief Remove remote device chipid from trunk group

 @param[in] lchip    local chip id

 @param[in] p_trunk     stacking uplink trunk group property

 @remark[D2.TM]    Remove stacking remote device chipid from trunk group which used for unicast uplink select
            remote_gchip represent remote device chipid

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_remove_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk);


/**
 @brief Get remote device trunk_id from gchip

 @param[in] lchip    local chip id

 @param[out] p_trunk    stacking uplink trunk group property

 @remark[D2.TM]    Get stacking trunk id from remote device chipid which used for unicast uplink select
            remote_gchip represent remote device chipid

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_get_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk);


/**
 @brief Set property of stacking

 @param[in] lchip    local chip id

 @param[in] p_prop   point to stacking property

 @remark[D2.TM]    set property of stacking
            CTC_STK_PROP_REMOTE_CHIP_END_BITMAP:
                 notify from the remote chip mcast packet will be end
            CTC_STK_PROP_BREAK_EN:
                 notify stacking exist break point

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_set_property(uint8 lchip, ctc_stacking_property_t* p_prop);

/**
 @brief Get property of stacking

 @param[in] lchip    local chip id

 @param[in|out] p_prop   point to stacking property

 @remark[D2.TM]    get property of stacking
            CTC_STK_PROP_REMOTE_CHIP_END_BITMAP:
                 notify from the remote chip mcast packet will be end
            CTC_STK_PROP_BREAK_EN:
                 notify stacking exist break point

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stacking_get_property(uint8 lchip, ctc_stacking_property_t* p_prop);

/**
 @brief This function is to create keeplive group for cpu-to-cpu

 @param[in] lchip    local chip id

 @param[in] group_id   keeplive group id

 @remark[D2.TM]  the keeplive group id is the global mcast group id, so the user must maintain the group id
          in case of conflict with mcast group id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stacking_create_keeplive_group(uint8 lchip, uint16 group_id);

/**
 @brief This function is to destroy keeplive group for cpu-to-cpu

 @param[in] lchip    local chip id

 @param[in] group_id   keeplive group id

 @remark[D2.TM]  the keeplive group id is the global mcast group id, so the user must maintain the group id
          in case of conflict with mcast group id.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stacking_destroy_keeplive_group(uint8 lchip, uint16 group_id);

/**
 @brief Add member to cpu-to-cpu keeplive group

 @param[in] lchip    local chip id

 @param[in] p_keeplive   point to keeplive member

 @remark[D2.TM]  cpu-to-cpu keeplive packet can go through from the assing trunk which the member
          trunk need be added. the master cpu port need be add to redirect to cpu.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_keeplive_add_member(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive);

/**
 @brief Remove member to cpu-to-cpu keeplive group

 @param[in] lchip    local chip id

 @param[in] p_keeplive   point to keeplive member

 @remark[D2.TM]  cpu-to-cpu keeplive packet can go through from the assing trunk which the member
          trunk need be added. remove api is used to reselect the path.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_keeplive_remove_member(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive);

/**
 @brief Get members from cpu-to-cpu keeplive group

 @param[in] lchip    local chip id

 @param[out] p_keeplive   point to keeplive member

 @remark[D2.TM]  Get keeplive group members.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_keeplive_get_members(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive);


/**
 @brief This function is to set trunk mcast profile

 @param[in] lchip    local chip id

 @param[in] p_mcast_profile   point to mcast profile

 @remark[D2.TM]  set trunk mcast group  profile which use to replicate packet to remote device

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stacking_set_trunk_mcast_profile(uint8 lchip, ctc_stacking_trunk_mcast_profile_t* p_mcast_profile);

/**
 @brief This function is to get trunk mcast profile

 @param[in] lchip    local chip id

 @param[in] p_mcast_profile  point to mcast profile


 @remark[D2.TM]  get trunk mcast group  profile which use to replicate packet to remote device

 @return CTC_E_XXX

*/
extern int32
ctc_usw_stacking_get_trunk_mcast_profile(uint8 lchip, ctc_stacking_trunk_mcast_profile_t* p_mcast_profile);


/**
 @brief Init stacking module

 @param[in] lchip    local chip id

 @param[in]  p_cfg  point to stacking global config information

 @remark[D2.TM]
            Initialize tables and registers structures for statcking management functions.
\p
            when p_cfg is NULL,stacking module will use default global config to initialize sdk.
\p
            By default:
\p
\d          mac_da_chk_en is initialized as disable and don't do mac da check aginst the packet mac
\d          ether_type_chk_en is initialized as disable and don't do ether type check aginst the packet ethtype
\d          vlan_tpid is initialized as 0x8100
\d          ether_type is initialized as 0x0800
\d          cos is initialized as 7 (high priority)
\d          ip_protocol is initialized as 255
\d          ip_ttl is initialized as 255 (max ttl value)
\d          ip_dscp is initialized as 63 (high priority)
\d          udp_en is initialized as 0  and default not support udp
\d          udp_src_port is initialized as 0x1234
\d          udp_dest_port is initialized as 0x5678


 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_init(uint8 lchip, void* p_cfg);


/**
 @brief De-Initialize stacking module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the stacking configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_stacking_deinit(uint8 lchip);


/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif

