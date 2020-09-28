#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_overlay_tunnel.h

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-25

 @version v2.0


\p
 The following is a summary of the supported overlay include NvGRE/VxLAN features.

\p
1)Support Tunnel Transit mode of operation
\d Packets are forwarded via normal L3 routing.
\d Packets can be load balanced using ECMP. NvGRE/VxLAN packet fields can be used in the load balancing computation.

\p
2)Support Gateway Mode of operation
\d Performs layer 2 switching between NvGRE/VxLAN and non-NvGRE/VxLAN device
\d NvGRE/VxLAN mapping and switching among different NvGRE/VxLAN network
\dd Terminating NvGRE/VxLAN Tunnel and perform L2 forwarding lookups based on the VN_ID of the incoming NvGRE/VxLAN network.
\dd Mapping the VN_ID to VN_ID and initating a new NvGRE/VxLAN network.

\p
3)Provides a full L2 lookup at the end of the TEP based on the inner L2 header to determine the
destination port or ports. This includes L2 lookups, L2 learning, and L2 unciast/multicast to learning of
MAC addresses on each logic_src_port within a Virtual Forwarding Instance and switch between them.
\d Support IPv4 unicast and multicast tunnels.
\d Support Mapping of multiple VN_IDs to one tunnel.
\d Support Load balancing of NvGRE/VxLAN packets as a gateway
\d Support for point-to-point(E-Line) Layer 2 virtual circuits that allows extension of a LAN segment across Layer 3 networks.
\d Support for point-to-multi-point (E-LAN) layer 2 virtual circuits that allows extension of a LAN segment across layer 3 networks among multiple users

\b
\p
The following is associate API supported NvGRE/VxLAN features.
\p
1) Create relation between VN_ID and fid (forwarding id)
\pp
Mapping VN_ID to fid(forwarding id) through API:
\pp
 ctc_overlay_tunnel_set_fid();

\p
2) Initiate the  NvGRE/VxLAN tunnel
\pp
support multiple access type, port/port+vlan/port+mac+vlan/vlan through API:
\pp
 ctc_vlan_add_vlan_mapping();

\p
3) Terminate the  NvGRE/VxLAN tunnel
\pp
Terminating NvGRE/VxLAN tunnel through API:
\pp
 ctc_overlay_tunnel_add_tunnel();
\pp
For E-LINE, CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID should be set, For E-LAN, it is default action, dst_vn_id should be used.

\S ctc_overlay_tunnel.h:ctc_overlay_tunnel_flag_t
\S ctc_overlay_tunnel.h:ctc_overlay_tunnel_param_t

*/

#ifndef _CTC_USW_OVERLAY_TUNNEL_H
#define _CTC_USW_OVERLAY_TUNNEL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_overlay_tunnel.h"

/**
 @brief Init overlay tunnel module

 @param[in] lchip    local chip id

 @param[in] param  point to init paramete

 @remark[D2.TM]
  init overlay include NvGRE/VxLAN features
 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_init(uint8 lchip, void* param);

/**
 @brief De-Initialize overlay_tunnel module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the overlay_tunnel configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_overlay_tunnel_deinit(uint8 lchip);


/**
 @brief add the vn_id , fid relation of an overlay network instance

 @param[in] lchip    local chip id

 @param[in] vn_id  vn_id value

 @param[in] fid  maping the value of fid, 0 means clear the mapping

 @remark[D2.TM]  maping vn_id to fid which used to forwarding lookup

 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_set_fid(uint8 lchip, uint32 vn_id, uint16 fid);

/**
 @brief get the vn_id , fid relation of an overlay network instance, only used for vn_id to fid that is 1:1 mode

 @param[in] lchip    local chip id

 @param[in] vn_id  vn_id value

 @param[out] p_fid get the value of fid

 @remark[D2.TM]  get maping vn_id to fid which used to forwarding lookup


 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_get_fid(uint8 lchip, uint32 vn_id, uint16* p_fid);

/**
 @brief get the vn_id , fid relation of an overlay network instance

 @param[in] lchip    local chip id

 @param[in] fid  fid value

 @param[out] p_vn_id get the value of vnid

 @remark[D2.TM]  get maping fid to vn_id which used to forwarding lookup


 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_get_vn_id(uint8 lchip,  uint16 fid, uint32* p_vn_id);


/**
 @brief add overlay network tunnel decapsulation function

 @param[in] lchip    local chip id

 @param[in] p_tunnel_param the property of decap

 @remark[D2.TM] terminating the NvGRE/VxLAN tunnel, default is E-LAN,
         for E-LINE, CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID should be set.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_add_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param);



/**
 @brief remove overlay network tunnel decapsulation function

 @param[in] lchip    local chip id

 @param[in] p_tunnel_param the property of decap

 @remark[D2.TM] remove the specific overlay tunnel

 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_remove_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param);

/**
 @brief update overlay network tunnel decapsulation function

 @param[in] lchip    local chip id

 @param[in] p_tunnel_param the property of decap

 @remark[D2.TM] update the specific overlay tunnel

 @return CTC_E_XXX

*/
extern int32
ctc_usw_overlay_tunnel_update_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param);


#ifdef __cplusplus
}
#endif

#endif /*end of _CTC_USW_OVERLAY_TUNNEL_H*/

#endif

