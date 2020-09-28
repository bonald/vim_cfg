/**
 @file ctc_usw_ftm_cli.c

 @date 2009-11-23

 @version v2.0

---file comments----
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_ftm_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_alloc.h"
#include "ctc_error.h"
#include "ctc_debug.h"

extern int32 sys_usw_opf_print_alloc_info(uint8 lchip, uint8 pool_type, uint8 pool_index);
extern int32 sys_usw_opf_print_type(uint8 lchip, uint8 opf_type, uint8 is_all);
extern int32 sys_usw_ftm_show_alloc_info_detail(uint8 lchip);
extern int32 sys_usw_ftm_show_alloc_info(uint8 lchip);
extern int32 drv_usw_ftm_get_tcam_tbl_info_detail(uint8 lchip, char* P_tbl_name);
extern int32 drv_usw_ftm_table_id_2_mem_id(uint8 lchip, uint32 tbl_id, uint32 tbl_idx, uint32* p_mem_id, uint32* p_offset);
extern int32 drv_usw_ftm_get_cam_by_tbl_id(uint32 tbl_id, uint8* cam_type, uint8* cam_num);
extern int32 drv_usw_get_tbl_id_by_string();
extern int32 sys_usw_opf_print_alloc_used_info(uint8 lchip, uint8 start_pool_type, uint8 end_pool_type);
extern int32 drv_usw_ftm_dump_table_info(uint8 lchip, uint32 tbl_id);

CTC_CLI(ctc_cli_usw_ftm_show_opf_type_info,
        ctc_cli_usw_ftm_show_opf_type_info_cmd,
        "show opf type (TYPE|all)",
        CTC_CLI_SHOW_STR,
        "Offset pool freelist",
        "OPF type",
        "OPF type value",
        "All types")
{
    uint8 opf_type;
    uint8 lchip = 0;
    int32 ret = CLI_SUCCESS;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if (0 == sal_memcmp("all", argv[0], 3))
    {
        ret = sys_usw_opf_print_type(lchip,0,1);
    }
    else
    {
        CTC_CLI_GET_UINT8_RANGE("opf type", opf_type, argv[0], 0, CTC_MAX_UINT8_VALUE);
        ret = sys_usw_opf_print_type(lchip, opf_type, 0);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ftm_show_opf_alloc_info,
        ctc_cli_usw_ftm_show_opf_alloc_info_cmd,
        "show opf alloc type TYPE index INDEX (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Offset pool freelist",
        "OPF alloc info",
        "OPF type",
        "OPF type value",
        "Index of opf pool",
        "<0-0xFF>")
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 pool_type = 0;
    uint8 pool_index = 0;

    CTC_CLI_GET_UINT8_RANGE("opf type", pool_type, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("opf index", pool_index, argv[1], 0, CTC_MAX_UINT8_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_opf_print_alloc_info(lchip, pool_type, pool_index);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_ftm_show_ftm_info,
        ctc_cli_usw_ftm_show_ftm_info_cmd,
        "show ftm info (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "Information of TCAM and Sram",
        "show detail ftm info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        ret = sys_usw_ftm_show_alloc_info_detail(lchip);
    }
    else
    {
        ret = sys_usw_ftm_show_alloc_info(lchip);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_ftm_show_opf_alloc_status,
        ctc_cli_usw_ftm_show_opf_alloc_status_cmd,
        "show opf alloc type TYPE end-type TYPE",
        CTC_CLI_SHOW_STR,
        "Offset pool freelist",
        "OPF alloc info",
        "OPF type",
        "OPF type value",
        "OPF end type",
        "OPF type value")
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 start_type = 0;
    uint8 end_type = 0;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    CTC_CLI_GET_UINT8_RANGE("opf type", start_type, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("end-type", end_type, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ret = sys_usw_opf_print_alloc_used_info(lchip,start_type,end_type);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_usw_ftm_show_tcam_detail_info,
        ctc_cli_usw_ftm_show_tcam_detail_info_cmd,
        "show ftm tcam alloc tbl TBL_NAME_ID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "Tcam",
        "Alloc",
        "Table",
        "Table id",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    char buf[256] = {0};
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;

    sal_strncpy((char*)buf, argv[0], 256);
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = drv_usw_ftm_get_tcam_tbl_info_detail(lchip, buf);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_ftm_get_tbl_ecc_info,
        ctc_cli_usw_ftm_get_tbl_ecc_info_cmd,
        "show ftm memory alloc tbl TBL_NAME_ID (index INDEX) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "Memory",
        "Alloc",
        "Table",
        "Table id",
        "Index",
        "Index value",
        "Local chip",
        "Chip id")
{
    int32  ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;
    uint32  table = 0;
    uint32 idx_value = 0;
    uint32 sram_id = 0;
    uint32 sram_offset = 0;
    char buf[256] = {0};

    sal_strncpy((char*)buf, argv[0], 256);

    index = CTC_CLI_GET_ARGC_INDEX("index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("index", idx_value, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret += drv_usw_get_tbl_id_by_string(lchip, &table, buf);
    ret += drv_usw_ftm_table_id_2_mem_id(lchip, table, idx_value, &sram_id, &sram_offset);

    ctc_cli_out("%-30s :%d\n", "Lchip ID", lchip);
    ctc_cli_out("%-30s :%s\n", "Memory Table ID", buf);
    ctc_cli_out("%-30s :%d\n", "Memory Table Index", idx_value);
    ctc_cli_out("%-30s :%d\n", "Memory Sram Id", sram_id);
    ctc_cli_out("%-30s :%d\n", "Memory Sram Offset",sram_offset);

    if (ret)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_ftm_tbl_cam_num,
        ctc_cli_usw_ftm_tbl_cam_num_cmd,
        "show ftm cam  tbl TBL_NAME_ID  (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "Cam",
        "Table",
        "Table id",
        "Local chip",
        "Chip id")
{
    int32  ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;
    uint32  table = 0;
    uint8 cam_type = 0;
    uint8 cam_num = 0;

    char buf[256] = {0};

    sal_strncpy((char*)buf, argv[0], 256);


    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret += drv_usw_get_tbl_id_by_string(lchip, &table, buf);
    if (ret)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ret += drv_usw_ftm_get_cam_by_tbl_id(table, &cam_type, &cam_num);
    if (ret)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-30s :%d\n", "Lchip ID", lchip);
    ctc_cli_out("%-30s :%s\n", "Memory Table ID", buf);
    ctc_cli_out("%-30s :%d\n", "cam num", cam_num);


    return CLI_SUCCESS;
}
#define ____________TEMP_FOR_OFB_TEST______________

#include "sys_usw_ofb.h"
CTC_CLI(ctc_cli_usw_ofb_init,
        ctc_cli_usw_ofb_init_cmd,
        "ofb init NUM SIZE (lchip LCHIP|)",
        "ofb",
        "init",
        "block number",
        "offset size",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16 block_num = 0;
    uint32 size = 0;
    uint8 ofb_type = 0;

    CTC_CLI_GET_UINT16("block_num", block_num, argv[0]);
    CTC_CLI_GET_UINT32("size", size, argv[1]);


    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_ofb_init(lchip, block_num, size, &ofb_type);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("new ofb type: %d\n", ofb_type);

    return ret;
}

CTC_CLI(ctc_cli_usw_ofb_offset_init,
        ctc_cli_usw_ofb_offset_init_cmd,
        "ofb offset-init TYPE BLOCK SIZE MULT MAX (lchip LCHIP|)",
        "ofb",
        "offset init",
        "ofp type",
        "block id",
        "offset size",
        "multiple",
        "max limit offset",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16 block_id = 0;
    uint8 ofb_type;
    sys_usw_ofb_param_t ofb_param;

    sal_memset(&ofb_param, 0, sizeof(ofb_param));
    CTC_CLI_GET_UINT8("ofb_type", ofb_type, argv[0]);
    CTC_CLI_GET_UINT16("block_id", block_id, argv[1]);
    CTC_CLI_GET_UINT32("size", ofb_param.size, argv[2]);
    CTC_CLI_GET_UINT8("multiple", ofb_param.multiple, argv[3]);
    CTC_CLI_GET_UINT32("max_limit_offset", ofb_param.max_limit_offset, argv[4]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_ofb_alloc,
        ctc_cli_usw_ofb_alloc_cmd,
        "ofb alloc TYPE BLOCK (lchip LCHIP|)",
        "ofb",
        "alloc",
        "ofp type",
        "block id",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16 block_id = 0;
    uint8 ofb_type;
    uint32 offset = 0;
    uint32* data = NULL;

    CTC_CLI_GET_UINT8("ofb_type", ofb_type, argv[0]);
    CTC_CLI_GET_UINT16("block_id", block_id, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    data = mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32));
    ret = sys_usw_ofb_alloc_offset(lchip, ofb_type, block_id, &offset, data);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("new offset: %d\n", offset);

    return ret;
}

CTC_CLI(ctc_cli_usw_ofb_free,
        ctc_cli_usw_ofb_free_cmd,
        "ofb free TYPE BLOCK OFFSET (lchip LCHIP|)",
        "ofb",
        "free",
        "ofp type",
        "block id",
        "offset",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16 block_id = 0;
    uint8 ofb_type;
    uint32 offset = 0;

    CTC_CLI_GET_UINT8("ofb_type", ofb_type, argv[0]);
    CTC_CLI_GET_UINT16("block_id", block_id, argv[1]);
    CTC_CLI_GET_UINT16("offset", offset, argv[2]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_ofb_free_offset(lchip, ofb_type, block_id, offset);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}


CTC_CLI(ctc_cli_usw_show_ofb_status,
        ctc_cli_usw_show_ofb_status_cmd,
        "show ofb status (lchip LCHIP|)",
        "show",
        "ofb",
        "status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_ofb_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_usw_show_ofb_offset,
        ctc_cli_usw_show_ofb_offset_cmd,
        "show ofb offset TYPE (lchip LCHIP|)",
        "show",
        "ofb",
        "offset",
        "ofp type",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 ofb_type;
    CTC_CLI_GET_UINT8("ofb_type", ofb_type, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_ofb_show_offset(lchip, ofb_type);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_show_tbl_info,
        ctc_cli_usw_show_tbl_info_cmd,
        "show ftm table TABLE_NAME (lchip LCHIP|)",
        "show",
        "ftm",
        "table",
        "table name like [DsMac]",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 tbl_id  = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    char buf[256] = {0};

    sal_strncpy((char*)buf, argv[0], 256);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ret = drv_usw_get_tbl_id_by_string(lchip, &tbl_id, buf);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = drv_usw_ftm_dump_table_info(lchip, tbl_id);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


#define ________________TEMP_END__________________




int32
ctc_usw_ftm_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_opf_type_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_opf_alloc_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_ftm_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_tcam_detail_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_get_tbl_ecc_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_tbl_cam_num_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_opf_alloc_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_tbl_info_cmd);


    install_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_init_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_offset_init_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_alloc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_free_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_ofb_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_ofb_offset_cmd);

    return CTC_E_NONE;
}

int32
ctc_usw_ftm_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_opf_type_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_opf_alloc_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_ftm_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_tcam_detail_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_get_tbl_ecc_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_tbl_cam_num_cmd);
	uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ftm_show_opf_alloc_status_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_init_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_offset_init_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_alloc_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ofb_free_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_ofb_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_ofb_offset_cmd);

    return CTC_E_NONE;
}

