#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_trill.c

 @date 2015-10-26

 @version v3.0

*/
/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"

#include "ctc_trill.h"
#include "sys_usw_trill.h"
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


int32
ctc_usw_trill_init(uint8 lchip, void* trill_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_trill_init(lchip, trill_global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_trill_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_trill_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_trill_add_route(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_TRILL);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
                                 ret,
                                 sys_usw_trill_add_route(lchip, p_trill_route));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_trill_remove_route(lchip, p_trill_route);
    }

    return ret;
}

int32
ctc_usw_trill_remove_route(uint8 lchip, ctc_trill_route_t* p_trill_route)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_TRILL);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_trill_remove_route(lchip, p_trill_route));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_trill_add_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_TRILL);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
                                 ret,
                                 sys_usw_trill_add_tunnel(lchip, p_trill_tunnel));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_trill_remove_tunnel(lchip, p_trill_tunnel);
    }

    return ret;
}

int32
ctc_usw_trill_remove_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_TRILL);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_trill_remove_tunnel(lchip, p_trill_tunnel));
    }
    return CTC_E_NONE;
}


int32
ctc_usw_trill_update_tunnel(uint8 lchip, ctc_trill_tunnel_t* p_trill_tunnel)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_TRILL);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_trill_update_tunnel(lchip, p_trill_tunnel));
    }
    return CTC_E_NONE;
}

#endif

