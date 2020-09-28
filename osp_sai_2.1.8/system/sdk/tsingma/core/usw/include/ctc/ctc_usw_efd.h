#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_efd.h

 @author Copyright (C) 2014 Centec Networks Inc.  All rights reserved.

 @date 2014-10-28

 @version v3.0

 The Module support the new feature EFD(Elephant Flow Detect). The EFD function is useful to detect
 elephant flow. Than you can do following things on the elephant flow.
\d   Use ACL to redirect the flow or modify the flow's priority and color.
\d   Do ECMP DLB(dynamic load balance) on elephant flow, make the DLB more efficient.
\d   Do IPFIX on elephant flow, make the IPFIX more efficient.
\p
 Here are 2K elephant flow hash tables. Use ctc_parser_set_flow_hash_field() to set flow hash key.
 You also can set inner hash key for tunnel packet.
\p
 To enable EFD, use ctc_port_set_property(gport, CTC_PORT_PROP_EFD_EN, value). It is enabled per port.
\p
 By default, a flow which bigger than about 8M will be detected to be elephant flow. Than a packet of
 the flow will be sent to CPU. When the flow is changed to small than about 8M, after a period, the
 elephant flow will be aginged. The aging info will be sent to CPU by DMA. You can use
 ctc_efd_register_cb() to register a callback function to get the aging info.
*/

#ifndef _CTC_USW_EFD_H
#define _CTC_USW_EFD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_efd.h"

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
 @addtogroup efd EFD
 @{
*/

/**
 @brief  Set efd global control info

 @param[in] lchip    local chip id

 @param[in] type    a type of global control

 @param[in] value   the value to be set

 @remark[D2.TM] This function set the global control register of EFD.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_efd_set_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value);

/**
 @brief  Get global control info

 @param[in] lchip    local chip id

 @param[in] type    a type of global control

 @param[out] value   the value to be get

 @remark[D2.TM] This function get the global control register of EFD.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_efd_get_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value);

/**
 @brief  Register efd callback function

 @param[in] lchip    local chip id

 @param[in] callback    callback function

 @param[in] userdata    user data

 @remark[D2.TM] This function register the callback function for flow aging.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_efd_register_cb(uint8 lchip, ctc_efd_fn_t callback, void* userdata);

/**
 @brief  efd init

 @param[in] lchip    local chip id

 @param[in] p_global_cfg    init global config

 @remark[D2.TM] This function initialize tables and registers structures for EFD module.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_efd_init(uint8 lchip, void* p_global_cfg);

/**
 @brief De-Initialize efd module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the efd configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_efd_deinit(uint8 lchip);

/**@} end of @addgroup   */

#ifdef __cplusplus
}
#endif

#endif

#endif

