#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_efd.c

 @date 2014-10-28

 @version v3.0

 The file apply APIs to config efd
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_efd.h"
#include "sys_usw_efd.h"
#include "sys_usw_common.h"

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

int32
ctc_usw_efd_set_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_EFD);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_efd_set_global_ctl(lchip, type, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_efd_get_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_EFD);
    LCHIP_CHECK(lchip);

    lchip_start = lchip;
    lchip_end = lchip + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_efd_get_global_ctl(lchip, type, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_efd_register_cb(uint8 lchip, ctc_efd_fn_t callback, void* userdata)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_EFD);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_efd_register_cb(lchip, callback, userdata));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_efd_init(uint8 lchip, void* p_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_efd_init(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_efd_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_efd_deinit(lchip));
    }

    return CTC_E_NONE;
}

#endif

