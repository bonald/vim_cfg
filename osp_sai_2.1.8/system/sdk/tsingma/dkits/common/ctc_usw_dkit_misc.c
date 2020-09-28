#include "sal.h"
#include "ctc_cli.h"
#include "usw/include/drv_enum.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_chip_ctrl.h"
#include "ctc_dkit.h"
#include "ctc_dkit_common.h"
#include "ctc_usw_dkit_misc.h"
#include "ctc_usw_dkit.h"
#include "common/duet2/ctc_dt2_dkit_serdes.h"
#include "common/tsingma/ctc_tm_dkit_serdes.h"
extern ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

#define MAX_PER_LINE                 512
#define CTC_DKIT_LINKLIST_MAX_OP_NUM 7

enum ctc_dkit_misc_integrity_op_e
{
    CTC_DKIT_MISC_INTEGRITY_OP_NONE,
    CTC_DKIT_MISC_INTEGRITY_OP_E,
    CTC_DKIT_MISC_INTEGRITY_OP_BE,
    CTC_DKIT_MISC_INTEGRITY_OP_LE,
    CTC_DKIT_MISC_INTEGRITY_OP_B,
    CTC_DKIT_MISC_INTEGRITY_OP_L,
    CTC_DKIT_MISC_INTEGRITY_OP_NUM
};
typedef enum ctc_dkit_misc_integrity_op_e ctc_dkit_misc_integrity_op_t;

enum ctc_dkit_misc_integrity_check_e
{
    CTC_DKIT_MISC_INTEGRITY_CHECK_NONE,
    CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS,
    CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST,
    CTC_DKIT_MISC_INTEGRITY_CHECK_INTR,
    CTC_DKIT_MISC_INTEGRITY_CHECK_NUM
};
typedef enum ctc_dkit_misc_integrity_check_e ctc_dkit_misc_integrity_check_t;
static uint32 bit_map[1024];
#define __SERDES__
int32
ctc_usw_dkit_misc_read_serdes(void* para)
{
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    int32 ret = 0;
    if(g_dkit_chip_api[p_para->lchip]->dkits_read_serdes)
    {
        ret = g_dkit_chip_api[p_para->lchip]->dkits_read_serdes(para);
    }

    return ret;
}


int32
ctc_usw_dkit_misc_write_serdes(void* para)
{
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    int32 ret = 0;
    if(g_dkit_chip_api[p_para->lchip]->dkits_write_serdes)
    {
        ret = g_dkit_chip_api[p_para->lchip]->dkits_write_serdes(para);
    }

    return ret;
}


int32
ctc_usw_dkit_misc_read_serdes_aec_aet(void* para)
{
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    int32 ret = 0;
    if(g_dkit_chip_api[p_para->lchip]->dkits_read_serdes_aec_aet)
    {
        ret = g_dkit_chip_api[p_para->lchip]->dkits_read_serdes_aec_aet(para);
    }

    return ret;
}

int32
ctc_usw_dkit_misc_write_serdes_aec_aet(void* para)
{
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    int32 ret = 0;
    if(g_dkit_chip_api[p_para->lchip]->dkits_write_serdes_aec_aet)
    {
        ret = g_dkit_chip_api[p_para->lchip]->dkits_write_serdes_aec_aet(para);
    }

    return ret;
}


int32
ctc_usw_dkit_misc_serdes_resert(uint8 lchip, uint16 serdes_id)
{
    int32 ret = 0;
    if(g_dkit_chip_api[lchip]->dkits_serdes_resert)
    {
        ret = g_dkit_chip_api[lchip]->dkits_serdes_resert(lchip, serdes_id);
    }

    return ret;
}


int32
ctc_usw_dkit_misc_serdes_status(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    int32 ret = 0;
    if(g_dkit_chip_api[lchip]->dkits_dump_serdes_status)
    {
        ret = g_dkit_chip_api[lchip]->dkits_dump_serdes_status(lchip, serdes_id, type, file_name);
    }

    return ret;
}

int32
ctc_usw_dkit_misc_serdes_ctl(void* p_para)
{
    ctc_dkit_serdes_ctl_para_t* p_serdes_para = (ctc_dkit_serdes_ctl_para_t*)p_para;
    int32 ret = 0;
    if(g_dkit_chip_api[p_serdes_para->lchip]->dkits_serdes_ctl)
    {
        ret = g_dkit_chip_api[p_serdes_para->lchip]->dkits_serdes_ctl(p_para);
    }

    return ret;
}
int32
ctc_usw_dkit_misc_serdes_register(uint8 lchip)
{
#ifdef DUET2
    if (DRV_IS_DUET2(lchip))
    {
        g_dkit_chip_api[lchip]->dkits_dump_serdes_status = ctc_dt2_dkit_misc_serdes_status;
        g_dkit_chip_api[lchip]->dkits_read_serdes = ctc_dt2_dkit_misc_read_serdes;
        g_dkit_chip_api[lchip]->dkits_write_serdes = ctc_dt2_dkit_misc_write_serdes;
        g_dkit_chip_api[lchip]->dkits_serdes_ctl = ctc_dt2_dkit_misc_serdes_ctl;
        g_dkit_chip_api[lchip]->dkits_read_serdes_aec_aet = ctc_dt2_dkit_misc_read_serdes_aec_aet;
        g_dkit_chip_api[lchip]->dkits_write_serdes_aec_aet = ctc_dt2_dkit_misc_write_serdes_aec_aet;
        g_dkit_chip_api[lchip]->dkits_serdes_resert = ctc_dt2_dkit_misc_serdes_resert;
        g_dkit_chip_api[lchip]->dkits_dump_indirect = ctc_dt2_dkits_misc_dump_indirect;
    }
#endif
#ifdef TSINGMA
    if (DRV_IS_TSINGMA(lchip))
    {
        g_dkit_chip_api[lchip]->dkits_dump_serdes_status = ctc_tm_dkit_misc_serdes_status;
        g_dkit_chip_api[lchip]->dkits_read_serdes = ctc_tm_dkit_misc_read_serdes;
        g_dkit_chip_api[lchip]->dkits_write_serdes = ctc_tm_dkit_misc_write_serdes;
        g_dkit_chip_api[lchip]->dkits_serdes_ctl = ctc_tm_dkit_misc_serdes_ctl;
        g_dkit_chip_api[lchip]->dkits_read_serdes_aec_aet = NULL;
        g_dkit_chip_api[lchip]->dkits_write_serdes_aec_aet = NULL;
        g_dkit_chip_api[lchip]->dkits_serdes_resert = ctc_tm_dkit_misc_serdes_resert;
    }
#endif
    return 0;

}
STATIC int32
_ctc_usw_dkit_misc_integrity_read_entry(uint8 lchip, tbls_id_t id, uint32 index, uint32 offset, uint32*value)
{
    uint32 hw_addr_base = 0;
    uint32 entry_size   = 0;
    uint32 max_entry    = 0;
    uint32 hw_addr_offset      = 0;
    int32  ret = 0;
    uint8 addr_offset = 0;
    CTC_DKIT_LCHIP_CHECK(lchip);
    if (MaxTblId_t <= id)
    {
        CTC_DKIT_PRINT("\nERROR! INVALID tbl/RegID! tbl/RegID: %d, file:%s line:%d function:%s\n",
                        id,__FILE__,__LINE__,__FUNCTION__);
        return CLI_ERROR;
    }
    ret = drv_usw_get_tbl_index_base(lchip, id, index, &addr_offset);
    if (ret < 0)
    {
        return ret;
    }

    entry_size      = TABLE_ENTRY_SIZE(lchip, id);
    max_entry       = TABLE_MAX_INDEX(lchip, id);
    hw_addr_base    = TABLE_DATA_BASE(lchip, id,addr_offset);

    if (index >= max_entry)
    {
        return CLI_ERROR;
    }
    /*TODO: dynamic table*/
    if ((4 == entry_size) || (8 == entry_size))
    {   /*1w or 2w*/
        hw_addr_offset  =   hw_addr_base + index * entry_size + offset;
    }
    else if (entry_size <= 4*4)
    {  /*4w*/
        hw_addr_offset  =   hw_addr_base + index * 4*4 + offset;
    }
    else if (entry_size <= 4*8)
    {   /*8w*/
        hw_addr_offset  =   hw_addr_base + index * 4*8 + offset;
    }
    /*
    else if (entry_size <= 4*12)
    {//12w
        hw_addr_offset  =   hw_addr_base + index * 4*12 + offset;
    }
    */
    else if (entry_size <= 4*16)
    {/*16w*/
        hw_addr_offset  =   hw_addr_base + index * 4*16 + offset;
    }
    /*
    else if (entry_size <= 4*20)
    {//20w
        hw_addr_offset  =   hw_addr_base + index * 4*20 + offset;
    }
    else if (entry_size <= 4*24)
    {//24w
        hw_addr_offset  =   hw_addr_base + index * 4*24 + offset;
    }
    else if (entry_size <= 4*28)
    {//28w/
        hw_addr_offset  =   hw_addr_base + index * 4*28 + offset;
    }
    */
    else if (entry_size <= 4*32)
    {/*32w*/
        hw_addr_offset  =   hw_addr_base + index * 4*32 + offset;
    }
    /*
    else if (entry_size <= 4*36)
    {//36w
        hw_addr_offset  =   hw_addr_base + index * 4*36 + offset;
    }
    else if (entry_size <= 4*40)
    {//40w/
        hw_addr_offset  =   hw_addr_base + index * 4*40 + offset;
    }
    */
    else if (entry_size <= 4*64)
    {/*64w*/
        hw_addr_offset  =   hw_addr_base + index * 4*64 + offset;
    }

    ret = drv_usw_chip_read(0, hw_addr_offset, value);

    if (ret < 0)
    {
        CTC_DKIT_PRINT("0x%08x address read ERROR!\n", hw_addr_offset);
        return ret;
    }

    return CLI_SUCCESS;
}

static
int32 _ctc_usw_dkit_misc_integrity_get_tbl_info(uint8 lchip, char *poly_str, tbls_id_t* p_tbl_id, fld_id_t* p_fld_id)
{
    char  tbl_str[64] = {0};
    char  fld_str[64] = {0};
    char* head = NULL;
    uint32 tblid = 0, fldid = 0;
    CTC_DKIT_LCHIP_CHECK(lchip);
    head = sal_strstr(poly_str, "[");

    if (NULL == head)
    {
        return CLI_ERROR;
    }
    else
    {
        sal_sscanf(head, "\x5b%[^.].%[^\x5d]", tbl_str, fld_str);

        if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
        {
            sal_sscanf(head, "\x5b%u.%u\x5d", &tblid, &fldid);
            *p_tbl_id = tblid;
            *p_fld_id = fldid;
        }
        else
        {
            if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, p_tbl_id, tbl_str))
            {
                CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                return CLI_ERROR;
            }
            if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, *p_tbl_id, p_fld_id, fld_str))
            {
                CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                return CLI_ERROR;
            }
        }

        if (0 != drv_usw_get_tbl_string_by_id(lchip, *p_tbl_id, tbl_str))
        {
            CTC_DKIT_PRINT("Invalid table id %u!\n", (uint32)*p_tbl_id);
            return CLI_ERROR;
        }
        if (0 != drv_usw_get_field_string_by_id(lchip, *p_tbl_id, *p_fld_id, fld_str))
        {
            CTC_DKIT_PRINT("%s invalid field id %u!\n", TABLE_NAME(lchip, *p_tbl_id), *p_fld_id);
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_misc_integrity_parser_inter(uint8 lchip, char* poly_str, uint32* cal_val)
{
    uint32 value[2] = {0};
    char op_code = 0;
    char tbl_str[64] = {0};
    char* op_str = NULL;
    char *head = NULL;
    uint32 tblid = 0;
    uint32 result = 0;
    uint32 offset = 0;
    uint32 index  = 0;
    int32 ret = 0;

    DRV_PTR_VALID_CHECK(poly_str);
    CTC_DKIT_LCHIP_CHECK(lchip);
    head = sal_strstr(poly_str, "[");

    if(NULL == head)
    {
        return CLI_ERROR;
    }

    sal_sscanf(head, "\x5b%[^.].%u.%u\x5d", tbl_str, &index, &offset);

    if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
    {
        sal_sscanf(head, "\x5b%u.%u.%u\x5d", &tblid, &index, &offset);
    }
    else
    {
        if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tblid, tbl_str))
        {
            CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
            return CLI_ERROR;
        }
    }

    ret = _ctc_usw_dkit_misc_integrity_read_entry(lchip, tblid, index, offset, &value[0]);
    if (CLI_SUCCESS != ret)
    {
        CTC_DKIT_PRINT("read tbl_str %s index %d offset %d error\n", tbl_str, index, offset);
        return CLI_ERROR;
    }

    op_str = sal_strstr(head, "{op ");
    if (NULL != op_str)
    {
        op_code = op_str[4];
        head = sal_strstr(op_str, "[");
        if (head)
        {
            sal_sscanf(head, "[0x%x]", &value[1]);
        }
    }
    else
    {

    }

    result = value[0];
    switch (op_code)
    {
        case '+':
            result += value[1];
            break;

        case '-':
            result -= value[1];
            break;

        case '&':
            result &= value[1];
            break;

        case '|':
            result |= value[1];
            break;

        default:
            break;
    }

    *cal_val = result;

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_misc_integrity_parser_tbl(uint8 lchip, uint32 index ,uint8 b_exted, uint8 mode, char* poly_str, char* p_result_str, uint32* p_result)
{
    char   op_code[CTC_DKIT_LINKLIST_MAX_OP_NUM] = {0};
    char   tbl_str[64] = {0};
    char   fld_str[64] = {0};
    char   tmp_fld_str[64] = {0};
    char*  op_str = NULL;
    char*  head = NULL;
    char*  bit_split = NULL;
    uint8  op_time = 0;
    uint32 tblid = 0;
    uint32 fldid = 0;
    tbls_id_t tbl_id = 0;
    fld_id_t  fld_id = 0;
    uint32 cmd = 0;
    uint32 cnt = 0;
    uint32 tbl_idx = 0;
    uint32 value[CTC_DKIT_LINKLIST_MAX_OP_NUM + 1][4];
    uint32 start_bit = 0, end_bit = 0;

    sal_memset(value, 0, sizeof(value));

    DRV_PTR_VALID_CHECK(poly_str);
    CTC_DKIT_LCHIP_CHECK(lchip);
    head = sal_strstr(poly_str, "[");
    if (NULL == head)
    {
        sal_sscanf(poly_str, "%u", p_result);
        return CLI_SUCCESS;
    }

    while ((op_str = sal_strstr(head, "{op ")))
    {
        op_code[op_time] = op_str[4];

        if (b_exted)
        {
            sal_sscanf(head, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &tbl_idx, fld_str);

            if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
            {
                sal_sscanf(head, "\x5b%u.%u.%u\x5d", &tblid, &tbl_idx, &fldid);
                tbl_id = tblid;
                fld_id = fldid;
            }
            else
            {
                if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                    return CLI_ERROR;
                }
                if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }
        }
        else
        {
            sal_sscanf(head, "\x5b%[^.].%[^\x5d]", tbl_str, fld_str);

            if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
            {
                sal_sscanf(head, "\x5b%u.%u\x5d", &tblid, &fldid);
                tbl_id = tblid;
                fld_id = fldid;
            }
            else
            {
                if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                    return CLI_ERROR;
                }
                if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }

            tbl_idx = index;
        }

        if (mode)
        {
            cmd = DRV_IOR(tbl_id, fld_id);
            if (DRV_E_NONE != DRV_IOCTL(lchip, tbl_idx, cmd, value[op_time]))
            {
                CTC_DKIT_PRINT("read tbl_str %s fld_str %s error\n", TABLE_NAME(lchip, tbl_id), fld_str);
            }
        }
        if (p_result_str)
        {
            sal_sprintf(p_result_str + sal_strlen(p_result_str), "[%s.%d.%s] {op %c} ",
                        TABLE_NAME(lchip, tbl_id), tbl_idx, fld_str, op_code[op_time]);
        }

        head = sal_strstr(op_str, "[");
        sal_memset(fld_str, 0, sizeof(fld_str));
        op_str = NULL;
        op_time ++;
        if (op_time > CTC_DKIT_LINKLIST_MAX_OP_NUM)
        {

        }
    }

    if (b_exted)
    {
        sal_sscanf(head, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &tbl_idx, fld_str);

        if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
        {
            sal_sscanf(head, "\x5b%u.%u.%u\x5d", &tblid, &tbl_idx, &fldid);
            tbl_id = tblid;
            fld_id = fldid;
        }
        else
        {
            if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
            {
                CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                return CLI_ERROR;
            }
            if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
            {
                CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                return CLI_ERROR;
            }
        }
    }
    else
    {
        sal_sscanf(head, "\x5b%[^.].%[^\x5d]", tbl_str, fld_str);

        if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
        {
            sal_sscanf(head, "\x5b%u.%u\x5d", &tblid, &fldid);
            tbl_id = tblid;
            fld_id = fldid;
        }
        else
        {
            if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
            {
                CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                return CLI_ERROR;
            }
            if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
            {
                CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                return CLI_ERROR;
            }
        }

        tbl_idx = index;
    }

    /*CTC_DKIT_PRINT("b_exted:%d,tbl_str:%s,fld_str:%s\n",b_exted,tbl_str,fld_str);
    */

    /*process such as (31:0), (63:32) kind of field, bits larger than 32*/
    bit_split = sal_strstr(fld_str, "(");
    if (bit_split)
    {
        sal_strcpy(tmp_fld_str, fld_str);
        sal_sscanf(tmp_fld_str, "%[^\x28]\x28%d:%d\x29", fld_str, &end_bit, &start_bit);
    }

    if (mode)
    {
        cmd = DRV_IOR(tbl_id, fld_id);
        if (0 != DRV_IOCTL(lchip, tbl_idx, cmd, value[op_time]))
        {
            CTC_DKIT_PRINT("read tbl_str %s fld_str %s error\n", TABLE_NAME(lchip, tbl_id), fld_str);
        }
    }
    if (p_result_str)
    {
        sal_sprintf(p_result_str + sal_strlen(p_result_str), "[%s.%d.%s] ",  TABLE_NAME(lchip, tbl_id), tbl_idx, fld_str);
    }

    cnt    = 0;
    /*process such as (31:0), (63:32) kind of field, bits larger than 32*/
    if (bit_split)
    {
        if (start_bit == 0 && end_bit == 31)
        {
            *p_result = value[cnt][0];
        }
        else if (start_bit == 32 && end_bit == 63)
        {
            *p_result = value[cnt][1];
        }
        else if (start_bit == 64 && end_bit == 95)
        {
            *p_result = value[cnt][2];
        }
        else if (start_bit == 96 && end_bit == 127)
        {
            *p_result = value[cnt][3];
        }
        else
        {
            CTC_DKIT_PRINT("Unsupported bit split\n");
        }

    }
    else
    {
        *p_result = value[cnt][0];
    }

    while (cnt < op_time)
    {
        switch (op_code[cnt])
        {
            case '+':
                *p_result += value[cnt + 1][0];
                break;

            case '-':
                *p_result -= value[cnt + 1][0];
                break;

            case '&':
                *p_result &= value[cnt + 1][0];
                break;

            case '|':
                *p_result |= value[cnt + 1][0];
                break;

            default:
                break;
        }
        cnt++;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_misc_integrity_error_info(uint8 lchip, ctc_dkit_misc_integrity_op_t op,
                                    ctc_dkit_misc_integrity_check_t check,
                                    char* p_left_str, char* p_right_str,
                                    int32 left_val, int32 rigth_val, char* p_result)
{
    uint32 tblid = 0;
    uint32 fldid = 0;
    tbls_id_t tbl_id = 0;
    fld_id_t  fld_id = 0;
    uint32 index = 0;
    char   tbl_str[64] = {0};
    char   fld_str[64] = {0};
    char*  operand[] = {" ", "=", ">=", "<=", ">", "<"};
    char*  p_op_str = NULL;

    CTC_DKIT_PTR_VALID_CHECK(p_left_str);
    CTC_DKIT_LCHIP_CHECK(lchip);
    if (CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS == check)
    {
        if (NULL == sal_strstr(p_right_str, "["))
        {
            sal_sscanf(p_left_str, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &index, fld_str);

            if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
            {
                sal_sscanf(p_left_str, "\x5b%u.%u.%u\x5d", &tblid, &index, &fldid);
                tbl_id = tblid;
                fld_id = fldid;
                if (CLI_SUCCESS != drv_usw_get_field_string_by_id(lchip, tblid, fldid, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }
            else
            {
                if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                    return CLI_ERROR;
                }
                if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }

            sal_sprintf(p_result, "%s[%u].%s:%d !%s %d\n",
                        TABLE_NAME(lchip, tbl_id), index, fld_str, left_val, operand[op], rigth_val);
        }
        else
        {
            sal_sscanf(p_left_str, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &index, fld_str);

            if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
            {
                sal_sscanf(p_left_str, "\x5b%u.%u.%u\x5d", &tblid, &index, &fldid);
                tbl_id = tblid;
                fld_id = fldid;
                if (CLI_SUCCESS != drv_usw_get_field_string_by_id(lchip, tbl_id, fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }
            else
            {
                if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                    return CLI_ERROR;
                }
                if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }

            sal_sprintf(p_result, "[%s.%u.%s]:%d !%s ",
                        TABLE_NAME(lchip, tbl_id), index, fld_str, left_val, operand[op]);


            sal_sscanf(p_right_str, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &index, fld_str);

            if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
            {
                sal_sscanf(p_right_str, "\x5b%u.%u.%u\x5d", &tblid, &index, &fldid);
                tbl_id = tblid;
                fld_id = fldid;
                if (CLI_SUCCESS != drv_usw_get_field_string_by_id(lchip, tbl_id, fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }
            else
            {
                if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                    return CLI_ERROR;
                }
                if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }

            sal_sprintf(p_result + sal_strlen(p_result), "[%s.%u.%s]:%d\n",
                        TABLE_NAME(lchip, tbl_id), index, fld_str, rigth_val);
        }
    }
    else if (CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST == check)
    {
        char   op_code[CTC_DKIT_LINKLIST_MAX_OP_NUM] = {0};
        uint32 op_time = 0;
        char   fld_str[64] = {0};

        while (p_left_str && (p_op_str = sal_strstr(p_left_str, "{op ")))
        {
            op_code[op_time] = p_op_str[4];

            sal_sscanf(p_left_str, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &index, fld_str);

            if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
            {
                sal_sscanf(p_left_str, "\x5b%u.%u.%u\x5d", &tblid, &index, &fldid);
                tbl_id = tblid;
                fld_id = fldid;
                if (CLI_SUCCESS != drv_usw_get_field_string_by_id(lchip, tbl_id, fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }
            else
            {
                if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                    return CLI_ERROR;
                }
                if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                    return CLI_ERROR;
                }
            }

            sal_sprintf(p_result + sal_strlen(p_result), "[%s.%u.%s] {op %c} ",
                        TABLE_NAME(lchip, tbl_id), index, fld_str, op_code[op_time]);
            p_left_str = sal_strstr(p_op_str, "[");
            p_op_str = NULL;
            op_time ++;
        }

        sal_sscanf(p_left_str, "\x5b%[^.].%u.%[^\x5d]", tbl_str, &index, fld_str);

        if (tbl_str[0] >= '0' && tbl_str[0] <= '9')
        {
            sal_sscanf(p_left_str, "\x5b%u.%u.%u\x5d", &tblid, &index, &fldid);
            tbl_id = tblid;
            fld_id = fldid;
            if (CLI_SUCCESS != drv_usw_get_field_string_by_id(lchip, tbl_id, fld_id, fld_str))
            {
                CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                return CLI_ERROR;
            }
        }
        else
        {
            if (CLI_SUCCESS != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
            {
                CTC_DKIT_PRINT("can't find tbl %s \n", tbl_str);
                return CLI_ERROR;
            }
            if (CLI_SUCCESS != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
            {
                CTC_DKIT_PRINT("can't find %s's field %s\n", tbl_str, fld_str);
                return CLI_ERROR;
            }
        }

        sal_sprintf(p_result + sal_strlen(p_result), "[%s.%u.%s]", TABLE_NAME(lchip, tbl_id), index, fld_str);
        sal_sprintf(p_result + sal_strlen(p_result), ":%u !%s %u\n", left_val, operand[op], rigth_val);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_misc_integrity_read_line(uint8 lchip, char* p_line, char* p_result, uint32 verbos, uint8* check_pass)
{
    uint8 op_code = CTC_DKIT_MISC_INTEGRITY_OP_NONE;

    char* poly_str = NULL;
    char* poly_str_right = NULL;

    uint32 exp_val = 0;
    uint32 rel_val = 0;

    uint32 counter  = 0;
    uint32 index    = 0;
    uint32 next_index = 0;

    uint32 tbl_id = 0;
    fld_id_t fld_id = 0;
    uint32 cmd = 0;

    static uint8 integrity_stat = CTC_DKIT_MISC_INTEGRITY_CHECK_NONE;
    static uint32 exp_head = 0;
    static uint32 exp_tail = 0;
    static uint32 exp_cnt = 0;
    int32 ret = DRV_E_NONE;
    CTC_DKIT_LCHIP_CHECK(lchip);
    poly_str = (char*)sal_malloc(MAX_PER_LINE * sizeof(char));
    if(NULL == poly_str)
    {
        return CLI_ERROR;
    }
    poly_str_right = (char*)sal_malloc(MAX_PER_LINE * sizeof(char));
    if(NULL == poly_str_right)
    {
        sal_free(poly_str);
        return CLI_ERROR;
    }

    sal_memset(poly_str, 0, MAX_PER_LINE * sizeof(char));
    sal_memset(poly_str_right, 0, MAX_PER_LINE * sizeof(char));

    if ((0x0A == p_line[0])
        || ((0x0D == p_line[0]) && (0x0A == p_line[1])))
    {
        sal_strncpy(p_result, p_line, MAX_PER_LINE);
    }
    else if ('#' == p_line[0])
    {
        if (sal_strstr(p_line, "Status Check"))
        {
            integrity_stat = CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS;
            sal_strncpy(p_result, p_line, MAX_PER_LINE);
        }
        else if (sal_strstr(p_line, "Link List Check Begin"))
        {
            integrity_stat = CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST;
            sal_memset(bit_map, 0, sizeof(bit_map));
            sal_strncpy(p_result, p_line, MAX_PER_LINE);
        }
        else if (sal_strstr(p_line, "Link List Check End"))
        {
            integrity_stat = CTC_DKIT_MISC_INTEGRITY_CHECK_NONE;
            sal_strncpy(p_result, p_line, MAX_PER_LINE);
        }
        else if (sal_strstr(p_line, "Interrupt Check"))
        {
            integrity_stat = CTC_DKIT_MISC_INTEGRITY_CHECK_INTR;
            sal_strncpy(p_result, p_line, MAX_PER_LINE);
        }
        else
        {
            sal_strcpy(p_result, "#@");
        }
    }
    else
    {
        if (sal_strstr(p_line, ">="))
        {
            op_code = CTC_DKIT_MISC_INTEGRITY_OP_BE;
        }
        else if (sal_strstr(p_line, "<="))
        {
            op_code = CTC_DKIT_MISC_INTEGRITY_OP_LE;
        }
        else if (sal_strstr(p_line, ">"))
        {
            op_code = CTC_DKIT_MISC_INTEGRITY_OP_B;
        }
        else if (sal_strstr(p_line, "<"))
        {
            op_code = CTC_DKIT_MISC_INTEGRITY_OP_L;
        }
        else if (sal_strstr(p_line, "="))
        {
            op_code = CTC_DKIT_MISC_INTEGRITY_OP_E;
        }
        else
        {
            op_code = CTC_DKIT_MISC_INTEGRITY_OP_NONE;
        }

        /*to calc poly*/
        /*status check*/
        if (CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS == integrity_stat)
        {
            switch (op_code)
            {
                case CTC_DKIT_MISC_INTEGRITY_OP_E:
                    sal_sscanf(p_line, "%[^=]=%s", poly_str, poly_str_right);
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }

                    if (rel_val != exp_val)
                    {
                        if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_E,
                                                                                          CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS,
                                                                                          poly_str, poly_str_right,
                                                                                          rel_val, exp_val, p_result))
                        {
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (verbos)
                        {
                            CTC_DKIT_PRINT("%s\n", p_result);
                        }
                        *check_pass = FALSE;
                    }
                    break;

                case CTC_DKIT_MISC_INTEGRITY_OP_BE:
                    sal_sscanf(p_line, "%[^>=]>=%s", poly_str, poly_str_right);
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (rel_val < exp_val)
                    {
                        if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_BE,
                                                                                          CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS,
                                                                                          poly_str, poly_str_right,
                                                                                          rel_val, exp_val, p_result))
                        {
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (verbos)
                        {
                            CTC_DKIT_PRINT("%s\n", p_result);
                        }
                        *check_pass = FALSE;
                    }
                    break;

                case CTC_DKIT_MISC_INTEGRITY_OP_LE:
                    sal_sscanf(p_line, "%[^<=]<=%s", poly_str, poly_str_right);
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (rel_val > exp_val)
                    {
                        if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_LE,
                                                                                          CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS,
                                                                                          poly_str, poly_str_right,
                                                                                          rel_val, exp_val, p_result))
                        {
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (verbos)
                        {
                            CTC_DKIT_PRINT("%s\n", p_result);
                        }
                        *check_pass = FALSE;
                    }
                    break;

                case CTC_DKIT_MISC_INTEGRITY_OP_B:
                    sal_sscanf(p_line, "%[^>]>%s", poly_str, poly_str_right);
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (rel_val <= exp_val)
                    {
                        if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_B,
                                                                                          CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS,
                                                                                          poly_str, poly_str_right,
                                                                                          rel_val, exp_val, p_result))
                        {
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (verbos)
                        {
                            CTC_DKIT_PRINT("%s\n", p_result);
                        }
                        *check_pass = FALSE;
                    }
                    break;

                case CTC_DKIT_MISC_INTEGRITY_OP_L:
                    sal_sscanf(p_line, "%[^<]<%s", poly_str, poly_str_right);
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                    {
                        *check_pass = FALSE;
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    if (rel_val >= exp_val)
                    {
                        if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_L,
                                                                                          CTC_DKIT_MISC_INTEGRITY_CHECK_STATUS,
                                                                                          poly_str, poly_str_right,
                                                                                          rel_val, exp_val, p_result))
                        {
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (verbos)
                        {
                            CTC_DKIT_PRINT("%s\n", p_result);
                        }
                        *check_pass = FALSE;
                    }
                    break;

                default:
                    break;
            }
        }
        /*link list check*/
        else if (CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST == integrity_stat)
        {
            if (sal_strstr(p_line, "{var"))
            {
                sal_sscanf(p_line, "%*[^=]=%s", poly_str);
            }
            else
            {

            }

            if (sal_strstr(p_line, "{var linkHeadPtr}"))
            {
                if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, FALSE, 1, poly_str, NULL, &exp_head))
                {
                    ret = CLI_ERROR;
                    goto clean;
                }
            }
            else if (sal_strstr(p_line, "{var linkTailPtr}"))
            {
                if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, FALSE, 1, poly_str, NULL, &exp_tail))
                {
                    ret = CLI_ERROR;
                    goto clean;
                }
            }
            else if (sal_strstr(p_line, "{var linkCnt}"))
            {
                if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, FALSE, 1, poly_str, NULL, &exp_cnt))
                {
                    ret = CLI_ERROR;
                    goto clean;
                }
            }
            else if (sal_strstr(p_line, "{var nextPtr}"))
            {
                counter   = 0;
                index     = exp_head;
                /*get tbl & field id*/
                if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_get_tbl_info(lchip, poly_str, &tbl_id, &fld_id))
                {
                    ret = CLI_ERROR;
                    goto clean;
                }

                for (;;)
                {
                    if (IS_BIT_SET(bit_map[index / 32], (index % 32)))
                    {
                        sal_sprintf(p_result, "@index (0x%x) looped\n", index);
                        break;
                    }
                    else
                    {
                        SET_BIT(bit_map[index / 32], (index % 32));
                    }
                    counter ++;
                    if ((index == exp_tail) || (counter == exp_cnt))
                    {
                         /*kal_printf("index %d, exp_tail %d\n", index, exp_tail);*/
                         /*kal_printf("counter %d, exp_cnt %d\n", counter, exp_cnt);*/
                        break;
                    }

                    cmd = DRV_IOR(tbl_id, fld_id);
                    DRV_IOCTL(lchip, index, cmd, &next_index);
                    if (next_index >= TABLE_MAX_INDEX(lchip, tbl_id))
                    {
                        sal_sprintf(p_result, "@index 0x%x Next Idx is 0x%x\n", index, next_index);
                        break;
                    }
                    index = next_index;

                }

                if (next_index != exp_tail)
                {
                    sal_sprintf(p_result, "@actual tail = 0x%x, expetc tail = 0x%x\n", next_index, exp_tail);
                    if (verbos)
                    {
                        CTC_DKIT_PRINT("%s\n", p_result);
                    }
                    *check_pass = FALSE;
                }
                else if (counter != exp_cnt)
                {
                    sal_sprintf(p_result, "@actual counter = 0x%x, expetc counter = 0x%x\n", counter, exp_cnt);
                    if (verbos)
                    {
                        CTC_DKIT_PRINT("%s\n", p_result);
                    }
                    *check_pass = FALSE;
                }
            }
            else
            {
                switch (op_code)
                {
                    case CTC_DKIT_MISC_INTEGRITY_OP_E:
                        sal_sscanf(p_line, "%[^=]=%s", poly_str, poly_str_right);
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (rel_val != exp_val)
                        {
                            if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_E,
                                                                                              CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST,
                                                                                              poly_str, poly_str_right,
                                                                                              rel_val, exp_val, p_result))
                            {
                                ret = CLI_ERROR;
                                goto clean;
                            }
                            if (verbos)
                            {
                                CTC_DKIT_PRINT("%s\n", p_result);
                            }
                            *check_pass = FALSE;
                        }
                        break;

                    case CTC_DKIT_MISC_INTEGRITY_OP_BE:
                        sal_sscanf(p_line, "%[^>=]>=%s", poly_str, poly_str_right);
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (rel_val < exp_val)
                        {
                            if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_BE,
                                                                                              CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST,
                                                                                              poly_str, poly_str_right,
                                                                                              rel_val, exp_val, p_result))
                            {
                                ret = CLI_ERROR;
                                goto clean;
                            }

                            if (verbos)
                            {
                                CTC_DKIT_PRINT("%s\n", p_result);
                            }
                            *check_pass = FALSE;
                        }
                        break;

                    case CTC_DKIT_MISC_INTEGRITY_OP_LE:
                        sal_sscanf(p_line, "%[^<=]<=%s", poly_str, poly_str_right);
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (rel_val > exp_val)
                        {
                            if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_LE,
                                                                                              CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST,
                                                                                              poly_str, poly_str_right,
                                                                                              rel_val, exp_val, p_result))
                            {
                                ret = CLI_ERROR;
                                goto clean;
                            }
                            if (verbos)
                            {
                                CTC_DKIT_PRINT("%s\n", p_result);
                            }
                            *check_pass = FALSE;
                        }
                        break;

                    case CTC_DKIT_MISC_INTEGRITY_OP_B:
                        sal_sscanf(p_line, "%[^>]>%s", poly_str, poly_str_right);
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (rel_val <= exp_val)
                        {
                            if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_B,
                                                                                              CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST,
                                                                                              poly_str, poly_str_right,
                                                                                              rel_val, exp_val, p_result))
                            {
                                ret = CLI_ERROR;
                                goto clean;
                            }
                            if (verbos)
                            {
                                CTC_DKIT_PRINT("%s\n", p_result);
                            }
                            *check_pass = FALSE;
                        }
                        break;

                    case CTC_DKIT_MISC_INTEGRITY_OP_L:
                        sal_sscanf(p_line, "%[^<]<%s", poly_str, poly_str_right);
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str, NULL, &rel_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, TRUE, 1, poly_str_right, NULL, &exp_val))
                        {
                            *check_pass = FALSE;
                            ret = CLI_ERROR;
                            goto clean;
                        }
                        if (rel_val >= exp_val)
                        {
                            if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_error_info(lchip, CTC_DKIT_MISC_INTEGRITY_OP_L,
                                                                                              CTC_DKIT_MISC_INTEGRITY_CHECK_LINKLIST,
                                                                                              poly_str, poly_str_right,
                                                                                              rel_val, exp_val, p_result))
                            {
                                ret = CLI_ERROR;
                                goto clean;
                            }
                            if (verbos)
                            {
                                CTC_DKIT_PRINT("%s\n", p_result);
                            }
                            *check_pass = FALSE;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        /*interrupt check*/
        else if (CTC_DKIT_MISC_INTEGRITY_CHECK_INTR == integrity_stat)
        {
            uint32 val = 0;

            sal_sscanf(p_line, "%[^=]=%s", poly_str, poly_str_right);
            if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_inter(lchip, poly_str, &val))
            {
                ret = CLI_ERROR;
                goto clean;
            }
            if (CLI_SUCCESS !=  _ctc_usw_dkit_misc_integrity_parser_tbl(lchip, 0, FALSE, 1, poly_str_right, NULL, &exp_val))
            {
                ret = CLI_ERROR;
                goto clean;
            }
            if (val != exp_val)
            {
                sal_sprintf(p_result, "%s = %u@%d\n", poly_str, val, exp_val);
                if (verbos)
                {
                    CTC_DKIT_PRINT("%s\n", p_result);
                }
                *check_pass = FALSE;
            }
        }
        /*none check*/
        else
        {
            exp_head = 0;
            exp_tail = 0;
            exp_cnt  = 0;
        }
    }

clean:
    if (poly_str)
    {
        sal_free(poly_str);
    }
    if (poly_str_right)
    {
        sal_free(poly_str_right);
    }

    return ret;
}

int32
ctc_usw_dkit_misc_integrity_check(uint8 lchip, void* p_para1, void* p_para2, void* p_para3)
{
    sal_file_t  p_golden_fp = NULL;
    sal_file_t  p_result_fp = NULL;
    char*   line = NULL;
    char*   result = NULL;
    uint8  check_pass = TRUE;
    int32  ret = DRV_E_NONE;
    char*  p_golden_file = NULL;
    char*  p_result_file = NULL;
    uint32 verbos = 0;

    DKITS_PTR_VALID_CHECK(p_para1);
    DKITS_PTR_VALID_CHECK(p_para2);
    DKITS_PTR_VALID_CHECK(p_para3);
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);

    line = (char*)sal_malloc(MAX_PER_LINE * sizeof(char));
    result = (char*)sal_malloc(MAX_PER_LINE * sizeof(char));

    if (!line || !result)
    {
        goto CTC_DKIT_MISC_INTEGRITY_CHECK_END;
    }

    sal_memset(line, 0, MAX_PER_LINE * sizeof(char));
    sal_memset(result, 0, MAX_PER_LINE * sizeof(char));

    p_golden_file = (char*)p_para1;
    p_result_file = (char*)p_para2;
    verbos        = *((uint32*)p_para3);

    p_golden_fp = sal_fopen(p_golden_file, "r");
    if (NULL == p_golden_fp)
    {
        CTC_DKIT_PRINT("Open golden file:%s failed!\n", p_golden_file);
        ret = CLI_ERROR;
        goto CTC_DKIT_MISC_INTEGRITY_CHECK_END;
    }

    p_result_fp = sal_fopen(p_result_file, "w+");
    if (NULL == p_result_fp)
    {
        CTC_DKIT_PRINT("Create integrity check result:%s failed!\n", p_result_file);
        ret = CLI_ERROR;
        goto CTC_DKIT_MISC_INTEGRITY_CHECK_END;
    }

    while (NULL != sal_fgets(line, MAX_PER_LINE, p_golden_fp))
    {
        if (CLI_SUCCESS != _ctc_usw_dkit_misc_integrity_read_line(lchip, line, result, verbos, &check_pass))
        {
            CTC_DKIT_PRINT(" Read integrity check file: %s error!\n", p_golden_file);
            CTC_DKIT_PRINT(" Fail to parser line:%s.\n", line);
            goto CTC_DKIT_MISC_INTEGRITY_CHECK_END;
        }
        if (('#' == result[0] && '@' != result[1])
           || (!(0 == sal_strcmp(result, "#@") || 0 == sal_strlen(result)
           || 0x0A == result[0] || (0x0D == result[0] && 0x0A == result[1]))))
        {
            sal_fprintf(p_result_fp, "%s", result);
        }

        sal_memset(line, 0, MAX_PER_LINE * sizeof(char));
        sal_memset(result, 0, MAX_PER_LINE * sizeof(char));
    }

    if (check_pass)
    {
        CTC_DKIT_PRINT("Integrity check passed!\n");
        ret = DRV_E_NONE;
    }
    else
    {
        ret = DRV_E_RESERVD_VALUE_ERROR;
    }

CTC_DKIT_MISC_INTEGRITY_CHECK_END:
    if (NULL != p_golden_fp)
    {
        sal_fclose(p_golden_fp);
        p_golden_fp = NULL;
    }
    if (NULL != p_result_fp)
    {
        sal_fclose(p_result_fp);
        p_result_fp = NULL;
    }
    if(NULL != line)
    {
        sal_free(line);
    }
    if(NULL != result)
    {
        sal_free(result);
    }

    return ret;
}

int32
ctc_usw_dkit_misc_chip_info(uint8 lchip, sal_file_t p_wf)
{
    uint32      cmd = 0;
    DevId_m    read_device_id;
    EcidRegMem_m ecid_reg;
    uint32      value = 0;
    uint32      get_device_id = 0;
    ctc_dkit_device_id_type_t dev_id;
    uint32      version_id = 0;
    char        chip_name[CTC_DKIT_CHIP_NAME_LEN] = {0};
    uint8       svb  = 0;
    CTC_DKIT_LCHIP_CHECK(lchip);
    sal_memset(&read_device_id, 0, sizeof(DevId_m));
    sal_memset(&ecid_reg, 0, sizeof(EcidRegMem_m));
    cmd = DRV_IOR(DevId_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &read_device_id);

    value = GetDevId(V,deviceId_f,&read_device_id);

    version_id = value & 0x0F;
    get_device_id  = (value >> 4) & 0x0F;
    sal_strcpy(chip_name, (get_device_id == 0x07) ? "CTC7148": "Not Support Chip");
    dev_id = (get_device_id == 0x07) ? CTC_DKIT_DEVICE_ID_DUET2_CTC7148 : CTC_DKIT_DEVICE_ID_INVALID;

    cmd = DRV_IOR(EcidRegMem_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 3, cmd, &ecid_reg);
    value = GetEcidRegMem(V, data_f, &ecid_reg);
    svb = (value >> 2) & 0xF;

    CTC_DKITS_PRINT_FILE(p_wf, "Chip name: %s, Device Id: %d, Version Id: %d Svb: %d\n",
                               chip_name, dev_id, version_id, svb);


    sal_memset(chip_name, 0, CTC_DKIT_CHIP_NAME_LEN);
    ctc_usw_dkit_get_chip_name(lchip, chip_name);
    CTC_DKITS_PRINT_FILE(p_wf, "Dkits %s Released at %s Chip Series: %s\nCopyright (C) %s Centec Networks Inc.  All rights reserved.\n"
            , CTC_DKITS_VERSION_STR, CTC_DKITS_RELEASE_DATE, chip_name, CTC_DKITS_COPYRIGHT_TIME);
    CTC_DKITS_PRINT_FILE(p_wf, "-------------------------------------------------------------\n");

    return CLI_SUCCESS;
}

#if 0
/********************************************************
###   temp for integrity check from cmodel
###
*******************************************************/

#define OP_NONE         0
#define OP_E            1
#define OP_BE           2
#define OP_LE           3
#define OP_B            4
#define OP_L            5

enum parser_type_e
{
    PARSER_TYPE_INST,
    PARSER_TYPE_STAT,
    PARSER_TYPE_INVALID,
};
typedef enum parser_type_e parser_type_t;


#define DbgTool_DBG_INFO(FMT, ...)                          \
            do                                                     \
            { \
              sal_printf(FMT, ##__VA_ARGS__); \
            } while (0)

/*the following is integrity check*/

STATIC int32 _ctc_dbg_tool_get_id_by_poly(uint8 lchip, char *poly_str, tbls_id_t* tbl_id, fld_id_t* fld_id)
{
    char tbl_str[64] = {0};
    char fld_str[64] = {0};
    char *head = NULL;

    head = sal_strstr(poly_str, "[");

    if (NULL == head)
    {
        return DRV_E_INVALID_PARAMETER;
    }
    else
    {
        sal_sscanf(head, "\x5b%[^.].%[^\x5d]", tbl_str, fld_str);
        if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, tbl_id, tbl_str))
        {
            sal_printf("can find table %s\n", tbl_str);
            return DRV_E_INVALID_TBL;
        }
        if (DRV_E_NONE != drv_usw_get_field_id_by_string(lchip, *tbl_id, fld_id, fld_str))
        {
            sal_printf("can find field %s\n", fld_str);
            return DRV_E_INVALID_FLD;
        }
    }
    return DRV_E_NONE;
}


STATIC int32
_ctc_dbg_tool_calc_inter_mask(uint8 lchip, char *poly_str, uint32 *cal_val)
{
    uint32 value[2] = {0};
    char op_code = 0;
    char tbl_str[64] = {0};

    char* op_str = NULL;
    char *head = NULL;
    tbls_id_t tbl_id = 0;
    uint32 result = 0;
    uint32 offset = 0;
    uint32 index  = 0;
    int32 ret = 0;

    DRV_PTR_VALID_CHECK(poly_str);
    head = sal_strstr(poly_str, "[");


    sal_sscanf(head, "[%[^.].%d.%d]", tbl_str, &index, &offset);

    if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
    {
        ctc_cli_out("can't find tbl %s \n", tbl_str);
        return DRV_E_INVALID_TBL;
    }

    ret = _ctc_usw_dkit_misc_integrity_read_entry(lchip, tbl_id, index, offset, &value[0]);
    if (DRV_E_NONE != ret)
    {
        sal_printf("read tbl_str %s index %d offset %d error\n", tbl_str, index, offset);
        return ret;
    }
     /*DRV_IF_ERROR_RETURN(read_entry(chip_id, tbl_id, index, offset, &value[0]));*/

    op_str = sal_strstr(head, "{op ");
    if (NULL != op_str)
    {
        op_code = op_str[4];
        head = sal_strstr(op_str, "[");
        sal_sscanf(head, "[0x%x]", &value[1]);
    }
    else
    {

    }

    result = value[0];
    switch (op_code)
    {
        case '+':
            result += value[1];
            break;
        case '-':
            result -= value[1];
            break;
        case '&':
            result &= value[1];
            break;
        case '|':
            result |= value[1];
            break;
        default:
            break;
    }
    *cal_val = result;
    return DRV_E_NONE;
}



STATIC int32
_ctc_dbg_tool_calc_poly(uint8 lchip, uint32 index ,uint8 b_exted, uint8 mode,char *poly_str, char *result_str)
{
    #define MAX_OP_NUM 7
    uint8 op_time = 0;

    uint32 value[MAX_OP_NUM+1][5];
    char op_code[MAX_OP_NUM] = {0};
    char tbl_str[64] = {0};
    char fld_str[64] = {0};
    char tmp_fld_str[64] = {0};

    char* op_str = NULL;
    char *head = NULL;

    tbls_id_t tbl_id = 0;
    fld_id_t fld_id = 0;

    uint32 cmd = 0;
    uint32 result = 0;
    uint32 cnt = 0;

    uint32 poly_idx = 0;

    char *bit_split = NULL;
    uint32 start_bit = 0, end_bit = 0;

     /*uint32 self_op_mult = 0;*/
     /*uint32 self_op_value = 0;*/

    sal_memset(value, 0, sizeof(value));

    DRV_PTR_VALID_CHECK(poly_str);
    head = sal_strstr(poly_str, "[");
    if (NULL == head)
    {
        sal_sscanf(poly_str, "%d", &result);
        return result;
    }
    else
    {
        while ((op_str = sal_strstr(head, "{op ")))
        {
            op_code[op_time] = op_str[4];
            DbgTool_DBG_INFO("debug op_code[%d]:%c\n",op_time, op_code[op_time]);

            #if 0
            if (op_code[op_time] == '*')
            {
                sal_sscanf(op_str,"{op * %d}",&self_op_value);
                self_op_mult = 1;
                head = sal_strstr(op_str+1, "{op ");
                continue;
            }
            #endif
            if (b_exted)
            {
                sal_sscanf(head, "\x5b%[^.].%d.%[^\x5d]", tbl_str, &poly_idx, fld_str);
            }
            else
            {
                sal_sscanf(head, "\x5b%[^.].%[^\x5d]", tbl_str, fld_str);
                poly_idx = index;
            }

            DbgTool_DBG_INFO("op,tbl_str:%s,fld_str:%s\n",tbl_str,fld_str);

            if (mode)
            {
                if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
                {
                    ctc_cli_out("can't find tbl %s \n", tbl_str);
                    return DRV_E_INVALID_TBL;
                }

                if (DRV_E_NONE != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
                {
                    sal_printf("can find field %s\n", fld_str);
                    return DRV_E_INVALID_FLD;
                }

                cmd = DRV_IOR(tbl_id, fld_id);
                if (DRV_E_NONE != DRV_IOCTL(lchip, poly_idx, cmd, value[op_time]))
                {
                    sal_printf("read tbl_str %s fld_str %s error\n", tbl_str, fld_str);
                }
                #if 0
                if (self_op_mult)
                {
                    value[op_time] = value[op_time] * self_op_value;
                    self_op_mult = 0;
                }
                #endif
            }
            if (result_str)
            {
                sal_sprintf(result_str + sal_strlen(result_str), "[%s.%d.%s] {op %c} " ,tbl_str, poly_idx, fld_str, op_code[op_time]);
            }

            head = sal_strstr(op_str, "[");
            sal_memset(tbl_str, 0, sizeof(tbl_str));
            sal_memset(fld_str, 0, sizeof(fld_str));
            op_str = NULL;
            op_time ++;
            if (op_time > MAX_OP_NUM)
            {

            }

        }

        if (b_exted)
        {
            sal_sscanf(head, "\x5b%[^.].%d.%[^\x5d]", tbl_str, &poly_idx, fld_str);

        }
        else
        {
            sal_sscanf(head, "\x5b%[^.].%[^\x5d]", tbl_str, fld_str);
            poly_idx = index;
        }

        DbgTool_DBG_INFO("b_exted:%d,tbl_str:%s,fld_str:%s\n",b_exted,tbl_str,fld_str);

         /*process such as (31:0), (63:32) kind of field, bits larger than 32*/
        bit_split = sal_strstr(fld_str, "(");
        if(bit_split)
        {
            sal_strcpy(tmp_fld_str,fld_str);
            sal_sscanf(tmp_fld_str, "%[^\x28]\x28%d:%d\x29", fld_str, &end_bit, &start_bit);
        }

        if (mode)
        {
            if ( DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_str))
            {
                ctc_cli_out("can't find tbl %s \n", tbl_str);
                return DRV_E_INVALID_TBL;
            }

            if (DRV_E_NONE != drv_usw_get_field_id_by_string(lchip, tbl_id, &fld_id, fld_str))
            {
                sal_printf("can find field %s\n", fld_str);
                return DRV_E_INVALID_FLD;
            }

            cmd = DRV_IOR(tbl_id, fld_id);
            if (DRV_E_NONE != DRV_IOCTL(lchip, poly_idx, cmd, value[op_time]))
            {
                sal_printf("read tbl_str %s fld_str %s error\n", tbl_str, fld_str);
            }
            #if 0
            if (self_op_mult)
            {
                value[op_time] = value[op_time] * self_op_value;
            }
            #endif
        }
        if (result_str)
        {
            sal_sprintf(result_str + sal_strlen(result_str), "[%s.%d.%s] " ,tbl_str, poly_idx, fld_str);
        }

        cnt    = 0;

         /*process such as (31:0), (63:32) kind of field, bits larger than 32*/
        if (bit_split)
        {
            if(start_bit == 0 && end_bit == 31)
            {
                result = value[cnt][0];
            }
            else if (start_bit == 32 && end_bit == 63)
            {
                result = value[cnt][1];
            }
            else if (start_bit == 64 && end_bit == 95)
            {
                result = value[cnt][2];
            }
            else if (start_bit == 96 && end_bit == 127)
            {
                result = value[cnt][4];
            }
            else
            {
                sal_printf("Unsupported bit split\n");
            }

        }
        else
        {
            result = value[cnt][0];
        }

        DbgTool_DBG_INFO("value[%d]:%d\n",cnt,value[cnt][0]);

        while (cnt < op_time)
        {
            switch (op_code[cnt])
            {
                case '+':
                    result += value[cnt+1][0];
                    break;
                case '-':
                    result -= value[cnt+1][0];
                    break;
                case '&':
                    result &= value[cnt+1][0];
                    break;
                case '|':
                    result |= value[cnt+1][0];
                    break;
                default:
                    break;
            }
            cnt++;
        }
    }
    return result;
}


STATIC int32
_ctc_dbg_tool_integrity_result_do(uint8 lchip, char *line, char *result, uint8 *check_pass)
{
#define CHECK_NONE      0
#define CHECK_STATUS    1
#define CHECK_LINK_LIST 2
#define CHECK_INTR      3

    uint8 op_code = OP_NONE;

    char* p_poly_str = NULL;
    char* p_poly_str_right = NULL;

    int32 exp_val = 0;
    int32 rel_val = 0;

    uint32 counter  = 0;
    uint32 index    = 0;
    uint32 next_index = 0;

    uint32 tbl_id = 0;
    fld_id_t fld_id = 0;
    uint32 cmd = 0;

    static uint8 integrity_stat = CHECK_NONE;
    static uint32 exp_head = 0;
    static uint32 exp_tail = 0;
    static uint32 exp_cnt = 0;
    int32 ret = DRV_E_NONE;

    p_poly_str = (char*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(char)*MAX_PER_LINE);
    p_poly_str_right = (char*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(char)*MAX_PER_LINE);
    if ((NULL == p_poly_str) || (NULL == p_poly_str_right))
    {
        ret = DRV_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_poly_str, 0, sizeof(char)*MAX_PER_LINE);
    sal_memset(p_poly_str_right, 0, sizeof(char)*MAX_PER_LINE);

    if (('\r' == line[0]) && ('\n' == line[1]))
    {
        sal_strncpy(result, line, MAX_PER_LINE);
    }
    else if ('#' == line[0])
    {
        if (sal_strstr(line, "Status Check"))
        {
            integrity_stat = CHECK_STATUS;
        }
        else if (sal_strstr(line, "Link List Check Begin"))
        {
            integrity_stat = CHECK_LINK_LIST;
            sal_memset(bit_map, 0, sizeof(bit_map));
        }
        else if (sal_strstr(line, "Link List Check End"))
        {
            integrity_stat = CHECK_NONE;
        }
        else if (sal_strstr(line, "Interrupt Check"))
        {
            integrity_stat = CHECK_INTR;
        }

        sal_strncpy(result, line, MAX_PER_LINE);
    }
    else
    {
        /*get op code*/
        if (sal_strstr(line, ">="))
        {
            op_code = OP_BE;
        }
        else if (sal_strstr(line, "<="))
        {
            op_code = OP_LE;
        }
        else if (sal_strstr(line, ">"))
        {
            op_code = OP_B;
        }
        else if (sal_strstr(line, "<"))
        {
            op_code = OP_L;
        }
        else if (sal_strstr(line, "="))
        {
            op_code = OP_E;
        }
        else
        {
            op_code = OP_NONE;
        }

        /*to calc poly*/
        if (OP_NONE == op_code)
        {
            sal_strncpy(result, line, MAX_PER_LINE);
        }
        else
        {
            /*status check*/
            if (CHECK_STATUS == integrity_stat)
            {
                switch (op_code)
                {
                    case OP_E:
                        sal_sscanf(line, "%[^=]=%s", p_poly_str, p_poly_str_right);
                        rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                        exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                        if (rel_val != exp_val)
                        {
                            sal_sprintf(result, "%s = %d@%d\n", p_poly_str, rel_val, exp_val);
                            sal_printf("%s", result);
                            *check_pass = FALSE;
                        }
                        break;
                    case OP_BE:
                        sal_sscanf(line, "%[^>=]>=%s", p_poly_str, p_poly_str_right);
                        rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                        exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                        if (rel_val < exp_val)
                        {
                            sal_sprintf(result, "%s >= %d@%d\n", p_poly_str, rel_val, exp_val);
                            sal_printf("%s", result);
                            *check_pass = FALSE;
                        }
                        break;
                    case OP_LE:
                        sal_sscanf(line, "%[^<=]<=%s", p_poly_str, p_poly_str_right);
                        rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                        exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                        if (rel_val > exp_val)
                        {
                            sal_sprintf(result, "%s <= %d@%d\n", p_poly_str, rel_val, exp_val);
                            sal_printf("%s", result);
                            *check_pass = FALSE;
                        }
                        break;
                    case OP_B:
                        sal_sscanf(line, "%[^>]>%s", p_poly_str, p_poly_str_right);
                        rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                        exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                        if (rel_val <= exp_val)
                        {
                            sal_sprintf(result, "%s > %d@%d\n", p_poly_str, rel_val, exp_val);
                            sal_printf("%s", result);
                            *check_pass = FALSE;
                        }
                        break;
                    case OP_L:
                        sal_sscanf(line, "%[^<]<%s", p_poly_str, p_poly_str_right);
                        rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                        exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                        if (rel_val >= exp_val)
                        {
                            sal_sprintf(result, "%s < %d@%d\n", p_poly_str, rel_val, exp_val);
                            sal_printf("%s", result);
                            *check_pass = FALSE;
                        }
                        break;
                    default:
                        break;
                }

            }
            /*link list check*/
            else if (CHECK_LINK_LIST == integrity_stat)
            {
                if (sal_strstr(line, "{var"))
                {
                    sal_sscanf(line, "%*[^=]=%s", p_poly_str);
                }
                else
                {

                }

                if (sal_strstr(line, "{var linkHeadPtr}"))
                {
                    exp_head = _ctc_dbg_tool_calc_poly(lchip, 0, FALSE, 1, p_poly_str, NULL);
                }
                else if (sal_strstr(line, "{var linkTailPtr}"))
                {
                    exp_tail = _ctc_dbg_tool_calc_poly(lchip, 0, FALSE, 1, p_poly_str, NULL);
                }
                else if (sal_strstr(line, "{var linkCnt}"))
                {
                    exp_cnt = _ctc_dbg_tool_calc_poly(lchip, 0, FALSE, 1, p_poly_str, NULL);
                }
                else if (sal_strstr(line, "{var nextPtr}"))
                {
                    counter   = 0;
                    index     = exp_head;
                    /*get tbl & field id*/
                    _ctc_dbg_tool_get_id_by_poly(lchip, p_poly_str, &tbl_id, &fld_id);

                    for (;;)
                    {
                        if (IS_BIT_SET(bit_map[index / 32], (index % 32)))
                        {
                            sal_sprintf(result, "@index (0x%x) looped\n", index);
                            break;
                        }
                        else
                        {
                            SET_BIT(bit_map[index / 32], (index % 32));
                        }
                        counter ++;
                        if ((index == exp_tail) || (counter == exp_cnt))
                        {
                             /*kal_printf("index %d, exp_tail %d\n", index, exp_tail);*/
                             /*kal_printf("counter %d, exp_cnt %d\n", counter, exp_cnt);*/
                            break;
                        }

                        cmd = DRV_IOR(tbl_id, fld_id);
                        DRV_IOCTL(lchip, index, cmd, &next_index);
                        if (next_index >= TABLE_MAX_INDEX(lchip, tbl_id))
                        {
                            sal_sprintf(result, "@index 0x%x Next Idx is 0x%x\n", index, next_index);
                            break;
                        }
                        index = next_index;

                    }

                    if (next_index != exp_tail)
                    {
                        sal_sprintf(result, "@actual tail = 0x%x, expetc tail = 0x%x\n", next_index, exp_tail);
                        sal_printf("%s", result);
                        *check_pass = FALSE;
                    }
                    else if (counter != exp_cnt)
                    {
                        sal_sprintf(result, "@actual counter = 0x%x, expetc counter = 0x%x\n", counter, exp_cnt);
                        sal_printf("%s", result);
                        *check_pass = FALSE;
                    }
                }
                else
                {
                    switch (op_code)
                    {
                        case OP_E:
                            sal_sscanf(line, "%[^=]=%s", p_poly_str, p_poly_str_right);
                            rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                            exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                            if (rel_val != exp_val)
                            {
                                sal_sprintf(result, "%s = %d@%d\n", p_poly_str, rel_val, exp_val);
                                sal_printf("%s", result);
                                *check_pass = FALSE;
                            }
                            break;
                        case OP_BE:
                            sal_sscanf(line, "%[^>=]>=%s", p_poly_str, p_poly_str_right);
                            rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                            exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                            if (rel_val < exp_val)
                            {
                                sal_sprintf(result, "%s >= %d@%d\n", p_poly_str, rel_val, exp_val);
                                sal_printf("%s", result);
                                *check_pass = FALSE;
                            }
                            break;
                        case OP_LE:
                            sal_sscanf(line, "%[^<=]<=%s", p_poly_str, p_poly_str_right);
                            rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                            exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                            if (rel_val > exp_val)
                            {
                                sal_sprintf(result, "%s <= %d@%d\n", p_poly_str, rel_val, exp_val);
                                sal_printf("%s", result);
                                *check_pass = FALSE;
                            }
                            break;
                        case OP_B:
                            sal_sscanf(line, "%[^>]>%s", p_poly_str, p_poly_str_right);
                            rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                            exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                            if (rel_val <= exp_val)
                            {
                                sal_sprintf(result, "%s > %d@%d\n", p_poly_str, rel_val, exp_val);
                                sal_printf("%s", result);
                                *check_pass = FALSE;
                            }
                            break;
                        case OP_L:
                            sal_sscanf(line, "%[^<]<%s", p_poly_str, p_poly_str_right);
                            rel_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str, NULL);
                            exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, TRUE, 1, p_poly_str_right, NULL);
                            if (rel_val >= exp_val)
                            {
                                sal_sprintf(result, "%s < %d@%d\n", p_poly_str, rel_val, exp_val);
                                sal_printf("%s", result);
                                *check_pass = FALSE;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            /*interrupt check*/
            else if (CHECK_INTR == integrity_stat)
            {
                uint32 val = 0;

                sal_sscanf(line, "%[^=]=%s", p_poly_str, p_poly_str_right);
                rel_val = _ctc_dbg_tool_calc_inter_mask(lchip, p_poly_str, &val);
                exp_val = _ctc_dbg_tool_calc_poly(lchip, 0, FALSE, 1, p_poly_str_right, NULL);
                if (val != exp_val)
                {
                    sal_sprintf(result, "%s = %u@%d\n", p_poly_str, val, exp_val);
                    sal_printf("%s", result);
                    *check_pass = FALSE;
                }
            }
            /*none check*/
            else
            {
                exp_head = 0;
                exp_tail = 0;
                exp_cnt  = 0;
            }
        }

    }

out:
    if (p_poly_str)
    {
        mem_free(p_poly_str);
    }

    if (p_poly_str_right)
    {
        mem_free(p_poly_str_right);
    }

    return ret;
}






int32 ctc_dbg_tool_integrity_check_result(uint8 lchip, void *gold_file, void *rlt_file, void* p_para3)
{
    sal_file_t gold_fp = NULL;
    sal_file_t rslt_fp = NULL;
    char* p_line     = NULL;
    char* p_result  = NULL;
    uint8 check_pass = TRUE;
    int32 ret = DRV_E_NONE;

    DRV_PTR_VALID_CHECK(gold_file);
    DRV_PTR_VALID_CHECK(rlt_file);

    p_line = (char*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(char)*MAX_PER_LINE);
    p_result = (char*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(char)*MAX_PER_LINE);
    if ((NULL == p_line) || (NULL == p_result))
    {
        ret = DRV_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_line, 0, sizeof(char)*MAX_PER_LINE);
    sal_memset(p_result, 0, sizeof(char)*MAX_PER_LINE);

    gold_fp = sal_fopen(gold_file, "r");
    if (NULL == gold_fp)
    {
        sal_printf("open golden file %s failed!\n", gold_file);
        ret = DRV_E_FILE_OPEN_FAILED;
        goto out;
    }

    rslt_fp = sal_fopen(rlt_file, "w+");
    if (NULL == rslt_fp)
    {
        sal_fclose(gold_fp);
        rslt_fp = NULL;
        sal_printf("open destination file %s failed!\n", rlt_file);
        ret =  DRV_E_FILE_OPEN_FAILED;
        goto out;
    }

    while (NULL != sal_fgets(p_line, MAX_PER_LINE, gold_fp))
    {
        _ctc_dbg_tool_integrity_result_do(lchip, p_line, p_result, &check_pass);
        sal_fprintf(rslt_fp, "%s", p_result);

        sal_memset(p_line, 0, MAX_PER_LINE);
        sal_memset(p_result, 0, MAX_PER_LINE);
    }

    if (check_pass)
    {
        sal_printf("integrity check passed!\n");
        ret = DRV_E_NONE;
    }
    else
    {
        ret = DRV_E_RESERVD_VALUE_ERROR;
    }

    /*close file*/
    if (NULL != gold_fp)
    {
        sal_fclose(gold_fp);
        gold_fp = NULL;
    }
    if (NULL != rslt_fp)
    {
        sal_fclose(rslt_fp);
        rslt_fp = NULL;
    }

out:
    if (p_line)
    {
        mem_free(p_line);
    }

    if (p_result)
    {
        mem_free(p_result);
    }

    return ret;
}

#endif



