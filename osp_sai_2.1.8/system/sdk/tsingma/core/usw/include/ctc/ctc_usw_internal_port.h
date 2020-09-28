/**
 @file ctc_usw_internal_port.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-03-29

 @version v2.0

   The module apply APIs to initialize, allocation, release internal port.
*/

#ifndef _CTC_USW_INTERNAL_PORT_H_
#define _CTC_USW_INTERNAL_PORT_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_internal_port.h"

/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @addtogroup internal_port INTERNAL_PORT
 @{
*/

/**
 @brief     Set internal port for special usage

 @param[in] lchip    local chip id

 @param[in] port_assign  Internal port assign parameters

 @remark[D2.TM]    N/A

 @return    CTC_E_XXX

*/
extern int32
ctc_usw_set_internal_port(uint8 lchip, ctc_internal_port_assign_para_t* port_assign);

/**
 @brief     Allocate internal port for special usage

 @param[in] lchip    local chip id

 @param[in] port_assign  Internal port assign parameters

 @remark[D2.TM]    N/A

 @return    CTC_E_XXX

*/
extern int32
ctc_usw_alloc_internal_port(uint8 lchip, ctc_internal_port_assign_para_t* port_assign);

/**
 @brief     Release internal port.

 @param[in] lchip    local chip id

 @param[in] port_assign  Internal port assign parameters

 @remark[D2.TM]    N/A

 @return    CTC_E_XXX

*/
extern int32
ctc_usw_free_internal_port(uint8 lchip, ctc_internal_port_assign_para_t* port_assign);

/**
 @brief     Initialize internal port.

 @param[in] lchip    local chip id

 @param[in] p_global_cfg  point to init global configure

 @remark[D2.TM]    N/A

 @return    CTC_E_XXX

*/
extern int32
ctc_usw_internal_port_init(uint8 lchip, void* p_global_cfg);


/**
 @brief De-Initialize internal_port module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the internal_port configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_internal_port_deinit(uint8 lchip);

/**@} end of @addtogroup internal_port INTERNAL_PORT  */

#ifdef __cplusplus
}
#endif

#endif

