/**
 @file ctc_dma_cli.c

 @date 2012-3-20

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
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_dma.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_usw_dma_cli.h"

extern int32
sys_usw_dma_show_status(uint8 lchip);
extern int32
sys_usw_dma_show_desc(uint8 lchip, uint8 chan_id, uint32 start_idx, uint32 end_idx);
extern int32
sys_usw_dma_function_pause(uint8 lchip, uint8 chan_id, uint8 en);
extern int32
sys_usw_dma_set_chan_en(uint8 lchip, uint8 chan_id, uint8 chan_en);
extern int32
sys_usw_dma_table_rw(uint8 lchip, uint8 is_read, uint32 addr, uint16 entry_len, uint16 entry_num, uint32* p_value);

extern int32
sys_usw_dma_show_tx_list(uint8 lchip);

extern int32
sys_usw_dma_show_stats(uint8 lchip);

extern int32
sys_usw_dma_clear_pkt_stats(uint8 lchip, uint8 para);
extern int32
sys_usw_dma_set_timer_init_en(uint8 lchip);

int32
ctc_dump8(uintptr addr, uint32 len)
{
    uint8* data = (uint8*)addr;
    uint32 cnt = 0;
    char line[256];
    char tmp[256];

    if (0 == len)
    {
        return CTC_E_NONE;
    }

    for (cnt = 0; cnt < len; cnt++)
    {
        if ((cnt % 16) == 0)
        {
            if (cnt != 0)
            {
                ctc_cli_out("%s", line);
            }

            sal_memset(line, 0, sizeof(line));
            sal_sprintf(tmp, "\n%p:  ", (void*)(addr + cnt));
            if (sal_strlen(tmp) > 32)
            {
                tmp[32] = '\0';
            }
            sal_strcat(line, tmp);
        }

        sal_sprintf(tmp, "%02X", data[cnt]);
        if (sal_strlen(tmp) > 32)
        {
            tmp[32] = '\0';
        }
        sal_strcat(line, tmp);

        if ((cnt % 2) == 1)
        {
            sal_strcat(line, " ");
        }
    }

    ctc_cli_out("%s", line);
    ctc_cli_out("\n");

    return CTC_E_NONE;
}
CTC_CLI(ctc_cli_usw_dma_show_tx_list,
        ctc_cli_usw_dma_show_tx_list_cmd,
        "show dma tx-list (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DMA_M_STR,
        "list of packet need to transmit",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_show_tx_list(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_dma_show_status,
        ctc_cli_usw_dma_show_status_cmd,
        "show dma status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DMA_M_STR,
        "State for global config",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_dma_mem_dump,
        ctc_cli_usw_dma_mem_dump_cmd,
        "dma mem dump addr ADDR len LEN (lchip LCHIP|)",
        "dma",
        "memory",
        "dump",
        "address",
        "address value",
        "dump mem len",
        "len value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uintptr addr = 0;
    uint32 len = 0;
    uint32 index = 0;
    uint8 lchip = 0;

    ctc_cli_out("Memory Dump:\n");

    /* parser fid */
    CTC_CLI_GET_UINT64("address", addr, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    /* parser key index */
    CTC_CLI_GET_UINT32("len", len, argv[1]);

    if (0)
    {
        for (index = 0; index < len; index++)
        {
            ctc_cli_out("%08X   ", *((uint32*)addr+index));

            if (((index+1) %8) == 0)
            {
                ctc_cli_out("\n");
            }
        }
    }
    else
    {
        ctc_dump8(addr, len);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_show_dma_desc,
        ctc_cli_usw_show_dma_desc_cmd,
        "show dma desc (chan CHAN) (start-idx START) (end-idx END) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "DMA module",
        "Descriptor",
        "DMA channel",
        "DMA channel ID",
        "Start desc index",
        "Start value",
        "End desc index",
        "End value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 index = 0;
    uint32 start_idx = 0;
    uint32 end_idx = 0;
    uint8 chan_id = 0;
    int32 ret = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("chan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("chan id", chan_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("start-idx");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("start idx", start_idx, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("end-idx");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("end idx", end_idx, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_show_desc(lchip, chan_id, start_idx, end_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));

        return ret;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_dma_func_pause,
        ctc_cli_usw_dma_func_pause_cmd,
        "dma set (chan CHAN) (pause|resume) (lchip LCHIP|)",
        "DMA module",
        "Set",
        "DMA channel",
        "DMA channel ID",
        "Func Pause",
        "Func Resume",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 is_pause = 0;
    uint8 chan_id = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("chan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("chan id", chan_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pause");
    if (index != 0xFF)
    {
        is_pause = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_function_pause(lchip, chan_id, is_pause);
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_dma_func_en,
        ctc_cli_usw_dma_func_en_cmd,
        "dma set (chan CHAN) (enable|disable) (lchip LCHIP|)",
        "DMA module",
        "Set",
        "DMA channel",
        "DMA channel ID",
        "Func Enable",
        "Func Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 is_en = 0;
    uint8 chan_id = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("chan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("chan id", chan_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        is_en = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_set_chan_en(lchip, chan_id, is_en);
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;
}

#ifdef SDK_IN_USERMODE
CTC_CLI(ctc_cli_usw_dma_tbl_wr,
        ctc_cli_usw_dma_tbl_wr_cmd,
        "dma write table TBL_ADDR entry-len LEN entry-num NUM (lchip LCHIP|)",
        CTC_CLI_DMA_M_STR,
        "Write table",
        "Asic table",
        "Table address",
        "Length of entry, unit:bytes",
        "Value <1-65535>",
        "Length of system buffer",
        "Value <1-65535>",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)

{
    int32 ret       = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;
    uint32 data = 0;
    uint32 buffer_len = 0;
    uint32 addr = 0;
    uint16 entry_len = 0;
    uint16 num = 0;
    uint32* buffer = NULL;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT32_RANGE("table addr", addr, argv[0], 0, CTC_MAX_UINT32_VALUE);

    CTC_CLI_GET_UINT16_RANGE("table entry length", entry_len, argv[1], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_UINT16_RANGE("number of entry", num, argv[2], 0, CTC_MAX_UINT16_VALUE);

    buffer_len = entry_len*num;

    buffer = (uint32*)mem_malloc(MEM_CLI_MODULE, buffer_len);
    if (NULL == buffer)
    {
        ctc_cli_out("%% Alloc  memory  failed \n");
        return CLI_ERROR;
    }

    sal_memset(buffer, 0, buffer_len);

    for(index = 0; index < buffer_len/4; index++)
    {
        ctc_cli_out("Please Input %dst Data:  \n", index);
        scanf("%x", &data);
        buffer[index] = data;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_table_rw(lchip, 0, addr, entry_len, num, buffer);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    mem_free(buffer);
    return ret;
}
#endif

CTC_CLI(ctc_cli_usw_dma_tbl_rd,
        ctc_cli_usw_dma_tbl_rd_cmd,
        "dma read table TBL_ADDR entry-len LEN entry-num NUM (lchip LCHIP|)",
        CTC_CLI_DMA_M_STR,
        "read table",
        "Asic table",
        "Table address",
        "Length of entry, unit:bytes",
        "Value <1-65535>",
        "Number of Entry",
        "Value <1-65535>",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;
    uint16 num = 0;
    uint16 entry_num = 0;
    uint32 buf_len = 0;
    uint32 addr = 0;
    uint16 entry_len = 0;
    uint32* buffer = NULL;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT32_RANGE("table addr", addr, argv[0], 0, CTC_MAX_UINT32_VALUE);

    CTC_CLI_GET_UINT16_RANGE("table entry length", entry_len, argv[1], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_UINT16_RANGE("number of entry", entry_num, argv[2], 0, CTC_MAX_UINT16_VALUE);

    buf_len = (entry_len*entry_num);

    buffer = mem_malloc(MEM_CLI_MODULE, buf_len);
    if (NULL == buffer)
    {
        ctc_cli_out("%% Alloc  memory  failed \n");
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memset(buffer, 0, buf_len);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_dma_table_rw(lchip, 1, addr, entry_len, entry_num, buffer);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("DMA Read Table Result: \n");

    for (index = 0; index < entry_num; index++)
    {
        ctc_cli_out("Entry Index: %d \n", index);
        for (num = 0; num < (entry_len)/4; num++)
        {
            ctc_cli_out("0x%08x  ", buffer[index*(entry_len)/4 + num]);
        }
        ctc_cli_out("\n");
    }

    mem_free(buffer);
    return ret;
}

CTC_CLI(ctc_cli_usw_dma_show_dma_stats,
        ctc_cli_usw_dma_show_dma_stats_cmd,
        "show stats dma (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Statistics",
        CTC_CLI_DMA_M_STR,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_dma_show_stats(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_dma_clear_dma_stats,
        ctc_cli_usw_dma_clear_dma_stats_cmd,
        "clear stats dma (rx | tx |) (lchip LCHIP|)",
        "Clear",
        "Statistics",
        CTC_CLI_DMA_M_STR,
        "Rx",
        "Tx",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 para = 0;

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (0xFF != index)
    {
        para = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tx");
    if (0xFF != index)
    {
        para = 2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_dma_clear_pkt_stats(lchip, para);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_dma_enable_tx_timer,
        ctc_cli_usw_dma_enable_tx_timer_cmd,
        "dma set tx timer enable (lchip LCHIP|)",
        "Dma",
        "Set",
        "Tx",
        "Timer",
        "Enable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_dma_set_timer_init_en(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dma_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_show_tx_list_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_mem_dump_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_dma_desc_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_func_pause_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_func_en_cmd);
#ifdef SDK_IN_USERMODE
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_tbl_wr_cmd);
#endif
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_tbl_rd_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_show_dma_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_dma_clear_dma_stats_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_dma_enable_tx_timer_cmd);

    return 0;
}

int32
ctc_usw_dma_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_show_tx_list_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_mem_dump_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_dma_desc_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_func_pause_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_func_en_cmd);
#ifdef SDK_IN_USERMODE
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_tbl_wr_cmd);
#endif
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_tbl_rd_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_show_dma_stats_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_dma_clear_dma_stats_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_dma_enable_tx_timer_cmd);

    return 0;
}

