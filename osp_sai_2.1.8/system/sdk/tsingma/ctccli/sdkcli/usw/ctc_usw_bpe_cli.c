#if (FEATURE_MODE == 0)
/**
 @file ctc_greatbel_bpe_cli.c

 @date 2012-04-06

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

int32
sys_usw_bpe_get_port_extend_mcast_en(uint8 lchip, uint8 *p_enable);

int32
sys_usw_bpe_set_port_extend_mcast_en(uint8 lchip, uint8 enable);

CTC_CLI(ctc_cli_usw_bpe_control_bridge_mcast_en,
        ctc_cli_usw_bpe_control_bridge_mcast_en_cmd,
        "bpe control-bridge mcast (enable|disable) (lchip LCHIP|)",
        CTC_CLI_BPE_M_STR,
        "Control bridge",
        "Mcast",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 enable = 0;
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");

    if (index != 0xFF)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_bpe_set_port_extend_mcast_en(lchip, enable);

    if (0 != ret)
    {
        ctc_cli_out("%% set nexthop cb-mcast fail\n");
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_bpe_show_control_bridge_mcast_en,
        ctc_cli_usw_bpe_show_control_bridge_mcast_en_cmd,
        "show bpe control-bridge mcast (enable|disable) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_BPE_M_STR,
        "Control bridge",
        "Mcast",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 enable = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_bpe_get_port_extend_mcast_en(lchip, &enable);

    if (0 != ret)
    {
        ctc_cli_out("%% set bpe control bridge mcast fail\n");
    }
    else
    {
        if (enable)
        {
            ctc_cli_out("%% bpe control bridge mcast enable.\n");
        }
        else
        {
            ctc_cli_out("%% bpe control bridge mcast disable.\n");
        }
    }

    return ret;
}

int32
ctc_usw_bpe_cli_init(void)
{
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_bpe_control_bridge_mcast_en_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_bpe_show_control_bridge_mcast_en_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_bpe_cli_deinit(void)
{
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_bpe_control_bridge_mcast_en_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_bpe_show_control_bridge_mcast_en_cmd);

    return CLI_SUCCESS;
}

#endif

