#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_wlan.c

 @date 2016-01-23

 @version v2.0


*/
/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"

#include "sys_usw_common.h"
#include "sys_usw_wlan.h"
#include "ctc_usw_wlan.h"

int32
ctc_usw_wlan_init(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    ctc_wlan_global_cfg_t global_cfg;

    LCHIP_CHECK(lchip);
    if (NULL == p_glb_param)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_wlan_global_cfg_t));
        global_cfg.control_pkt_decrypt_en = 1;
        global_cfg.dtls_version = 0xFEFD;
        global_cfg.fc_swap_enable = 1;
        global_cfg.udp_dest_port0 = WLAN_WLAN_CONTROL_L4_PORT;
        global_cfg.udp_dest_port1 = WLAN_WLAN_CONTROL_L4_PORT;
    }
    else
    {
        sal_memcpy(&global_cfg, p_glb_param, sizeof(ctc_wlan_global_cfg_t));
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_init(lchip, &global_cfg));
    }

    return CTC_E_NONE;

}

int32
ctc_usw_wlan_deinit(uint8 lchip)
{
    uint8 lchip_start       = 0;
    uint8 lchip_end         = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_wlan_add_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_wlan_add_tunnel(lchip, p_tunnel_param));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_wlan_remove_tunnel(lchip, p_tunnel_param);
    }

    return ret;
}

int32
ctc_usw_wlan_remove_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_remove_tunnel(lchip, p_tunnel_param));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_wlan_update_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_update_tunnel(lchip, p_tunnel_param));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_wlan_add_client(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_wlan_add_client(lchip, p_client_param));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_wlan_remove_client(lchip, p_client_param);
    }

    return ret;
}

int32
ctc_usw_wlan_remove_client(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_remove_client(lchip, p_client_param));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_wlan_update_client(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_update_client(lchip, p_client_param));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_wlan_set_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_set_crypt(lchip, p_crypt_param));
    }

    return ret;
}

int32
ctc_usw_wlan_get_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_wlan_get_crypt(lchip, p_crypt_param));

    return CTC_E_NONE;
}

int32
ctc_usw_wlan_set_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_set_global_cfg(lchip, p_glb_param));
    }

    return ret;
}

int32
ctc_usw_wlan_get_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_WLAN);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_wlan_get_global_cfg(lchip, p_glb_param));

    return CTC_E_NONE;
}

#endif

