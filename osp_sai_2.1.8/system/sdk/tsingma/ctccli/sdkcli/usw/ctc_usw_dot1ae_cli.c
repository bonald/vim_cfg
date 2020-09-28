#if (FEATURE_MODE == 0)
/**
 @file ctc_greatbel_dot1ae_cli.c

 @date 2017-08-22

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_dot1ae_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

extern int32
sys_usw_dot1ae_show_status(uint8 lchip);

CTC_CLI(ctc_cli_usw_show_dot1ae_sec_chan,
        ctc_cli_usw_show_dot1ae_sec_chan_cmd,
        "show dot1ae status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Dot1AE",
        "Dot1AE overall status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8  index = 0;
    uint8  lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_dot1ae_show_status(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_usw_dot1ae_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_dot1ae_sec_chan_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_dot1ae_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_dot1ae_sec_chan_cmd);

    return CLI_SUCCESS;
}

#endif

