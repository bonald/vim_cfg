#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file ctc_usw_ipuc.c

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

#include "ctc_ipuc.h"
#include "sys_usw_common.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_rpf_spool.h"
#include "sys_usw_l3if.h"
#include "sys_usw_chip.h"
#include "sys_usw_ipuc.h"
#include "sys_usw_ip_tunnel.h"

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
ctc_usw_ipuc_init(uint8 lchip, void* ipuc_global_cfg)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    ctc_ipuc_global_cfg_t global_cfg;

    LCHIP_CHECK(lchip);
    if (NULL == ipuc_global_cfg)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_ipuc_global_cfg_t));
        global_cfg.use_hash8 = 1;
    }
    else
    {
        sal_memcpy(&global_cfg, ipuc_global_cfg, sizeof(ctc_ipuc_global_cfg_t));
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipuc_init(lchip, &global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipuc_deinit(uint8 lchip)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipuc_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipuc_add(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_ipuc_add(lchip, p_ipuc_param));
    }

    /* rollback if error exist */
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_ipuc_remove(lchip,p_ipuc_param);
    }

    return ret;
}

int32
ctc_usw_ipuc_remove(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
            ret,
            sys_usw_ipuc_remove(lchip,p_ipuc_param));
    }
    return ret;
}

int32
ctc_usw_ipuc_get(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipuc_get(lchip, p_ipuc_param));

    return CTC_E_NONE;
}

int32
ctc_usw_ipuc_add_nat_sa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_ip_tunnel_add_natsa(lchip, p_ipuc_nat_sa_param));
    }

    /* rollback if error exist */
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_ip_tunnel_remove_natsa(lchip, p_ipuc_nat_sa_param);
    }

    return ret;
}

int32
ctc_usw_ipuc_remove_nat_sa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_ip_tunnel_remove_natsa(lchip, p_ipuc_nat_sa_param));
    }

    return ret;
}

int32
ctc_usw_ipuc_set_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8 hit)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_ip_tunnel_set_natsa_entry_hit(lchip, p_ipuc_nat_sa_param, hit));
    }

    return ret;
}

int32
ctc_usw_ipuc_get_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8* hit)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_ip_tunnel_get_natsa_entry_hit(lchip, p_ipuc_nat_sa_param, hit));
    }

    return ret;
}


int32
ctc_usw_ipuc_traverse(uint8 lchip, uint8 ip_ver, ctc_ipuc_traverse_fn fn, void* data)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    return sys_usw_ipuc_traverse(lchip, ip_ver, fn, data);
}

int32
ctc_usw_ipuc_add_default_entry(uint8 lchip, uint8 ip_ver, uint32 nh_id)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipuc_add_default_entry(lchip, ip_ver, nh_id));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_ipuc_add_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ipuc_tunnel_param)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IP_TUNNEL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_ip_tunnel_add_tunnel(lchip, p_ipuc_tunnel_param));
    }

    /* rollback if error exist */
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_ip_tunnel_remove_tunnel(lchip, p_ipuc_tunnel_param);
    }

    return ret;
}

int32
ctc_usw_ipuc_remove_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ipuc_tunnel_param)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IP_TUNNEL);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
            ret,
            sys_usw_ip_tunnel_remove_tunnel(lchip, p_ipuc_tunnel_param));
    }

    return ret;
}

int32
ctc_usw_ipuc_set_global_property(uint8 lchip, ctc_ipuc_global_property_t* p_global_prop)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipuc_set_global_property(lchip, p_global_prop));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipuc_arrange_fragment(uint8 lchip, void* p_arrange_info)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipuc_arrange_fragment(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipuc_set_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8 hit)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipuc_set_entry_hit(lchip, p_ipuc_param, hit));
    }
    return CTC_E_NONE;

}

int32
ctc_usw_ipuc_get_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8* hit)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPUC);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipuc_get_entry_hit(lchip, p_ipuc_param, hit));

    return CTC_E_NONE;
}

#endif

