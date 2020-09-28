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
#include "ctc_usw_dkit_dump_cfg.h"

#define DKIT_SERDES_ID_MAX 32
#define TAP_MAX_VALUE 64
#define PATTERN_NUM 12

#define DKITS_SERDES_ID_CHECK(ID) \
    do { \
        if(ID >= DKIT_SERDES_ID_MAX) \
        {\
            CTC_DKIT_PRINT("serdes id %d exceed max id %d!!!\n", ID, DKIT_SERDES_ID_MAX-1);\
            return DKIT_E_INVALID_PARAM; \
        }\
    } while (0)

static uint32 g_tm_common_28g_address[6] = {0xE9,0xEB,0xED,0xF3,0xFA,0xFC};
static uint32 g_tm_link_28g_address[4] = {0x01,0x02,0x03,0x05};
/* 0x8zXX */
static uint32 g_tm_control_28g_address[77] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0b,0x0c,0x0d,0x14,0x15,0x16,0x2a,0x2b,
                                    0x2c,0x2e,0x3b,0x4d,0x4e,0x4f,0x50,0x5d,0x5e,0x60,0x61,0x62,0x63,0x64,0x65,
                                    0x66,0x67,0x6e,0x6f,0x71,0x72,0x73,0x75,0x76,0x77,0x78,0x79,0x7a,0x7f,0x80,
                                    0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x8b,0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,
                                    0xa6,0xa7,0xa8,0xe6,0xf3,0xf4,0xf5,0xf7,0xf8,0xfa,0xfb,0xfd,0xfe,0xff};

static uint8 g_tm_serdes_poly_tx[24] = {0};
static uint8 g_tm_serdes_poly_rx[24] = {0};

extern ctc_dkit_master_t* g_usw_dkit_master[CTC_DKITS_MAX_LOCAL_CHIP_NUM];

STATIC int32
_ctc_tm_dkit_misc_serdes_12g_eye(ctc_dkit_serdes_wr_t* p_ctc_dkit_serdes_para, ctc_dkit_serdes_ctl_para_t* p_serdes_para,
                                            uint8 height_flag, uint8 center_flag, uint32 times, uint32* p_rslt_sum);

STATIC int32
_ctc_tm_dkit_misc_serdes_28g_eye(ctc_dkit_serdes_wr_t* p_ctc_dkit_serdes_para, ctc_dkit_serdes_ctl_para_t* p_serdes_para,
                                            uint32 times, uint32* p_rslt_sum);

int32
_ctc_tm_dkit_misc_serdes_dfe_get_val(ctc_dkit_serdes_ctl_para_t* p_serdes_para);




STATIC int32
_ctc_tm_dkit_internal_convert_serdes_addr(ctc_dkit_serdes_wr_t* p_para, uint32* addr, uint8* hss_id)
{
    uint8 lane_id = 0;
    uint8 is_hss15g = 0;
    uint8 lchip = 0;
    DKITS_PTR_VALID_CHECK(p_para);
    DKITS_PTR_VALID_CHECK(addr);
    DKITS_PTR_VALID_CHECK(hss_id);

    lchip = p_para->lchip;
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_para->serdes_id);
    if (is_hss15g)
    {
        lane_id = p_para->serdes_id%8;
    }
    else
    {
        lane_id = p_para->serdes_id%4;
    }
    *hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
    if(CTC_DKIT_SERDES_LINK_TRAINING == p_para->type && !is_hss15g)
    {
        *addr = 0x6000 + lane_id*0x100;
    }
    else if(CTC_DKIT_SERDES_ALL == p_para->type && !is_hss15g)
    {
        *addr = 0x8000 + lane_id*0x100;
    }
    else if(CTC_DKIT_SERDES_COMMON_PLL == p_para->type && !is_hss15g)
    {
        *addr = 0x8400;
    }
    else if (CTC_DKIT_SERDES_COMMON_PLL == p_para->type && is_hss15g)
    {
        *addr = p_para->sub_type*0x100;
    }
    else if (CTC_DKIT_SERDES_BIST == p_para->type)
    {
        *addr = 0x9800;
    }
    else if (CTC_DKIT_SERDES_FW == p_para->type)
    {
        *addr = 0x9f00;
    }
    else if (CTC_DKIT_SERDES_ALL == p_para->type&& is_hss15g)
    {
        *addr = (lane_id+3)*0x100;
    }

    *addr = *addr  + p_para->addr_offset;

    CTC_DKIT_PRINT_DEBUG("hssid:%d, lane:%d\n", *hss_id, lane_id);

    return CLI_SUCCESS;
}

int32
ctc_tm_dkit_misc_read_serdes(void* para)
{
    uint32 addr = 0;
    uint8 is_hss15g = 0;
    uint8 hss_id = 0;
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    uint8 lchip = 0;

    DKITS_PTR_VALID_CHECK(p_para);
    DKITS_SERDES_ID_CHECK(p_para->serdes_id);
    CTC_DKIT_LCHIP_CHECK(p_para->lchip);

    lchip = p_para->lchip;
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_para->serdes_id);
    _ctc_tm_dkit_internal_convert_serdes_addr(p_para, &addr, &hss_id);

    if (is_hss15g)
    {
        drv_chip_read_hss15g(p_para->lchip, hss_id, addr, &p_para->data);
    }
    else
    {
        drv_chip_read_hss28g(p_para->lchip, hss_id, addr, &p_para->data);
    }
    CTC_DKIT_PRINT_DEBUG("read  chip id:%d, hss%s id:%d, addr: 0x%04x, value: 0x%04x\n", p_para->lchip, is_hss15g?"15g":"28g", hss_id, addr, p_para->data);
    return CLI_SUCCESS;
}


int32
ctc_tm_dkit_misc_write_serdes(void* para)
{
    uint32 addr = 0;
    uint8 is_hss15g = 0;
    uint8 hss_id = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    DKITS_PTR_VALID_CHECK(p_para);
    DKITS_SERDES_ID_CHECK(p_para->serdes_id);
    CTC_DKIT_LCHIP_CHECK(p_para->lchip);
    lchip = p_para->lchip;
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_para->serdes_id);
    _ctc_tm_dkit_internal_convert_serdes_addr(p_para, &addr, &hss_id);

    if (is_hss15g)
    {
        drv_chip_write_hss15g(p_para->lchip, hss_id, addr, p_para->data);
    }
    else
    {
        drv_chip_write_hss28g(p_para->lchip, hss_id, addr, p_para->data);
    }
    CTC_DKIT_PRINT_DEBUG("write chip id:%d, hss%s id:%d, addr: 0x%04x, value: 0x%04x\n", p_para->lchip, is_hss15g?"15g":"28g", hss_id, addr, p_para->data);
    return CLI_SUCCESS;
}

STATIC int32
ctc_tm_dkit_misc_serdes_dump_bathtub_28g(uint8 lchip, uint16 serdes_id, sal_file_t p_file)
{
    uint8 i = 0;
    uint8 bathtub_data[95] = {0};
    uint16 lane_id = 0;
    uint16 fw_cmd = 0;
    uint16 status = 0;
    uint16 data = 0;
    int16 margin = 0;
    int16 m = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint16 exp = 9;
    uint32 time_cnt1 = 300;
    uint32 time_cnt2 = 300;
    //uint16 extent = 0;

    cmd = DRV_IOR(OobFcReserved_t, OobFcReserved_reserved_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);
    if (1 == field_val)
    {
        CTC_DKIT_PRINT("%% before dump bathtub data, must close link monitor thread\n");
        return CLI_ERROR;
    }

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    ctc_dkit_serdes_wr.lchip = lchip;
    ctc_dkit_serdes_wr.serdes_id = serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST; /*0x98XX*/
    lane_id = serdes_id % 4;

    fw_cmd = 0x1000 | (lane_id & 0x3) | ((exp & 0xf) << 4) | (1 << 3);
    ctc_dkit_serdes_wr.addr_offset = 0x15;
    ctc_dkit_serdes_wr.data = fw_cmd;
    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    while (TRUE)
    {
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        if (0 == (ctc_dkit_serdes_wr.data >> 12))
        {
            break;
        }
        CTC_DKITS_PRINT_FILE(p_file, ".");
        sal_task_sleep(10);
    }
    status = (ctc_dkit_serdes_wr.data>>8) & 0xf;
    if (0x0 == status)
    {
        CTC_DKITS_PRINT_FILE(p_file, "Bathtub collection started...\n");
    }
    else
    {
        return CLI_SUCCESS;
    }

    while (TRUE)
    {
        fw_cmd = 0x2000;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST; /*0x98XX*/
        ctc_dkit_serdes_wr.data = fw_cmd;
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
        while (TRUE)
        {
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            if (0 == (ctc_dkit_serdes_wr.data >> 12))
            {
                break;
            }
            CTC_DKITS_PRINT_FILE(p_file, ".");
            sal_task_sleep(100);
            if(0 == (--time_cnt1))
            {
                CTC_DKIT_PRINT("%% Read data timeout!\n");
                return CLI_ERROR;
            }
        }
        status = (ctc_dkit_serdes_wr.data>>8) & 0xf;
        data = ctc_dkit_serdes_wr.data & 0xff;
        if ((0x1 == status) && (100 == data))
        {
            break;
        }
        CTC_DKITS_PRINT_FILE(p_file, ".");
        sal_task_sleep(100);
        if(0 == (--time_cnt2))
        {
            CTC_DKIT_PRINT("%% Data collection timeout!\n");
            return CLI_ERROR;
        }
    }


    CTC_DKITS_PRINT_FILE(p_file, "\nbathtub_data = [");
    for (margin = -47; margin < 48;)
    {
        data = (margin&0xffff);
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST; /*0x98XX*/
        ctc_dkit_serdes_wr.addr_offset = 0x16;
        ctc_dkit_serdes_wr.data = data;
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
        ctc_dkit_serdes_wr.addr_offset = 0x15;
        ctc_dkit_serdes_wr.data = 0x3000;
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        while (TRUE)
        {
            ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST; /*0x98XX*/
            ctc_dkit_serdes_wr.addr_offset = 0x15;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            if (0 == (ctc_dkit_serdes_wr.data >> 12))
            {
                break;
            }
            sal_task_sleep(1);
        }
        status = (ctc_dkit_serdes_wr.data>>8) & 0xf;
        data = ctc_dkit_serdes_wr.data & 0xff;
        if (0x2 == status)
        {
            for (i = 0; i < 16; i++)
            {
                m = margin+i;
                if (m < 48)
                {
                    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_FW; /*0x9FXX*/
                    ctc_dkit_serdes_wr.addr_offset = (0x00+i);
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    bathtub_data[47+m] = ctc_dkit_serdes_wr.data;
                    CTC_DKITS_PRINT_FILE(p_file, "%d, ", bathtub_data[47+m]);
                }
            }
        }
        else
        {
            for (i = 0; i < 16; i++)
            {
                m = margin+i;
                if (m < 48)
                {
                    bathtub_data[47+m] = 0;
                    CTC_DKITS_PRINT_FILE(p_file, "%d, ", bathtub_data[47+m]);
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "\n");
        margin += 16;
    }
    CTC_DKITS_PRINT_FILE(p_file, "]\n");

    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST; /*0x98XX*/
    ctc_dkit_serdes_wr.addr_offset = 0x16;
    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    CTC_DKITS_PRINT_FILE(p_file, "       Read BIST 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);


    return CLI_SUCCESS;
}

STATIC int32
ctc_tm_dkit_misc_serdes_dump_isi_28g(uint8 lchip, uint16 serdes_id, sal_file_t p_file)
{
    uint8 lane_id = 0;
    uint16 fm1 = 0;
    uint16 f1  = 0;
    uint16 f2  = 0;
    uint16 f3  = 0;
    uint16 i = 0;
    uint16 tap2[14] = {0};
    int16 signed_tap2[14] = {0};
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    ctc_dkit_serdes_wr.lchip = lchip;
    ctc_dkit_serdes_wr.serdes_id = serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL; /*0x8zXX*/
    lane_id =  serdes_id % 4;

    ctc_dkit_serdes_wr.addr_offset = 0x2b;
    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    f1 = ctc_dkit_serdes_wr.data & 0x7f;

    ctc_dkit_serdes_wr.addr_offset = 0x2c;
    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    f2 = (ctc_dkit_serdes_wr.data >> 8) & 0x7f;
    f3 = ctc_dkit_serdes_wr.data & 0x7f;

    ctc_dkit_serdes_wr.addr_offset = 0x65;
    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    ctc_dkit_serdes_wr.data &= 0xffe1;
    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    sal_task_sleep(100);

    ctc_dkit_serdes_wr.addr_offset = 0x2d;
    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    fm1 = (ctc_dkit_serdes_wr.data >> 8) & 0x7f;

    CTC_DKITS_PRINT_FILE(p_file, "get ISI value(serdes %d, lane %d): \n", serdes_id, lane_id);

    for (i = 0; i < 4; i++)
    {
        tap2[0] = fm1;
        tap2[1] = f1;
        tap2[2] = f2;
        tap2[3] = f3;
        if (tap2[i] >= 64)
        {
            signed_tap2[i] = tap2[i] - 128;
        }
        else
        {
            signed_tap2[i] = tap2[i];
        }
        CTC_DKITS_PRINT_FILE(p_file, "%-5s%-29d:  %d\n", "tap ", i, signed_tap2[i]);
    }

    for (i = 0; i < 10; i++)
    {
        ctc_dkit_serdes_wr.addr_offset = 0x65;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        ctc_dkit_serdes_wr.data &= 0xffe1;
        ctc_dkit_serdes_wr.data |= ((i+1)<<1);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        sal_task_sleep(100);

        ctc_dkit_serdes_wr.addr_offset = 0x2d;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        tap2[i+4] = (ctc_dkit_serdes_wr.data >> 8) & 0x7f;

        if (tap2[i+4] >= 64)
        {
            signed_tap2[i+4] = tap2[i+4] - 128;
        }
        else
        {
            signed_tap2[i+4] = tap2[i+4];
        }

        CTC_DKITS_PRINT_FILE(p_file, "%-5s%-29d:  %d\n", "tap ", (i+4), signed_tap2[i+4]);
    }

    return CLI_SUCCESS;
}

STATIC int32
ctc_tm_dkit_misc_serdes_status_15g(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    sal_time_t tv;
    char* p_time_str = NULL;
    sal_file_t p_file = NULL;
    uint8 i = 0;
    uint8 j = 0;
    uint32 cmd     = 0;
    uint8  hss_id = 0;
    uint8  lane_id = 0;
    uint32 tbl_id  = 0;
    uint32 rslt_sum = 0;
    uint8  step     = 0;
    uint32 value = 0;
    ctc_dkit_serdes_ctl_para_t serdes_para;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;
    Hss12GLaneRxEqCfg0_m rx_eq_cfg;
    Hss12GLaneTxEqCfg0_m tx_eq_cfg;

    DKITS_SERDES_ID_CHECK(serdes_id);
    CTC_DKIT_LCHIP_CHECK(lchip);

    if (file_name)
    {
        p_file = sal_fopen(file_name, "wt");
        if (!p_file)
        {
            CTC_DKIT_PRINT("Open file %s fail!!!\n", file_name);
            return CLI_SUCCESS;
        }
    }

    /*get systime*/
    sal_time(&tv);
    p_time_str = sal_ctime(&tv);

    CTC_DKITS_PRINT_FILE(p_file, "# Serdes %d Regs Status, %s", serdes_id, p_time_str);

    if ((CTC_DKIT_SERDES_COMMON_PLL == type)||(CTC_DKIT_SERDES_ALL == type)|| (CTC_DKIT_SERDES_DETAIL == type))
    {
        /*Common*/
        CTC_DKITS_PRINT_FILE(p_file, "# COMMON Registers, 0x00-0xe3\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_COMMON_PLL;
        for (j = 0; j <= 2; j++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "#    CMU %u\n", j);
            ctc_dkit_serdes_wr.sub_type = j;
            for (i = 0; i <= 0xe3; i++)
            {
                ctc_dkit_serdes_wr.addr_offset = i;
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            }
        }
    }

    if ((CTC_DKIT_SERDES_ALL == type) || (CTC_DKIT_SERDES_DETAIL == type))
    {
        /*Common*/
        CTC_DKITS_PRINT_FILE(p_file, "# LANE Registers, 0x00-0xf6\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
        for (i = 0; i <= 0xf6; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
    }

    /*Extended info*/
    if ((CTC_DKIT_SERDES_ALL == type) || (CTC_DKIT_SERDES_DETAIL == type))
    {
        sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
        sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
        
        /*Eye scan*/
        serdes_para.serdes_id = serdes_id;
        serdes_para.lchip     = lchip;
        serdes_para.type      = CTC_DKIT_SERDIS_CTL_EYE;
        serdes_para.para[0]   = CTC_DKIT_SERDIS_EYE_ALL;

        ctc_dkit_serdes_para.lchip = lchip;
        ctc_dkit_serdes_para.serdes_id = serdes_id;
        ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL;
        
        CTC_DKITS_PRINT_FILE(p_file, "\n Eye height measure\n");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%s\n", "No.", "An", "Ap", "Amin", "EyeOpen");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        rslt_sum = 0;
        for (i = 0; i < 3; i++)
        {
            _ctc_tm_dkit_misc_serdes_12g_eye(&ctc_dkit_serdes_para, &serdes_para, TRUE, TRUE, i, &rslt_sum);
        }
        rslt_sum /= 3;
        CTC_DKITS_PRINT_FILE(p_file, "Avg. %u\n", rslt_sum);
        
        CTC_DKITS_PRINT_FILE(p_file, "\n Eye width measure\n");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%-10s%s\n", "No.", "Width", "Az", "Oes", "Ols", "DeducedWidth");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        rslt_sum = 0;
        for (i = 0; i < 3; i++)
        {
            _ctc_tm_dkit_misc_serdes_12g_eye(&ctc_dkit_serdes_para, &serdes_para, FALSE, TRUE, i, &rslt_sum);
        }
        rslt_sum /= 3;
        CTC_DKITS_PRINT_FILE(p_file, "Avg. %u\n", rslt_sum);

        /*DFE*/
        serdes_para.type = CTC_DKIT_SERDIS_CTL_GET_DFE;
        _ctc_tm_dkit_misc_serdes_dfe_get_val(&serdes_para);
        CTC_DKITS_PRINT_FILE(p_file, "------------------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "DFE status\n");
        CTC_DKITS_PRINT_FILE(p_file, "Dfe : %s\n", (CTC_DKIT_SERDIS_CTL_DFE_ON ==serdes_para.para[0])?"enable":"disable");
        CTC_DKITS_PRINT_FILE(p_file, "Tap1 val: %d\n", (serdes_para.para[1] & 0xFF));
        CTC_DKITS_PRINT_FILE(p_file, "Tap2 val: %d\n", ((serdes_para.para[1]>>8) & 0xFF));
        CTC_DKITS_PRINT_FILE(p_file, "Tap3 val: %d\n", ((serdes_para.para[1]>>16) & 0xFF));
        CTC_DKITS_PRINT_FILE(p_file, "Tap4 val: %d\n", ((serdes_para.para[1]>>24) & 0xFF));
        CTC_DKITS_PRINT_FILE(p_file, "Tap5 val: %d\n", (serdes_para.para[2] & 0xFF));
        CTC_DKITS_PRINT_FILE(p_file, "------------------------------------------------\n");

        /*FFE*/
        hss_id  = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id);
        tbl_id = Hss12GLaneTxEqCfg0_t + hss_id * (Hss12GLaneTxEqCfg1_t - Hss12GLaneTxEqCfg0_t);
        step = Hss12GLaneTxEqCfg0_cfgHssL1Pcs2PmaTxMargin_f - Hss12GLaneTxEqCfg0_cfgHssL0Pcs2PmaTxMargin_f;
        CTC_DKITS_PRINT_FILE(p_file, "------------------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "FFE status: \n");
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &tx_eq_cfg);
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneTxEqCfg0_cfgHssL0PcsTapAdv_f + step*lane_id), &value, &tx_eq_cfg);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %u\n", value);
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneTxEqCfg0_cfgHssL0Pcs2PmaTxMargin_f + step*lane_id), &value, &tx_eq_cfg);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %u\n", value);
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneTxEqCfg0_cfgHssL0PcsTapDly_f + step*lane_id), &value, &tx_eq_cfg);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %u\n", value);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %u\n", 0);
        CTC_DKITS_PRINT_FILE(p_file, "C4: %u\n", 0);
        
        /*CTLE*/
        hss_id  = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id);
        tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
        step = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaVgaCtrl_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaVgaCtrl_f;
        CTC_DKITS_PRINT_FILE(p_file, "------------------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "CTLE status: ");
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaVgaCtrl_f + step*lane_id), &value, &rx_eq_cfg);
        CTC_DKITS_PRINT_FILE(p_file, "%u ", value);
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEqcForce_f + step*lane_id), &value, &rx_eq_cfg);
        CTC_DKITS_PRINT_FILE(p_file, "%u ", value);
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEqRes_f + step*lane_id), &value, &rx_eq_cfg);
        CTC_DKITS_PRINT_FILE(p_file, "%u\n", value);
        CTC_DKITS_PRINT_FILE(p_file, "------------------------------------------------\n");
    }

    if (p_file)
    {
        sal_fclose(p_file);
        CTC_DKIT_PRINT("Save result to %s\n", file_name);
    }

    return CLI_SUCCESS;
}

STATIC int32
ctc_tm_dkit_misc_serdes_status_28g(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    sal_time_t tv;
    char* p_time_str = NULL;
    sal_file_t p_file = NULL;
    uint16 i = 0;
    uint32 rslt_sum = 0;
    ctc_dkit_serdes_ctl_para_t serdes_para;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;

    DKITS_SERDES_ID_CHECK(serdes_id);
    CTC_DKIT_LCHIP_CHECK(lchip);

    if (file_name)
    {
        p_file = sal_fopen(file_name, "wt");
        if (!p_file)
        {
            CTC_DKIT_PRINT("Open file %s fail!!!\n", file_name);
            return CLI_SUCCESS;
        }
    }
    /*get systime*/
    sal_time(&tv);
    p_time_str = sal_ctime(&tv);
    CTC_DKITS_PRINT_FILE(p_file, "# Serdes %d Regs Status, %s", serdes_id, p_time_str);

    if ((CTC_DKIT_SERDES_ALL == type) || (CTC_DKIT_SERDES_DETAIL == type))
    {
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
        for (i = 0; i <= 0xff; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Addr: 0x8%d%02x, Value: 0x%04x\n", serdes_id%4, 
                ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST;
        for (i = 0; i <= 0xff; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Addr: 0x98%02x, Value: 0x%04x\n", 
                ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_FW;
        for (i = 0; i <= 0xff; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Addr: 0x9f%02x, Value: 0x%04x\n", 
                ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_LINK_TRAINING;
        for (i = 0; i <= 0xff; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Addr: 0x6%d%02x, Value: 0x%04x\n", serdes_id%4, 
                ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_COMMON_PLL;
        for (i = 0; i <= 0xff; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Addr: 0x84%02x, Value: 0x%04x\n", 
                ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        if (CTC_DKIT_SERDES_DETAIL == type)
        {
            ctc_tm_dkit_misc_serdes_dump_bathtub_28g(lchip, serdes_id, p_file);
            ctc_tm_dkit_misc_serdes_dump_isi_28g(lchip, serdes_id, p_file);

            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
            sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
            
            /*Eye scan*/
            serdes_para.serdes_id = serdes_id;
            serdes_para.lchip     = lchip;
            serdes_para.type      = CTC_DKIT_SERDIS_CTL_EYE;
            serdes_para.para[0]   = CTC_DKIT_SERDIS_EYE_ALL;

            ctc_dkit_serdes_para.lchip = lchip;
            ctc_dkit_serdes_para.serdes_id = serdes_id;
            ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL;

            CTC_DKITS_PRINT_FILE(p_file, "\n Eye height measure\n");
            CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
            CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%s\n", "No.", "An", "Ap", "Amin", "EyeOpen");
            CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
            rslt_sum = 0;
            for (i = 0; i < 3; i++)
            {
                _ctc_tm_dkit_misc_serdes_28g_eye(&ctc_dkit_serdes_para, &serdes_para, i, &rslt_sum);
            }
            rslt_sum /= 3;
            CTC_DKITS_PRINT_FILE(p_file, " Avg. %u\n", rslt_sum);

            /*DFE*/
            serdes_para.type = CTC_DKIT_SERDIS_CTL_GET_DFE;
            _ctc_tm_dkit_misc_serdes_dfe_get_val(&serdes_para);
            CTC_DKITS_PRINT_FILE(p_file, " ---------------------------------------------------------------\n");
            CTC_DKITS_PRINT_FILE(p_file, " DFE status\n");
            CTC_DKITS_PRINT_FILE(p_file, " Dfe : %s\n", (CTC_DKIT_SERDIS_CTL_DFE_ON ==serdes_para.para[0])?"enable":"disable");
            CTC_DKITS_PRINT_FILE(p_file, " Tap1 val: %d\n", (serdes_para.para[1] & 0xFF));
            CTC_DKITS_PRINT_FILE(p_file, " Tap2 val: %d\n", ((serdes_para.para[1]>>8) & 0xFF));
            CTC_DKITS_PRINT_FILE(p_file, " Tap3 val: %d\n", ((serdes_para.para[1]>>16) & 0xFF));
            CTC_DKITS_PRINT_FILE(p_file, " Tap4 val: %d\n", ((serdes_para.para[1]>>24) & 0xFF));
            CTC_DKITS_PRINT_FILE(p_file, " Tap5 val: %d\n", (serdes_para.para[2] & 0xFF));
            CTC_DKITS_PRINT_FILE(p_file, " ---------------------------------------------------------------\n");
            
            /*FFE*/
            sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
            ctc_dkit_serdes_wr.lchip = lchip;
            ctc_dkit_serdes_wr.serdes_id = serdes_id;
            ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
            CTC_DKITS_PRINT_FILE(p_file, " FFE config\n");
            ctc_dkit_serdes_wr.addr_offset = 0xa8;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "  c0: %d\n", ((ctc_dkit_serdes_wr.data>>8) & 0x3f));
            ctc_dkit_serdes_wr.addr_offset = 0xa7;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "  c1: %d\n", ((ctc_dkit_serdes_wr.data>>8) & 0x3f));
            ctc_dkit_serdes_wr.addr_offset = 0xa6;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "  c2: %d\n", ((ctc_dkit_serdes_wr.data>>8) & 0x3f));
            CTC_DKITS_PRINT_FILE(p_file, " ---------------------------------------------------------------\n");
            /*CTLE*/
            sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
            ctc_dkit_serdes_wr.lchip = lchip;
            ctc_dkit_serdes_wr.serdes_id = serdes_id;
            ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
            CTC_DKITS_PRINT_FILE(p_file, " CTLE config:  ");
            ctc_dkit_serdes_wr.addr_offset = 0x4e;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "%d\n", ((ctc_dkit_serdes_wr.data>>11 & 7)));
            CTC_DKITS_PRINT_FILE(p_file, " ---------------------------------------------------------------\n");
        }
    }
    
    if (p_file)
    {
        sal_fclose(p_file);
        CTC_DKIT_PRINT("Save result to %s\n", file_name);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_set_15g_lane_reset(uint8 lchip, uint8 serdes_id)
{
    uint8  hss_id    = 0;
    uint8  lane_id   = 0;
    uint8  step      = Hss12GGlbCfg0_cfgHssL1RxRstN_f - Hss12GGlbCfg0_cfgHssL0RxRstN_f;
    uint32 cmd       = 0;
    uint32 tb_id     = Hss12GGlbCfg0_t;
    uint32 value     = 0;
    uint32 fld_id    = Hss12GGlbCfg0_cfgHssL0RxRstN_f;
    Hss12GGlbCfg0_m glb_cfg;

    hss_id  = serdes_id/CTC_DKIT_SERDES_HSS15G_LANE_NUM;
    lane_id = serdes_id%CTC_DKIT_SERDES_HSS15G_LANE_NUM;
    tb_id  += hss_id;

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &glb_cfg);

    fld_id += step*lane_id;
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &glb_cfg);

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &glb_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &glb_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &glb_cfg);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_set_28g_lane_reset(uint8 lchip, uint8 serdes_id)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_wr.lchip = lchip;
    ctc_dkit_serdes_wr.serdes_id = serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
    ctc_dkit_serdes_wr.addr_offset = 0x81;
    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 11);/*LINKRST = 1*/
    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 11);/*LINKRST = 0*/
    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    return CLI_SUCCESS;
}

int32
ctc_tm_dkit_misc_serdes_resert(uint8 lchip, uint16 serdes_id)
{
    uint8 is_hss15g = 0;
    DKITS_SERDES_ID_CHECK(serdes_id);
    CTC_DKIT_LCHIP_CHECK(lchip);
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(serdes_id);

    if (is_hss15g)
    {
        _ctc_tm_dkit_misc_serdes_set_15g_lane_reset(lchip, serdes_id);
    }
    else
    {
        _ctc_tm_dkit_misc_serdes_set_28g_lane_reset(lchip, serdes_id);
    }
    return CLI_SUCCESS;
}

int32
ctc_tm_dkit_misc_serdes_status(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    uint8 is_hss15g = 0;
    DKITS_SERDES_ID_CHECK(serdes_id);
    CTC_DKIT_LCHIP_CHECK(lchip);
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(serdes_id);

    if (is_hss15g)
    {
        ctc_tm_dkit_misc_serdes_status_15g(lchip, serdes_id, type, file_name);
    }
    else
    {
        ctc_tm_dkit_misc_serdes_status_28g(lchip, serdes_id, type, file_name);
    }

    return 0;
}

int32
ctc_tm_dkits_misc_dump_indirect(uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag)
{
    int32  ret = CLI_SUCCESS;
    uint8 is_hss15g = 0;
    uint16 i = 0;
    uint8 j = 0;
    uint32 c[4] = {0}, h[10] = {0}, id = 0;
    uint8 flag1 = 0;
    uint8  serdes_id = 0, addr_offset = 0;
    ctc_dkit_serdes_wr_t serdes_para;
    ctc_dkits_dump_serdes_tbl_t serdes_tbl;

    for (serdes_id = 0; serdes_id < DKIT_SERDES_ID_MAX; serdes_id++)
    {
        sal_udelay(10);
        is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(serdes_id);
        if (is_hss15g)
        {
            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
            serdes_para.lchip = lchip;
            serdes_para.serdes_id = serdes_id;
            serdes_para.type = CTC_DKIT_SERDES_COMMON_PLL;
            for (i = 0; i <= 0xe3; i++)
            {
                for (j = 0; j <= 2; j++)
                {
                    serdes_para.addr_offset = i;
                    serdes_para.sub_type = j;
                    ret = ret ? ret : ctc_tm_dkit_misc_read_serdes(&serdes_para);
                    serdes_tbl.serdes_id = serdes_id;
                    serdes_tbl.serdes_mode = CTC_DKIT_SERDES_COMMON;
                    serdes_tbl.offset = addr_offset;
                    serdes_tbl.data = serdes_para.data;
                    if (CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
                    {
                        id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                        ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss28GRegAccCtl_t + id, c[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                    }
                    else
                    {
                        id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                        ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss12GRegAccCtl0_t + id, h[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                    }
                }
            }

            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
            serdes_para.lchip = lchip;
            serdes_para.serdes_id = serdes_id;
            serdes_para.type = CTC_DKIT_SERDES_ALL;
            for (i = 0; i <= 0xf6; i++)
            {
                serdes_para.addr_offset = i;
                ret = ret ? ret : ctc_tm_dkit_misc_read_serdes(&serdes_para);
                serdes_tbl.serdes_id = serdes_id;
                serdes_tbl.serdes_mode = CTC_DKIT_SERDES_COMMON;
                serdes_tbl.offset = addr_offset;
                serdes_tbl.data = serdes_para.data;
                if (CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss28GRegAccCtl_t + id, c[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
                else
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss12GRegAccCtl0_t + id, h[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
            }
        }
        else
        {
            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
            serdes_para.lchip = lchip;
            serdes_para.serdes_id = serdes_id;
            serdes_para.type = CTC_DKIT_SERDES_COMMON_PLL;
            for (i = 0xE8; i <= 0xFF; i++)
            {
                flag1 = 0;
                for (j = 0; j <= 5; j++)
                {
                    if(i == g_tm_common_28g_address[j])
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if(flag1 == 1)
                {
                    continue;
                }
                serdes_para.addr_offset = i;
                ret = ret ? ret : ctc_tm_dkit_misc_read_serdes(&serdes_para);
                serdes_tbl.serdes_id = serdes_id;
                serdes_tbl.serdes_mode = CTC_DKIT_SERDES_COMMON;
                serdes_tbl.offset = addr_offset;
                serdes_tbl.data = serdes_para.data;
                if (CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss28GRegAccCtl_t + id, c[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
                else
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss12GRegAccCtl0_t + id, h[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
            }

            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
            serdes_para.lchip = lchip;
            serdes_para.serdes_id = serdes_id;
            serdes_para.type = CTC_DKIT_SERDES_LINK_TRAINING;
            for (i = 0; i <= 0x2C; i++)
            {
                flag1 = 0;
                for (j = 0; j <= 3; j++)
                {
                    if(i == g_tm_link_28g_address[j])
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if(flag1 == 1)
                {
                    continue;
                }
                serdes_para.addr_offset = i;
                ret = ret ? ret : ctc_tm_dkit_misc_read_serdes(&serdes_para);
                serdes_tbl.serdes_id = serdes_id;
                serdes_tbl.serdes_mode = CTC_DKIT_SERDES_COMMON;
                serdes_tbl.offset = addr_offset;
                serdes_tbl.data = serdes_para.data;
                if (CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss28GRegAccCtl_t + id, c[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
                else
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss12GRegAccCtl0_t + id, h[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
            }

            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
            serdes_para.lchip = lchip;
            serdes_para.serdes_id = serdes_id;
            serdes_para.type = CTC_DKIT_SERDES_ALL;
            for (i = 0; i <= 76; i++)
            {
                serdes_para.addr_offset = g_tm_control_28g_address[i];
                ret = ret ? ret : ctc_tm_dkit_misc_read_serdes(&serdes_para);
                serdes_tbl.serdes_id = serdes_id;
                serdes_tbl.serdes_mode = CTC_DKIT_SERDES_COMMON;
                serdes_tbl.offset = addr_offset;
                serdes_tbl.data = serdes_para.data;
                if (CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss28GRegAccCtl_t + id, c[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
                else
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, Hss12GRegAccCtl0_t + id, h[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
            }
        }
    }

    return ret;
}

int32
_ctc_tm_dkit_misc_prbs_en(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint8 lchip = p_serdes_para->lchip;
    uint16 data   = 0;
    uint16 pattern_type = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_para.lchip = lchip;
    ctc_dkit_serdes_para.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL; //per-lane base, 28G 0x8000 + lane_id*0x100, 12G (lane_id+3)*0x100

    if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
    {
        /* prbs is already enable, do not need operation */
        ctc_dkit_serdes_para.addr_offset = 0x00a0;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        if((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) && (0xe800 == (ctc_dkit_serdes_para.data & 0xe800)))
        {
            return CLI_EOL;
        }

        switch(p_serdes_para->para[1])
        {
            case 2:
            case 3:
                /*PRBS15*/
                pattern_type = 0x0100;
                break;
            case 4:
            case 5:
                /*PRBS23*/
                pattern_type = 0x0200;
                break;
            case 6:
            case 7:
                /*PRBS31*/
                pattern_type = 0x0300;
                break;
            case 8:
                /*PRBS9*/
                pattern_type = 0x0000;
                break;
            case 9:
                /*PRBS8081*/
                pattern_type = 0x0300;
                break;
            case 11:
                /*PRBS1T*/
                pattern_type = 0x2800;
                break;
            default:
                CTC_DKIT_PRINT("Pattern not supported!\n");
                return CLI_ERROR;
        }

        /*1. User-Defined pattern(if necessary)*/
        if (9 == p_serdes_para->para[1]) /* user-defined PRBS 8`0'8`1' */
        {
            data = 0xff00;
            /* TX Test Pattern High */
            ctc_dkit_serdes_para.addr_offset = 0x00a1;
            ctc_dkit_serdes_para.data = data;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            /* TX Test Pattern Low */
            ctc_dkit_serdes_para.addr_offset = 0x00a2;
            ctc_dkit_serdes_para.data = data;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /* set TX_TEST_DATA_SRC (reg0x8zA0[15]) = 1'b0 */
            /* set TX_PRBS_CLOCK_EN (reg0x8zA0[14]) = 1'b1 */
            /* set TX_TEST_EN (reg0x8zA0[13]) = 1'b1 */
            /* set TX_PRBS_GEN_EN (0x8zA0[11]) = 1'b1 */
            data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x6800 : 0);
            ctc_dkit_serdes_para.addr_offset = 0x00a0;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x17ff) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }
        else if (11 == p_serdes_para->para[1])  /* user-defined PRBS 1T(1010) */
        {
            data = 0xaaaa;
            /* TX Test Pattern High */
            ctc_dkit_serdes_para.addr_offset = 0x00a1;
            ctc_dkit_serdes_para.data = data;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            /* TX Test Pattern Low */
            ctc_dkit_serdes_para.addr_offset = 0x00a2;
            ctc_dkit_serdes_para.data = data;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /* set TX_TEST_DATA_SRC (reg0x8zA0[15]) = 1'b0 */
            /* set TX_PRBS_CLOCK_EN (reg0x8zA0[14]) = 1'b1 */
            /* set TX_TEST_EN (reg0x8zA0[13]) = 1'b1 */
            /* set TX_PRBS_GEN_EN (0x8zA0[11]) = 1'b1 */
            data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x6800 : 0);
            ctc_dkit_serdes_para.addr_offset = 0x00a0;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x17ff) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }
        else  /* Typical PRBS mode */
        {
            /*1. PRBS mode select  set TX_PRBS_MODE(reg0x8zA0[9:8]) */
            data = pattern_type;
            ctc_dkit_serdes_para.addr_offset = 0x00a0;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfcff) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /* set TX_TEST_DATA_SRC (reg0x8zA0[15]) = 1'b1 */
            /* set TX_PRBS_CLOCK_EN (reg0x8zA0[14]) = 1'b1 */
            /* set TX_TEST_EN (reg0x8zA0[13]) = 1'b1 */
            /* set TX_PRBS_GEN_EN (0x8zA0[11]) = 1'b1 */
            data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0xe800 : 0);
            ctc_dkit_serdes_para.addr_offset = 0x00a0;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x17ff) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }
    }
    else
    {
        /* prbs is already enable, do not need operation */
        ctc_dkit_serdes_para.addr_offset = 0x76;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        if((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) && (0x01 == (ctc_dkit_serdes_para.data & 0x01)))
        {
            return CLI_EOL;
        }

        switch(p_serdes_para->para[1])
        {
            case 0:
            case 1:
                /*PRBS7*/
                pattern_type = 0x00;
                break;
            case 2:
            case 3:
                /*PRBS15*/
                pattern_type = 0x30;
                break;
            case 4:
            case 5:
                /*PRBS23*/
                pattern_type = 0x40;
                break;
            case 6:
            case 7:
                /*PRBS31*/
                pattern_type = 0x50;
                break;
            case 8:
                /*PRBS9*/
                pattern_type = 0x10;
                break;
            case 10:
                /*PRBS11*/
                pattern_type = 0x20;
                break;
            case 9:
                /*User defined pattern*/
                pattern_type = 0x70;
                break;
            default:
                CTC_DKIT_PRINT("Pattern not supported!\n");
                return CLI_ERROR;
        }

        /*1. PRBS mode select  set r_bist_mode(reg0x76[6:4]) */
        data = pattern_type;
        ctc_dkit_serdes_para.addr_offset = 0x76;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x8f) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        if (9 == p_serdes_para->para[1])  /* HSS12G PRBS 8081 */
        {
            data = 0xff;
            ctc_dkit_serdes_para.addr_offset = 0x96;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0x0;
            ctc_dkit_serdes_para.addr_offset = 0x97;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0xff;
            ctc_dkit_serdes_para.addr_offset = 0x98;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0x0;
            ctc_dkit_serdes_para.addr_offset = 0x99;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0xff;
            ctc_dkit_serdes_para.addr_offset = 0x9a;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0x0;
            ctc_dkit_serdes_para.addr_offset = 0x9b;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0xff;
            ctc_dkit_serdes_para.addr_offset = 0x9c;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            data = 0x0;
            ctc_dkit_serdes_para.addr_offset = 0x9d;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf0) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }

        /*2. Enable r_bist_en  set r_bist_en(reg0x76[0]) = 1*/
        data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 1 : 0);
        ctc_dkit_serdes_para.addr_offset = 0x76;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfe) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
    }

    return CLI_SUCCESS;
}

int32
_ctc_tm_dkit_misc_prbs_check(ctc_dkit_serdes_ctl_para_t* p_serdes_para, bool* pass)
{
    uint8 lchip = p_serdes_para->lchip;
    uint16 data   = 0;
    uint32 err_cnt = 0;
    uint32 err_cnt_buf = 0;
    uint16 pattern_type = 0;
    uint16 time = 1000;
    uint32 delay = 0;
    uint8 is_keep = 0;
    uint8 errcntrst_flag = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_para.lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_para.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL; //per-lane base, 28G 0x8000 + lane_id*0x100, 12G (lane_id+3)*0x100

    delay = p_serdes_para->para[3];
    if (delay)
    {
        sal_task_sleep(delay); /*delay before check*/
    }
    else
    {
        sal_task_sleep(1000);
    }
    is_keep = p_serdes_para->para[2];

    if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
    {
        switch(p_serdes_para->para[1])
        {
            case 2:
            case 3:
                /*PRBS15*/
                pattern_type = 0x1000;
                break;
            case 4:
            case 5:
                /*PRBS23*/
                pattern_type = 0x2000;
                break;
            case 6:
            case 7:
                /*PRBS31*/
                pattern_type = 0x3000;
                break;
            case 8:
                /*PRBS9*/
                pattern_type = 0x0000;
                break;
            default:
                CTC_DKIT_PRINT("Pattern not supported!\n");
                return CLI_ERROR;
        }
        /*1. PRBS Mode Select  set RX_PRBS_MODE (reg0x8z61[13:12])*/
        data = pattern_type;
        ctc_dkit_serdes_para.addr_offset = 0x0061;//reg0x8z61[13:12]
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xcfff) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

        /*2. PRBS Check Enable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b1*/
        data |= 0x0400;
        ctc_dkit_serdes_para.addr_offset = 0x0061;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        if (0 == ((ctc_dkit_serdes_para.data >> 10) & 0x1))
        {
            errcntrst_flag = 1;
        }
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x7bff) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /* reset error cnt: (reg0x8z61[15]) = 1'b1 ,then 1'b0 */
        /* if Keep, DO NOT need do this */
        ctc_dkit_serdes_para.addr_offset = 0x0061;
        if (errcntrst_flag)
        {
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data |= 0x8000;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data &= 0x7fff;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }

        /*3. PRBS Check Status Read*/
        err_cnt = 0;
        sal_task_sleep(1);
        ctc_dkit_serdes_para.data = 0;
        ctc_dkit_serdes_para.addr_offset = 0x0066; //high reg0x8z66[15:0]
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt = ((uint32)ctc_dkit_serdes_para.data) << 16;
        ctc_dkit_serdes_para.addr_offset = 0x0067; //low reg0x8z67[15:0]
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt |= (ctc_dkit_serdes_para.data);
        err_cnt /= 3;
        if(0 != err_cnt)
        {
            *pass = FALSE;
        }
        else
        {
            *pass = TRUE;
        }

        /*4. PRBS Check disable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b0*/
        if(!is_keep)
        {
            data = 0;
            ctc_dkit_serdes_para.addr_offset = 0x0061; //reg0x8z61[10]
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfbff) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }
    }
    else
    {
        switch(p_serdes_para->para[1])
        {
            case 0:
            case 1:
                /*PRBS7*/
                pattern_type = 0x00;
                break;
            case 2:
            case 3:
                /*PRBS15*/
                pattern_type = 0x30;
                break;
            case 4:
            case 5:
                /*PRBS23*/
                pattern_type = 0x40;
                break;
            case 6:
            case 7:
                /*PRBS31*/
                pattern_type = 0x50;
                break;
            case 8:
                /*PRBS9*/
                pattern_type = 0x10;
                break;
            case 10:
                /*PRBS11*/
                pattern_type = 0x20;
                break;
            case 11:
                /*User defined pattern*/
                pattern_type = 0x70;
                break;
            default:
                CTC_DKIT_PRINT("Pattern not supported!\n");
                return CLI_ERROR;
        }

        /*1. PRBS mode select  set r_bist_mode(reg0x76[6:4]) */
        data = pattern_type;
        ctc_dkit_serdes_para.addr_offset = 0x76;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x8f) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

        /*1. Enable r_bist_chk  set r_bist_chk(reg0x77[0]) = 1 and r_bist_chk_zero(reg0x77[1]) = 1*/
        data = 0x3;
        ctc_dkit_serdes_para.addr_offset = 0x77;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfc) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

        /*2. Read bist status*/
        data = 0;
        ctc_dkit_serdes_para.addr_offset = 0xe0;
        ctc_dkit_serdes_para.data = 0;
        while(--time)
        {
            sal_task_sleep(1);
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            data = (3 & (ctc_dkit_serdes_para.data));
            if(0 != data)
            {
                break;
            }
        }
        if((0 == time) || (3 != data)) //timeout, or error occur
        {
            *pass = FALSE;
        }
        else
        {
            *pass = TRUE;
        }

        ctc_dkit_serdes_para.addr_offset = 0xe1;
        ctc_dkit_serdes_para.data = 0;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt_buf = ctc_dkit_serdes_para.data;
        err_cnt = err_cnt_buf;

        ctc_dkit_serdes_para.addr_offset = 0xe2;
        ctc_dkit_serdes_para.data = 0;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt_buf = ctc_dkit_serdes_para.data;
        err_cnt |= (err_cnt_buf << 8);

        ctc_dkit_serdes_para.addr_offset = 0xe3;
        ctc_dkit_serdes_para.data = 0;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt_buf = ctc_dkit_serdes_para.data;
        err_cnt |= (err_cnt_buf << 16);

        ctc_dkit_serdes_para.addr_offset = 0xe4;
        ctc_dkit_serdes_para.data = 0;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt_buf = ctc_dkit_serdes_para.data;
        err_cnt |= (err_cnt_buf << 24);

        /*3. Disable r_bist_chk  set r_bist_chk(reg0x77[0]) = 0 and r_bist_chk_zero(reg0x77[1]) = 0*/
        if(!is_keep)
        {
            data = 0;
            ctc_dkit_serdes_para.addr_offset = 0x77;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfc) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }
    }
    p_serdes_para->para[4] = err_cnt;

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_do_serdes_prbs(ctc_dkit_serdes_ctl_para_t* p_serdes_para, bool* pass)
{
    int ret = CLI_SUCCESS;

    switch(p_serdes_para->para[0])
    {
        case CTC_DKIT_SERDIS_CTL_ENABLE:
        case CTC_DKIT_SERDIS_CTL_DISABLE:
            ret = _ctc_tm_dkit_misc_prbs_en(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_CEHCK:
            ret = _ctc_tm_dkit_misc_prbs_check(p_serdes_para, pass);
            break;
        default:
            CTC_DKIT_PRINT("Feature not supported!\n");
            break;
    }
    return ret;
}

STATIC void
_ctc_tm_dkit_misc_serdes_monitor_handler(void* arg)
{
    ctc_dkit_serdes_ctl_para_t* p_serdes_para = (ctc_dkit_serdes_ctl_para_t*)arg;
    sal_time_t tv;
    char* p_time_str = NULL;
    uint8 lchip = p_serdes_para->lchip;
    uint16 data   = 0;
    uint32 err_cnt = 0;
    uint16 pattern_type = 0;
    uint16 time_count = 1000;
    sal_file_t p_file = NULL;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;
    uint8 errcntrst_flag = 0;

    sal_time(&tv);
    p_time_str = sal_ctime(&tv);
    CTC_DKIT_PRINT("Serdes %d prbs check monitor begin, %s\n", p_serdes_para->serdes_id, p_time_str);
    if (0 != p_serdes_para->str[0])
    {
        p_file = sal_fopen(p_serdes_para->str, "wt");
        if (!p_file)
        {
            CTC_DKIT_PRINT("Open file %s fail!!!\n", p_serdes_para->str);
            return;
        }
        CTC_DKITS_PRINT_FILE(p_file, "Serdes %d prbs check moniter begin, %s\n", p_serdes_para->serdes_id, p_time_str);
        sal_fclose(p_file);
        p_file = NULL;
    }

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_para.lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_para.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL; //per-lane base, 28G 0x8000 + lane_id*0x100, 12G (lane_id+3)*0x100

    if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
    {
        switch(p_serdes_para->para[1])
        {
            case 2:
            case 3:
                /*PRBS15*/
                pattern_type = 0x1000;
                break;
            case 4:
            case 5:
                /*PRBS23*/
                pattern_type = 0x2000;
                break;
            case 6:
            case 7:
                /*PRBS31*/
                pattern_type = 0x3000;
                break;
            case 8:
                /*PRBS9*/
                pattern_type = 0x0000;
                break;
            default:
                CTC_DKIT_PRINT("Pattern not supported!\n");
                return;
        }
        /*1. PRBS Mode Select  set RX_PRBS_MODE (reg0x8z61[13:12])*/
        data = pattern_type;
        ctc_dkit_serdes_para.addr_offset = 0x0061;//reg0x8z61[13:12]
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xcfff) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

        /*2. PRBS Check Enable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b1*/
        data |= 0x0400;
        ctc_dkit_serdes_para.addr_offset = 0x0061;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        if (0 == ((ctc_dkit_serdes_para.data >> 10) & 0x1))
        {
            errcntrst_flag = 1;
        }
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x7bff) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /* reset error cnt: (reg0x8z61[15]) = 1'b1 ,then 1'b0 */
        /* if Keep, DO NOT need do this */
        ctc_dkit_serdes_para.addr_offset = 0x0061;
        if (errcntrst_flag)
        {
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data |= 0x8000;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data &= 0x7fff;
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }

        /*3. PRBS Check Status Read*/
        err_cnt = 0;
        sal_task_sleep(1);
        ctc_dkit_serdes_para.data = 0;
        ctc_dkit_serdes_para.addr_offset = 0x0066; //high reg0x8z66[15:0]
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        err_cnt = ((uint32)ctc_dkit_serdes_para.data) << 16;
        ctc_dkit_serdes_para.addr_offset = 0x0067; //low reg0x8z67[15:0]
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);

        while (1)
        {
            if (g_usw_dkit_master[lchip]->monitor_task[CTC_DKIT_MONITOR_TASK_PRBS].task_end)
            {
                goto End;
            }
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            err_cnt |= (ctc_dkit_serdes_para.data);
            err_cnt /= 3;
            if (p_serdes_para->str[0])
            {
                p_file = sal_fopen(p_serdes_para->str, "a");
            }

            /*get systime*/
            sal_time(&tv);
            p_time_str = sal_ctime(&tv);
            if(0 != err_cnt)  //timeout, or error occur
            {
                CTC_DKITS_PRINT_FILE(p_file, "Serdes %d prbs check error, sync = %d, %s", p_serdes_para->serdes_id, !time_count, p_time_str);
                goto End;
            }
            else
            {
                CTC_DKITS_PRINT_FILE(p_file, "Serdes %d prbs check success, %s", p_serdes_para->serdes_id, p_time_str);
            }

            if (p_file)
            {
                sal_fclose(p_file);
                p_file = NULL;
            }
            sal_task_sleep(p_serdes_para->para[2]*1000);
        }
    }
    else
    {
        switch(p_serdes_para->para[1])
        {
            case 0:
            case 1:
                /*PRBS7*/
                pattern_type = 0x00;
                break;
            case 2:
            case 3:
                /*PRBS15*/
                pattern_type = 0x30;
                break;
            case 4:
            case 5:
                /*PRBS23*/
                pattern_type = 0x40;
                break;
            case 6:
            case 7:
                /*PRBS31*/
                pattern_type = 0x50;
                break;
            case 8:
                /*PRBS9*/
                pattern_type = 0x10;
                break;
            case 10:
                /*PRBS11*/
                pattern_type = 0x20;
                break;
            case 11:
                /*User defined pattern*/
                pattern_type = 0x70;
                break;
            default:
                CTC_DKIT_PRINT("Pattern not supported!\n");
                return;
        }

        /*1. PRBS mode select  set r_bist_mode(reg0x76[6:4]) */
        data = pattern_type;
        ctc_dkit_serdes_para.addr_offset = 0x76;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x8f) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

        /*1. Enable r_bist_chk  set r_bist_chk(reg0x77[0]) = 1 and r_bist_chk_zero(reg0x77[1]) = 1*/
        data = 0x3;
        ctc_dkit_serdes_para.addr_offset = 0x77;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfc) | data);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

        /*2. Read bist status*/
        data = 0;
        ctc_dkit_serdes_para.addr_offset = 0xe0;
        ctc_dkit_serdes_para.data = 0;
        while(--time_count)
        {
            sal_task_sleep(1);
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            data = (3 & (ctc_dkit_serdes_para.data));
            if(0 != data)
            {
                break;
            }
        }

        while (1)
        {
            if (g_usw_dkit_master[lchip]->monitor_task[CTC_DKIT_MONITOR_TASK_PRBS].task_end)
            {
                goto End;
            }
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            data = (3 & (ctc_dkit_serdes_para.data));
            if (p_serdes_para->str[0])
            {
                p_file = sal_fopen(p_serdes_para->str, "a");
            }

            /*get systime*/
            sal_time(&tv);
            p_time_str = sal_ctime(&tv);
            if ((0 == time_count) || (3 != data))  //timeout, or error occur
            {
                CTC_DKITS_PRINT_FILE(p_file, "Serdes %d prbs check error, sync = %d, %s", p_serdes_para->serdes_id, !time_count, p_time_str);
                goto End;
            }
            else
            {
                CTC_DKITS_PRINT_FILE(p_file, "Serdes %d prbs check success, %s", p_serdes_para->serdes_id, p_time_str);
            }

            if (p_file)
            {
                sal_fclose(p_file);
                p_file = NULL;
            }
            sal_task_sleep(p_serdes_para->para[2]*1000);
        }
    }

End:
    if (p_file)
    {
        sal_fclose(p_file);
        p_file = NULL;
    }
    return;
}
STATIC int32
_ctc_tm_dkit_misc_serdes_prbs(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    int ret = CLI_SUCCESS;
    char* pattern[PATTERN_NUM] = {"PRBS7+","PRBS7-","PRBS15+","PRBS15-","PRBS23+","PRBS23-","PRBS31+","PRBS31-", "PRBS9", "Squareware", "PRBS11", "PRBS1T"};
    char* enable = NULL;
    bool pass = FALSE;
    sal_file_t p_file = NULL;
    uint8 task_id = CTC_DKIT_MONITOR_TASK_PRBS;
    uint8 lchip = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    char buffer[SAL_TASK_MAX_NAME_LEN] = {0};
    lchip = p_serdes_para->lchip;

    if (p_serdes_para->para[1] > (PATTERN_NUM-1))
    {
        CTC_DKIT_PRINT("Pattern not supported!\n");
        return CLI_ERROR;
    }

    cmd = DRV_IOR(OobFcReserved_t, OobFcReserved_reserved_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);
    if (1 == field_val)
    {
        CTC_DKIT_PRINT("do prbs check, must close link monitor thread\n");
        return CLI_ERROR;
    }

    if (CTC_DKIT_SERDIS_CTL_MONITOR == p_serdes_para->para[0])
    {
        if (NULL == g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task)
        {
            if (NULL == g_usw_dkit_master[lchip]->monitor_task[task_id].para)
            {
                g_usw_dkit_master[lchip]->monitor_task[task_id].para
                = (ctc_dkit_monitor_para_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_monitor_para_t));
                if (NULL == g_usw_dkit_master[lchip]->monitor_task[task_id].para)
                {
                    return CLI_ERROR;
                }
            }
            g_usw_dkit_master[lchip]->monitor_task[task_id].task_end = 0;
            sal_memcpy(g_usw_dkit_master[lchip]->monitor_task[task_id].para, p_serdes_para , sizeof(ctc_dkit_monitor_para_t));

            sal_sprintf(buffer, "SerdesPrbs-%d", lchip);
            ret = sal_task_create(&g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task,
                                  buffer,
                                  SAL_DEF_TASK_STACK_SIZE,
                                  SAL_TASK_PRIO_DEF,
                                  _ctc_tm_dkit_misc_serdes_monitor_handler,
                                  g_usw_dkit_master[lchip]->monitor_task[task_id].para);
        }
        return CLI_SUCCESS;
    }
    else if (CTC_DKIT_SERDIS_CTL_DISABLE == p_serdes_para->para[0])
    {
        if (g_usw_dkit_master[p_serdes_para->lchip]->monitor_task[task_id].monitor_task)
        {
            g_usw_dkit_master[lchip]->monitor_task[task_id].task_end = 1;
            sal_task_destroy(g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task);
            g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task = NULL;

            if (g_usw_dkit_master[lchip]->monitor_task[task_id].para)
            {
                mem_free(g_usw_dkit_master[lchip]->monitor_task[task_id].para);
            }
            return CLI_SUCCESS;
        }
    }

    ret = _ctc_tm_dkit_misc_do_serdes_prbs(p_serdes_para, &pass);
    if(CLI_EOL == ret)
    {
        CTC_DKIT_PRINT("prbs is enable, need disable first(default prbs is enable with 6, so must disable prbs 6)\n");
        return CLI_ERROR;
    }
    else if (CLI_ERROR == ret)
    {
        return CLI_ERROR;
    }

    if (0 != p_serdes_para->str[0])
    {
        p_file = sal_fopen(p_serdes_para->str, "wt");
        if (!p_file)
        {
            CTC_DKIT_PRINT("Open file %s fail!!!\n", p_serdes_para->str);
            return CLI_ERROR;
        }
    }
    if ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
           ||(CTC_DKIT_SERDIS_CTL_DISABLE == p_serdes_para->para[0]))
    {
        enable = (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])? "Yes":"No";
        CTC_DKITS_PRINT_FILE(p_file, "\n%-12s%-6s%-10s%-9s%-9s\n", "Serdes_ID", "Dir", "Pattern", "Enable", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-12d%-6s%-10s%-9s%-9s\n", p_serdes_para->serdes_id, "TX", pattern[p_serdes_para->para[1]], enable, "  -");
    }
    else if (CTC_DKIT_SERDIS_CTL_CEHCK == p_serdes_para->para[0])
    {
        if (CLI_ERROR == ret)
        {
            enable = "Not Sync";
        }
        else
        {
            enable = pass? "Pass" : "Fail";
        }
        CTC_DKITS_PRINT_FILE(p_file, "\n%-12s%-6s%-10s%-9s%-9s%-9s\n", "Serdes_ID", "Dir", "Pattern", "Enable", "Result", "Err_cnt");
        CTC_DKITS_PRINT_FILE(p_file, "-----------------------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-12d%-6s%-10s%-9s%-9s%-9u\n", p_serdes_para->serdes_id, "RX", pattern[p_serdes_para->para[1]], "  -", enable, p_serdes_para->para[4]);
    }
    if (p_file)
    {
        sal_fclose(p_file);
    }
    return CLI_SUCCESS;
}

int32
_ctc_tm_dkit_misc_loopback_12g_clear(uint8 lchip, uint32 serdes_id)
{
    uint32 addr[4] = {0x06, 0x0e, 0x13, 0x91};
    uint16 mask[4] = {0xe7, 0xcf, 0xfb, 0xfe};
    uint16 data[4] = {0   , 0   , 0x04, 0   };
    uint8  cnt;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));

    ctc_dkit_serdes_para.type        = CTC_DKIT_SERDES_ALL; //(lane+3) * 0x100
    ctc_dkit_serdes_para.serdes_id   = serdes_id;
    ctc_dkit_serdes_para.lchip       = lchip;

    /*reg0x06[4] cfg_rx2tx_lp_en  reg0x06[3] cfg_tx2rx_lp_en  0*/
    /*reg0x0e[5] cfg_txlb_en  reg0x0e[4] cfg_rxlb_en  0*/
    /*reg0x13[2] cfg_cdrck_en  1*/
    /*reg0x91[0] r_lbslv_in_pmad  0*/
    for(cnt = 0; cnt < 4; cnt++)
    {
        ctc_dkit_serdes_para.addr_offset = addr[cnt];
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask[cnt]) | data[cnt]);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_loopback(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint16 data   = 0;
    uint8  recfg_poly = FALSE;
    uint8 lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));

    /*p_serdes_para->para[0]:  CTC_DKIT_SERDIS_CTL_ENABLE/CTC_DKIT_SERDIS_CTL_DISABLE*/
    /*p_serdes_para->para[1]:  CTC_DKIT_SERDIS_LOOPBACK_INTERNAL/CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL*/
    /*p_serdes_para->para[2]:  CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE0/1/2  LS1/LS2/LS3*/

    if(p_serdes_para->serdes_id > (DRV_IS_DUET2(lchip)?39:31))
    {
        CTC_DKIT_PRINT("Parameter Error, invalid serdes ID!\n");
        return CLI_ERROR;
    }
    if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
    {
        /*28G INTERNAL LOOP  not support*/
        if(CTC_DKIT_SERDIS_LOOPBACK_INTERNAL == p_serdes_para->para[1])
        {
            CTC_DKIT_PRINT("Feature not supported!\n");
            return CLI_SUCCESS;
        }
        /*28G EXTERNAL LOOP*/
        else
        {
            data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x8000 : 0);
            /*Register 9803h: LOOPBACK FIFO    Bit15 - LOOPBACK_EN*/
            ctc_dkit_serdes_para.type        = CTC_DKIT_SERDES_COMMON_PLL; //0x8400
            ctc_dkit_serdes_para.serdes_id   = p_serdes_para->serdes_id;
            ctc_dkit_serdes_para.addr_offset = 0x1403; //(0x9803-0x8400)
            ctc_dkit_serdes_para.lchip       = p_serdes_para->lchip;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0x7fff) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        }
    }
    else
    {
        ctc_dkit_serdes_para.type        = CTC_DKIT_SERDES_ALL; //(lane+3) * 0x100
        ctc_dkit_serdes_para.serdes_id   = p_serdes_para->serdes_id;
        ctc_dkit_serdes_para.lchip       = p_serdes_para->lchip;
        /*12G INTERNAL LOOP  LM1*/
        if(CTC_DKIT_SERDIS_LOOPBACK_INTERNAL == p_serdes_para->para[1])
        {
            _ctc_tm_dkit_misc_loopback_12g_clear(p_serdes_para->lchip, p_serdes_para->serdes_id);
            /*reg0x06[3] cfg_tx2rx_lp_en*/
            data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x08 : 0);
            ctc_dkit_serdes_para.addr_offset = 0x06;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xf7) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /*reg0x0e[5] cfg_txlb_en*/
            /*reg0x0e[4] cfg_txlb_en*/
            data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x30 : 0);
            ctc_dkit_serdes_para.addr_offset = 0x0e;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xcf) | data);
            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /*When LM1 enable and polarity is inverted (read from reg), save poly status and set as normal;*/
            /*when LM1 disable and polarity is inverted (read from global variable), set reg as inverded.*/
            ctc_dkit_serdes_para.addr_offset = 0x83;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            recfg_poly = FALSE;
            if(CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
            {
                /*TX: r_tx_pol_inv(reg0x83[1]): 1'b0, Normal(default)  1'b1, Inverted*/
                g_tm_serdes_poly_tx[ctc_dkit_serdes_para.serdes_id] = (uint8)((ctc_dkit_serdes_para.data >> 1) & 0x1);
                /*RX: r_rx_pol_inv(reg0x83[3]): 1'b0, Normal(default)  1'b1, Inverted*/
                g_tm_serdes_poly_rx[ctc_dkit_serdes_para.serdes_id] = (uint8)((ctc_dkit_serdes_para.data >> 3) & 0x1);
                if(1 == g_tm_serdes_poly_tx[ctc_dkit_serdes_para.serdes_id])
                {
                    ctc_dkit_serdes_para.data &= 0xfffd;
                    recfg_poly = TRUE;
                }
                if(1 == g_tm_serdes_poly_rx[ctc_dkit_serdes_para.serdes_id])
                {
                    ctc_dkit_serdes_para.data &= 0xfff7;
                    recfg_poly = TRUE;
                }
            }
            else
            {
                if(1 == g_tm_serdes_poly_tx[ctc_dkit_serdes_para.serdes_id])
                {
                    ctc_dkit_serdes_para.data |= 0x2;
                    recfg_poly = TRUE;
                }
                if(1 == g_tm_serdes_poly_rx[ctc_dkit_serdes_para.serdes_id])
                {
                    ctc_dkit_serdes_para.data |= 0x8;
                    recfg_poly = TRUE;
                }
            }
            if(recfg_poly)
            {
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            }
        }
        /*12G EXTERNAL LOOP  3 modes*/
        else
        {
            switch(p_serdes_para->para[2])
            {
                case CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE0: //LS1
                    _ctc_tm_dkit_misc_loopback_12g_clear(p_serdes_para->lchip, p_serdes_para->serdes_id);
                    /*reg0x06[4] cfg_rx2tx_lp_en*/
                    data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x10 : 0);
                    ctc_dkit_serdes_para.addr_offset = 0x06;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xef) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    /*reg0x0e[4] cfg_rxlb_en*/
                    data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x10 : 0);
                    ctc_dkit_serdes_para.addr_offset = 0x0e;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xef) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    /*reg0x13[2] cfg_cdrck_en*/
                    data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0 : 0x04);
                    ctc_dkit_serdes_para.addr_offset = 0x13;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfb) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    break;
                case CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE1: //LS2
                    _ctc_tm_dkit_misc_loopback_12g_clear(p_serdes_para->lchip, p_serdes_para->serdes_id);
                    /*reg0x06[4] cfg_rx2tx_lp_en*/
                    data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x10 : 0);
                    ctc_dkit_serdes_para.addr_offset = 0x06;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xef) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    /*reg0x0e[4] cfg_rxlb_en*/
                    data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x10 : 0);
                    ctc_dkit_serdes_para.addr_offset = 0x0e;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xef) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    /*reg0x13[2] cfg_cdrck_en*/
                    data = 0x04;
                    ctc_dkit_serdes_para.addr_offset = 0x13;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfb) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    break;
                case CTC_DKIT_SERDIS_LOOPBACK_EXTERNAL_MODE2: //LS3
                    _ctc_tm_dkit_misc_loopback_12g_clear(p_serdes_para->lchip, p_serdes_para->serdes_id);
                    /*reg0x91[0] r_lbslv_in_pmad*/
                    data = ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]) ? 0x01 : 0);
                    ctc_dkit_serdes_para.addr_offset = 0x91;
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
                    ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & 0xfe) | data);
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

                    break;
                default:
                    CTC_DKIT_PRINT("Feature not supported!\n");
                    return CLI_SUCCESS;
            }
        }
    }
    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_status(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    char* pll_select = NULL;
    char* sigdet = NULL;
    uint8 lane_id = 0;
    uint8 hss_id = 0;
    uint8 is_hss15g = 0;
    uint32 value = 0;
    uint32 lol_value = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    char* pll_ready = "-";
    uint8 lchip = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    DKITS_SERDES_ID_CHECK(p_serdes_para->serdes_id);
    lchip = p_serdes_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);

    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id);
    if (is_hss15g)
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_serdes_para->serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_serdes_para->serdes_id);
    }
    else
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_serdes_para->serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_serdes_para->serdes_id);
    }

    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;

    CTC_DKIT_PRINT("\n------Serdes%d Hss%d Lane%d Status-----\n", p_serdes_para->serdes_id, hss_id, lane_id);
    if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
    {
        pll_select = "PLLA";
        ctc_dkit_serdes_wr.addr_offset = 0x002e;  //0x8z2e
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        pll_ready = DKITS_IS_BIT_SET(ctc_dkit_serdes_wr.data, 2) ? "YES" : "NO";
        sigdet = DKITS_IS_BIT_SET(ctc_dkit_serdes_wr.data, 3) ? "YES" : "NO";
    }
    else
    {
        tbl_id = Hss12GLaneMiscCfg0_t + hss_id * (Hss12GLaneMiscCfg1_t - Hss12GLaneMiscCfg0_t);
        fld_id = Hss12GLaneMiscCfg0_cfgHssL0AuxTxCkSel_f +
                 lane_id * (Hss12GLaneMiscCfg0_cfgHssL1AuxTxCkSel_f - Hss12GLaneMiscCfg0_cfgHssL0AuxTxCkSel_f);
        cmd = DRV_IOR(tbl_id, fld_id);
        DRV_IOCTL(lchip, 0, cmd, &value);
        if(4 > lane_id)
        {
            if(0 == value)
            {
                pll_select = "PLLA";
            }
            else if(1 == value)
            {
                pll_select = "PLLB";
            }
            else
            {
                CTC_DKIT_PRINT("Get serdes %u pll select error!\n", p_serdes_para->serdes_id);
                return CLI_ERROR;
            }
        }
        else
        {
            if(0 == value)
            {
                pll_select = "PLLB";
            }
            else if(1 == value)
            {
                pll_select = "PLLC";
            }
            else
            {
                CTC_DKIT_PRINT("Get serdes %u pll select error!\n", p_serdes_para->serdes_id);
                return CLI_ERROR;
            }
        }

        tbl_id = Hss12GLaneMon0_t + hss_id * (Hss12GLaneMon1_t - Hss12GLaneMon0_t);
        fld_id = Hss12GLaneMon0_monHssL0PmaRstDone_f +
                 lane_id * (Hss12GLaneMon0_monHssL1PmaRstDone_f - Hss12GLaneMon0_monHssL0PmaRstDone_f);
        cmd = DRV_IOR(tbl_id, fld_id);
        DRV_IOCTL(lchip, 0, cmd, &value);
        fld_id = Hss12GLaneMon0_monHssL0LolUdl_f +
                 lane_id * (Hss12GLaneMon0_monHssL1LolUdl_f - Hss12GLaneMon0_monHssL0LolUdl_f);
        cmd = DRV_IOR(tbl_id, fld_id);
        DRV_IOCTL(lchip, 0, cmd, &lol_value);
        pll_ready = (((1 == value) && (0 == lol_value)) ? "YES" : "NO");

        fld_id = Hss12GLaneMon0_monHssL0RxEiFiltered_f +
                 lane_id * (Hss12GLaneMon0_monHssL1RxEiFiltered_f - Hss12GLaneMon0_monHssL0RxEiFiltered_f);
        cmd = DRV_IOR(tbl_id, fld_id);
        DRV_IOCTL(lchip, 0, cmd, &value);
        sigdet = ((1 == value) ? "YES" : "NO");
    }

    CTC_DKIT_PRINT("%-20s:%s\n", "PLL Select", pll_select);
    CTC_DKIT_PRINT("%-20s:%s\n", "PLL Ready", pll_ready);
    CTC_DKIT_PRINT("%-20s:%s\n", "Sigdet", sigdet);

    CTC_DKIT_PRINT("------------------------------------\n");

    return CLI_SUCCESS;
}

int32
_ctc_tm_dkit_misc_serdes_polr_check(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    int ret = CLI_SUCCESS;
    bool pass = 0;
    ctc_dkit_serdes_ctl_para_t serdes_para;

    sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    serdes_para.serdes_id = p_serdes_para->serdes_id;
    serdes_para.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
    serdes_para.para[1] = 6;  /* PRBS pattern 6 */
    _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para, &pass);

    serdes_para.serdes_id = p_serdes_para->para[0];
    serdes_para.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
    serdes_para.para[1] = 6;  /* PRBS pattern 6 */
    ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para, &pass);
    if (CLI_ERROR == ret)
    {
        CTC_DKIT_PRINT("%% SerDes %d TX and SerDes %d RX PRBS not match!\n", p_serdes_para->serdes_id, p_serdes_para->para[0]);
    }
    else
    {
        if (pass)
        {
            CTC_DKIT_PRINT("SerDes %d TX and SerDes %d RX PRBS match successfully!\n", p_serdes_para->serdes_id, p_serdes_para->para[0]);
        }
        else
        {
            CTC_DKIT_PRINT("%% SerDes %d TX and SerDes %d RX PRBS check fail!\n", p_serdes_para->serdes_id, p_serdes_para->para[0]);
        }
    }

    serdes_para.serdes_id = p_serdes_para->serdes_id;
    serdes_para.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
    serdes_para.para[1] = 6;  /* PRBS pattern 6 */
    _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para, &pass);

    return CLI_SUCCESS;
}

int32
_ctc_tm_dkit_misc_serdes_dfe_set_en(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint8  lchip = p_serdes_para->lchip;
    uint8 mask_reg = 0;
    uint8 lane = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;
    uint8 serdes_id = p_serdes_para->serdes_id;
    uint8 opr = p_serdes_para->para[0];
    uint32 value   = 0;
    uint32 tbl_id    = 0;
    uint32 fld_id    = 0;
    uint32 cmd = 0;
    uint8  step      = 0;
    uint8  hss_id    = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
    Hss12GLaneRxEqCfg0_m rx_eq_cfg;

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));

    ctc_dkit_serdes_para.serdes_id = serdes_id;
    ctc_dkit_serdes_para.lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id) ? CTC_DKIT_SERDES_COMMON_PLL : CTC_DKIT_SERDES_ALL;
    lane = CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
    {
        CTC_DKIT_PRINT("Cannot change HSS28G DFE status (always enable)! Serdes ID=%u\n", serdes_id);
        return CLI_SUCCESS;
    }
    if(CTC_DKIT_SERDIS_CTL_DFE_OFF == opr)
    {
        /*0x23[0]    cfg_dfe_pd           0-enable 1-disable  set 0 if loopback enabled*/
        mask_reg = 0xfe;
        ctc_dkit_serdes_para.addr_offset = 0x23;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x01);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x23[2]    cfg_dfeck_en         1-enable 0-disable                   */
        mask_reg = 0xfb;
        ctc_dkit_serdes_para.addr_offset = 0x23;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x23[3]    cfg_erramp_pd        0-enable 1-disable                   */
        mask_reg = 0xf7;
        ctc_dkit_serdes_para.addr_offset = 0x23;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x08);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x22[4:0]  cfg_dfetap_en        5'h1f-enable 5'h0-disable            */
        mask_reg = 0xe0;
        ctc_dkit_serdes_para.addr_offset = 0x22;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x1a[2]    cfg_pi_dfe_en        1-enable 0-disable                   */
        mask_reg = 0xfb;
        ctc_dkit_serdes_para.addr_offset = 0x1a;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x30[0]    cfg_summer_en        1                                    */

        /*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe  0*/
        value = 0;
        tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);
        step = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaHoldDfe_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f;
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f + step * lane;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &rx_eq_cfg);
        /*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaEnDfeDig 1-enable 0-disable*/
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEnDfeDig_f + step * lane;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &rx_eq_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);

        /*0x31[2]    cfg_rstn_dfedig_gen1 1-enable 0-disable                   */
        mask_reg = 0xfb;
        ctc_dkit_serdes_para.addr_offset = 0x31;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
    }
    else if(CTC_DKIT_SERDIS_CTL_DFE_ON == opr)
    {
        /*0x23[0]    cfg_dfe_pd           0-enable 1-disable  set 0 if loopback enabled*/
        mask_reg = 0xfe;
        ctc_dkit_serdes_para.addr_offset = 0x23;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x23[2]    cfg_dfeck_en         1-enable 0-disable                   */
        mask_reg = 0xfb;
        ctc_dkit_serdes_para.addr_offset = 0x23;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x04);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x23[3]    cfg_erramp_pd        0-enable 1-disable                   */
        mask_reg = 0xf7;
        ctc_dkit_serdes_para.addr_offset = 0x23;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x22[4:0]  cfg_dfetap_en        5'h1f-enable 5'h0-disable            */
        mask_reg = 0xe0;
        ctc_dkit_serdes_para.addr_offset = 0x22;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x1f);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x1a[2]    cfg_pi_dfe_en        1-enable 0-disable                   */
        mask_reg = 0xfb;
        ctc_dkit_serdes_para.addr_offset = 0x1a;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x04);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
        /*0x30[0]    cfg_summer_en        1                                    */

        /*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe  0*/
        value = 0;
        tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);
        step = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaHoldDfe_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f;
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f + step * lane;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &rx_eq_cfg);
        /*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaEnDfeDig 1-enable 0-disable*/
        value = 1;
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEnDfeDig_f + step * lane;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &rx_eq_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);

        /*0x31[2]    cfg_rstn_dfedig_gen1 1-enable 0-disable                   */
        mask_reg = 0xfb;
        ctc_dkit_serdes_para.addr_offset = 0x31;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        ctc_dkit_serdes_para.data = ((ctc_dkit_serdes_para.data & mask_reg) | 0x04);
        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
    }
    return CLI_SUCCESS;
}

int32
_ctc_tm_dkit_misc_serdes_dfe_get_val(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint8  lchip = p_serdes_para->lchip;
    uint8 mask_reg = 0;
    uint8 lane = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;
    uint8 serdes_id = p_serdes_para->serdes_id;
    uint32 value   = 0;
    uint32 tbl_id    = 0;
    uint32 fld_id    = 0;
    uint32 cmd = 0;
    uint8  step      = 0;
    uint8  hss_id    = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
    uint8  en_flag = TRUE;
    Hss12GLaneRxEqCfg0_m rx_eq_cfg;

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));

    ctc_dkit_serdes_para.serdes_id = serdes_id;
    ctc_dkit_serdes_para.lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL;
    lane = CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
    {
        p_serdes_para->para[0] = CTC_DKIT_SERDIS_CTL_DFE_ON;
        /*tap 1*/
        ctc_dkit_serdes_para.addr_offset = 0x002b;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x007f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[1] = value;
        /*tap 2*/
        ctc_dkit_serdes_para.addr_offset = 0x002c;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x007f & (ctc_dkit_serdes_para.data>>8);
        p_serdes_para->para[1] |= (value<<8);
        /*tap 3*/
        ctc_dkit_serdes_para.addr_offset = 0x002c;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x007f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[1] |= (value<<16);
    }
    else
    {
        /*0x23[0]    cfg_dfe_pd           0-enable 1-disable  set 0 if loopback enabled*/
        if(en_flag)
        {
            mask_reg = 0xfe;
            ctc_dkit_serdes_para.addr_offset = 0x23;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            if(0 != (ctc_dkit_serdes_para.data & (~mask_reg)))
            {
                en_flag = FALSE;
            }
        }
        /*0x23[2]    cfg_dfeck_en         1-enable 0-disable                   */
        if(en_flag)
        {
            mask_reg = 0xfb;
            ctc_dkit_serdes_para.addr_offset = 0x23;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            if(0 == (ctc_dkit_serdes_para.data & (~mask_reg)))
            {
                en_flag = FALSE;
            }
        }
        /*0x23[3]    cfg_erramp_pd        0-enable 1-disable                   */
        if(en_flag)
        {
            mask_reg = 0xf7;
            ctc_dkit_serdes_para.addr_offset = 0x23;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            if(0 != (ctc_dkit_serdes_para.data & (~mask_reg)))
            {
                en_flag = FALSE;
            }
        }
        /*0x22[4:0]  cfg_dfetap_en        5'h1f-enable 5'h0-disable            */
        if(en_flag)
        {
            mask_reg = 0xe0;
            ctc_dkit_serdes_para.addr_offset = 0x22;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            if(0 == (ctc_dkit_serdes_para.data & (~mask_reg)))
            {
                en_flag = FALSE;
            }
        }
        /*0x1a[2]    cfg_pi_dfe_en        1-enable 0-disable                   */
        if(en_flag)
        {
            mask_reg = 0xfb;
            ctc_dkit_serdes_para.addr_offset = 0x1a;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            if(0 == (ctc_dkit_serdes_para.data & (~mask_reg)))
            {
                en_flag = FALSE;
            }
        }
        /*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaEnDfeDig 1-enable 0-disable*/
        value = 0;
        tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);
        step = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaHoldDfe_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f;
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEnDfeDig_f + step * lane;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &value, &rx_eq_cfg);
        if(en_flag)
        {
            if(0 == value)
            {
                en_flag = FALSE;
            }
        }

        /*0x31[2]    cfg_rstn_dfedig_gen1 1-enable 0-disable                   */
        if(en_flag)
        {
            mask_reg = 0xfb;
            ctc_dkit_serdes_para.addr_offset = 0x31;
            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            if(0 == (ctc_dkit_serdes_para.data & (~mask_reg)))
            {
                en_flag = FALSE;
            }
        }

        p_serdes_para->para[0] = en_flag ? CTC_DKIT_SERDIS_CTL_DFE_ON : CTC_DKIT_SERDIS_CTL_DFE_OFF;

        /*tap 1*/
        ctc_dkit_serdes_para.addr_offset = 0xc5;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x3f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[1] = value;
        /*tap 2*/
        ctc_dkit_serdes_para.addr_offset = 0xc6;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x1f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[1] |= (value<<8);
        /*tap 3*/
        ctc_dkit_serdes_para.addr_offset = 0xc7;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x0f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[1] |= (value<<16);
        /*tap 4*/
        ctc_dkit_serdes_para.addr_offset = 0xc8;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x0f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[1] |= (value<<24);
        /*tap 5*/
        ctc_dkit_serdes_para.addr_offset = 0xc9;
        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
        value = 0x0f & ctc_dkit_serdes_para.data;
        p_serdes_para->para[2] = value;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_12g_eye(ctc_dkit_serdes_wr_t* p_ctc_dkit_serdes_para, ctc_dkit_serdes_ctl_para_t* p_serdes_para,
                                            uint8 height_flag, uint8 center_flag, uint32 times, uint32* p_rslt_sum)
{
    uint32 val_0x23  = 0;  //save 0x23 value
    uint16 mask_0x23 = 0x0d;  //bit 0, 2, 3
    uint32 val_0x1a  = 0;  //save 0x1a value
    uint16 mask_0x1a = 0x4;  //bit 2
    uint16 mask_reg  = 0;
    uint32 val_isc   = 0;
    uint8  lchip     = 0;
    uint16 data      = 0;
    uint32 proc_wait = 1000;
    uint32 cmd       = 0;
    uint8  step      = 0;
    uint32 tbl_id    = 0;
    uint32 fld_id    = 0;
    uint8  hss_id    = CTC_DKIT_MAP_SERDES_TO_HSSID(p_serdes_para->serdes_id);
    uint8  lane      = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_serdes_para->serdes_id);
    uint32 val_don   = 0;
    uint32 val_ret[3]= {0};
    int32  ret = 0;
    uint32 lock_id   = DRV_MCU_LOCK_EYE_SCAN_0 + lane;
    Hss12GIscanMon0_m isc_mon;
    Hss12GLaneRxEqCfg0_m rx_eq_cfg;

    ret = drv_usw_mcu_lock(lchip, lock_id, hss_id);
    if(0 != ret)
    {
        CTC_DKIT_PRINT("Get lock error (eye scan)! hss_id=%u, lane=%u\n", hss_id, lane);
        return CLI_ERROR;
    }

    /*
    1. 0x94[4] r_iscan_reg write 1
    2. 0x2b[4] hwt_fom_sel write 0
    2a. 0x49[4] cfg_figmerit_sel   0~central phase 1~all phase
    2b. 0x2b[6] cfg_man_volt_en
    2c. 0x2c[4] cfg_os_man_en
    */
    /*1. 0x94[4] r_iscan_reg write 1*/
    mask_reg = 0xef;
    p_ctc_dkit_serdes_para->addr_offset = 0x94;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x10);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    /*2. 0x2b[4] hwt_fom_sel write 0*/
    mask_reg = 0xef;
    p_ctc_dkit_serdes_para->addr_offset = 0x2b;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x00);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    /*2a. 0x49[4] cfg_figmerit_sel   0~central phase 1~all phase*/
    mask_reg = 0xef;
    p_ctc_dkit_serdes_para->addr_offset = 0x49;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    if(center_flag)
    {
        p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x00);
    }
    else
    {
        p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x10);
    }
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);


    if(TRUE == height_flag)
    {
        /*2b. 0x2b[6] cfg_man_volt_en*/
        mask_reg = 0xbf;
        p_ctc_dkit_serdes_para->addr_offset = 0x2b;
        ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
        p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);
        /*2c. 0x2c[4] cfg_os_man_en*/
        mask_reg = 0xef;
        p_ctc_dkit_serdes_para->addr_offset = 0x2c;
        ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
        p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x00);
        ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);
    }
    else
    {
        /*2b. 0x2b[6] cfg_man_volt_en*/
        mask_reg = 0xbf;
        p_ctc_dkit_serdes_para->addr_offset = 0x2b;
        ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
        p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x40);
        ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);
        /*2c. 0x2c[4] cfg_os_man_en*/
        mask_reg = 0xef;
        p_ctc_dkit_serdes_para->addr_offset = 0x2c;
        ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
        p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x10);
        ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);
    }

    /*
    read procedure
    0. write Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe value 1
    1. 0x2b[1] cfg_iscan_sel write 1
    2. DFE related reg 0x23 0x1a save
       2a. wait > 100ns
    3. 0x2b[0] cfg_iscan_en write 1
    4. wait Hss12GIscanMon[0~2].monHssL[0~7]IscanDone jumps to 1
    5. read Hss12GIscanMon[0~2].monHssL[0~7]IscanResults to get value
    5. write Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe value 0
    */
    /*0. write Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe value 1*/
    val_isc = 1;
    tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);
    step = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaHoldDfe_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f;
    fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f + step * lane;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &val_isc, &rx_eq_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);

    /*1. 0x2b[1] cfg_iscan_sel write 1*/
    mask_reg = 0xfd;
    p_ctc_dkit_serdes_para->addr_offset = 0x2b;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x02);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);


    /*2. DFE related reg*/
    p_ctc_dkit_serdes_para->addr_offset = 0x23;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    val_0x23 = (mask_0x23 & p_ctc_dkit_serdes_para->data);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & (~mask_0x23)) | 0x4);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    p_ctc_dkit_serdes_para->addr_offset = 0x1a;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    val_0x1a = (mask_0x1a & p_ctc_dkit_serdes_para->data);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & (~mask_0x1a)) | 0x4);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    sal_task_sleep(1);

    /*3. 0x2b[0] cfg_iscan_en write 1*/
    mask_reg = 0xfe;
    p_ctc_dkit_serdes_para->addr_offset = 0x2b;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x01);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    /*4. wait Hss12GIscanMon[0~2].monHssL[0~7]IscanDone jumps to 1*/
    tbl_id = Hss12GIscanMon0_t + hss_id * (Hss12GIscanMon1_t - Hss12GIscanMon0_t);
    step = Hss12GIscanMon0_monHssL1IscanDone_f - Hss12GIscanMon0_monHssL0IscanDone_f;
    fld_id = Hss12GIscanMon0_monHssL0IscanDone_f + step * lane;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);

    while(--proc_wait)
    {
        DRV_IOCTL(lchip, 0, cmd, &isc_mon);
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &val_don, &isc_mon);
        if(val_don)
        {
            break;
        }
        sal_task_sleep(5);
    }
    if(0 == proc_wait)
    {
        CTC_DKIT_PRINT("Get eye diagram result timeout!\n");
    }

    /*5. read Hss12GIscanMon[0~2].monHssL[0~7]IscanResults to get value*/
    fld_id = Hss12GIscanMon0_monHssL0IscanResults_f + step * lane;
    DRV_IOR_FIELD(lchip, tbl_id, fld_id, val_ret, &isc_mon);
    if(val_ret[2])
    {
        CTC_DKIT_PRINT("Error %u\n", val_ret[2]);
    }

    /*
    disable iscan
    1. 0x2b[1] cfg_iscan_sel write 0
    2. recover DFE related reg value
    3. 0x2b[0] cfg_iscan_en write 0
    4. write Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe value 0
    */
    /*1. 0x2b[1] cfg_iscan_sel write 0*/
    mask_reg = 0xfd;
    p_ctc_dkit_serdes_para->addr_offset = 0x2b;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x00);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    /*2. recover DFE related reg value*/
    p_ctc_dkit_serdes_para->addr_offset = 0x23;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & (~mask_0x23)) | val_0x23);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    p_ctc_dkit_serdes_para->addr_offset = 0x1a;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & (~mask_0x1a)) | val_0x1a);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    /*3. 0x2b[0] cfg_iscan_en write 0*/
    mask_reg = 0xfe;
    p_ctc_dkit_serdes_para->addr_offset = 0x2b;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    p_ctc_dkit_serdes_para->data = ((p_ctc_dkit_serdes_para->data & mask_reg) | 0x00);
    ctc_tm_dkit_misc_write_serdes(p_ctc_dkit_serdes_para);

    /*4. write Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe value 0*/
    val_isc = 0;
    tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);
    step = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaHoldDfe_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f;
    fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f + step * lane;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &val_isc, &rx_eq_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg);

    ret = drv_usw_mcu_unlock(lchip, lock_id, hss_id);
    if (DRV_E_NONE != ret)
    {
        CTC_DKIT_PRINT("Get unlock error (eye scan)! hss_id=%u, lane=%u\n", hss_id, lane);
        return CLI_ERROR;
    }

    if(TRUE == height_flag)
    {
        data = (uint16)(0x000000ff & val_ret[0]);
        CTC_DKIT_PRINT(" %-10d%-10s%-10s%-10s%10d\n", times + 1, "-", "-", "-", data);
    }
    else
    {
        data = (uint16)(0x0000003f & val_ret[0]);
        CTC_DKIT_PRINT(" %-10d%-10d%-10s%-10s%-10s%-10s\n", times + 1, data, "-", "-", "-", "-");
    }
    (*p_rslt_sum) += data;

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_28g_eye(ctc_dkit_serdes_wr_t* p_ctc_dkit_serdes_para, ctc_dkit_serdes_ctl_para_t* p_serdes_para,
                                            uint32 times, uint32* p_rslt_sum)
{
    uint32 data = 0;
    int16  signed_data = 0;
    int32  signed_data32 = 0;
    uint32 mlt_factor = 1000;
    uint8  dac = 0;

    /*0x802a + 0x100*lane*/
    p_ctc_dkit_serdes_para->addr_offset = 0x002a;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    signed_data = (p_ctc_dkit_serdes_para->data) & 0x7fff;
    signed_data <<= 1;
    signed_data >>= 1;

    /*0x804c + 0x100*lane*/
    p_ctc_dkit_serdes_para->addr_offset = 0x004c;
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);

    if((p_ctc_dkit_serdes_para->data) & (1<<7))
    {
        /* overwrite enabled  0x804f + 0x100*lane*/
        p_ctc_dkit_serdes_para->addr_offset = 0x004f;
    }
    else
    {
        /* overwrite disabled  0x807f + 0x100*lane*/
        p_ctc_dkit_serdes_para->addr_offset = 0x007f;
    }
    ctc_tm_dkit_misc_read_serdes(p_ctc_dkit_serdes_para);
    dac = (uint8)(((p_ctc_dkit_serdes_para->data) >> 12) & 0xf);

    signed_data32 = signed_data * mlt_factor;
    data = (uint32)(signed_data32 / 2048 * (dac*50 + 200));
    data /= mlt_factor;
    CTC_DKIT_PRINT(" %-10d%-10s%-10s%-10s%10d\n", times + 1, "-", "-", "-", data);

    (*p_rslt_sum) += data;

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_eye(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint32 times = 0;
    uint32 i = 0;
    uint8  lchip = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;
    uint32 rslt_sum  = 0;

    DKITS_PTR_VALID_CHECK(p_serdes_para);
    lchip = p_serdes_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);

    sal_memset(&ctc_dkit_serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));
    times = p_serdes_para->para[1];
    if (0 == times)
    {
        times = 10;
    }

    ctc_dkit_serdes_para.lchip = lchip;
    ctc_dkit_serdes_para.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_ALL;

    if ((CTC_DKIT_SERDIS_EYE_HEIGHT == p_serdes_para->para[0])
       || (CTC_DKIT_SERDIS_EYE_ALL == p_serdes_para->para[0]))
    {
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%s\n", "No.", "An", "Ap", "Amin", "EyeOpen");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");

        if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
        {
            for (i = 0; i < times; i++)
            {
                _ctc_tm_dkit_misc_serdes_28g_eye(&ctc_dkit_serdes_para, p_serdes_para, i, &rslt_sum);
            }
        }
        else
        {
            for (i = 0; i < times; i++)
            {
                _ctc_tm_dkit_misc_serdes_12g_eye(&ctc_dkit_serdes_para, p_serdes_para, TRUE,
                    ((CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E6 == p_serdes_para->precision) ? TRUE : FALSE),
                    i, &rslt_sum);
            }
        }

        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%10d\n","Avg", "-", "-", "-", (rslt_sum/times));
    }

    if ((CTC_DKIT_SERDIS_EYE_WIDTH == p_serdes_para->para[0])
       || (CTC_DKIT_SERDIS_EYE_ALL == p_serdes_para->para[0]))
    {
        rslt_sum = 0;
        CTC_DKIT_PRINT("\n Eye width measure\n");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%-10s%s\n", "No.", "Width", "Az", "Oes", "Ols", "DeducedWidth");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");

        if(CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
        {
            CTC_DKIT_PRINT(" Feature not support!\n");
        }
        else
        {
            for (i = 0; i < times; i++)
            {
                _ctc_tm_dkit_misc_serdes_12g_eye(&ctc_dkit_serdes_para, p_serdes_para, FALSE,
                    ((CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E6 == p_serdes_para->precision) ? TRUE : FALSE),
                    i, &rslt_sum);
            }
            CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
            CTC_DKIT_PRINT(" %-10s%-10d%-10s%-10s%-10s%-10s\n\n", "Avg", (rslt_sum/times), "-", "-", "-", "-");
        }
    }
    else if(CTC_DKIT_SERDIS_EYE_HEIGHT != p_serdes_para->para[0])
    {
        CTC_DKIT_PRINT(" Feature not support!\n");
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(uint16* tap_count, uint8 tap_id)
{
    uint8 tap_best = 0;
    uint8 i = 0;
    uint16 tap_count_equal[TAP_MAX_VALUE] = {0};
    uint16 tap_count_num = 0;
    uint8 min = 0,max = 0;
    uint8 count_valid = 0;

    if (0 == tap_id)
    {
        min = 0;
        max = 10;
    }
    else if (1 == tap_id)
    {
        min = 40;
        max = TAP_MAX_VALUE - 1;
    }
    else if (2 == tap_id)
    {
        min = 0;
        max = 20;
    }

    for (i = min; i <= max; i++)
    {
        if (tap_count[i] != 0)
        {
            tap_best = (tap_count[i] >= tap_count[tap_best])? i : tap_best;
            count_valid = 1;
        }
    }

    if (0 == count_valid)
    {
        return 0xFF;
    }

    for (i = 0; i < TAP_MAX_VALUE; i++)
    {
        if (tap_count[tap_best] == tap_count[i])
        {
            tap_count_equal[tap_count_num++] = i;
        }
    }
    tap_best = tap_count_equal[tap_count_num / 2];

    return tap_best;
}

STATIC int32
_ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(uint16* tap_count, uint8 tap_id)
{
    uint8 tap_best = 0;
    uint8 i = 0;
    uint16 tap_count_equal[TAP_MAX_VALUE] = {0};
    uint16 tap_count_num = 0;
    uint8 min = 0,max = 0;
    uint8 count_valid = 0;

    if (0 == tap_id)
    {
        min = 0;
        max = 5;
    }
    else if (1 == tap_id)
    {
        min = 0;
        max = 15;
    }
    else if (2 == tap_id)
    {
        min = 30;
        max = TAP_MAX_VALUE - 1;
    }
    else if (3 == tap_id)
    {
        min = 0;
        max = 10;
    }

    for (i = min; i <= max; i++)
    {
        if (tap_count[i] != 0)
        {
            tap_best = (tap_count[i] >= tap_count[tap_best])? i : tap_best;
            count_valid = 1;
        }
    }

    if (0 == count_valid)
    {
        return 0xFF;
    }

    for (i = 0; i < TAP_MAX_VALUE; i++)
    {
        if (tap_count[tap_best] == tap_count[i])
        {
            tap_count_equal[tap_count_num++] = i;
        }
    }
    tap_best = tap_count_equal[tap_count_num / 2];

    return tap_best;
}

/*
28G:
Coeff Register Name      Address
C0    TX_FIR_POST_3      8za4h[13:8]
C1    TX_FIR_POST_2      8za5h[13:8]
C2    TX_FIR_POST_1      8za6h[13:8]
C3    TX_FIR_MAIN        8za7h[13:8]
C4    TX_FIR_PRECURSOR   8za8h[13:8]
12G:
Coeff Register Name                Address
C0    cfg_itx_ipdriver_base[2:0]   33h[2:0]
C1    cfg_ibias_tune_reserve[5:0]  52h[5:0]
C2    pcs_tap_adv[4:0]             03h[4:0]
C3    pcs_tap_dly[4:0]             04h[4:0]
*/

STATIC int32
_ctc_tm_dkit_misc_serdes_ffe_scan(void* p_para)
{
    uint8 lchip = 0;
    uint16 tap0 = 0;
    uint16 tap1 = 0;
    uint16 tap2 = 0;
    uint16 tap3 = 0;
    uint16 tap4 = 0;
    uint16 tap0_best = 0;
    uint16 tap1_best = 0;
    uint16 tap2_best = 0;
    uint16 tap3_best = 0;
    uint16 tap4_best = 0;
    uint16 tap0_count[TAP_MAX_VALUE] = {0};
    uint16 tap1_count[TAP_MAX_VALUE] = {0};
    uint16 tap2_count[TAP_MAX_VALUE] = {0};
    uint16 tap3_count[TAP_MAX_VALUE] = {0};
    uint16 tap4_count[TAP_MAX_VALUE] = {0};
    uint32 tap_sum_thrd = 0;
    uint8 is_hss15g = 0;
    uint8 pattern = 0;
    uint32 delay = 0;
    bool pass = FALSE;
    sal_file_t p_file = NULL;
    sal_time_t tv;
    char* p_time_str = NULL;
    uint8 i = 0;
    int32 ret = CLI_SUCCESS;
    char* pattern_desc[PATTERN_NUM] = {"PRBS7+","PRBS7-","PRBS15+","PRBS15-","PRBS23+","PRBS23-","PRBS31+","PRBS31-", "PRBS9", "PRBS11"};
    ctc_dkit_serdes_ctl_para_t serdes_para_prbs;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;

    ctc_dkit_serdes_ctl_para_t* p_serdes_para = (ctc_dkit_serdes_ctl_para_t*)p_para;
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id);
    lchip = p_serdes_para->lchip;
    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    pattern = (uint8)p_serdes_para->para[1];
    if(pattern > 8)
    {
        pattern++;
    }
    tap_sum_thrd = p_serdes_para->para[2];
    delay = p_serdes_para->para[3];

    if (p_serdes_para->serdes_id == p_serdes_para->para[0])
    {
        CTC_DKIT_PRINT("Source serdes should not equal to dest serdes!!!\n");
        return CLI_ERROR;
    }

    if ((p_serdes_para->ffe.coefficient0_min > p_serdes_para->ffe.coefficient0_max)
        || (p_serdes_para->ffe.coefficient1_min > p_serdes_para->ffe.coefficient1_max)
        || (p_serdes_para->ffe.coefficient2_min > p_serdes_para->ffe.coefficient2_max)
        || (p_serdes_para->ffe.coefficient3_min > p_serdes_para->ffe.coefficient3_max)
        || (p_serdes_para->ffe.coefficient4_min > p_serdes_para->ffe.coefficient4_max))
    {
            CTC_DKIT_PRINT("Invalid ffe para!!!\n");
            return CLI_ERROR;
    }

    if(is_hss15g)
    {
        if ((p_serdes_para->ffe.coefficient0_max > 7)
        || (p_serdes_para->ffe.coefficient1_max >= TAP_MAX_VALUE)
        || (p_serdes_para->ffe.coefficient2_max > 31)
        || (p_serdes_para->ffe.coefficient3_max > 31))
        {
            CTC_DKIT_PRINT("Invalid ffe para!!!\n");
            return CLI_ERROR;
        }
    }
    else
    {
        if ((p_serdes_para->ffe.coefficient0_max >= TAP_MAX_VALUE)
        || (p_serdes_para->ffe.coefficient1_max >= TAP_MAX_VALUE)
        || (p_serdes_para->ffe.coefficient2_max >= TAP_MAX_VALUE)
        || (p_serdes_para->ffe.coefficient3_max >= TAP_MAX_VALUE)
        || (p_serdes_para->ffe.coefficient4_max >= TAP_MAX_VALUE))
        {
            CTC_DKIT_PRINT("Invalid ffe para!!!\n");
            return CLI_ERROR;
        }
    }

    if (pattern > (PATTERN_NUM-1))
    {
        CTC_DKIT_PRINT("PRBS pattern is invalid!!!\n");
        return CLI_ERROR;
    }

    if (0 != p_serdes_para->str[0])
    {
        p_file = sal_fopen(p_serdes_para->str, "wt");
        if (!p_file)
        {
            CTC_DKIT_PRINT("Open file %s fail!!!\n", p_serdes_para->str);
            return CLI_ERROR;
        }
    }

    /*get systime*/
    sal_time(&tv);
    p_time_str = sal_ctime(&tv);


    CTC_DKITS_PRINT_FILE(p_file, "Serdes %d %s FFE scan record, destination serdes %d, %s\n",
                       p_serdes_para->serdes_id, pattern_desc[p_serdes_para->para[1]], p_serdes_para->para[0], p_time_str);
    if (is_hss15g)/************************************ hss15g *******************************************/
    {
        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        /*for tap0*/
        for (tap0 = p_serdes_para->ffe.coefficient0_min; tap0 <= p_serdes_para->ffe.coefficient0_max; tap0++)
        {
            for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
            {
                for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
                {
                    for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
                    {
                        if (tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                        {
                            /*set ffe*/
                            ctc_dkit_serdes_wr.addr_offset = 0x33; /* C0    cfg_itx_ipdriver_base[2:0]   33h[2:0] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xf8) | tap0;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0x52; /* C1    cfg_ibias_tune_reserve[5:0]  52h[5:0] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0) | tap1;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0x03; /* C2    pcs_tap_adv[4:0]             03h[4:0] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap2;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0x04; /* C3    pcs_tap_dly[4:0]             04h[4:0] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap3;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            /*enable tx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                            serdes_para_prbs.para[1] = pattern;
                            _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                            /*check rx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                            serdes_para_prbs.para[1] = pattern;
                            serdes_para_prbs.para[3] = delay;
                            ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                            if (CLI_ERROR == ret)
                            {
                                CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Not Sync");
                            }
                            else if (pass)
                            {
                                CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Pass");
                                tap0_count[tap0]++;
                                tap1_count[tap1]++;
                                tap2_count[tap2]++;
                                tap3_count[tap3]++;
                            }
                            else
                            {
                                CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Fail");
                            }

                            /*disable tx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                            serdes_para_prbs.para[1] = pattern;
                            _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        }
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap0-------\n");
        tap0_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap0_count, 0);
        tap1_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap1_count, 1);
        tap2_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap2_count, 2);
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < 8; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i]);
        }
        for (; i < 32; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10d%-10d%-10d\n", i, "-", tap1_count[i], tap2_count[i], tap3_count[i]);
        }
        for (; i < 64; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10d%-10s%-10s\n", i, "-", tap1_count[i], "-", "-");
        }

        /*for tap1*/
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap1_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap2_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap3_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
        {
            for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
            {
                for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
                {
                    if (tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                    {
                        /*set ffe*/
                        ctc_dkit_serdes_wr.addr_offset = 0x33; /* C0    cfg_itx_ipdriver_base[2:0]   33h[2:0] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xf8) | tap0;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x52; /* C1    cfg_ibias_tune_reserve[5:0]  52h[5:0] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0) | tap1;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x03; /* C2    pcs_tap_adv[4:0]             03h[4:0] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap2;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x04; /* C3    pcs_tap_dly[4:0]             04h[4:0] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap3;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        /*enable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                        /*check rx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                        serdes_para_prbs.para[1] = pattern;
                        serdes_para_prbs.para[3] = delay;
                        ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        if (CLI_ERROR == ret)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Not Sync");
                        }
                        else if (pass)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Pass");
                            tap1_count[tap1]++;
                            tap2_count[tap2]++;
                            tap3_count[tap3]++;
                        }
                        else
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Fail");
                        }

                        /*disable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap1-------\n");
        tap1_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap1_count, 1);
        tap2_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap2_count, 2);
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < 32; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10d%-10d%-10d\n", i, "-", tap1_count[i], tap2_count[i], tap3_count[i]);
        }
        for (; i < 64; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10d%-10s%-10s\n", i, "-", tap1_count[i], "-", "-");
        }

        /*for tap2*/
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap2_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap3_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
        {
            for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
            {
                if (tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                {
                    /*set ffe*/
                    ctc_dkit_serdes_wr.addr_offset = 0x33; /* C0    cfg_itx_ipdriver_base[2:0]   33h[2:0] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xf8) | tap0;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x52; /* C1    cfg_ibias_tune_reserve[5:0]  52h[5:0] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0) | tap1;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x03; /* C2    pcs_tap_adv[4:0]             03h[4:0] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap2;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x04; /* C3    pcs_tap_dly[4:0]             04h[4:0] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap3;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    /*enable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                    /*check rx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                    serdes_para_prbs.para[1] = pattern;
                    serdes_para_prbs.para[3] = delay;
                    ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    if (CLI_ERROR == ret)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Not Sync");
                    }
                    else if (pass)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Pass");
                        tap2_count[tap2]++;
                        tap3_count[tap3]++;
                    }
                    else
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Fail");
                    }

                    /*disable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap2-------\n");
        tap2_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap2_count, 2);
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < 32; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10s%-10d%-10d\n", i, "-", "-", tap2_count[i], tap3_count[i]);
        }

        /*for tap3*/
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap3_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        tap2 = tap2_best;
        for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
        {
            if (tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
            {
                /*set ffe*/
                ctc_dkit_serdes_wr.addr_offset = 0x33; /* C0    cfg_itx_ipdriver_base[2:0]   33h[2:0] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xf8) | tap0;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x52; /* C1    cfg_ibias_tune_reserve[5:0]  52h[5:0] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0) | tap1;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x03; /* C2    pcs_tap_adv[4:0]             03h[4:0] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap2;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x04; /* C3    pcs_tap_dly[4:0]             04h[4:0] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xe0) | tap3;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                /*enable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                /*check rx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                serdes_para_prbs.para[1] = pattern;
                serdes_para_prbs.para[3] = delay;
                ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                if (CLI_ERROR == ret)
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Not Sync");
                }
                else if (pass)
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Pass");
                    tap3_count[tap3]++;
                }
                else
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Fail");
                }

                /*disable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap3-------\n");
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_12g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < 32; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10s%-10s%-10d\n", i, "-", "-", "-", tap3_count[i]);
        }

        CTC_DKITS_PRINT_FILE(p_file, "\n--------------Scan result-------------\n");
        CTC_DKIT_PRINT( "C0: %5d\n", tap0_best);
        CTC_DKIT_PRINT( "C1: %5d\n", tap1_best);
        CTC_DKIT_PRINT( "C2: %5d\n", tap2_best);
        CTC_DKIT_PRINT( "C3: %5d\n", tap3_best);
    }
    else /************************************ hss28g *******************************************/
    {
        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "C4", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        /*for tap0*/
        for (tap0 = p_serdes_para->ffe.coefficient0_min; tap0 <= p_serdes_para->ffe.coefficient0_max; tap0++)
        {
            for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
            {
                for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
                {
                    for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
                    {
                        for (tap4 = p_serdes_para->ffe.coefficient4_min; tap4 <= p_serdes_para->ffe.coefficient4_max; tap4++)
                        {
                            if (tap0 + tap1 + tap2 + tap3 + tap4 <= tap_sum_thrd)
                            {
                                /*set ffe*/
                                ctc_dkit_serdes_wr.addr_offset = 0xa7; /* C0    TX_FIR_MAIN        8za7h[13:8] */
                                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap0;
                                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                                ctc_dkit_serdes_wr.addr_offset = 0xa8; /* C1    TX_FIR_PRECURSOR   8za8h[13:8] */
                                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap1;
                                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                                ctc_dkit_serdes_wr.addr_offset = 0xa6; /* C2    TX_FIR_POST_1      8za6h[13:8] */
                                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap2;
                                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                                ctc_dkit_serdes_wr.addr_offset = 0xa5; /* C3    TX_FIR_POST_2      8za5h[13:8] */
                                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap3;
                                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                                ctc_dkit_serdes_wr.addr_offset = 0xa4; /* C4    TX_FIR_POST_3      8za4h[13:8] */
                                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap4;
                                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                                /*enable tx*/
                                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                                serdes_para_prbs.para[1] = pattern;
                                _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                                /*check rx*/
                                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                                serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                                serdes_para_prbs.para[1] = pattern;
                                serdes_para_prbs.para[3] = delay;
                                ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                                if (CLI_ERROR == ret)
                                {
                                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Not Sync");
                                }
                                else if (pass)
                                {
                                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Pass");
                                    tap0_count[tap0]++;
                                    tap1_count[tap1]++;
                                    tap2_count[tap2]++;
                                    tap3_count[tap3]++;
                                    tap4_count[tap4]++;
                                }
                                else
                                {
                                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Fail");
                                }

                                /*disable tx*/
                                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                                serdes_para_prbs.para[1] = pattern;
                                _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                            }
                        }
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap0-------\n");
        tap0_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap0_count, 0);
        tap1_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap1_count, 1);
        tap2_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap2_count, 2);
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        tap4_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap4_count, 4);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C4: %5d, count: %d\n", tap4_best, tap4_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3", "C4");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i], tap4_count[i]);
        }

        /*for tap1*/
        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "C4", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap1_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap2_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap3_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap4_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
        {
            for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
            {
                for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
                {
                    for (tap4 = p_serdes_para->ffe.coefficient4_min; tap4 <= p_serdes_para->ffe.coefficient4_max; tap4++)
                    {
                        if (tap0 + tap1 + tap2 + tap3 + tap4 <= tap_sum_thrd)
                        {
                            /*set ffe*/
                            ctc_dkit_serdes_wr.addr_offset = 0xa7; /* C0    TX_FIR_MAIN        8za7h[13:8] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap0;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0xa8; /* C1    TX_FIR_PRECURSOR   8za8h[13:8] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap1;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0xa6; /* C2    TX_FIR_POST_1      8za6h[13:8] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap2;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0xa5; /* C3    TX_FIR_POST_2      8za5h[13:8] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap3;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0xa4; /* C4    TX_FIR_POST_3      8za4h[13:8] */
                            ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                            ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap4;
                            ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            /*enable tx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                            serdes_para_prbs.para[1] = pattern;
                            _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                            /*check rx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                            serdes_para_prbs.para[1] = pattern;
                            serdes_para_prbs.para[3] = delay;
                            ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                            if (CLI_ERROR == ret)
                            {
                                CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Not Sync");
                            }
                            else if (pass)
                            {
                                CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Pass");
                                tap1_count[tap1]++;
                                tap2_count[tap2]++;
                                tap3_count[tap3]++;
                                tap4_count[tap4]++;
                            }
                            else
                            {
                                CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Fail");
                            }

                            /*disable tx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                            serdes_para_prbs.para[1] = pattern;
                            _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        }
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap1-------\n");
        tap1_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap1_count, 1);
        tap2_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap2_count, 2);
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        tap4_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap4_count, 4);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C4: %5d, count: %d\n", tap4_best, tap4_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3", "C4");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i], tap4_count[i]);
        }

        /*for tap2*/
        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "C4", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap2_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap3_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap4_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
        {
            for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
            {
                for (tap4 = p_serdes_para->ffe.coefficient4_min; tap4 <= p_serdes_para->ffe.coefficient4_max; tap4++)
                {
                    if (tap0 + tap1 + tap2 + tap3 + tap4 <= tap_sum_thrd)
                    {
                        /*set ffe*/
                        ctc_dkit_serdes_wr.addr_offset = 0xa7; /* C0    TX_FIR_MAIN        8za7h[13:8] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap0;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0xa8; /* C1    TX_FIR_PRECURSOR   8za8h[13:8] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap1;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0xa6; /* C2    TX_FIR_POST_1      8za6h[13:8] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap2;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0xa5; /* C3    TX_FIR_POST_2      8za5h[13:8] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap3;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0xa4; /* C4    TX_FIR_POST_3      8za4h[13:8] */
                        ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap4;
                        ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        /*enable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                        /*check rx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                        serdes_para_prbs.para[1] = pattern;
                        serdes_para_prbs.para[3] = delay;
                        ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        if (CLI_ERROR == ret)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Not Sync");
                        }
                        else if (pass)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Pass");
                            tap1_count[tap1]++;
                            tap2_count[tap2]++;
                            tap3_count[tap3]++;
                            tap4_count[tap4]++;
                        }
                        else
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Fail");
                        }

                        /*disable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap2-------\n");
        tap2_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap2_count, 2);
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        tap4_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap4_count, 4);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C4: %5d, count: %d\n", tap4_best, tap4_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3", "C4");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i], tap4_count[i]);
        }

        /*for tap3*/
        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "C4", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap3_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        sal_memset(tap4_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        tap2 = tap2_best;
        for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
        {
            for (tap4 = p_serdes_para->ffe.coefficient4_min; tap4 <= p_serdes_para->ffe.coefficient4_max; tap4++)
            {
                if (tap0 + tap1 + tap2 + tap3 + tap4 <= tap_sum_thrd)
                {
                    /*set ffe*/
                    ctc_dkit_serdes_wr.addr_offset = 0xa7; /* C0    TX_FIR_MAIN        8za7h[13:8] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap0;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0xa8; /* C1    TX_FIR_PRECURSOR   8za8h[13:8] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap1;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0xa6; /* C2    TX_FIR_POST_1      8za6h[13:8] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap2;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0xa5; /* C3    TX_FIR_POST_2      8za5h[13:8] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap3;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0xa4; /* C4    TX_FIR_POST_3      8za4h[13:8] */
                    ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap4;
                    ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    /*enable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                    /*check rx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                    serdes_para_prbs.para[1] = pattern;
                    serdes_para_prbs.para[3] = delay;
                    ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    if (CLI_ERROR == ret)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Not Sync");
                    }
                    else if (pass)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Pass");
                        tap1_count[tap1]++;
                        tap2_count[tap2]++;
                        tap3_count[tap3]++;
                        tap4_count[tap4]++;
                    }
                    else
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Fail");
                    }

                    /*disable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap3-------\n");
        tap3_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        tap4_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap4_count, 4);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C4: %5d, count: %d\n", tap4_best, tap4_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3", "C4");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i], tap4_count[i]);
        }

        /*for tap4*/
        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "C4", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap4_count, 0, sizeof(uint16)*TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        tap2 = tap2_best;
        tap3 = tap3_best;
        for (tap4 = p_serdes_para->ffe.coefficient4_min; tap4 <= p_serdes_para->ffe.coefficient4_max; tap4++)
        {
            if (tap0 + tap1 + tap2 + tap3 + tap4 <= tap_sum_thrd)
            {
                /*set ffe*/
                ctc_dkit_serdes_wr.addr_offset = 0xa7; /* C0    TX_FIR_MAIN        8za7h[13:8] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap0;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0xa8; /* C1    TX_FIR_PRECURSOR   8za8h[13:8] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap1;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0xa6; /* C2    TX_FIR_POST_1      8za6h[13:8] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap2;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0xa5; /* C3    TX_FIR_POST_2      8za5h[13:8] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap3;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0xa4; /* C4    TX_FIR_POST_3      8za4h[13:8] */
                ctc_tm_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.data = (ctc_dkit_serdes_wr.data & 0xc0ff) | tap4;
                ctc_tm_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                /*enable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                /*check rx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                serdes_para_prbs.para[1] = pattern;
                serdes_para_prbs.para[3] = delay;
                ret = _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                if (CLI_ERROR == ret)
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Not Sync");
                }
                else if (pass)
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Pass");
                    tap1_count[tap1]++;
                    tap2_count[tap2]++;
                    tap3_count[tap3]++;
                    tap4_count[tap4]++;
                }
                else
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, tap4, "Fail");
                }

                /*disable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_tm_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap4-------\n");
        tap4_best = _ctc_tm_dkit_misc_serdes_ffe_get_28g_best_tap(tap4_count, 4);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %5d, count: %d\n", tap3_best, tap3_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C4: %5d, count: %d\n", tap4_best, tap4_count[tap3_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2", "C3", "C4");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i], tap4_count[i]);
        }

        CTC_DKITS_PRINT_FILE(p_file, "\n--------------Scan result-------------\n");
        CTC_DKIT_PRINT( "C0: %5d\n", tap0_best);
        CTC_DKIT_PRINT( "C1: %5d\n", tap1_best);
        CTC_DKIT_PRINT( "C2: %5d\n", tap2_best);
        CTC_DKIT_PRINT( "C3: %5d\n", tap3_best);
        CTC_DKIT_PRINT( "C4: %5d\n", tap4_best);
    }

    return ret;
}

int32
ctc_tm_dkit_misc_serdes_ctl(void* p_para)
{
    ctc_dkit_serdes_ctl_para_t* p_serdes_para = (ctc_dkit_serdes_ctl_para_t*)p_para;

    DKITS_PTR_VALID_CHECK(p_serdes_para);
    CTC_DKIT_LCHIP_CHECK(p_serdes_para->lchip);
    DRV_INIT_CHECK(p_serdes_para->lchip);

    switch (p_serdes_para->type)
    {
        case CTC_DKIT_SERDIS_CTL_LOOPBACK:
            _ctc_tm_dkit_misc_serdes_loopback(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_PRBS:
            _ctc_tm_dkit_misc_serdes_prbs(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_EYE:
            _ctc_tm_dkit_misc_serdes_eye(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_FFE:
            _ctc_tm_dkit_misc_serdes_ffe_scan(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_STATUS:
            _ctc_tm_dkit_misc_serdes_status(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_POLR_CHECK:
            _ctc_tm_dkit_misc_serdes_polr_check(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_DFE:
            _ctc_tm_dkit_misc_serdes_dfe_set_en(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_GET_DFE:
            _ctc_tm_dkit_misc_serdes_dfe_get_val(p_serdes_para);
            break;
        default:
            break;
    }

    return CLI_SUCCESS;
}
