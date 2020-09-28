#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_dot1ae.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-2-4

 @version v2.0

   This file define ctc functions of SDK.
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_usw_dot1ae.h"
#include "sys_usw_dot1ae.h"
#include "sys_usw_common.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @brief  Init security module

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_init(uint8 lchip, void* dot1ae_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_dot1ae_init(lchip));
    }
    return CTC_E_NONE;
}

/**
 @brief  deinit dot1ae module

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_dot1ae_deinit(lchip));
    }
    return CTC_E_NONE;
}
/**
 @brief  Add Dot1AE secure channel

 @param[in] chan_id  chan id

 @param[in] p_chan   chan info

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_add_sec_chan(uint8 lchip_id,  ctc_dot1ae_sc_t* p_chan)
{
    uint8 lchip = lchip_id;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip_id);
    CTC_PTR_VALID_CHECK(p_chan);

    CTC_ERROR_RETURN(sys_usw_dot1ae_add_sec_chan(lchip_id,  p_chan));
    return CTC_E_NONE;
}

/**
 @brief  Remove Dot1AE secure channel

 @param[in] chan_id  chan id

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_remove_sec_chan(uint8 lchip_id,  ctc_dot1ae_sc_t* p_chan)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_dot1ae_remove_sec_chan(lchip_id, p_chan));

    return CTC_E_NONE;
}

/**
 @brief  Update Dot1AE secure channel

 @param[in] chan_id  chan id

 @param[in] p_chan   chan info

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_update_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_dot1ae_update_sec_chan(lchip_id, p_chan));

    return CTC_E_NONE;
}
/**
 @brief  Update Dot1AE secure channel

 @param[in] chan_id  chan id

 @param[in] p_chan   chan info

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_get_sec_chan(uint8 lchip_id, ctc_dot1ae_sc_t* p_chan)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_dot1ae_get_sec_chan(lchip_id, p_chan));
    return CTC_E_NONE;
}
/**
 @brief  Set global configure of Dot1AE

 @param[out] p_glb_cfg  global configure of Dot1AE

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_set_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_dot1ae_set_global_cfg(lchip, p_glb_cfg));
    }
    return CTC_E_NONE;
}

/**
 @brief  Get global configure of Dot1AE

 @param[out] p_glb_cfg  global configure of Dot1AE

 @return CTC_E_XXX

*/
int32
ctc_usw_dot1ae_get_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_dot1ae_get_global_cfg(lchip, p_glb_cfg));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_dot1ae_get_stats(uint8 lchip_id, uint32 chan_id, ctc_dot1ae_stats_t* p_stats)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_dot1ae_get_stats( lchip_id,  chan_id, p_stats));

    return CTC_E_NONE;
}

int32
ctc_usw_dot1ae_clear_stats(uint8 lchip_id, uint32 chan_id, uint8 an)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DOT1AE);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_dot1ae_clear_stats( lchip_id,  chan_id, an));

    return CTC_E_NONE;
}
#endif

