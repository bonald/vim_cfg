#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_fcoe.c

 @date 2011-11-07

 @version v2.0


*/
/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"

#include "ctc_fcoe.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_fcoe.h"


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

int32
ctc_usw_fcoe_init(uint8 lchip, void* fcoe_global_cfg)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_fcoe_init(lchip, fcoe_global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_fcoe_deinit(uint8 lchip)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_fcoe_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_fcoe_add_route(uint8 lchip, ctc_fcoe_route_t* p_fcoe_route)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_FCOE);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_fcoe_add_route(lchip, p_fcoe_route));
    }

    /* rollback if error exist */
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_fcoe_remove_route(lchip,p_fcoe_route);
    }

    return ret;
}

int32
ctc_usw_fcoe_remove_route(uint8 lchip, ctc_fcoe_route_t* p_fcoe_route)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_FCOE);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
            ret,
            sys_usw_fcoe_remove_route(lchip,p_fcoe_route));
    }
    return ret;
}

#endif

