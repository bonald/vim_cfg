#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_npm_cli.c

 @date 2012-12-6

 @version v2.0

 The file apply clis of ipuc module
*/

#include "sal.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_npm_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_npm.h"

#define CTC_CLI_NPM_M_STR "NPM Module"

extern int32
sys_usw_npm_show_status(uint8 lchip);


CTC_CLI(ctc_cli_usw_npm_show_status,
        ctc_cli_usw_npm_show_status_cmd,
        "show npm status (lchip LCHIP_ID|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NPM_M_STR,
        "NPM status",
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_npm_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}



int32
ctc_usw_npm_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_npm_show_status_cmd);
    return CLI_SUCCESS;
}

int32
ctc_usw_npm_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_npm_show_status_cmd);
    return CLI_SUCCESS;
}

#endif

