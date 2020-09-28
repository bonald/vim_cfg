
/**
 @file ctc_greatbel_wlan_cli.c

 @date 2011-11-25

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_wlan_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

extern int32
sys_usw_wlan_show_status(uint8 lchip, uint8 type, uint8 param);

CTC_CLI(ctc_cli_wlan_show_tunnel_client,
        ctc_cli_wlan_show_tunnel_client_cmd,
        "show wlan (tunnel (network|bssid|bssid-rid|all)|client (local|remote)) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_WLAN_M_STR,
        "tunnel",
        "network tunnel",
        "bssid tunnel",
        "bssid+rid tunnel",
        "all tunnel",
        "client",
        "Local client",
        "Remote client",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 tunnel_type = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    
    index = CTC_CLI_GET_ARGC_INDEX("tunnel");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("network");
        if (index != 0xFF)
        {
            tunnel_type = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("bssid");
        if (index != 0xFF)
        {
            tunnel_type = 2;
        }
        index = CTC_CLI_GET_ARGC_INDEX("bssid-rid");
        if (index != 0xFF)
        {
            tunnel_type = 3;
        }
        ret = sys_usw_wlan_show_status(lchip, tunnel_type, 1);
    }
    
    index = CTC_CLI_GET_ARGC_INDEX("client");
    if (index != 0xFF)
    {
        if (CLI_CLI_STR_EQUAL("local", (index+1)))
        {
            ret = sys_usw_wlan_show_status(lchip, 1, 0);
        }
        else
        {
            ret = sys_usw_wlan_show_status(lchip, 0, 0);
        }       
    }
    
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_wlan_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_show_tunnel_client_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_wlan_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_wlan_show_tunnel_client_cmd);

    return CLI_SUCCESS;
}

