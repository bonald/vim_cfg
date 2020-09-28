#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_fcoe.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2015-09-29

 @version v2.0

 The FCOE module supports FCoE(Fiber Channel over Ethernet) routing. It is enabled per port, use
 ctc_port_set_property(gport, CTC_PORT_PROP_FCOE_EN, value).Use ctc_fcoe_add_route() to add
 a fcoe route and use ctc_fcoe_remove_route() to remove a fcoe route. Use ipuc nexthop to set
 the output port and edit macsa and macda address.
 When add and remove fcoe route will also operate RPF check entry.
 Reverse path forwarding check is an additional security function. It is enabled per port, use
 ctc_port_set_property(gport, CTC_PORT_PROP_RPF_EN, value). If RPF is enabled, then only
 those packets whose source FCID addresses and source port is consistent with the routing table
 are forwarded. Feature also supports source FCID addresses match discard and check with macsa.
\b


\S ctc_fcoe.h:ctc_fcoe_flag_e
\S ctc_fcoe.h:ctc_fcoe_route_t
*/

#ifndef _CTC_USW_FCOE_H
#define _CTC_USW_FCOE_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_fcoe.h"

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
 @addtogroup fcoe fcoe
 @{
*/

/**
 @brief Initialize the fcoe module

 @param[in] lchip    local chip id

 @param[in] fcoe_global_cfg  fcoe module global config

 @remark[D2.TM] This function initialize tables and registers structures for fcoe module.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_fcoe_init(uint8 lchip, void* fcoe_global_cfg);

/**
 @brief De-Initialize fcoe module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the fcoe configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_fcoe_deinit(uint8 lchip);

/**
 @brief Add a  fcoe route

 @param[in] lchip    local chip id

 @param[in] p_fcoe_route Data of the fcoe route

 @remark[D2.TM] This function add a fcoe route.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_fcoe_add_route(uint8 lchip, ctc_fcoe_route_t* p_fcoe_route);

/**
 @brief Remove a route entry

 @param[in] lchip    local chip id

 @param[in] p_fcoe_route Data of the fcoe route

 @remark[D2.TM] This function remove a fcoe route.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_fcoe_remove_route(uint8 lchip, ctc_fcoe_route_t* p_fcoe_route);



/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif

