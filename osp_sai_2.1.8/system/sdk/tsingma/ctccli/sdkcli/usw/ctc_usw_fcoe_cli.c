#if (FEATURE_MODE == 0)
/**
 @file ctc_ipfix_cli.c

 @date 2010-06-09

 @version v2.0

 This file defines functions for ptp cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_fcoe_cli.h"
#include "ctc_fcoe.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"


extern int32
sys_usw_fcoe_dump_entry(uint8 lchip, void* user_data);


CTC_CLI(ctc_cli_usw_fcoe_dump,
        ctc_cli_usw_fcoe_dump_cmd,
        "show fcoe route (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_FCOE_M_STR,
        "route info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)

{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_fcoe_dump_entry(lchip, NULL);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/**
 @brief  Initialize sdk fcoe module CLIs
*/
int32
ctc_usw_fcoe_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_fcoe_dump_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_fcoe_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_fcoe_dump_cmd);

    return CLI_SUCCESS;
}

#endif
