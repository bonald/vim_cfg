/**
 @file ctc_usw_diag.h

 @author  Copyright (C) 2019 Centec Networks Inc.  All rights reserved.

 @date 2019-04-01

 @version v2.0

 This module provides various functions to diagnosis chip.

*/

#ifndef _CTC_USW_DIAG_H
#define _CTC_USW_DIAG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_diag.h"

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
 @addtogroup diag DIAG
 @{
*/

/**
 @brief Init diag module

 @param[in] lchip    local chip id

 @param[in]  init_cfg  diag global config information

 @remark[TM] Initialize the module diag, including global variable, soft table, asic table, etc.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_diag_init(uint8 lchip, void* init_cfg);

/**
 @brief De-Initialize chip module

 @param[in] lchip    local chip id

 @remark[TM]  User can de-initialize the diag configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_diag_deinit(uint8 lchip);

/**
 @brief The function is to trigger packet trace

 @param[in] lchip_id    local chip id

 @param[in|out] p_trace  trace confition

 @remark[TM]   To trigger packet trace with the given watch point and watch key.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_diag_trigger_pkt_trace(uint8 lchip_id, ctc_diag_pkt_trace_t* p_trace);

/**
 @brief The function is to get packet trace result info

 @param[in] lchip_id    local chip id

 @param[in|out] p_rslt  trace reault info

 @remark[TM]   To get packet trace result info with the given watch point.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_diag_get_pkt_trace(uint8 lchip_id, ctc_diag_pkt_trace_result_t* p_rslt);

/**
 @brief The function is to get chip droped info

 @param[in] lchip_id    local chip id

 @param[in|out] p_drop  drop info

 @remark[TM]   To get chip droped info with the given condition.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_diag_get_drop_info(uint8 lchip_id, ctc_diag_drop_t* p_drop);

/**
 @brief The function is to set diag module property

 @param[in] lchip_id    local chip id

 @param[in] prop    diag property

 @param[in] p_value        diag property value

 @remark[TM]  This API provides a ability to control the various features.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_diag_set_property(uint8 lchip_id, ctc_diag_property_t prop, void* p_value);

/**
 @brief The function is to get diag module property

 @param[in] lchip_id    local chip id

 @param[in] prop    diag property

 @param[in] p_value        diag property value

 @remark[TM]  This API provides a ability to obtain the various features' value.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_diag_get_property(uint8 lchip_id, ctc_diag_property_t prop, void* p_value);

/**@} end of @addtogroup diag DIAG*/

#ifdef __cplusplus
}
#endif

#endif  /*_CTC_USW_DIAG_H*/

