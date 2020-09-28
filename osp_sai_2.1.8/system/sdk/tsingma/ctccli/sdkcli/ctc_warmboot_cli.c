#include "sal.h"
#include "ctc_cli.h"
#include "ctc_api.h"
#include "ctc_cmd.h"
#include "ctc_cli_common.h"


extern int32 ctc_wb_sync(uint8 lchip);
extern int32 ctc_wb_sync_done(uint8 lchip, int32 result);

CTC_CLI(ctc_cli_common_wb_sync,
        ctc_cli_common_wb_sync_cmd,
        "warmboot sync",
        "Warmboot module",
        "Sync up all data")
{
    int32 ret = 0;
    uint8 lchip = 0;

    ret = ctc_wb_sync(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_wb_sync_done,
        ctc_cli_common_wb_sync_done_cmd,
        "warmboot sync-done",
        "Warmboot module",
        "Sync up done")
{
    int32 ret = 0;
    uint8 lchip = 0;

    ret = ctc_wb_sync_done(lchip, 0);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_wb_set_cpu_rx_en,
        ctc_cli_common_wb_set_cpu_rx_en_cmd,
        "warmboot cpu-rx (enable|disable)",
        "Warmboot module",
        "Cpu rx",
        "Enable",
        "Disable")
{
    int32 ret = CLI_SUCCESS;
    bool enable = FALSE;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (INDEX_VALID(index))
    {
        enable = TRUE;
    }

    ret = ctc_wb_set_cpu_rx_en(lchip, enable);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_wb_show_status,
        ctc_cli_common_wb_show_status_cmd,
        "show warmboot status",
        "Show",
        "Warmboot module",
        "Status")
{
    int32 ret = CLI_SUCCESS;
    bool enable = FALSE;
    uint8 lchip = 0;

    if (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_DONE)
    {
        ctc_cli_out("warmboot status : done! \n");
    }
    else if (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        ctc_cli_out("warmboot status : reloading! \n");
    }
    else if (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_SYNC)
    {
        ctc_cli_out("warmboot status : sync! \n");
    }

    ret = ctc_wb_get_cpu_rx_en(lchip, &enable);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("warmboot cpu-rx enable : %d\n", enable?1:0);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_wb_debug_on,
        ctc_cli_common_wb_debug_on_cmd,
        "debug warmboot (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "Warmboot module",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint8 level = CTC_DEBUG_LEVEL_INFO;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    ctc_debug_set_flag("wb", "wb", WB_CTC, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_common_wb_debug_off,
        ctc_cli_common_wb_debug_off_cmd,
        "no debug warmboot",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Warmboot moudle")
{
    uint8 level = 0;

    ctc_debug_set_flag("wb", "wb", WB_CTC, level, FALSE);

    return CLI_SUCCESS;
}

int32
ctc_warmboot_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_common_wb_sync_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_wb_sync_done_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_wb_set_cpu_rx_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_wb_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_wb_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_common_wb_debug_off_cmd); 

    return CTC_E_NONE;
}

