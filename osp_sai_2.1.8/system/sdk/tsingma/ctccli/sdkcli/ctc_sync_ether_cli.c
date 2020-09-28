/**
 @file ctc_sync_ether_cli.c

 @date 2012-10-18

 @version v2.0

 This file defines functions for SyncE cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_sync_ether_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"

CTC_CLI(ctc_cli_sync_ether_debug_on,
        ctc_cli_sync_ether_debug_on_cmd,
        "debug sync-ether (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SYNC_ETHER_M_STR,
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
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

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = SYNC_ETHER_CTC;
    }
    else
    {
        typeenum = SYNC_ETHER_SYS;
    }

    ctc_debug_set_flag("sync_ether", "sync_ether", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_sync_ether_debug_off,
        ctc_cli_sync_ether_debug_off_cmd,
        "no debug sync-ether (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SYNC_ETHER_M_STR,
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = SYNC_ETHER_CTC;
    }
    else
    {
        typeenum = SYNC_ETHER_SYS;
    }

    ctc_debug_set_flag("sync_ether", "sync_ether", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_sync_ether_set_cfg,
        ctc_cli_sync_ether_set_cfg_cmd,
        "sync-ether CHIP clock CLOCK_ID recovered-port LPORT divider DIVIDER {output (enable | disable) | link-status-detect \
        (enable | disable)|}",
        CTC_CLI_SYNC_ETHER_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "SyncE clock",
        "SyncE clock ID",
        "The port used for recover clock",
        "Local port ID",
        "Clock divider",
        "Divider value",
        "Clock output enable or disable",
        "Clock output enable",
        "Clock output disable",
        "Link status detect enable or disable",
        "Link status detect enable",
        "Link status detect disable")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 sync_ether_clock = 0;
    uint8 index = 0;
    ctc_sync_ether_cfg_t sync_ether_cfg;

    sal_memset(&sync_ether_cfg, 0, sizeof(sync_ether_cfg));

    CTC_CLI_GET_UINT8_RANGE("local-chip", lchip, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("clock-id", sync_ether_clock, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("lport", sync_ether_cfg.recovered_clock_lport, argv[2], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("divider", sync_ether_cfg.divider, argv[3], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("output");
    if (index != 0xFF)
    {
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            sync_ether_cfg.clock_output_en = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("link-status-detect");
    if (index != 0xFF)
    {
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            sync_ether_cfg.link_status_detect_en = 1;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_sync_ether_set_cfg(g_api_lchip, sync_ether_clock, &sync_ether_cfg);
    }
    else
    {
        ret = ctc_sync_ether_set_cfg(lchip, sync_ether_clock, &sync_ether_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_sync_ether_show_cfg,
        ctc_cli_sync_ether_show_cfg_cmd,
        "show sync-ether CHIP clock CLOCK_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "SyncE clock",
        "SyncE clock ID")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 sync_ether_clock = 0;
    char* str = NULL;
    ctc_sync_ether_cfg_t sync_ether_cfg;

    sal_memset(&sync_ether_cfg, 0, sizeof(sync_ether_cfg));

    CTC_CLI_GET_UINT8_RANGE("local-chip", lchip, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("clock-id", sync_ether_clock, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_sync_ether_get_cfg(g_api_lchip, sync_ether_clock, &sync_ether_cfg);
    }
    else
    {
        ret = ctc_sync_ether_get_cfg(lchip, sync_ether_clock, &sync_ether_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip         :%d\n", lchip);
    ctc_cli_out("sync-ether clock   :%d\n", sync_ether_clock);
    ctc_cli_out("recovered-port     :%d\n", sync_ether_cfg.recovered_clock_lport);
    ctc_cli_out("divider            :%d\n", sync_ether_cfg.divider);
    str = sync_ether_cfg.clock_output_en ? "enable" : "disable";
    ctc_cli_out("output             :%s\n", str);
    str = sync_ether_cfg.link_status_detect_en ? "enable" : "disable";
    ctc_cli_out("link-status-detect :%s\n", str);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

/**
 @brief  Initialize sdk SyncE module CLIs
*/
int32
ctc_sync_ether_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_sync_ether_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_sync_ether_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_sync_ether_set_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_sync_ether_show_cfg_cmd);

    return CLI_SUCCESS;
}

