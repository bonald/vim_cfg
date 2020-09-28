#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"

#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_dkit_common.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_memory.h"
#include "ctc_usw_dkit_drv.h"
#include "ctc_usw_dkit_captured_info.h"
#include "ctc_usw_dkit_path.h"
#include "ctc_usw_dkit_discard.h"
#include "ctc_usw_dkit_monitor.h"
#if 0
#include "ctc_usw_dkit_interface.h"
#endif
#include "ctc_usw_dkit_tcam.h"

#include "ctc_usw_dkit_dump_cfg.h"
#include "ctc_usw_dkit_misc.h"
#if 1
#include "ctc_usw_dkit_internal.h"
#endif

ctc_dkit_master_t* g_usw_dkit_master[CTC_DKITS_MAX_LOCAL_CHIP_NUM] = {NULL};
extern ctc_dkit_api_t* g_dkit_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];
extern ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

int32 ctc_usw_dkit_chip_init(uint8 lchip)
{
    if (g_dkit_chip_api[lchip] == NULL)
    {
        return CLI_ERROR;
    }
    g_dkit_chip_api[lchip]->dkits_dump_decode_entry = ctc_usw_dkits_dump_decode_entry;
    g_dkit_chip_api[lchip]->drv_chip_read = drv_usw_chip_read;
    g_dkit_chip_api[lchip]->drv_chip_write = drv_usw_chip_write;
    g_dkit_chip_api[lchip]->drv_get_tbl_string_by_id = drv_usw_get_tbl_string_by_id;
    g_dkit_chip_api[lchip]->drv_get_host_type = drv_get_host_type;
    g_dkit_chip_api[lchip]->dkits_dump_data2tbl = ctc_usw_dkits_dump_data2tbl;
    ctc_usw_dkit_misc_serdes_register(lchip);
    ctc_usw_dkit_monitor_sensor_register(lchip);

    return 0;
}

/*special for emulation*/
#define __EMULATION__
extern fields_t *
drv_usw_find_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id);
extern int32
drv_usw_chip_write(uint8 lchip, uint32 offset, uint32 value);
#define TIPS_NONE           0
#define TIPS_TBL_REG        1
#define TIPS_TBL_REG_FIELD  2

int32 show_tips_info(uint8 lchip, uint8 tips_type, char* inc_str, tbls_id_t id)
{
    tbls_id_t tbl_reg_id = 0;
    char comment[128] = {0};
    char up_comment[128] = {0};
    char up_str[128] = {0};
    uint8  i = 0;
    fld_id_t field_id = 0;

    fields_t* first_f = NULL;
    fields_t* end_f = NULL;
    uint32 num_f = 0;

    if (((NULL == inc_str)&&(TIPS_TBL_REG == tips_type))
        || (TIPS_NONE == tips_type))
    {
        return CLI_ERROR;
    }

    if (NULL != inc_str)
    {
        for (i = 0; i < sal_strlen(inc_str); i++)
        {
            up_str[i] = sal_toupper(inc_str[i]);
        }
    }

    if (TIPS_TBL_REG == tips_type)
    {
        for (tbl_reg_id = 0; tbl_reg_id  < MaxTblId_t; tbl_reg_id++)
        {
            if (DRV_E_NONE != drv_usw_get_tbl_string_by_id(lchip, tbl_reg_id, comment))
            {
                ctc_cli_out(" Get TBL_REG_Name by tbl/RegId error! tbl/RegId: %d, Line = %d\n",
                                      tbl_reg_id, __LINE__);
            }
            else
            {
                for (i = 0; i < sal_strlen(comment); i++)
                {
                    up_comment[i] = sal_toupper(comment[i]);
                }
                if (NULL != sal_strstr(up_comment, up_str))
                {
                    ctc_cli_out("ID: %04d, \tNAME: %s\n", tbl_reg_id, comment);
                }
                sal_memset(comment, 0, sizeof(comment));
                sal_memset(up_comment, 0, sizeof(up_comment));
            }
        }
    }
    else if (TIPS_TBL_REG_FIELD == tips_type)
    {
        if (MaxTblId_t <= id)
        {
            ctc_cli_out("\nERROR! INVALID tbl/Reg ID! tbl/Reg ID: %d, file:%s line:%d function:%s\n",
                        id,__FILE__,__LINE__,__FUNCTION__);
            return CLI_ERROR;
        }

        first_f = TABLE_INFO(lchip, id).ptr_fields;
        num_f = TABLE_INFO(lchip, id).field_num;
        end_f = &first_f[num_f];

        while (first_f < end_f)
        {
            ctc_cli_out("FIELD_ID = %2d, \tFIELD_NAME: %-32s\n", field_id, first_f->ptr_field_name);
            first_f++;
            field_id++;
        }
    }

    return CLI_SUCCESS;
}

int write_entry(uint8 lchip, tbls_id_t id, uint32 index, uint32 offset, uint32 value)
{
    uint8 lchip_base  = 0;
    uint32 hw_addr_base = 0;
    uint32 entry_size   = 0;
    uint32 max_entry    = 0;
    uint32 hw_addr_offset      = 0;
    int32  ret = 0;
    uint8 addr_offset  = 0;

    if (MaxTblId_t <= id)
    {
        ctc_cli_out("\nERROR! INVALID tbl/RegID! tbl/RegID: %d, file:%s line:%d function:%s\n",
                    id,__FILE__,__LINE__,__FUNCTION__);
        return CLI_ERROR;
    }

    DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, id, index, &addr_offset));
    entry_size      = TABLE_ENTRY_SIZE(lchip, id);
    max_entry       = TABLE_MAX_INDEX(lchip, id);
    hw_addr_base    = TABLE_DATA_BASE(lchip, id, addr_offset);
    if (index >= max_entry)
    {
        return CLI_ERROR;
    }
    /*TODO: dynamic table*/
    if ((4 == entry_size) || (8 == entry_size))
    {/*1w or 2w*/
        hw_addr_offset  =   hw_addr_base + index * entry_size + offset;
    }
    else if (entry_size <= 4*4)
    {/*4w*/
        hw_addr_offset  =   hw_addr_base + index * 4*4 + offset;
    }
    else if (entry_size <= 4*8)
    {/*8w*/
        hw_addr_offset  =   hw_addr_base + index * 4*8 + offset;
    }
    else if (entry_size <= 4*16)
    {/*16w*/
        hw_addr_offset  =   hw_addr_base + index * 4*16 + offset;
    }
    else if (entry_size <= 4*32)
    {/*32w*/
        hw_addr_offset  =   hw_addr_base + index * 4*32 + offset;
    }
    else if (entry_size <= 4*64)
    {/*64w*/
        hw_addr_offset  =   hw_addr_base + index * 4*64 + offset;
    }

    lchip_base = 0;


     /*-ctc_cli_out("Offset write +++! entry_size = %d, max_entry = %d\n", entry_size,max_entry);*/
     /*-ctc_cli_out("Offset write +++! hw_addr_offset = 0x%08x, hw_addr_base = 0x%08x\n", hw_addr_offset,hw_addr_base);*/
    ret = drv_usw_chip_write(lchip-lchip_base, hw_addr_offset, value);

    if (ret < DRV_E_NONE)
    {
        ctc_cli_out("0x%08x address write ERROR! Value = 0x%08x\n", hw_addr_offset, value);
        return ret;
    }

    return CLI_SUCCESS;
}

int32 write_tbl_reg(uint8 lchip, tbls_id_t id, uint32 index, fld_id_t field_id, uint32* value)
{
    uint32 cmd = 0;
    int32 ret = 0;

    if (MaxTblId_t <= id)
    {
        ctc_cli_out("\nERROR! INVALID tbl/RegID! tbl/RegID: %d, file:%s line:%d function:%s\n",
                    id,__FILE__,__LINE__,__FUNCTION__);
        return CLI_ERROR;
    }

    cmd = DRV_IOW(id, field_id);
    ret = DRV_IOCTL(lchip, index, cmd, value);

    if (ret < 0)
    {
        return ret;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkit_get_chip_name(uint8 lchip, char* chip_name)
{
    if (NULL == chip_name)
    {
        return CLI_ERROR;
    }

    if (DRV_IS_DUET2(lchip))
    {
        sal_strcpy(chip_name, "duet2");
    }
    else if (DRV_IS_TSINGMA(lchip))
    {
        sal_strcpy(chip_name, "tsingma");
    }
    else
    {
        sal_strcpy(chip_name, "not support chip");
    }

    return CLI_SUCCESS;
}

int32 ctc_usw_dkit_init(uint8 lchip)
{
    uint8 gchip = 0;
    int32 ret = 0;

    /*check drv db valid*/
    ret = drv_get_gchip_id(lchip, &gchip);
    if(DRV_E_NOT_INIT == ret)
    {
        return CLI_SUCCESS;
    }
    if(lchip >= CTC_DKITS_MAX_LOCAL_CHIP_NUM)
    {
        return CLI_SUCCESS;
    }

    g_dkit_api[lchip] = (ctc_dkit_api_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_api_t));
    if (NULL == g_dkit_api[lchip])
    {
        CTC_DKIT_PRINT("No memory for dkits!!!\n");
        goto ERROE;
    }

    g_dkit_chip_api[lchip] = (ctc_dkit_chip_api_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_chip_api_t));
    if (NULL == g_dkit_chip_api[lchip])
    {
        CTC_DKIT_PRINT("No memory for dkits!!!\n");
        goto ERROE;
    }

    sal_memset(g_dkit_api[lchip], 0 , sizeof(ctc_dkit_api_t));
    sal_memset(g_dkit_chip_api[lchip], 0 , sizeof(ctc_dkit_chip_api_t));

    g_usw_dkit_master[lchip] = (ctc_dkit_master_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_master_t));
    if (NULL == g_usw_dkit_master[lchip])
    {
        CTC_DKIT_PRINT("No memory for dkits!!!\n");
        goto ERROE;
    }
    sal_memset(g_usw_dkit_master[lchip], 0 , sizeof(ctc_dkit_master_t));

    g_usw_dkit_master[lchip]->discard_enable_bitmap = mem_malloc(MEM_CLI_MODULE, ((CTC_DKIT_DISCARD_MAX / 32) + 1)*4);
    if (NULL == g_usw_dkit_master[lchip]->discard_enable_bitmap)
    {
        CTC_DKIT_PRINT("No memory for dkits!!!\n");
        goto ERROE;
    }
    sal_memset(g_usw_dkit_master[lchip]->discard_enable_bitmap, 0xFF , ((CTC_DKIT_DISCARD_MAX / 32) + 1)*4);


    /*register API*/
    /*1. Basic*/
    g_dkit_api[lchip]->memory_process = ctc_usw_dkit_memory_process;
    g_dkit_api[lchip]->show_discard = ctc_usw_dkit_discard_process;
    g_dkit_api[lchip]->show_discard_type = ctc_usw_dkit_discard_show_type;
    g_dkit_api[lchip]->discard_to_cpu_en = ctc_usw_dkit_discard_to_cpu_en;
    g_dkit_api[lchip]->show_device_info = ctc_usw_dkit_device_info;


    /*2. Normal*/
    g_dkit_api[lchip]->install_captured_flow = ctc_usw_dkit_captured_install_flow;
    g_dkit_api[lchip]->clear_captured_flow = ctc_usw_dkit_captured_clear;

    g_dkit_api[lchip]->show_path = ctc_usw_dkit_path_process;
    g_dkit_api[lchip]->show_discard_stats = ctc_usw_dkit_discard_show_stats;
    g_dkit_api[lchip]->discard_display_en = ctc_usw_dkit_discard_display_en;

    g_dkit_api[lchip]->show_queue_depth = ctc_usw_dkit_monitor_show_queue_depth;
    g_dkit_api[lchip]->show_queue_id = ctc_usw_dkit_monitor_show_queue_id;
    #if 0
    g_dkit_api[lchip]->show_tcam_key_type = ctc_usw_dkits_tcam_show_key_type;
    g_dkit_api[lchip]->tcam_capture_start = ctc_usw_dkits_tcam_capture_start;
    g_dkit_api[lchip]->tcam_capture_stop = ctc_usw_dkits_tcam_capture_stop;
    g_dkit_api[lchip]->tcam_capture_show = ctc_usw_dkits_tcam_capture_show;
    g_dkit_api[lchip]->tcam_scan = ctc_usw_dkits_tcam_scan;
    #endif

    g_dkit_api[lchip]->cfg_usage = ctc_usw_dkits_dump_memory_usage;
    g_dkit_api[lchip]->cfg_dump = ctc_usw_dkits_dump_cfg_dump;
    g_dkit_api[lchip]->cfg_decode = ctc_usw_dkits_dump_cfg_decode;
    g_dkit_api[lchip]->cfg_cmp = ctc_usw_dkits_dump_cfg_cmp;

    /*3. Advanced*/
    g_dkit_api[lchip]->show_sensor_result = ctc_usw_dkit_monitor_show_sensor_result;
    g_dkit_api[lchip]->serdes_ctl = ctc_usw_dkit_misc_serdes_ctl;
    /*4. dkit api for user*/
    g_dkit_api[lchip]->read_table = ctc_usw_dkit_memory_read_table;
    g_dkit_api[lchip]->write_table = ctc_usw_dkit_memory_write_table;
    g_dkit_api[lchip]->integrity_check = ctc_usw_dkit_misc_integrity_check;

    ctc_usw_dkit_chip_init(lchip);

    return CLI_SUCCESS;

ERROE:
    CTC_DKIT_PRINT("Dkits init fail!!!\n");
    if (g_dkit_api[lchip])
    {
        mem_free(g_dkit_api[lchip]);
        g_dkit_api[lchip] = NULL;
    }

    if (g_dkit_chip_api[lchip])
    {
        mem_free(g_dkit_chip_api[lchip]);
        g_dkit_chip_api[lchip] = NULL;
    }

    if (g_usw_dkit_master[lchip])
    {
        mem_free(g_usw_dkit_master[lchip]);
        g_usw_dkit_master[lchip] = NULL;
    }

    if (g_usw_dkit_master[lchip]->discard_enable_bitmap)
    {
        mem_free(g_usw_dkit_master[lchip]->discard_enable_bitmap);
        g_usw_dkit_master[lchip]->discard_enable_bitmap = NULL;
    }

    return CLI_SUCCESS;
}

