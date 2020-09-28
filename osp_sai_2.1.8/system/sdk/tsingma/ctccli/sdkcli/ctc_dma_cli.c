/**
 @file ctc_dma_cli.c

 @date 2012-07-09

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_const.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_dma.h"


CTC_CLI(ctc_cli_dma_debug_on,
        ctc_cli_dma_debug_on_cmd,
        "debug dma (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "dma module",
        "CTC layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
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
        typeenum = DMA_CTC;

    }
    else
    {
        typeenum = DMA_SYS;

    }

    ctc_debug_set_flag("dma", "dma", typeenum, level, TRUE);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_dma_debug_off,
        ctc_cli_dma_debug_off_cmd,
        "no debug dma (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "DMA Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DMA_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DMA_SYS;
    }

    ctc_debug_set_flag("dma", "dma", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_dma_show_debug,
        ctc_cli_dma_show_debug_cmd,
        "show debug dma (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "DMA Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DMA_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DMA_SYS;
    }

    en = ctc_debug_get_flag("dma", "dma", typeenum, &level);

    ctc_cli_out("DMA:%s debug %s level: %s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

int32
ctc_dma_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_dma_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dma_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dma_show_debug_cmd);

    return 0;
}
