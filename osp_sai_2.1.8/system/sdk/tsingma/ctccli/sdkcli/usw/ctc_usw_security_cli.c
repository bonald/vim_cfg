/**
 @file ctc_greatbel_security_cli.c

 @date 2011-11-25

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_security_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

extern int32
sys_usw_mac_limit_set_mode(uint8 lchip, uint8 mode);

extern int32
sys_usw_security_show_storm_ctl(uint8 lchip, uint8 op, uint16 gport_or_vlan);


CTC_CLI(ctc_cli_usw_security_mac_limit_set_mode,
        ctc_cli_usw_security_mac_limit_set_mode_cmd,
        "security learn-limit mode (hw |sw) (lchip LCHIP|)",
        CTC_CLI_SECURITY_STR,
        "Learn limit",
        "Learn limit mode",
        "Hardware mode",
        "Software mode",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 mode = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("hw");
    if(0xFF != index)
    {
        mode = 0;
    }
    else
    {
        mode = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_limit_set_mode(lchip, mode);
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_security_show_storm_ctl,
        ctc_cli_usw_security_show_storm_ctl_cmd,
        "show security storm-ctl (port GPHYPORT_ID| vlan VLAN_ID) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Storm control",
        "Port configure",
        CTC_CLI_GPORT_DESC,
        "Vlan based Storm control",
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 op = 0;
    uint16 gport_vlan = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", gport_vlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        op = CTC_SECURITY_STORM_CTL_OP_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan id", gport_vlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        op = CTC_SECURITY_STORM_CTL_OP_VLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_security_show_storm_ctl(lchip, op, gport_vlan);
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_usw_security_cli_init(void)
{
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_security_mac_limit_set_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_security_show_storm_ctl_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_security_cli_deinit(void)
{
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_security_mac_limit_set_mode_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_security_show_storm_ctl_cmd);
	
    return CLI_SUCCESS;
}

