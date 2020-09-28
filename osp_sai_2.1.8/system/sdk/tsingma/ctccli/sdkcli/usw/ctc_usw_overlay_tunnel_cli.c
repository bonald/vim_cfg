#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_overlay_tunnel_cli.c

 @date 2013-11-10

 @version v2.0

 The file apply clis of overlay tunnel module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_overlay_tunnel.h"
#include "ctc_usw_overlay_tunnel.h"
#include "ctc_usw_overlay_tunnel_cli.h"

extern int32
sys_usw_overlay_tunnel_show_status(uint8 lchip);

extern int32
sys_usw_overlay_tunnel_cloud_sec_en(uint8 lchip, uint8 enable);

CTC_CLI(ctc_cli_usw_overlay_tunnel_set_global_cfg,
        ctc_cli_usw_overlay_tunnel_set_global_cfg_cmd,
        "overlay-tunnel global-config (nvgre-key | vxlan-key) (ipda-ipsa-vni|ipda-vni) (uc-ipv4da-share|uc-ipv4da-individual) (vni-to-fid | fid-to-vni |)",
        "overlay tunnel module, used to terminate an overlay tunnel",
        "Global config for overlay tunnel",
        "NvGre key",
        "VxLAN/eVxLAN/GENEVE key",
        "Use ipda+ipsa+vni as decap key",
        "Use ipda+vni as decap key",
        "Use share ipda key when ipv4 unicast",
        "Use individual ipda key when ipv4 unicast",
        "vni-to-fid using 1:1 mode",
        "fid-to-vni using N:1 mode")
{
    ctc_overlay_tunnel_global_cfg_t init_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 mode = 0;

    sal_memset(&init_param, 0 ,sizeof(init_param));

    index = CTC_CLI_GET_ARGC_INDEX("ipda-vni");
    if (0xFF != index)
    {
        mode |= CTC_OVERLAY_TUNNEL_DECAP_BY_IPDA_VNI;
    }

    index = CTC_CLI_GET_ARGC_INDEX("uc-ipv4da-individual");
    if (0xFF != index)
    {
       mode |= CTC_OVERLAY_TUNNEL_DECAP_IPV4_UC_USE_INDIVIDUL_IPDA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nvgre-key");
    if (0xFF != index)
    {
        init_param.nvgre_mode = mode;
    }
    else
    {
        init_param.vxlan_mode = mode;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid-to-vni");
    if (0xFF != index)
    {
        init_param.vni_mapping_mode = 1;
    }
    else
    {
        init_param.vni_mapping_mode = 0;
    }

    /* deinit overlay module first and then init this module again */
    if (g_ctcs_api_en)
    {
        ret = ctcs_overlay_tunnel_deinit(g_api_lchip);
        ret = ret ? ret : ctcs_overlay_tunnel_init(g_api_lchip, &init_param);
    }
    else
    {
        ret = ctc_overlay_tunnel_deinit();
        ret = ret ? ret : ctc_overlay_tunnel_init(&init_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_overlay_tunnel_show_status,
        ctc_cli_overlay_tunnel_show_status_cmd,
        "show overlay-tunnel status (lchip LCHIP|)",
        "Show",
        "overlay tunnel module, used to terminate an overlay tunnel",
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;

    ret = sys_usw_overlay_tunnel_show_status(lchip);

    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_overlay_tunnel_could_sec_en,
        ctc_cli_usw_overlay_tunnel_could_sec_en_cmd,
        "overlay-tunnel could-sec (enable|disable) (lchip LCHIP|)",
        "Overlay tunnel module",
        "Cloud security",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 enable = 0;
    uint8 index = 0;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_overlay_tunnel_cloud_sec_en(lchip, enable);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_usw_overlay_tunnel_cli_init(void)
{
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_overlay_tunnel_set_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_show_status_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_overlay_tunnel_could_sec_en_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_overlay_tunnel_cli_deinit(void)
{
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_overlay_tunnel_set_global_cfg_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_show_status_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_overlay_tunnel_could_sec_en_cmd);

    return CLI_SUCCESS;
}

#endif

