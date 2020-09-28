#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_trill_cli.c

 @date 2015-10-26

 @version v3.0

 The file apply clis of trill module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_usw_trill_cli.h"

extern int32
sys_usw_trill_dump_route_entry(uint8 lchip);

CTC_CLI(ctc_cli_usw_trill_dump,
        ctc_cli_usw_trill_dump_cmd,
        "show trill route (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_TRILL_M_STR,
        "Route entry",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_trill_dump_route_entry(lchip);
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


int32
ctc_usw_trill_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_trill_dump_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_trill_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_trill_dump_cmd);

    return CLI_SUCCESS;
}

#endif

