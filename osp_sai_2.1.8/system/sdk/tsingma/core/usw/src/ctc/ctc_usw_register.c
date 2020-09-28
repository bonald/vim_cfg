/**
 @file ctc_usw_register.c

 @date 2009-11-6

 @version v2.0

 The file apply APIs to initialize common register of humber
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_usw_register.h"
#include "sys_usw_register.h"
#include "sys_usw_common.h"

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

/**
 @brief  Set global control info

 @param[in] type    a type of global control

 @param[in] value   the value to be set

 @return CTC_E_XXX

*/
int32
ctc_usw_global_ctl_set(uint8 lchip, ctc_global_control_type_t type, void* value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_chip                 = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_REGISTER);
    LCHIP_CHECK(lchip);

    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_global_ctl_set(lchip, type, value));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get global control info

 @param[in] type    a type of global control

 @param[out] value   the value to be get

 @return CTC_E_XXX

*/
int32
ctc_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_REGISTER);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_global_ctl_get(lchip, type, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_register_init(uint8 lchip, void* p_global_cfg)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_register_init(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_register_deinit(uint8 lchip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_register_deinit(lchip));
    }

    return CTC_E_NONE;
}

