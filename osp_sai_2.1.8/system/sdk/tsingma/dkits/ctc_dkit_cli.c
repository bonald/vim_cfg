#include "sal.h"
#include "dal.h"

#include "ctc_cli.h"
#include "ctc_dkit_cli.h"
#include "ctc_dkit.h"
#include "ctc_dkit_api.h"
#include "ctc_dkit_common.h"

#define CTC_DKIT_INIT_CHECK() \
    do { \
        CTC_DKIT_LCHIP_CHECK(lchip); \
        do { \
            if (NULL == g_dkit_api[lchip]){ \
                CTC_DKIT_PRINT("dkits api not install, lchip id %d\n", lchip);\
                return CLI_ERROR; } \
        } while (0); \
    } while (0)

#define CTC_DKIT_NOT_SUPPORT() \
    do { \
            CTC_DKIT_PRINT("Function not support in this chip!!!\n"); \
    } while (0)


ctc_dkit_api_t* g_dkit_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM] = {NULL};
ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM] = {NULL};
bool dkits_debug = FALSE;
sal_file_t dkits_log_file = NULL;
static ctc_dkit_memory_para_t memory_para_com;

#ifdef GREATBELT
extern int32 ctc_greatbelt_dkit_init(uint8 lchip);
extern int32 ctc_greatbelt_dkit_internal_cli_init(uint8 cli_tree_mode);
extern int32 ctc_greatbelt_dkit_internal_cli_deinit(uint8 cli_tree_mode);
#endif
#ifdef GOLDENGATE
extern int32 ctc_goldengate_dkit_init(uint8 lchip);
extern int32 ctc_goldengate_dkit_internal_cli_init(uint8 cli_tree_mode);
extern int32 ctc_goldengate_dkit_internal_cli_deinit(uint8 cli_tree_mode);
#endif
#if defined(DUET2) || defined(TSINGMA)
extern int32 ctc_usw_dkit_init(uint8 lchip);
extern int32 ctc_usw_dkit_internal_cli_init(uint8 lchip, uint8 cli_tree_mode);
extern int32 ctc_usw_dkit_internal_cli_deinit(uint8 lchip, uint8 cli_tree_mode);
#endif

extern uint8 ctc_get_chip_type();
extern uint8 ctcs_get_chip_type();
extern int32 dal_get_chip_number(uint8* p_num);

CTC_CLI(cli_dkit_show_version,
        cli_dkit_show_version_cmd,
        "show version",
        CTC_CLI_SHOW_STR,
        "DKits version")
{
    char chipname[20] = {0};
    uint32 dev_id = 0;

    g_ctcs_api_en ? dal_get_chip_dev_id(g_api_lchip, &dev_id) : dal_get_chip_dev_id(0, &dev_id);

    switch(dev_id)
    {
        case DAL_HUMBER_DEVICE_ID:
            sal_strcpy(chipname, "humber");
            break;
        case DAL_GREATBELT_DEVICE_ID:
            sal_strcpy(chipname, "greatbelt");
            break;
        case DAL_GOLDENGATE_DEVICE_ID:
        case DAL_GOLDENGATE_DEVICE_ID1:
            sal_strcpy(chipname, "goldengate");
            break;
        case DAL_DUET2_DEVICE_ID:
            sal_strcpy(chipname, "duet2");
            break;
        case DAL_TSINGMA_DEVICE_ID:
            sal_strcpy(chipname, "tsingma");
            break;
        default:
            return CLI_ERROR;
            break;
    }
    CTC_DKIT_PRINT("    DKits %s Released at %s. Chip Series: %s \n\
    Copyright (C) %s Centec Networks Inc.  All rights reserved.\n",
    CTC_DKITS_VERSION_STR, CTC_DKITS_RELEASE_DATE, chipname, CTC_DKITS_COPYRIGHT_TIME);

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_show_device_info,
        cli_dkit_show_device_info_cmd,
        "show device info (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Device",
        "Infomation",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{

    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;
    if (g_dkit_api[lchip]->show_device_info)
    {
        return g_dkit_api[lchip]->show_device_info((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_list_tbl,
        cli_dkit_list_tbl_cmd,
        "list STRING:TBL_NAME (grep SUB_STR | )(detail |) (lchip LCHIP|)",
        "List tables",
        "Substring of table name",
        "Grep sub string from result",
        "Sub string",
        "Detail display",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_memory_para_t* req = &memory_para_com;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));
    req->type = CTC_DKIT_MEMORY_LIST;

    if (argc > 0)
    {
        sal_strncpy((char*)req->buf, argv[0], CTC_DKITS_RW_BUF_SIZE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("grep");
    if (index != 0xFF)
    {
        sal_strncpy((char*)req->buf2, argv[index+1], CTC_DKITS_RW_BUF_SIZE);
        req->grep = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        req->detail = 1;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process((void*)req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_show_tbl_reg_by_data,
        cli_dkit_show_tbl_reg_by_data_cmd,
        "show tbl-data STRING:TBL_NAME_ID VALUE (mask MASK|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Table data",
        "Table name like [DsMac] or table id like [4870]",
        "Value",
        "Mask",
        "Mask value",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_memory_para_t* req = &memory_para_com;
    uint32 tbl_id = 0;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));

    req->type = CTC_DKIT_MEMORY_SHOW_TBL_BY_DATA;

    /*get tbl name or tbl id*/
    sal_strncpy((char*)req->buf, argv[0], CTC_DKITS_RW_BUF_SIZE);

    tbl_id = CTC_DKITS_INVALID_TBL_ID;

    if ((req->buf[0] >= '0') && (req->buf[0] <= '9'))
    {
        CTC_CLI_GET_UINT32_RANGE("table-id", tbl_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }


    /*get field value*/
    CTC_CLI_GET_INTEGER_N("value", (uint32 *)(req->value), argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("mask");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER_N("mask", (uint32 *)(req->mask), argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    req->param[1] = lchip;
    req->param[2] = 0;
    req->param[3] = tbl_id;
    req->param[4] = 0;

    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process((void*)req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_read_reg_or_tbl,
        cli_dkit_read_reg_or_tbl_cmd,
        "read STRING:TBL_NAME_ID INDEX ( direct | count COUNT (step STEP | ) | ) (lchip CHIP_ID|)  (grep SUB_STR | )",
        "Read table or register",
        "Table name like [DsMac] or table id like [4870]",
        "Table index value <0-0xFFFFFFFF>",
        "Read table by direct I/O, otherwise by ACC I/O",
        "The counts of sequential registers to read from the index",
        "Counts",
        "Step how many count. Default:1",
        "Step count. 2 means 0 2 4...",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC,
        "Grep",
        "Regex")
{
    ctc_dkit_memory_para_t* req = &memory_para_com;
    uint32 lchip = 0;
    uint32 index = 0;
    uint32 count = 1;
    uint32 tbl_id = 0;
    uint8 step = 1;
    uint8 arg_index = 0;

    arg_index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != arg_index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[arg_index + 1]);
    }

    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));
    req->type = CTC_DKIT_MEMORY_READ;

    /*get tbl name or tbl id*/
    sal_strncpy((char*)req->buf, argv[0], CTC_DKITS_RW_BUF_SIZE);
    tbl_id = CTC_DKITS_INVALID_TBL_ID;
    if ((req->buf[0] >= '0') && (req->buf[0] <= '9'))
    {
        CTC_CLI_GET_UINT32_RANGE("table-id", tbl_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    /*get tbl index*/
    CTC_CLI_GET_UINT32_RANGE("Tbl-index", index, argv[1], 0, 0xFFFFFFFF);

    arg_index = CTC_CLI_GET_ARGC_INDEX("direct");
    if (arg_index != 0xFF)
    {
        req->direct_io = 1;
    }
    arg_index = CTC_CLI_GET_ARGC_INDEX("count");
    if (arg_index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("count", count, argv[arg_index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    arg_index = CTC_CLI_GET_ARGC_INDEX("step");
    if (arg_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("step", step, argv[arg_index + 1], 0, CTC_MAX_UINT8_VALUE);
    }


    arg_index = CTC_CLI_GET_ARGC_INDEX("grep");
    if (arg_index != 0xFF)
    {
        sal_strncpy((char*)req->buf2, argv[arg_index+1], CTC_DKITS_RW_BUF_SIZE);
        req->grep = 1;
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    req->param[1] = lchip;
    req->param[2] = index;
    req->param[3] = tbl_id;
    req->param[4] = count;
    req->param[5] = step;
    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process((void*)req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_write_reg_or_tbl,
        cli_dkit_write_reg_or_tbl_cmd,
        "write STRING:TBL_NAME_ID INDEX STRING:FLD_NAME_ID VALUE ( mask MASK| direct | ) (lchip CHIP_ID|)",
        "Write table or register",
        "Table name like [DsMac] or table id like [4870]",
        "Table index value <0-0xFFFFFFFF>",
        "Field name or field id",
        "Value",
        "Tcam mask",
        "Tcam mask",
        "Write table by direct I/O, otherwise by ACC I/O",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_memory_para_t* req = &memory_para_com;
    uint32 lchip = 0;
    uint32 field_id = 0;
    uint32 index = 0;
    static uint32 value[CTC_DKITS_MAX_ENTRY_WORD] = {0};
    static uint32 mask[CTC_DKITS_MAX_ENTRY_WORD] = {0};
    uint32 tbl_id = 0;
    uint8 arg_index = 0;

    arg_index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != arg_index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[arg_index + 1]);
    }

    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));
    sal_memset(&value, 0, sizeof(value));
    sal_memset(&mask, 0, sizeof(mask));
    req->type = CTC_DKIT_MEMORY_WRITE;

    /*get tbl name or tbl id*/
    sal_strncpy((char*)req->buf, argv[0], CTC_DKITS_RW_BUF_SIZE);
    tbl_id = CTC_DKITS_INVALID_TBL_ID;
    if ((req->buf[0] >= '0') && (req->buf[0] <= '9'))
    {
        CTC_CLI_GET_UINT32_RANGE("table-id", tbl_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    /*get tbl index*/
    CTC_CLI_GET_UINT32_RANGE("Tbl-index", index, argv[1], 0, 0xFFFFFFFF);

    /*get field name*/
    sal_strncpy((char*)req->buf2, argv[2], CTC_DKITS_RW_BUF_SIZE);
    field_id = 0xFFFFFFFF;
    if ((req->buf2[0] >= '0') && (req->buf2[0] <= '9'))
    {
        CTC_CLI_GET_UINT32_RANGE("field-id", field_id, argv[2], 0, CTC_MAX_UINT32_VALUE);
    }

    /*get field value*/
    CTC_CLI_GET_INTEGER_N("fld-value", (uint32 *)(value), argv[3]);
    arg_index = CTC_CLI_GET_ARGC_INDEX("mask");
    if (arg_index != 0xFF)
    {
        CTC_CLI_GET_INTEGER_N("fld-value", (uint32 *)(mask), argv[arg_index + 1]);
    }

    arg_index = CTC_CLI_GET_ARGC_INDEX("direct");
    if (arg_index != 0xFF)
    {
        req->direct_io = 1;
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    req->param[1] = lchip;
    req->param[2] = index;
    req->param[3] = tbl_id;
    req->param[4] = field_id;
    sal_memcpy(req->value, value, CTC_DKITS_MAX_ENTRY_WORD*4);
    sal_memcpy(req->mask, mask, CTC_DKITS_MAX_ENTRY_WORD*4);

    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process((void*)req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_check_reg_or_tbl,
    cli_dkit_check_reg_or_tbl_cmd,
    "check STRING:TBL_NAME_ID INDEX ( count COUNT | ) (lchip CHIP_ID|)",
    "Check table or register",
    "Table name like [DsMac] or table id like [4870]",
    "Table index value <0-0xFFFFFFFF>",
    "The counts of sequential registers to read from the index",
    "Counts",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_memory_para_t* req = &memory_para_com;
    uint32 lchip = 0;
    uint32 index = 0;
    uint32 count = 1;
    uint32 tbl_id = 0;
    uint8 arg_index = 0;

    arg_index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != arg_index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[arg_index + 1]);
    }

    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));
    req->type = CTC_DKIT_MEMORY_CHECK;

    /*get tbl name or tbl id*/
    sal_strncpy((char*)req->buf, argv[0], CTC_DKITS_RW_BUF_SIZE);
    tbl_id = CTC_DKITS_INVALID_TBL_ID;
    if ((req->buf[0] >= '0') && (req->buf[0] <= '9'))
    {
        CTC_CLI_GET_UINT32_RANGE("table-id", tbl_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    /*get tbl index*/
    CTC_CLI_GET_UINT32_RANGE("Tbl-index", index, argv[1], 0, 0xFFFFFFFF);

    arg_index = CTC_CLI_GET_ARGC_INDEX("count");
    if (arg_index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("count", count, argv[arg_index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    req->param[1] = lchip;
    req->param[2] = index;
    req->param[3] = tbl_id;
    req->param[4] = count;
    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process((void*)req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_reset_reg_or_tbl,
        cli_dkit_reset_reg_or_tbl_cmd,
        "reset STRING:TBL_NAME_ID INDEX (lchip CHIP_ID|)",
        CTC_CLI_CLEAR_STR,
        "Table name like [DsMac] or table id like [4870]",
        "Table index value <0-0xFFFFFFFF>",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_memory_para_t* req = &memory_para_com;
    uint32 lchip = 0;
    uint32 index = 0;
    uint32 tbl_id = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));
    req->type = CTC_DKIT_MEMORY_RESET;

    /*get tbl name or tbl id*/
    sal_strncpy((char*)req->buf, argv[0], CTC_DKITS_RW_BUF_SIZE);
    tbl_id = CTC_DKITS_INVALID_TBL_ID;
    if ((req->buf[0] >= '0') && (req->buf[0] <= '9'))
    {
        CTC_CLI_GET_UINT32_RANGE("table-id", tbl_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }

    /*get tbl index*/
    CTC_CLI_GET_UINT32_RANGE("Tbl-index", index, argv[1], 0, 0xFFFFFFFF);

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    req->param[1] = lchip;
    req->param[2] = index;
    req->param[3] = tbl_id;

    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process(req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_show_tbl_reg,
        cli_dkit_show_tbl_reg_cmd,
        "show tbl-name STRING:TBL_ID (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Table or register name",
        "Table id value <0-CTC_DKITS_INVALID_TBL_ID>",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 id = 0;
    int32 ret = 0;
    char    buf[CTC_DKITS_RW_BUF_SIZE];
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT32_RANGE("Table/Reg Index", id, argv[0], 0, CTC_DKITS_INVALID_TBL_ID);

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ret = g_dkit_chip_api[lchip]->drv_get_tbl_string_by_id(lchip, id, buf);

    if (CLI_SUCCESS == ret)
    {
        CTC_DKIT_PRINT("------------------------------------------------\n");
        CTC_DKIT_PRINT("ID:%d -> %s\n", id, buf);
        CTC_DKIT_PRINT("------------------------------------------------\n");
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_show_tbl_reg_by_addr,
        cli_dkit_show_tbl_reg_by_addr_cmd,
        "show tbl-name addr ADDRESS (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Table or register name",
        "read address 0xXXXXXXXX",
        "address value")
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_memory_para_t* req = &memory_para_com;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(req, 0, sizeof(ctc_dkit_memory_para_t));
    req->type = CTC_DKIT_MEMORY_SHOW_TBL_BY_ADDR;

    CTC_CLI_GET_INTEGER("addressOffset", req->param[0], argv[0]);

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->memory_process)
    {
        return g_dkit_api[lchip]->memory_process(req);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_read_address,
    cli_dkit_read_address_cmd,
    "read address ADDR_OFFSET (count COUNT |) (lchip CHIP_ID|)",
    "read address value cmd",
    "read address 0xXXXXXXXX",
    "address value",
    "replicate time",
    "replicate time",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 address_offset = 0, rep_time = 1;
    uint32 tmp_i = 0, lchip = 0;
    uint32 tmp_value = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    CTC_CLI_GET_INTEGER("addressOffset", address_offset, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("count");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("count", rep_time, argv[index + 1]);
    }

    for (tmp_i = 0 ; tmp_i < rep_time; tmp_i++)
    {
        ret = g_dkit_chip_api[lchip]->drv_chip_read(lchip, address_offset, &tmp_value);

        if (ret < CLI_SUCCESS)
        {
            continue;
        }

        CTC_DKIT_PRINT("Address 0x%08x: 0x%08x\n", address_offset, tmp_value);
        address_offset = address_offset + 4;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_write_address,
    cli_dkit_write_address_cmd,
    "write address ADDR_OFFSET WRITE_VALUE (lchip CHIP_ID|)",
    "write value to address cmd",
    "write address 0xXXXXXXXX <value>",
    "address value",
    "value to write",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    uint32 address_offset = 0, lchip = 0, value = 0;
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    CTC_CLI_GET_INTEGER("address", address_offset, argv[0]);
    CTC_CLI_GET_INTEGER("value", value, argv[1]);
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ret = g_dkit_chip_api[lchip]->drv_chip_write(lchip, address_offset, value);

    if (ret < CLI_SUCCESS)
    {
        CTC_DKIT_PRINT("0x%08x address write ERROR! Value = 0x%08x\n", address_offset, value);
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_check_address,
    cli_dkit_check_address_cmd,
    "check address ADDR_OFFSET CHECK_VALUE CHECK_MASK (lchip CHIP_ID|)",
    "check address value cmd",
    "check address 0xXXXXXXXX",
    "address value",
    "check value",
    "check mask",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 address_offset = 0;
    uint32 rd_val = 0;
    uint32 chk_val = 0;
    uint32 chk_mask = 0;
    uint32 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    CTC_CLI_GET_INTEGER("addressOffset", address_offset, argv[0]);
    CTC_CLI_GET_INTEGER("checkValue", chk_val, argv[1]);
    CTC_CLI_GET_INTEGER("checkMask", chk_mask, argv[2]);

    ret = g_dkit_chip_api[lchip]->drv_chip_read(lchip, address_offset, &rd_val);
    if (ret < CLI_SUCCESS)
    {
        CTC_DKIT_PRINT("Address 0x%08x read error!\n", address_offset);
    }
    else
    {
        if ((rd_val&chk_mask) == (chk_val&chk_mask))
        {
            CTC_DKIT_PRINT("Address 0x%08x check success! Value = 0x%08x\n", address_offset, rd_val);
        }
        else
        {
            CTC_DKIT_PRINT("Address 0x%08x check error! Value = 0x%08x\n", address_offset, rd_val);
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_show_interface_status,
    cli_dkit_show_interface_status_cmd,
    "show interface ((mac-id MAC_ID)| (port GPORT) | (mac-en (enable | disable)) | (link (up|down)) \
       | (speed (1g|10g|40g|100g)) | ) (lchip CHIP_ID|)",
    CTC_CLI_SHOW_STR,
    "Interface",
    "Mac id",
    "Mac id, <0-128>",
    "Global port",
    "Global port",
    "Mac enable/disable",
    "Mac enable",
    "Mac disable",
    "Link status",
    "Link up",
    "Link down",
    "Speed",
    "Speed 1g",
    "Speed 10g",
    "Speed 40g",
    "Speed 100g",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_interface_para_t para;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    para.type = CTC_DKIT_INTERFACE_ALL;
    para.value = 0;

    index = CTC_CLI_GET_ARGC_INDEX("mac-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("mac-id", para.value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        para.type = CTC_DKIT_INTERFACE_MAC_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", para.value, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.type = CTC_DKIT_INTERFACE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-en");
    if (index != 0xFF)
    {
        para.type = CTC_DKIT_INTERFACE_MAC_EN;
        index = CTC_CLI_GET_ARGC_INDEX("enable");
        if (index != 0xFF)
        {
            para.value = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("link");
    if (index != 0xFF)
    {
        para.type = CTC_DKIT_INTERFACE_LINK_STATUS;
        index = CTC_CLI_GET_ARGC_INDEX("up");
        if (index != 0xFF)
        {
            para.value = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("speed");
    if (index != 0xFF)
    {
        para.type = CTC_DKIT_INTERFACE_SPEED;
        index = CTC_CLI_GET_ARGC_INDEX("1g");
        if (index != 0xFF)
        {
            para.value = CTC_DKIT_INTERFACE_SGMII;
        }
        index = CTC_CLI_GET_ARGC_INDEX("10g");
        if (index != 0xFF)
        {
            para.value = CTC_DKIT_INTERFACE_XFI;
        }
        index = CTC_CLI_GET_ARGC_INDEX("40g");
        if (index != 0xFF)
        {
            para.value = CTC_DKIT_INTERFACE_XLG;
        }
        index = CTC_CLI_GET_ARGC_INDEX("100g");
        if (index != 0xFF)
        {
            para.value = CTC_DKIT_INTERFACE_CG;
        }
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->show_interface_status)
    {
        return g_dkit_api[lchip]->show_interface_status((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

}


CTC_CLI(cli_dkit_show_discard_type,
        cli_dkit_show_discard_type_cmd,
        "show discard-type (REASON_ID | ) (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Discard type description",
        "Discard reason id",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    para.reason_id = 0xFFFFFFFF;

    if ((argc > 0) && (argc != 2))
    {
        CTC_CLI_GET_UINT32_RANGE("reason-id", para.reason_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->show_discard_type)
    {
        return g_dkit_api[lchip]->show_discard_type((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_show_discard_stats,
        cli_dkit_show_discard_stats_cmd,
        "show discard-stats (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Discard stats",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    para.reason_id = 0xFFFFFFFF;
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->show_discard_stats)
    {
        return g_dkit_api[lchip]->show_discard_stats((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_discard_to_cpu,
        cli_dkit_discard_to_cpu_cmd,
        "discard discard-to-cpu (enable|disable) (reason REASON_ID|) (lchip CHIP_ID|)",
        "Discard",
        "Discard packet to cpu",
        "Enable",
        "Disable",
        "Discard reason id",
        "Discard type",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t para;

    sal_memset(&para, 0, sizeof(para));
    para.reason_id = 0xFFFFFFFF;

    index = CTC_CLI_GET_ARGC_INDEX("reason");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("reason-id", para.reason_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        para.discard_to_cpu_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;
    CTC_DKIT_INIT_CHECK();

    if (g_dkit_api[lchip]->discard_to_cpu_en)
    {
        return g_dkit_api[lchip]->discard_to_cpu_en((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_discard_display_en,
        cli_dkit_discard_display_en_cmd,
        "discard display (enable|disable) (reason REASON_ID|) (lchip CHIP_ID|)",
        "Discard",
        "Discard display",
        "Enable",
        "Disable",
        "Discard reason id",
        "Discard type",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t para;

    sal_memset(&para, 0, sizeof(para));
    para.reason_id = 0xFFFFFFFF;

    index = CTC_CLI_GET_ARGC_INDEX("reason");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("reason-id", para.reason_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        para.discard_display_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;
    CTC_DKIT_INIT_CHECK();

    if (g_dkit_api[lchip]->discard_display_en)
    {
        return g_dkit_api[lchip]->discard_display_en((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_show_discard,
        cli_dkit_show_discard_cmd,
        "show discard (on-line| history | port GPORT|) (lchip CHIP_ID|}",
        CTC_CLI_SHOW_STR,
        "Discard",
        "On line status",
        "Discard history",
        "Global port",
        "Global port",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{

    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_discard_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));

    index = CTC_CLI_GET_ARGC_INDEX("on-line");
    if (index != 0xFF)
    {
        para.on_line = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("capture");
    if (index != 0xFF)
    {
        para.captured = 1;
        CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("history");
    if (index != 0xFF)
    {
        para.history = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        para.match_port = 1;
        CTC_CLI_GET_UINT32("port", para.gport, argv[index + 1]);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->show_discard)
    {
        return g_dkit_api[lchip]->show_discard((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_install_capture_flow,
        cli_dkit_install_capture_flow_cmd,
        "install capture SESSION_ID flow ({gport PORT|src-channel CHAN |mac-da MACDA| mac-sa MACSA| svlan-id SVLANID| cvlan-id CVLANID| ether-type ETHTYPE|\
        ( ipv4 {da A.B.C.D | sa A.B.C.D | checksum CHECKSUM}| \
          ipv6 {da X:X::X:X | sa X:X::X:X }| \
          mpls {label0 LABEL| label1 LABEL | label2 LABEL| label3 LABEL} | \
          slow-protocol {sub-type SUBTYPE|flags FLAGS|code CODE} | \
          ether-oam {level LEVEL|version VERSION|opcode OPCODE|rx-fcb RXFCB|tx-fcf TXFCF|tx-fcb TXFCB}) | \
        l4-src-port L4PORT | l4-dest-port L4PORT} |) (to-cpu-en|)(lchip CHIP_ID|)",
        "Install capture flow",
        "Install capture flow",
        "Captured session id",
        "Captured packet from Network port",
        "Global source port",
        "Gport",
        "Source channel",
        "Channel id",
        "MAC DA address",
        "MAC address in HHHH.HHHH.HHHH format",
        "MAC SA address",
        "MAC address in HHHH.HHHH.HHHH format",
        "Svlan id",
        "<0-4095>",
        "Cvlan id",
        "<0-4095>",
        "Ether type",
        "<0-0xFFFF>",
        "Ipv4 packet",
        "Ipv4 da",
        "IPv4 address in A.B.C.D format",
        "Ipv4 sa",
        "IPv4 address in A.B.C.D format",
        "Ipv4 checksum",
        "<0-0xFFFF>",
        "Ipv6 packet",
        "Ipv6 da",
        "IPv6 address in X:X::X:X format",
        "Ipv6 sa",
        "IPv6 address in X:X::X:X format",
        "Mpls packet",
        "The first lable",
        "<0-0xFFFFFFFF>",
        "The second lable",
        "<0-0xFFFFFFFF>",
        "The third lable",
        "<0-0xFFFFFFFF>",
        "The fourth lable",
        "<0-0xFFFFFFFF>",
        "Slow protocal packet",
        "Sub type",
        "<0-0xFF>",
        "Flags",
        "<0-0xFFFF>",
        "Code",
        "<0-0xFF>",
        "Ether oam packet",
        "Level",
        "<0-7>",
        "Version",
        "Version",
        "Opcode",
        "0-0xFF",
        "Rx fcb","0-0xFFFFFFFF",
        "Tx fcf","0-0xFFFFFFFF",
        "Tx fcb","0-0xFFFFFFFF",
        "Layer4 source port",
        "<0-0xFFFF>",
        "Layer4 dest port",
        "<0-0xFFFF>",
        "Captured pkt to cpu enable",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
	uint8 lchip = 0;
    ipv6_addr_t ipv6_address;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);
    para.mode = CTC_DKIT_FLOW_MODE_ALL;

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu-en");
    if (index != 0xFF)
    {
        para.to_cpu_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", para.flow_info.port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.port = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-channel");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-channel", para.flow_info.channel_id_0, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_0 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", para.flow_info.mac_da, argv[index + 1]);
        para.flow_info_mask.mac_da[0] = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", para.flow_info.mac_sa, argv[index + 1]);
        para.flow_info_mask.mac_sa[0] = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan-id", para.flow_info.svlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.svlan_id = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan-id", para.flow_info.cvlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.cvlan_id = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ether-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether-type", para.flow_info.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.ether_type = 1;
    }

    /*Layer3*/
    /*ipv4*/
    index = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (index != 0xFF)
    {
        para.flow_info.l3_type = CTC_DKIT_CAPTURED_IPV4;
        para.flow_info_mask.l3_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ip", para.flow_info.u_l3.ipv4.ip_da, argv[index + 1]);
            para.flow_info_mask.u_l3.ipv4.ip_da = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ip", para.flow_info.u_l3.ipv4.ip_sa, argv[index + 1]);
            para.flow_info_mask.u_l3.ipv4.ip_sa = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("checksum");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("checksum", para.flow_info.u_l3.ipv4.ip_check_sum, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            para.flow_info_mask.u_l3.ipv4.ip_check_sum = 1;
        }
    }
    /*ipv6*/
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (index != 0xFF)
    {
        para.flow_info.l3_type = CTC_DKIT_CAPTURED_IPV6;
        para.flow_info_mask.l3_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ip", ipv6_address, argv[index + 1]);
            /* adjust endian */
            para.flow_info.u_l3.ipv6.ip_da[0] = sal_htonl(ipv6_address[0]);
            para.flow_info.u_l3.ipv6.ip_da[1] = sal_htonl(ipv6_address[1]);
            para.flow_info.u_l3.ipv6.ip_da[2] = sal_htonl(ipv6_address[2]);
            para.flow_info.u_l3.ipv6.ip_da[3] = sal_htonl(ipv6_address[3]);
            para.flow_info_mask.u_l3.ipv6.ip_da[0] = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ip", ipv6_address, argv[index + 1]);
            /* adjust endian */
            para.flow_info.u_l3.ipv6.ip_sa[0] = sal_htonl(ipv6_address[0]);
            para.flow_info.u_l3.ipv6.ip_sa[1] = sal_htonl(ipv6_address[1]);
            para.flow_info.u_l3.ipv6.ip_sa[2] = sal_htonl(ipv6_address[2]);
            para.flow_info.u_l3.ipv6.ip_sa[3] = sal_htonl(ipv6_address[3]);
            para.flow_info_mask.u_l3.ipv6.ip_sa[0] = 1;
        }
    }
    /*mpls*/
    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (index != 0xFF)
    {
        para.flow_info.l3_type = CTC_DKIT_CAPTURED_MPLS;
        para.flow_info_mask.l3_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("label0");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("label", para.flow_info.u_l3.mpls.mpls_label0, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.mpls.mpls_label0 = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("label1");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("label", para.flow_info.u_l3.mpls.mpls_label1, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.mpls.mpls_label1 = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("label2");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("label", para.flow_info.u_l3.mpls.mpls_label2, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.mpls.mpls_label2 = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("label3");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("label", para.flow_info.u_l3.mpls.mpls_label3, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.mpls.mpls_label3 = 1;
        }
    }
    /*slow protocol*/
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol");
    if (index != 0xFF)
    {
        para.flow_info.l3_type = CTC_DKIT_CAPTURED_SLOW_PROTOCOL;
        para.flow_info_mask.l3_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("sub-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8_RANGE("sub-type", para.flow_info.u_l3.slow_proto.sub_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
            para.flow_info_mask.u_l3.slow_proto.sub_type = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("flags");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("flags", para.flow_info.u_l3.slow_proto.flags, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            para.flow_info_mask.u_l3.slow_proto.flags = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("code");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8_RANGE("code", para.flow_info.u_l3.slow_proto.code, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
            para.flow_info_mask.u_l3.slow_proto.code = 1;
        }
    }
    /*ether oam*/
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam");
    if (index != 0xFF)
    {
        para.flow_info.l3_type = CTC_DKIT_CAPTURED_ETHOAM;
        para.flow_info_mask.l3_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("level");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8_RANGE("level", para.flow_info.u_l3.ether_oam.level, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
            para.flow_info_mask.u_l3.ether_oam.level = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("version");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8_RANGE("version", para.flow_info.u_l3.ether_oam.version, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
            para.flow_info_mask.u_l3.ether_oam.version = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("opcode");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8_RANGE("opcode", para.flow_info.u_l3.ether_oam.opcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
            para.flow_info_mask.u_l3.ether_oam.opcode = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("rx-fcb");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("rx-fcb", para.flow_info.u_l3.ether_oam.rx_fcb, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.ether_oam.rx_fcb = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("tx-fcf");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("tx-fcf", para.flow_info.u_l3.ether_oam.tx_fcf, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.ether_oam.tx_fcf = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX("tx-fcb");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32_RANGE("tx-fcb", para.flow_info.u_l3.ether_oam.tx_fcb, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
            para.flow_info_mask.u_l3.ether_oam.tx_fcb = 1;
        }
    }
    /*Layer4*/
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        para.flow_info.l4_type = CTC_DKIT_CAPTURED_L4_SOURCE_PORT;
        para.flow_info_mask.l4_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("l4-src-port", para.flow_info.u_l4.l4port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            para.flow_info_mask.u_l4.l4port.source_port = 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("l4-dest-port");
    if (index != 0xFF)
    {
        para.flow_info.l4_type = CTC_DKIT_CAPTURED_L4_DEST_PORT;
        para.flow_info_mask.l4_type = 1;
        index = CTC_CLI_GET_ARGC_INDEX("l4-dest-port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("l4-dest-port", para.flow_info.u_l4.l4port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            para.flow_info_mask.u_l4.l4port.dest_port = 1;
        }
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->install_captured_flow)
    {
        return g_dkit_api[lchip]->install_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_install_capture_ipe,
        cli_dkit_install_capture_ipe_cmd,
        "install capture SESSION_ID iloop {src-channel-0 CHAN|src-channel-1 CHAN|src-channel-2 CHAN|src-channel-3 CHAN|from-cflex-channel|is-dbg-pkt|} (to-cpu-en|) (lchip CHIP_ID|)",
        "Install capture flow",
        "Install capture flow",
        "Captured session id",
        "Captured packet from ILoop",
        "source channel 0",
        "Channel id 0",
        "source channel 1",
        "Channel id 1",
        "source channel 2",
        "Channel id 2",
        "source channel 3",
        "Channel id 3",
        "Debugged packet from cflex channel",
        "Is debugged packet",
        "Captured pkt to cpu enable",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
	uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);
    para.mode = CTC_DKIT_FLOW_MODE_IPE;

    index = CTC_CLI_GET_ARGC_INDEX("src-channel-0");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-channel-0", para.flow_info.channel_id_0, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_0 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-channel-1");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-channel-1", para.flow_info.channel_id_1, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_1 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-channel-2");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-channel-2", para.flow_info.channel_id_2, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_2 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-channel-3");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-channel-3", para.flow_info.channel_id_3, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_3 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("from-cflex-channel");
    if(index != 0xFF)
    {
        para.flow_info.from_cflex_channel = 1;
        para.flow_info_mask.from_cflex_channel = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-dbg-pkt");
    if(index != 0xFF)
    {
        para.flow_info.is_debugged_pkt= 1;
        para.flow_info_mask.is_debugged_pkt = 1;
    }


    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", para.flow_info.port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.port = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu-en");
    if (index != 0xFF)
    {
        para.to_cpu_en = 1;
    }

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->install_captured_flow)
    {
        return g_dkit_api[lchip]->install_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_install_capture_bsr,
        cli_dkit_install_capture_bsr_cmd,
        "install capture SESSION_ID bsr {src-channel CHAN|src-port PORT|(is-mcast GROUPID|dest-port PORT)|is-dbg-pkt|} (lchip CHIP_ID|)",
        "Install capture flow",
        "Install capture flow",
        "Captured session id",
        "Captured packet income BSR",
        "Source channel",
        "Source channel id",
        "Source port",
        "Source port",
        "Is mcast",
        "Mcast group id",
        "Dest port",
        "Dest port",
        "Is debugged packet",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
	uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);

    para.mode = CTC_DKIT_FLOW_MODE_BSR;


    index = CTC_CLI_GET_ARGC_INDEX("src-channel");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-channel", para.flow_info.channel_id_0, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_0 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-port", para.flow_info.port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.port = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-dbg-pkt");
    if(index != 0xFF)
    {
        para.flow_info.is_debugged_pkt= 1;
        para.flow_info_mask.is_debugged_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-mcast");
    if(index != 0xFF)
    {
        para.flow_info.is_mcast = 1;
        para.flow_info_mask.is_mcast = 1;
        CTC_CLI_GET_UINT16_RANGE("group-id", para.flow_info.group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.group_id = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dest-port");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dest-port", para.flow_info.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.dest_port = 1;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->install_captured_flow)
    {
        return g_dkit_api[lchip]->install_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_install_capture_metfifo,
        cli_dkit_install_capture_metfifo_cmd,
        "install capture SESSION_ID metfifo (forwarding-flow | exception-flow |log-flow |) {dest-port PORT|is-dbg-pkt|} (lchip CHIP_ID|)",
        "Install capture flow",
        "Install capture flow",
        "Captured session id",
        "Captured packet income Metfifo",
        "Forwarding flow",
        "Exception flow",
        "Log flow",
        "Dest port",
        "Dest port",
        "Is debugged packet",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
	uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);

    para.mode = CTC_DKIT_FLOW_MODE_METFIFO;

    index = CTC_CLI_GET_ARGC_INDEX("is-dbg-pkt");
    if(index != 0xFF)
    {
        para.flow_info.is_debugged_pkt= 1;
        para.flow_info_mask.is_debugged_pkt = 1;
    }

    para.flow_info.is_mcast = 1;
    para.flow_info_mask.is_mcast = 1;
    index = CTC_CLI_GET_ARGC_INDEX("dest-port");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dest-port", para.flow_info.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.dest_port = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("forwarding-flow");
    if(index != 0xFF)
    {
        para.path_sel = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("exception-flow");
    if(index != 0xFF)
    {
        para.path_sel = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("log-flow");
    if(index != 0xFF)
    {
        para.path_sel = 2;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->install_captured_flow)
    {
        return g_dkit_api[lchip]->install_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_install_capture_oam,
        cli_dkit_install_capture_oam_cmd,
        "install capture SESSION_ID oam (rx|tx) (mep-index INDEX (enable|disable)) (lchip CHIP_ID|)",
        "Install capture flow",
        "Install capture flow",
        "Captured session id",
        "Captured packet income or from OAMEngine",
        "OAM rx process",
        "OAM tx process",
        "Mep index",
        "Mep index",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
	uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_OAM_RX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tx");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_OAM_TX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-dbg-pkt");
    if(index != 0xFF)
    {
        para.flow_info.is_debugged_pkt= 1;
        para.flow_info_mask.is_debugged_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mep-index");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("mep-index", para.flow_info.mep_index, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        para.flow_info_mask.mep_index = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if(index != 0xFF)
    {
        para.flow_info.oam_dbg_en = 1;
        para.flow_info_mask.oam_dbg_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if(index != 0xFF)
    {
        para.flow_info.oam_dbg_en = 0;
        para.flow_info_mask.oam_dbg_en = 0;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->install_captured_flow)
    {
        return g_dkit_api[lchip]->install_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(cli_dkit_install_capture_epe,
        cli_dkit_install_capture_epe_cmd,
        "install capture SESSION_ID epe {dest-channel CHAN|src-port PORT|(is-mcast|)(dest-port PORT)|is-dbg-pkt|} (lchip CHIP_ID|)",
        "Install capture flow",
        "Install capture flow",
        "Captured session id",
        "Captured packet income EPE",
        "Dest channel",
        "Dest channel id",
        "Source port",
        "Source port",
        "Is mcast",
        "Dest port",
        "Dest port",
        "Is debugged packet",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
	uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);

    para.mode = CTC_DKIT_FLOW_MODE_EPE;


    index = CTC_CLI_GET_ARGC_INDEX("dest-channel");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dest-channel", para.flow_info.channel_id_0, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.channel_id_0 = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-port", para.flow_info.port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.port = 1;
    }


    index = CTC_CLI_GET_ARGC_INDEX("is-dbg-pkt");
    if(index != 0xFF)
    {
        para.flow_info.is_debugged_pkt= 1;
        para.flow_info_mask.is_debugged_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-mcast");
    if(index != 0xFF)
    {
        para.flow_info.is_mcast = 1;
        para.flow_info_mask.is_mcast = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dest-port");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dest-port", para.flow_info.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        para.flow_info_mask.dest_port = 1;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->install_captured_flow)
    {
        return g_dkit_api[lchip]->install_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_clear_capture,
        cli_dkit_clear_capture_cmd,
        "clear capture SESSION_ID (flow |result | ) (lchip CHIP_ID|)",
        CTC_CLI_CLEAR_STR,
        "Clear capture",
        "Captured session id",
        "Clear captured flow",
        "Clear captured result",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_captured_t para;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&para, 0, sizeof(para));
    para.flag = CTC_DKIT_CAPTURED_CLEAR_ALL;

    CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("flow");
    if (index != 0xFF)
    {
        para.flag = CTC_DKIT_CAPTURED_CLEAR_FLOW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("result");
    if (index != 0xFF)
    {
        para.flag = CTC_DKIT_CAPTURED_CLEAR_RESULT;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;

    if (g_dkit_api[lchip]->clear_captured_flow)
    {
        return g_dkit_api[lchip]->clear_captured_flow((void*)&para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_show_path,
        cli_dkit_show_path_cmd,
        "show path {capture SESSION_ID | detail | clear (flow|result) | } (ipe|bsr|metfifo|epe|oam-rx|oam-tx|) (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Show packet process path",
        "Use captured infomation",
        "Captured session id",
        "Detail display",
        "Clear capture",
        "Clear captured flow",
        "Clear captured result",
        "IPE process",
        "BSR process",
        "Metfifo process",
        "EPE process",
        "OAM rx process",
        "OAm tx process",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_path_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    para.captured = 0;
    para.on_line = 0;

    index = CTC_CLI_GET_ARGC_INDEX("capture");
    if (index != 0xFF)
    {
        para.captured = 1;
        para.on_line = 1;
        CTC_CLI_GET_UINT8_RANGE("capture session id", para.captured_session, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipe");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_IPE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("bsr");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_BSR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("metfifo");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_METFIFO;
    }
    index = CTC_CLI_GET_ARGC_INDEX("epe");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_EPE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("oam-rx");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_OAM_RX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("oam-tx");
    if (index != 0xFF)
    {
        para.mode = CTC_DKIT_FLOW_MODE_OAM_TX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("clear");
    if (index != 0xFF)
    {
        para.flag = CTC_DKIT_CAPTURED_CLEAR_ALL;
        index = CTC_CLI_GET_ARGC_INDEX("flow");
        if (index != 0xFF)
        {
            para.flag = CTC_DKIT_CAPTURED_CLEAR_FLOW;
        }
        index = CTC_CLI_GET_ARGC_INDEX("result");
        if (index != 0xFF)
        {
            para.flag = CTC_DKIT_CAPTURED_CLEAR_RESULT;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        para.detail = 1;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;


    if (g_dkit_api[lchip]->show_path)
    {
        return g_dkit_api[lchip]->show_path((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_show_monitor_q_id,
        cli_dkit_show_monitor_q_id_cmd,
        "show monitor queue-id (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Show monitor result",
        "Queue ID",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_monitor_para_t para;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;


    if (g_dkit_api[lchip]->show_queue_id)
    {
        return g_dkit_api[lchip]->show_queue_id((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_show_monitor_q_depth,
        cli_dkit_show_monitor_q_depth_cmd,
        "show monitor (queue-depth | queue-resource (ingress | egress ) (port GPORT | )) (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Show monitor result",
        "Queue depth",
        "Queue resource manager",
        "Ingress queue monitor",
        "Egress queue monitor",
        "Global port",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_monitor_para_t para;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    para.gport = 0xFFFF;
    para.dir = CTC_DKIT_BOTH_DIRECTION;

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        para.dir = CTC_DKIT_INGRESS;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("egress");
        if (index != 0xFF)
        {
            para.dir = CTC_DKIT_EGRESS;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", para.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;


    if (g_dkit_api[lchip]->show_queue_depth)
    {
        return g_dkit_api[lchip]->show_queue_depth((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_show_monitor_sensor,
        cli_dkit_show_monitor_sensor_cmd,
        "show monitor (temperature | voltage) (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Show monitor result",
        "Temperature value",
        "Voltage value",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_monitor_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));

    index = CTC_CLI_GET_ARGC_INDEX("temperature");
    if (index != 0xFF)
    {
        para.sensor_mode = CTC_DKIT_MONITOR_SENSOR_TEMP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("voltage");
    if (index != 0xFF)
    {
        para.sensor_mode = CTC_DKIT_MONITOR_SENSOR_VOL;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;


    if (g_dkit_api[lchip]->show_sensor_result)
    {
        return g_dkit_api[lchip]->show_sensor_result((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_monitor_temperature,
        cli_dkit_monitor_temperature_cmd,
        "monitor temperature (enable TMPMPERTURE0 TMPMPERTURE1 INTERVAL (log FILE |) | disable) (lchip CHIP_ID|)",
        "Monitor",
        "Temperature",
        "Enable",
        "Monitor temperature",
        "Power off temperature",
        "Monitor interval, 0~255, second",
        "Record to log",
        "Log file name",
        "Disable",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_monitor_para_t para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&para, 0, sizeof(para));
    para.sensor_mode = CTC_DKIT_MONITOR_SENSOR_TEMP_NOMITOR;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        para.enable = 1;
        CTC_CLI_GET_UINT8_RANGE("Temperature", para.temperature, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("Temperature", para.power_off_temp, argv[index+2], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("interval", para.interval, argv[index+3], 0, CTC_MAX_UINT8_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX("log");
        if (index != 0xFF)
        {
            para.log = 1;
            sal_strncpy(para.str, argv[index + 1], 32);
        }

    }
    else
    {
        para.enable = 0;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    para.lchip = lchip;


    if (g_dkit_api[lchip]->show_sensor_result)
    {
        return g_dkit_api[lchip]->show_sensor_result((void*)(&para));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_tcam_scan,
        cli_dkit_tcam_scan_cmd,
        "tcam scan (((tcam-type ({igs-scl | igs-acl | egs-acl | lpm-ip | lpm-nat | cid | service-queue} (priority PRIORITY |))) ((key-type KEY_TYPE (index INDEX | all)) |)) | all) (key-size (half|single|double|quad)|) (stress|)(lchip CHIP_ID|)",
        "Tcam module",
        "Scan tcam key",
        "Tcam type",
        "Ingress scl tcam",
        "Ingress acl tcam",
        "Egress acl tcam",
        "Lpm ip tcam",
        "Lpm nat/pbr tcam",
        "CategoryID tcam",
        "Service queue tcam",
        "Priority",
        "<0-7>",
        "Key type",
        "Tcam key type value",
        "Key index",
        "<0-0xFFFFFFFF>",
        "Specified tcam type's all key index",
        "All Tcam type and all its key index",
        "Key size",
        "Half Tcam key size value",
        "Single Tcam key size value",
        "Double Tcam key size value",
        "Quadruple Tcam key size value",
        "Tcam bist stress"
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkits_tcam_info_t tcam_info;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&tcam_info, 0, sizeof(ctc_dkits_tcam_info_t));
    tcam_info.index = 0x0FFFFFFFF;
    tcam_info.key_type = 0x0FFFFFFFF;
    tcam_info.priority = 0x0FFFFFFFF;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcam-type");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-scl");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-acl");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-acl");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-ip");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-nat");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_CATEGORYID);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-queue");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_SERVICE_QUEUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("priority", tcam_info.priority, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("key-type");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("key-type", tcam_info.key_type, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("index");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("index", tcam_info.index, argv[index + 1]);
        }

    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("all");
        if (0xFF != index)
        {
            DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_ALL);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("key-size");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("half");
        if (0xFF != index)
        {
            tcam_info.key_size = CTC_DKITS_TCAM_BIST_SIZE_HALF;
        }
        index = CTC_CLI_GET_ARGC_INDEX("single");
        if (0xFF != index)
        {
            tcam_info.key_size = CTC_DKITS_TCAM_BIST_SIZE_SINGLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("double");
        if (0xFF != index)
        {
            tcam_info.key_size = CTC_DKITS_TCAM_BIST_SIZE_DOUBLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("quad");
        if (0xFF != index)
        {
            tcam_info.key_size = CTC_DKITS_TCAM_BIST_SIZE_QUAD;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        tcam_info.is_stress = 1;
    }
    else
    {
        tcam_info.index = 0;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    tcam_info.lchip = lchip;

    if (g_dkit_api[lchip]->tcam_scan)
    {
        return g_dkit_api[lchip]->tcam_scan((void*)(&tcam_info));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_tcam_show_key_type,
        cli_dkit_tcam_show_key_type_cmd,
        "show tcam key-type (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Tcam module",
        "Tcam type's key type"
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }


    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->show_tcam_key_type)
    {
        return g_dkit_api[lchip]->show_tcam_key_type(lchip);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_tcam_capture_start,
        cli_dkit_tcam_capture_start_cmd,
        "tcam capture start ({igs-scl | igs-acl | egs-acl | lpm-ip | lpm-nat | fwd-cid | service-queue} (priority PRIORITY |) | all) (lchip CHIP_ID|)",
        "Tcam module",
        "Capture tcam key",
        "Start",
        "Ingress scl tcam",
        "Ingress acl tcam",
        "Egress acl tcam",
        "Lpm ip tcam",
        "Lpm nat/pbr tcam",
        "CategoryID tcam",
        "Service queue tcam",
        "Priority",
        "<0-7>",
        "All tcam",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkits_tcam_info_t tcam_info;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&tcam_info, 0, sizeof(ctc_dkits_tcam_info_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-scl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-acl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-acl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-ip");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-nat");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("fwd-cid");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_CATEGORYID);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-queue");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_SERVICE_QUEUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("priority", tcam_info.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("all");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    tcam_info.lchip = lchip;

    if (g_dkit_api[lchip]->tcam_capture_start)
    {
        return g_dkit_api[lchip]->tcam_capture_start(&tcam_info);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_tcam_capture_stop,
        cli_dkit_tcam_capture_stop_cmd,
        "tcam capture stop ({igs-scl | igs-acl | egs-acl | lpm-ip | lpm-nat | cid | service-queue} (priority PRIORITY |) | all) (lchip CHIP_ID|)",
        "Tcam module",
        "Capture tcam key",
        "Stop",
        "Ingress Scl tcam",
        "Ingress acl tcam",
        "Egress acl tcam",
        "Lpm ip tcam",
        "Lpm nat/pbr tcam",
        "CategoryID tcam",
        "Service queue tcam",
        "Priority",
        "<0-7>",
        "All tcam",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkits_tcam_info_t tcam_info;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&tcam_info, 0, sizeof(ctc_dkits_tcam_info_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-scl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-acl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-acl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-ip");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-nat");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_CATEGORYID);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-queue");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_SERVICE_QUEUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("priority", tcam_info.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("all");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    tcam_info.lchip = lchip;

    if (g_dkit_api[lchip]->tcam_capture_stop)
    {
        return g_dkit_api[lchip]->tcam_capture_stop(&tcam_info);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_tcam_capture_show,
        cli_dkit_tcam_capture_show_cmd,
        "show tcam capture ({igs-scl | igs-acl | egs-acl | lpm-ip | lpm-nat | cid | service-queue} (priority PRIORITY |) | all) ((brief | detail NUMBER)|) (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Tcam module",
        "Capture tcam key",
        "Ingress Scl tcam",
        "Ingress acl tcam",
        "Egress acl tcam",
        "Lpm ip tcam",
        "Lpm nat/pbr tcam",
        "Forward cid tcam",
        "Service queue tcam",
        "Priority",
        "<0-7>",
        "All tcam",
        "Brief",
        "Detail",
        "<0-7>",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkits_tcam_info_t tcam_info;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&tcam_info, 0, sizeof(ctc_dkits_tcam_info_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-scl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-acl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-acl");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-ip");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lpm-nat");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_CATEGORYID);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-queue");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_SERVICE_QUEUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("priority", tcam_info.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("all");
    if (0xFF != index)
    {
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX);
        DKITS_SET_FLAG(tcam_info.tcam_type, CTC_DKITS_TCAM_TYPE_FLAG_IPNAT);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("brief");
    if (0xFF != index)
    {
        tcam_info.index = 0xFFFFFFFF;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("detail");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32("detail", tcam_info.index, argv[index + 1]);
        }
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    tcam_info.lchip = lchip;

    if (g_dkit_api[lchip]->tcam_capture_show)
    {
        return g_dkit_api[lchip]->tcam_capture_show(&tcam_info);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_dkit_dump_cfg_dump,
        ctc_dkit_dump_cfg_dump_cmd,
        "dump-cfg store (static (serdes | clktree | ipe | bsr | epe |all) | dynamic ({scl | acl | ipfix | oam | fdb | ipuc | ipmc | cid | service-queue} | all)) FILE_NAME (lchip CHIP_ID|)",
        "Dump cfg cmd",
        "Store centec file",
        "Static table",
        "Hss and clktree",
        "Serdes table",
        "Ipe static table",
        "Bsr static table",
        "Epe static table",
        "All static table",
        "Dynamic table",
        "Scl key",
        "Acl key",
        "Ipfix Key",
        "Oam key",
        "Fdb key",
        "Ipuc key",
        "Ipmc key",
        "CategoryID key",
        "Service queue key",
        "All key",
        "Centec file",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkits_dump_cfg_t dump_cfg;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    sal_memset(&dump_cfg, 0, sizeof(ctc_dkits_dump_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("ipe");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_IPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("bsr");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_BSR);
        }
        index = CTC_CLI_GET_ARGC_INDEX("epe");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_EPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("all");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_ALL);
        }

        index = CTC_CLI_GET_ARGC_INDEX("clktree");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_HSS);
        }

        index = CTC_CLI_GET_ARGC_INDEX("serdes");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_SERDES);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("oam");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_OAM);
        }

        index = CTC_CLI_GET_ARGC_INDEX("fdb");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_FDB);
        }

        index = CTC_CLI_GET_ARGC_INDEX("scl");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_SCL);
        }

        index = CTC_CLI_GET_ARGC_INDEX("acl");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_ACL);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipuc");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_IPUC);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipmc");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_IPMC);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipfix");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_IPFIX);
        }

        index = CTC_CLI_GET_ARGC_INDEX("cid");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_CID);
        }

        index = CTC_CLI_GET_ARGC_INDEX("service-queue");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_QUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("all");
        if (0xFF != index)
        {
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_FDB);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_SCL);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_ACL);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_OAM);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_IPUC);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_IPMC);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_CID);
            DKITS_BIT_SET(dump_cfg.func_flag, CTC_DKITS_DUMP_FUNC_DYN_QUE);
        }
    }

    sal_memset(dump_cfg.file, 0, sizeof(dump_cfg.file));
    sal_memcpy(dump_cfg.file, argv[argc - 1], sal_strlen(argv[argc - 1]));
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    dump_cfg.lchip = lchip;

    if (g_dkit_api[lchip]->cfg_dump)
    {
        (g_dkit_api[lchip]->cfg_dump(lchip, &dump_cfg));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_dkit_dump_cfg_cmp,
        ctc_dkit_dump_cfg_cmp_cmd,
        "dump-cfg diff FILE_NAME1 FILE_NAME2 (lchip CHIP_ID|)",
        "Dump chip cfg cmd",
        "Diff centec file",
        "Centec file1",
        "Centec file2",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->cfg_cmp)
    {
        (g_dkit_api[lchip]->cfg_cmp(lchip, argv[0], argv[1]));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_dkit_dump_cfg_decode,
        ctc_dkit_dump_cfg_decode_cmd,
        "dump-cfg decode FILE_NAME (data-translate|) (lchip CHIP_ID|)",
        "Dump centec configure file cmd",
        "Decode centec file",
        "Centec file",
        "Translate data by table field",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 index = 0;
    uint8 lchip = 0;
    ctc_dkits_dump_cfg_t dump_cfg;

    sal_memset(&dump_cfg, 0, sizeof(ctc_dkits_dump_cfg_t));
    sal_memcpy(dump_cfg.file, argv[0], sal_strlen(argv[0]));

    index = CTC_CLI_GET_ARGC_INDEX("data-translate");
    if (0xFF != index)
    {
        dump_cfg.data_translate = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->cfg_decode)
    {
        (g_dkit_api[lchip]->cfg_decode(lchip, &dump_cfg));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_dump_memory_usage,
        cli_dkit_dump_memory_usage_cmd,
        "dump-cfg usage (hash (key (detail|) | memory) | tcam) (lchip CHIP_ID|)",
        "Dump centec cfg usage cmd",
        "Key and memory usage",
        "Hash",
        "Key",
        "Detail",
        "Memory",
        "Tcam",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 index = 0, type = 0;
    uint8 lchip = 0;


    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    index = CTC_CLI_GET_ARGC_INDEX("hash");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("memory");
        if (0xFF != index)
        {
            type = CTC_DKITS_DUMP_USAGE_HASH_MEM;
        }
        else
        {
            index = CTC_CLI_GET_ARGC_INDEX("detail");
            if (0xFF != index)
            {
                type = CTC_DKITS_DUMP_USAGE_HASH_DETAIL;
            }
            else
            {
                type = CTC_DKITS_DUMP_USAGE_HASH_BRIEF;
            }
        }
    }
    else
    {
        type = CTC_DKITS_DUMP_USAGE_TCAM_KEY;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->cfg_usage)
    {
        (g_dkit_api[lchip]->cfg_usage(lchip, &type));
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(cli_dkit_serdes_loopback,
        cli_dkit_serdes_loopback_cmd,
        "serdes SERDES_ID loopback (internal | external(mode VALUE|)) (enable|disable) (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Set serdes loopback",
        "Internal loopback, Tx->Rx",
        "External loopback, Rx->Tx",
        "Mode",
        "Value",
        "Enable",
        "Disable",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_LOOPBACK;
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("mode", ctc_dkit_serdes_ctl_para.para[2], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("internal");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[1] = CTC_DKIT_SERDIS_LOOPBACK_INTERNAL;
    }
    else
    {
        ctc_dkit_serdes_ctl_para.para[1] = CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
    }
    else
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;


    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(cli_dkit_serdes_pbrs,
        cli_dkit_serdes_pbrs_cmd,
        "serdes SERDES_ID prbs PATTERN (enable | disable | check (delay TIME|)(keep |) | monitor INTERVAL (log FILE|)) (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Set serdes prbs",
        "PRBS pattern, 0:PRBS7+, 1:PRBS7-, 2:PRBS15+, 3:PRBS15-, 4:PRBS23+, 5:PRBS23-, 6:PRBS31+, 7:PRBS31-, 8:PRBS9, 9:Squareware(8081), 10:PRBS11, 11:PRBS1T(1010)",
        "Tx enable",
        "Tx disable",
        "Rx check",
        "Delay before check",
        "Time ms"
        "Keep current status after check",
        "Monitor pbrs check result",
        "Monitor interval, 0~255, second",
        "Record to log",
        "Log file name",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
	ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_PRBS;
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT8_RANGE("pattern", ctc_dkit_serdes_ctl_para.para[1], argv[1], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("disable");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("check");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("keep");
        if (0xFF != index)
        {
            ctc_dkit_serdes_ctl_para.para[2] = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("delay");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("time", ctc_dkit_serdes_ctl_para.para[3], argv[index+1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("monitor");
     if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_MONITOR;
        CTC_CLI_GET_UINT8_RANGE("interval", ctc_dkit_serdes_ctl_para.para[2], argv[index+1], 0, CTC_MAX_UINT8_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX("log");
        if (0xFF != index)
        {
            ctc_dkit_serdes_ctl_para.str = argv[index + 1];
        }
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_serdes_polr_check,
        cli_dkit_polr_check_cmd,
        "serdes SERDES_ID polr-check paired-serdes SERDES_ID (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Polarity check",
        "Paired Serdes",
        "Serdes ID",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_POLR_CHECK;
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("paired-serdes-id", ctc_dkit_serdes_ctl_para.para[0], argv[1], 0, CTC_MAX_UINT32_VALUE);
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_serdes_eye,
        cli_dkit_serdes_eye_cmd,
        "serdes SERDES_ID eye (((height | width | width-slow | ) times TIMES ((precision (e3 | e6)) |)) | scope ) (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Digital eye measure",
        "Eye height",
        "Eye width",
        "Eye width test slowly",
        "Times of measure",
        "Times of measure",
        "Precision",
        "E3",
        "E6",
        "Eye scope",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("scope");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_EYE_SCOPE;
    }
    else
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_EYE_ALL;
        ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_EYE;
        index = CTC_CLI_GET_ARGC_INDEX("times");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("times", ctc_dkit_serdes_ctl_para.para[1] , argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("height");
        if (0xFF != index)
        {
            ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_EYE_HEIGHT;
        }

        index = CTC_CLI_GET_ARGC_INDEX("width");
        if (0xFF != index)
        {
            ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_EYE_WIDTH;
        }

        index = CTC_CLI_GET_ARGC_INDEX("width-slow");
        if (0xFF != index)
        {
            ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_EYE_WIDTH_SLOW;
        }

        index = CTC_CLI_GET_ARGC_INDEX("e3");
        if (0xFF != index)
        {
            ctc_dkit_serdes_ctl_para.precision = CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E3;
        }
        else
        {
            ctc_dkit_serdes_ctl_para.precision = CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E6;
        }
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_serdes_ffe,
        cli_dkit_serdes_ffe_cmd,
        "serdes SERDES_ID ffe DEST_SERDES_ID PATTERN COEFFICIENT (c0 C0_MIN C0_MAX) (c1 C1_MIN C1_MAX) (c2 C2_MIN C2_MAX) \
        (c3 C3_MIN C3_MAX |) (c4 C4_MIN C4_MAX |) { file FILENAME | delay DELAY | } (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "FFE para test",
        "Dest serdes ID",
        "PRBS pattern, 0:PRBS7+, 1:PRBS7-, 2:PRBS15+, 3:PRBS15-, 4:PRBS23+, 5:PRBS23-, 6:PRBS31+, 7:PRBS31-, 8:PRBS9, 9:PRBS11",
        "Coefficient sum threshold",
        "Coefficient0",
        "Coefficient0 minimum value",
        "Coefficient0 maximum value",
        "Coefficient1",
        "Coefficient1 minimum value",
        "Coefficient1 maximum value",
        "Coefficient2",
        "Coefficient2 minimum value",
        "Coefficient2 maximum value",
        "Coefficient3",
        "Coefficient3 minimum value",
        "Coefficient3 maximum value",
        "Coefficient4",
        "Coefficient4 minimum value",
        "Coefficient4 maximum value",
        "File name",
        "File name",
        "Delay between scan",
        "Unit: ms",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_FFE;
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.para[0], argv[1], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT8_RANGE("pattern", ctc_dkit_serdes_ctl_para.para[1], argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT32_RANGE("coefficient-sum", ctc_dkit_serdes_ctl_para.para[2], argv[3], 0, CTC_MAX_UINT32_VALUE);


    index = CTC_CLI_GET_ARGC_INDEX("c0");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("c0", ctc_dkit_serdes_ctl_para.ffe.coefficient0_min, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("c0", ctc_dkit_serdes_ctl_para.ffe.coefficient0_max, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("c1");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("c1", ctc_dkit_serdes_ctl_para.ffe.coefficient1_min, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("c1", ctc_dkit_serdes_ctl_para.ffe.coefficient1_max, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("c2");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("c2", ctc_dkit_serdes_ctl_para.ffe.coefficient2_min, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("c2", ctc_dkit_serdes_ctl_para.ffe.coefficient2_max, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("c3");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("c3", ctc_dkit_serdes_ctl_para.ffe.coefficient3_min, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("c3", ctc_dkit_serdes_ctl_para.ffe.coefficient3_max, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("c4");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("c4", ctc_dkit_serdes_ctl_para.ffe.coefficient4_min, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("c4", ctc_dkit_serdes_ctl_para.ffe.coefficient4_max, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("delay");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("delay", ctc_dkit_serdes_ctl_para.para[3], argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("file");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.str= argv[index + 1];
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(cli_dkit_serdes_status,
        cli_dkit_serdes_status_cmd,
        "serdes SERDES_ID status (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Serdes status",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_STATUS;

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_serdes_dfe,
        cli_dkit_serdes_dfe_cmd,
        "serdes SERDES_ID dfe (enable|disable) (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Set serdes dfe",
        "Enable",
        "Disable",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_DFE;
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (0xFF != index)
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_DFE_ON;
    }
    else
    {
        ctc_dkit_serdes_ctl_para.para[0] = CTC_DKIT_SERDIS_CTL_DFE_OFF;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        return g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_serdes_show_dfe,
        cli_dkit_serdes_show_dfe_cmd,
        "show serdes SERDES_ID dfe status (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Serdes",
        "Serdes ID",
        "Dfe",
        "Dfe status",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_ctl_para_t ctc_dkit_serdes_ctl_para;
    int32 ret = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();

    sal_memset(&ctc_dkit_serdes_ctl_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    ctc_dkit_serdes_ctl_para.type = CTC_DKIT_SERDIS_CTL_GET_DFE;
    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_ctl_para.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ctc_dkit_serdes_ctl_para.lchip = lchip;

    if (g_dkit_api[lchip]->serdes_ctl)
    {
        ret= g_dkit_api[lchip]->serdes_ctl(&ctc_dkit_serdes_ctl_para);
        if (ret)
        {
            return CLI_ERROR;
        }
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    if (CLI_SUCCESS == ret)
    {
        CTC_DKIT_PRINT("------------------------------------------------\n");
        CTC_DKIT_PRINT("Serdes id:%d\n", ctc_dkit_serdes_ctl_para.serdes_id);
        CTC_DKIT_PRINT("Dfe : %s\n", (CTC_DKIT_SERDIS_CTL_DFE_ON ==ctc_dkit_serdes_ctl_para.para[0])?"enable":"disable");
        CTC_DKIT_PRINT("Tap1 val: %d\n", (ctc_dkit_serdes_ctl_para.para[1] & 0xFF));
        CTC_DKIT_PRINT("Tap2 val: %d\n", ((ctc_dkit_serdes_ctl_para.para[1]>>8) & 0xFF));
        CTC_DKIT_PRINT("Tap3 val: %d\n", ((ctc_dkit_serdes_ctl_para.para[1]>>16) & 0xFF));
        CTC_DKIT_PRINT("Tap4 val: %d\n", ((ctc_dkit_serdes_ctl_para.para[1]>>24) & 0xFF));
        CTC_DKIT_PRINT("Tap5 val: %d\n", (ctc_dkit_serdes_ctl_para.para[2] & 0xFF));
        CTC_DKIT_PRINT("------------------------------------------------\n");
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_integrity_check,
        cli_dkit_integrity_check_cmd,
        "integrity-check GOLDEN_FILE RESULT_FILE (verbos |) (lchip CHIP_ID|)",
        "integrity check cmd",
        "Integrity check file name",
        "Result file name",
        "Show detail error message",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8  index = 0;
    int32  ret = CLI_SUCCESS;
    char*  p_golden_file = NULL;
    char*  p_result_file = NULL;
    uint32 verbos = 0;
    sal_file_t p_f = NULL;
    uint8 lchip = 0;

    p_golden_file = argv[0];
    p_result_file = argv[1];

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    index = CTC_CLI_GET_ARGC_INDEX("verbos");
    if (0xFF != index)
    {
        verbos = 1;
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->integrity_check)
    {
        ret = g_dkit_api[lchip]->integrity_check(lchip, p_golden_file, p_result_file, &verbos);
        if (CLI_SUCCESS != ret)
        {
            CTC_DKIT_PRINT("Integrity check result error!\n");
            if (argc > 2)
            {
                p_f = sal_fopen(argv[3], "r");
                if (NULL == p_f)
                {
                    CTC_DKIT_PRINT("Open stats debug file:%s failed!\n", argv[2]);
                }
                else
                {
                    sal_fclose(p_f);
                    ret = ctc_vti_command(g_ctc_vti, argv[3]);
                    if(ret && source_quiet_on)
                    {
                        CTC_DKIT_PRINT("%s\n", argv[3]);
                    }
                }
            }
        }
        return ret;
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_debug,
    cli_dkit_debug_cmd,
    "debug dkits (on | off)",
    "Debug info",
    "Dkits",
    "On",
    "Off")
{
    uint8 index = 0;
    index = CTC_CLI_GET_ARGC_INDEX("on");
    if (0xFF != index)
    {
        dkits_debug = TRUE;
    }
    else
    {
        dkits_debug = FALSE;
    }
    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_log,
    cli_dkit_log_cmd,
    "log (FILE_NAME| off)",
    "Log redirect to file",
    "File name",
    "Off")
{
    uint8 index = 0;
    char file_name[256] = {0};
    sal_time_t begine_time;
    char*      p_time_str = NULL;
    index = CTC_CLI_GET_ARGC_INDEX("off");
    if (0xFF != index)
    {
        if (dkits_log_file)
        {
            sal_fclose(dkits_log_file);
            dkits_log_file = NULL;
        }
    }
    else
    {
        if (dkits_log_file)
        {
            ctc_cli_out("Already log on!\n");
            return CLI_ERROR;
        }
        sal_memset(&begine_time, 0, sizeof(begine_time));
        sal_memcpy(file_name, argv[0], sal_strlen(argv[0]));
        sal_sprintf(file_name, "%s.txt", argv[0]);
        dkits_log_file = sal_fopen(file_name, "wb+");
        if (NULL == dkits_log_file)
        {
            return CLI_ERROR;
        }
        sal_time(&begine_time);
        p_time_str = sal_ctime(&begine_time);
        sal_fprintf(dkits_log_file, "----------------------------------------------------\n");
        sal_fprintf(dkits_log_file, "[Begin Time] %s", p_time_str);
        sal_fprintf(dkits_log_file, "----------------------------------------------------\n");
        ctc_cli_out("Dkits log file: %s\n", file_name);
    }
    return CLI_SUCCESS;
}


CTC_CLI(ctc_dkit_packet_dump,
        ctc_dkit_packet_dump_cmd,
        "packet (capture net-tx | dump (net-tx|net-rx|bsr (pkt-len PKT_LEN|))) gport GPORT (lchip CHIP_ID|)",
        "Packet cmd",
        "Capture",
        "Net tx buffer",
        "Dump",
        "Net tx buffer",
        "Net rx buffer",
        "Bsr buffer",
        "Packet length",
        "<0-4096> Byte",
        "Global port",
        "Gchip(5bit) | lport(8bit)",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8  index = 0;
    uint16 gport = 0;
    uint32 pkt_len = 0;
    uint8 lchip = 0;
    ctc_dkits_dump_packet_type_t dump_packet_type = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    CTC_CLI_GET_UINT16("gport", gport, argv[argc - 1]);

    index = CTC_CLI_GET_ARGC_INDEX("capture");
    if (0xFF != index)
    {
        dump_packet_type = CTC_DKITS_DUMP_PACKET_TYPE_CAPTURE;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("net-tx");
        if (0xFF != index)
        {
            dump_packet_type = CTC_DKITS_DUMP_PACKET_TYPE_NETTX;
        }
        index = CTC_CLI_GET_ARGC_INDEX("net-rx");
        if (0xFF != index)
        {
            dump_packet_type = CTC_DKITS_DUMP_PACKET_TYPE_NETRX;
        }
        index = CTC_CLI_GET_ARGC_INDEX("bsr");
        if (0xFF != index)
        {
            dump_packet_type = CTC_DKITS_DUMP_PACKET_TYPE_BSR;

            index = CTC_CLI_GET_ARGC_INDEX("pkt-len");
            if (0xFF != index)
            {
                CTC_CLI_GET_UINT32("pkt-len", pkt_len, argv[index + 1]);
            }
        }
    }
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    if (g_dkit_api[lchip]->packet_dump)
    {
        g_dkit_api[lchip]->packet_dump(lchip, &dump_packet_type, &gport, &pkt_len);
    }
    else
    {
        CTC_DKIT_NOT_SUPPORT();
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_dkit_table_read,
    cli_dkit_table_read_cmd,
    "read TBL_NAME INDEX FLD_NAME (lchip CHIP_ID|)",
    "Read",
    "Table name like [DsMac]",
    "Table index value",
    "Field name",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_dkit_tbl_reg_wr_t* p_table_info = NULL;
    uint8 table_name[CTC_MAX_TABLE_DATA_LEN];
    uint8 field_name[CTC_MAX_TABLE_DATA_LEN];
    uint32 table_index = 0;

    sal_memset(table_name, 0, CTC_MAX_TABLE_DATA_LEN);
    sal_memset(field_name, 0, CTC_MAX_TABLE_DATA_LEN);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    /*get tabel name*/
    sal_strncpy((char*)table_name, argv[0], sal_strlen(argv[0]));

    /*get tabel index*/
    CTC_CLI_GET_UINT32("table index", table_index, argv[1]);

    /*get field name*/
    sal_strncpy((char*)field_name, argv[2], sal_strlen(argv[2]));

    p_table_info = (ctc_dkit_tbl_reg_wr_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_dkit_tbl_reg_wr_t));
    if (NULL == p_table_info)
    {
        return CLI_ERROR;
    }
    sal_memset(p_table_info, 0, sizeof(ctc_dkit_tbl_reg_wr_t));
    p_table_info->table_name = table_name;
    p_table_info->field_name = field_name;
    p_table_info->table_index = table_index;
    ret = ctc_dkit_read_table(lchip, p_table_info);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", "error to read table");
        mem_free(p_table_info);
        return CLI_ERROR;
    }

    ctc_cli_out(" Tbl_name %s Fld_name %s index %u\n", p_table_info->table_name, p_table_info->field_name, p_table_info->table_index);
    ctc_cli_out(" Table Value:");
    ctc_cli_out(" 0x%x 0x%x\n", p_table_info->value[0], p_table_info->value[1]);
    mem_free(p_table_info);

    return CLI_SUCCESS;
}

CTC_CLI(cli_dkit_table_write,
    cli_dkit_table_write_cmd,
    "write TBL_NAME INDEX FLD_NAME VALUE (mask MASK |)(lchip CHIP_ID|)",
    "Write",
    "Table name like [DsMac]",
    "Table index value",
    "Field name",
    "Value",
    "Table mask",
    "Table mask value",
    CTC_DKITS_CHIP_DESC,
    CTC_DKITS_CHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_dkit_tbl_reg_wr_t* p_table_info = NULL;
    uint8 table_name[CTC_MAX_TABLE_DATA_LEN];
    uint8 field_name[CTC_MAX_TABLE_DATA_LEN];
    uint32 value = 0;
    uint32 mask = 0;
    uint32 table_index = 0;

    sal_memset(table_name, 0, CTC_MAX_TABLE_DATA_LEN);
    sal_memset(field_name, 0, CTC_MAX_TABLE_DATA_LEN);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_DKIT_INIT_CHECK();
    /*get tabel name*/
    sal_strncpy((char*)table_name, argv[0], sal_strlen(argv[0]));

    /*get tabel index*/
    CTC_CLI_GET_UINT32("table index", table_index, argv[1]);

    /*get field name*/
    sal_strncpy((char*)field_name, argv[2], sal_strlen(argv[2]));

    CTC_CLI_GET_UINT32("table value", value, argv[3]);

    index = CTC_CLI_GET_ARGC_INDEX("mask");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("table mask", mask, argv[index + 1]);
    }

    p_table_info = (ctc_dkit_tbl_reg_wr_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_dkit_tbl_reg_wr_t));
    if (NULL == p_table_info)
    {
        return CLI_ERROR;
    }
    sal_memset(p_table_info, 0, sizeof(ctc_dkit_tbl_reg_wr_t));

    p_table_info->table_name = table_name;
    p_table_info->field_name = field_name;
    p_table_info->table_index = table_index;
    p_table_info->value[0] = value;
    p_table_info->mask[0] = mask;
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    ret = ctc_dkit_write_table(lchip, p_table_info);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", "error to write table");
        mem_free(p_table_info);
        return CLI_ERROR;
    }
    mem_free(p_table_info);

    return CLI_SUCCESS;
}

int32
ctc_dkit_cli_chip_special_deinit(uint8 lchip)
{
    uint8 chip_type = 0;

    chip_type = g_ctcs_api_en ? ctcs_get_chip_type(g_api_lchip) : ctc_get_chip_type();

    switch (chip_type)
    {
#ifdef GREATBELT
        case CTC_DKIT_CHIP_GREATBELT:
            ctc_greatbelt_dkit_internal_cli_deinit(CTC_DKITS_MODE);
            break;
#endif
#ifdef GOLDENGATE
        case CTC_DKIT_CHIP_GOLDENGATE:
            ctc_goldengate_dkit_internal_cli_deinit(CTC_DKITS_MODE);
            break;
#endif
#if defined(DUET2) || defined(TSINGMA)
        case CTC_DKIT_CHIP_DUET2:
            ctc_usw_dkit_internal_cli_deinit(g_api_lchip, CTC_DKITS_MODE);
            break;
#endif
        default:
            break;
    }

    return CLI_SUCCESS;
}

int32
ctc_dkit_cli_chip_special_init(uint8 lchip)
{
    uint8 chip_type = 0;

    chip_type = g_ctcs_api_en ? ctcs_get_chip_type(g_api_lchip) : ctc_get_chip_type();

    switch (chip_type)
    {
#ifdef GREATBELT
        case CTC_DKIT_CHIP_GREATBELT:
            ctc_greatbelt_dkit_internal_cli_init(CTC_DKITS_MODE);
            break;
#endif
#ifdef GOLDENGATE
        case CTC_DKIT_CHIP_GOLDENGATE:
            ctc_goldengate_dkit_internal_cli_init(CTC_DKITS_MODE);
            break;
#endif
#if defined(DUET2) || defined(TSINGMA)
        case CTC_DKIT_CHIP_DUET2:
        case CTC_DKIT_CHIP_TSINGMA:
            ctc_usw_dkit_internal_cli_init(g_api_lchip, CTC_DKITS_MODE);
            break;
#endif
        default:
            break;
    }
    return CLI_SUCCESS;
}

int32
ctc_dkit_cli_init(uint8 cli_tree_mode)
{
    uint8 lchip = 0;
    uint8 chip_num = 0;
    uint32 dev_id = 0;

    /*init drv and api*/
    dal_get_chip_number(&chip_num);
    for (lchip = 0; lchip < chip_num; lchip++)
    {
        dal_get_chip_dev_id(lchip,  &dev_id);
        switch (dev_id)
        {
#if defined(GREATBELT)
            case DAL_GREATBELT_DEVICE_ID:
                ctc_greatbelt_dkit_init(lchip);
                break;
#endif
#if defined(GOLDENGATE)
            case DAL_GOLDENGATE_DEVICE_ID:
            case DAL_GOLDENGATE_DEVICE_ID1:
                ctc_goldengate_dkit_init(lchip);
                break;
#endif
#if defined(DUET2) || defined(TSINGMA)
            case DAL_DUET2_DEVICE_ID:
            case DAL_TSINGMA_DEVICE_ID:
                ctc_usw_dkit_init(lchip);
                break;
#endif
            default:
                break;
        }
    }
    ctc_dkit_cli_chip_special_init(0);
    ctc_com_cli_init(cli_tree_mode);

    /*1. Basic function*/
    install_element(cli_tree_mode, &cli_dkit_show_version_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_device_info_cmd);

    install_element(cli_tree_mode, &cli_dkit_list_tbl_cmd);
    install_element(cli_tree_mode, &cli_dkit_read_reg_or_tbl_cmd);
    install_element(cli_tree_mode, &cli_dkit_write_reg_or_tbl_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_tbl_reg_by_data_cmd);

    install_element(CTC_SDK_MODE, &cli_dkit_list_tbl_cmd);
    install_element(CTC_SDK_MODE, &cli_dkit_read_reg_or_tbl_cmd);
    install_element(CTC_SDK_MODE, &cli_dkit_write_reg_or_tbl_cmd);
    install_element(CTC_SDK_MODE, &cli_dkit_check_reg_or_tbl_cmd);

#if 0
    install_element(CTC_SDK_MODE, &cli_dbg_tool_write_tbl_reg_fld_cmd);
#endif

     /*install_element(cli_tree_mode, &cli_dkit_reset_reg_or_tbl_cmd);*/
    install_element(cli_tree_mode, &cli_dkit_show_tbl_reg_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_interface_status_cmd);

#if(SDK_WORK_PLATFORM == 0)
    install_element(cli_tree_mode, &cli_dkit_read_address_cmd);
    install_element(cli_tree_mode, &cli_dkit_write_address_cmd);
    install_element(cli_tree_mode, &cli_dkit_check_address_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_tbl_reg_by_addr_cmd);
#endif

    /*2. Normal function*/
    install_element(cli_tree_mode, &cli_dkit_install_capture_flow_cmd);
    install_element(cli_tree_mode, &cli_dkit_install_capture_ipe_cmd);
    install_element(cli_tree_mode, &cli_dkit_install_capture_bsr_cmd);
    install_element(cli_tree_mode, &cli_dkit_install_capture_metfifo_cmd);
    install_element(cli_tree_mode, &cli_dkit_install_capture_oam_cmd);
    install_element(cli_tree_mode, &cli_dkit_install_capture_epe_cmd);
    install_element(cli_tree_mode, &cli_dkit_clear_capture_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_discard_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_discard_type_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_discard_stats_cmd);
    install_element(cli_tree_mode, &cli_dkit_discard_to_cpu_cmd);
    install_element(cli_tree_mode, &cli_dkit_discard_display_en_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_path_cmd);
    install_element(cli_tree_mode, &cli_dkit_tcam_scan_cmd);
    install_element(cli_tree_mode, &cli_dkit_tcam_show_key_type_cmd);
    install_element(cli_tree_mode, &cli_dkit_tcam_capture_start_cmd);
    install_element(cli_tree_mode, &cli_dkit_tcam_capture_stop_cmd);
    install_element(cli_tree_mode, &cli_dkit_tcam_capture_show_cmd);

    /*3. Advanced function*/
    install_element(cli_tree_mode, &cli_dkit_show_monitor_q_id_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_monitor_q_depth_cmd);
    install_element(cli_tree_mode, &cli_dkit_show_monitor_sensor_cmd);
    install_element(cli_tree_mode, &cli_dkit_monitor_temperature_cmd);

    install_element(cli_tree_mode, &ctc_dkit_dump_cfg_dump_cmd);
    install_element(cli_tree_mode, &ctc_dkit_dump_cfg_decode_cmd);
    install_element(cli_tree_mode, &ctc_dkit_dump_cfg_cmp_cmd);
    install_element(cli_tree_mode, &cli_dkit_dump_memory_usage_cmd);

    install_element(cli_tree_mode, &cli_dkit_serdes_loopback_cmd);
    install_element(cli_tree_mode, &cli_dkit_serdes_pbrs_cmd);
    install_element(cli_tree_mode, &cli_dkit_polr_check_cmd);
    install_element(cli_tree_mode, &cli_dkit_serdes_eye_cmd);
    install_element(cli_tree_mode, &cli_dkit_serdes_ffe_cmd);
    install_element(cli_tree_mode, &cli_dkit_serdes_status_cmd);

    install_element(cli_tree_mode, &cli_dkit_integrity_check_cmd);
    install_element(cli_tree_mode, &ctc_dkit_packet_dump_cmd);

    install_element(cli_tree_mode, &cli_dkit_debug_cmd);
    install_element(cli_tree_mode, &cli_dkit_log_cmd);

    install_element(cli_tree_mode, &cli_dkit_serdes_dfe_cmd);
    install_element(cli_tree_mode, &cli_dkit_serdes_show_dfe_cmd);

    /*dkit api*/
    install_element(CTC_INTERNAL_MODE, &cli_dkit_table_read_cmd);
    install_element(CTC_INTERNAL_MODE, &cli_dkit_table_write_cmd);

    return CLI_SUCCESS;
}

