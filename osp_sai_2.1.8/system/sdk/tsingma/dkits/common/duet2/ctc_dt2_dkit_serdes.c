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
#include "ctc_dt2_dkit_serdes.h"
#include "ctc_usw_dkit_dump_cfg.h"

#if 1
/*Hss15G internal lane: A, E, F, B, C, G, H, D*/
uint8 g_dt2_dkits_hss15g_lane_map[CTC_DKIT_SERDES_HSS15G_LANE_NUM] = {0, 4, 5, 1, 2, 6, 7, 3};
/*for hss28g , E,F,G,H is not used */
uint8 g_dt2_dkits_hss_tx_addr_map[CTC_DKIT_SERDES_HSS15G_LANE_NUM] =
{
    DRV_HSS_TX_LINKA_ADDR, DRV_HSS_TX_LINKB_ADDR, DRV_HSS_TX_LINKC_ADDR, DRV_HSS_TX_LINKD_ADDR,
    DRV_HSS_TX_LINKE_ADDR, DRV_HSS_TX_LINKF_ADDR, DRV_HSS_TX_LINKG_ADDR, DRV_HSS_TX_LINKH_ADDR
};

/*for hss28g , E,F,G,H is not used */
uint8 g_dt2_dkits_hss_rx_addr_map[CTC_DKIT_SERDES_HSS15G_LANE_NUM] =
{
    DRV_HSS_RX_LINKA_ADDR, DRV_HSS_RX_LINKB_ADDR, DRV_HSS_RX_LINKC_ADDR, DRV_HSS_RX_LINKD_ADDR,
    DRV_HSS_RX_LINKE_ADDR, DRV_HSS_RX_LINKF_ADDR, DRV_HSS_RX_LINKG_ADDR, DRV_HSS_RX_LINKH_ADDR
};


#define DKIT_SERDES_ID_MAX 40

#define DKITS_SERDES_ID_CHECK(ID) \
    do { \
        if(ID >= DKIT_SERDES_ID_MAX) \
        {\
            CTC_DKIT_PRINT("serdes id %d exceed max id %d!!!\n", ID, DKIT_SERDES_ID_MAX-1);\
            return DKIT_E_INVALID_PARAM; \
        }\
    } while (0)

#define DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr) \
    do { \
        ctc_dkit_serdes_wr.addr_offset = 0x02; /*Transmit Coefficient Control Register*/\
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);\
        DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 4);\
        DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 0);\
        DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 7);/*sum of coef mode if set 0*/\
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);\
    } while (0)
#endif

#define TAP_MAX_VALUE 64

#define PATTERN_NUM 10

static uint16 serdes_data[64];
STATIC int32
_ctc_dt2_dkit_internal_convert_serdes_addr(ctc_dkit_serdes_wr_t* p_para, uint32* addr, uint8* hss_id)
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
        *hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_para->serdes_id);
        lane_id = g_dt2_dkits_hss15g_lane_map[lane_id];
    }
    else
    {
        *hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_para->serdes_id);
    }

    if (CTC_DKIT_SERDES_TX == p_para->type)
    {
        *addr = g_dt2_dkits_hss_tx_addr_map[lane_id];
    }
    else if (CTC_DKIT_SERDES_RX == p_para->type)
    {
        *addr = g_dt2_dkits_hss_rx_addr_map[lane_id];
    }
    else if (CTC_DKIT_SERDES_PLLA == p_para->type)
    {
        *addr = DRV_HSS_PLLA_ADDR;
    }
    else if (CTC_DKIT_SERDES_PLLB == p_para->type)
    {
        *addr = DRV_HSS_PLLB_ADDR;
    }
    else if (CTC_DKIT_SERDES_COMMON == p_para->type)
    {
        *addr = DRV_HSS_COMMON_ADDR;
    }


    *addr = (*addr << 6) + p_para->addr_offset;

    CTC_DKIT_PRINT_DEBUG("hssid:%d, lane:%d\n", *hss_id, lane_id);

    return CLI_SUCCESS;
}

int32
ctc_dt2_dkit_misc_read_serdes(void* para)
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
    _ctc_dt2_dkit_internal_convert_serdes_addr(p_para, &addr, &hss_id);

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
ctc_dt2_dkit_misc_write_serdes(void* para)
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
    _ctc_dt2_dkit_internal_convert_serdes_addr(p_para, &addr, &hss_id);

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


int32
ctc_dt2_dkit_misc_read_serdes_aec_aet(void* para)
{
    uint32 addr = 0;
    uint8 is_hss15g = 0;
    uint8 hss_id = 0;
    uint8 is_aet = 0;
    uint8 lane_id = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    DKITS_PTR_VALID_CHECK(p_para);
    DKITS_SERDES_ID_CHECK(p_para->serdes_id);
    CTC_DKIT_LCHIP_CHECK(p_para->lchip);
    lchip = p_para->lchip;

    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_para->serdes_id);
    addr = p_para->addr_offset;
    is_aet = (p_para->type == CTC_DKIT_SERDES_AET);

    lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_para->serdes_id);

    if (is_hss15g)
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
        drv_chip_read_hss15g_aec_aet(p_para->lchip, hss_id, lane_id, is_aet,  addr, &p_para->data);
    }
    else
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
        drv_chip_read_hss28g_aec_aet(p_para->lchip, hss_id, lane_id, is_aet,  addr, &p_para->data);
    }
    CTC_DKIT_PRINT_DEBUG("read  chip id:%d, hss%s id:%d, lane id: %d, addr: 0x%04x, value: 0x%04x\n", p_para->lchip, is_hss15g?"15g":"28g", hss_id, lane_id, addr, p_para->data);
    return CLI_SUCCESS;
}

int32
ctc_dt2_dkit_misc_write_serdes_aec_aet(void* para)
{
    uint32 addr = 0;
    uint8 is_hss15g = 0;
    uint8 hss_id = 0;
    uint8 is_aet = 0;
    uint8 lane_id = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_wr_t* p_para = (ctc_dkit_serdes_wr_t*)para;
    DKITS_PTR_VALID_CHECK(p_para);
    DKITS_SERDES_ID_CHECK(p_para->serdes_id);
    CTC_DKIT_LCHIP_CHECK(p_para->lchip);
    lchip = p_para->lchip;
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_para->serdes_id);
    addr = p_para->addr_offset;
    is_aet = (p_para->type == CTC_DKIT_SERDES_AET);

    lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(p_para->serdes_id);

    if (is_hss15g)
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
        drv_chip_write_hss15g_aec_aet(p_para->lchip, hss_id, lane_id, is_aet,  addr, p_para->data);
    }
    else
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(p_para->serdes_id);
        drv_chip_write_hss28g_aec_aet(p_para->lchip, hss_id, lane_id, is_aet,  addr, p_para->data);
    }
    CTC_DKIT_PRINT_DEBUG("Write chip id:%d, hss%s id:%d, lane id: %d, addr: 0x%04x, value: 0x%04x\n", p_para->lchip, is_hss15g?"15g":"28g", hss_id, lane_id, addr, p_para->data);
    return CLI_SUCCESS;
}


int32
ctc_dt2_dkit_misc_serdes_resert(uint8 lchip, uint16 serdes_id)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    DKITS_SERDES_ID_CHECK(serdes_id );
    CTC_DKIT_LCHIP_CHECK(lchip);

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_wr.lchip = lchip;
    ctc_dkit_serdes_wr.serdes_id = serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
    ctc_dkit_serdes_wr.addr_offset = 0;/*Receiver Configuration Mode Register*/

    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 6);/*LINKRST = 1*/
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 6);/*LINKRST = 0*/
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    return CLI_SUCCESS;
}


STATIC int32
ctc_dt2_dkit_misc_serdes_status_15g(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    sal_time_t tv;
    char* p_time_str = NULL;
    sal_file_t p_file = NULL;
    uint8 i = 0;

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
    if((CTC_DKIT_SERDES_RX == type)||(CTC_DKIT_SERDES_ALL == type))
    {
        /*RX*/
        CTC_DKITS_PRINT_FILE(p_file, "# RX Registers, 0x00-0x3F\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
        for (i = 0; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
    }

    if ((CTC_DKIT_SERDES_TX == type) || (CTC_DKIT_SERDES_ALL == type))
    {
        /*TX*/
        CTC_DKITS_PRINT_FILE(p_file, "# TX Registers, 0x00-0x3F\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        for (i = 0; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
    }

    if ((CTC_DKIT_SERDES_COMMON_PLL == type) || (CTC_DKIT_SERDES_ALL == type))
    {
        /*Common*/
        CTC_DKITS_PRINT_FILE(p_file, "# COMMON Registers, 0x00-0x23\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_COMMON;
        for (i = 0; i <= 0x23; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        /*PLLA*/
        CTC_DKITS_PRINT_FILE(p_file, "# PLL A Registers, 0x00-0x0a\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLA;
        for (i = 0; i <= 0x0a; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        /*PLLB*/
        CTC_DKITS_PRINT_FILE(p_file, "# PLL B Registers, 0x00-0x0a\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLB;
        for (i = 0; i <= 0x0a; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
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
ctc_dt2_dkit_misc_serdes_status_28g(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    sal_time_t tv;
    char* p_time_str = NULL;
    sal_file_t p_file = NULL;
    uint8 i = 0;
    uint16 val1 = 0;
    uint16 val2 = 0;

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

    if((CTC_DKIT_SERDES_RX == type)||(CTC_DKIT_SERDES_ALL == type))
    {
        /*RX*/
        CTC_DKITS_PRINT_FILE(p_file, "Serdes %d RX Regs Status, %s", serdes_id, p_time_str);

        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;

        for (i = 0; i <= 0x11; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 2; i <= 3; i++)
        {
            ctc_dkit_serdes_wr.data = i;
            ctc_dkit_serdes_wr.addr_offset = 0x11;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x12;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0x13; i <= 0x17; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 20; i++)
        {
            ctc_dkit_serdes_wr.data = i;
            ctc_dkit_serdes_wr.addr_offset = 0x17;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x18;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0x19; i <= 0x21; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 26; i++)
        {
            ctc_dkit_serdes_wr.data = i + (0 << 5);
            ctc_dkit_serdes_wr.addr_offset = 0x21;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x22;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            val1 = ctc_dkit_serdes_wr.data;

            ctc_dkit_serdes_wr.data = i + (1 << 5);
            ctc_dkit_serdes_wr.addr_offset = 0x21;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x22;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            val2 = ctc_dkit_serdes_wr.data;


            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, val1, val2);
        }

        for (i = 0x23; i <= 0x24; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 15; i++)
        {
            ctc_dkit_serdes_wr.data = i << 11;
            ctc_dkit_serdes_wr.addr_offset = 0x25;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x25;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0x26; i <= 0x30; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 11; i++)
        {
            ctc_dkit_serdes_wr.data = i;
            ctc_dkit_serdes_wr.addr_offset = 0x30;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x31;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0x32; i <= 0x39; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 3; i++)
        {
            ctc_dkit_serdes_wr.data = (1 << 6) + (i << 3);
            ctc_dkit_serdes_wr.addr_offset = 0x3A;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            ctc_dkit_serdes_wr.addr_offset = 0x3A;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 4; i++)
        {
            ctc_dkit_serdes_wr.data = i << 10;
            ctc_dkit_serdes_wr.addr_offset = 0x21;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dkit_serdes_wr.addr_offset = 0x3B;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 3; i++)
        {
            ctc_dkit_serdes_wr.data = i << 8;
            ctc_dkit_serdes_wr.addr_offset = 0x21;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dkit_serdes_wr.addr_offset = 0x3C;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0; i <= 1; i++)
        {
            ctc_dkit_serdes_wr.data = i << 13;
            ctc_dkit_serdes_wr.addr_offset = 0x21;
            CTC_DKITS_PRINT_FILE(p_file, "       Write RX 0x%02x: 0x%04x", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
            ctc_dkit_serdes_wr.addr_offset = 0x3D;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        for (i = 0x3E; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read RX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

    }

    if ((CTC_DKIT_SERDES_TX == type) || (CTC_DKIT_SERDES_ALL == type))
    {
        /*TX*/
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;

        for (i = 0; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read TX 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
    }

    if ((CTC_DKIT_SERDES_COMMON_PLL == type) || (CTC_DKIT_SERDES_ALL == type))
    {

        /*Common*/
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_COMMON;
        for (i = 0; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read Common 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        /*PLLA*/
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLA;
        for (i = 0; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read PLLA 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

        /*PLLB*/
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLB;
        for (i = 0; i <= 0x3F; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read PLLB 0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
    }


    if ((CTC_DKIT_SERDES_AEC_AET == type) || (CTC_DKIT_SERDES_ALL == type))
    {
        CTC_DKITS_PRINT_FILE(p_file, "Serdes %d AEC and AET Regs Status, %s", serdes_id, p_time_str);
        /*AEC*/
        CTC_DKITS_PRINT_FILE(p_file, "Step1: Read AEC Registers\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_AEC;
        for (i = 0x00; i <= 0x07; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }
        /*AET*/
        CTC_DKITS_PRINT_FILE(p_file, "Step1: Read AET Registers\n");
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_AET;
        for (i = 0x00; i <= 0x06; i++)
        {
            ctc_dkit_serdes_wr.addr_offset = i;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            CTC_DKITS_PRINT_FILE(p_file, "       Read  0x%02x: 0x%04x\n", ctc_dkit_serdes_wr.addr_offset, ctc_dkit_serdes_wr.data);
        }

    }

    if (p_file)
    {
        sal_fclose(p_file);
        CTC_DKIT_PRINT("Save result to %s\n", file_name);
    }

    return CLI_SUCCESS;
}


int32
ctc_dt2_dkit_misc_serdes_status(uint8 lchip, uint16 serdes_id, uint32 type, char* file_name)
{
    uint8 is_hss15g = 0;
    DKITS_SERDES_ID_CHECK(serdes_id);
    CTC_DKIT_LCHIP_CHECK(lchip);
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(serdes_id);

    if (is_hss15g)
    {
        ctc_dt2_dkit_misc_serdes_status_15g(lchip, serdes_id, type, file_name);
    }
    else
    {
        ctc_dt2_dkit_misc_serdes_status_28g(lchip, serdes_id, type, file_name);
    }
    
    return 0;
}


int32
ctc_dt2_dkits_misc_dump_indirect(uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag)
{
    int32  ret = CLI_SUCCESS;
    uint32 c[4] = {0}, h[10] = {0}, id = 0;
    uint8  serdes_id = 0, addr_offset = 0;
    ctc_dkit_serdes_mode_t serdes_mode = 0;
    ctc_dkit_serdes_wr_t serdes_para;
    ctc_dkits_dump_serdes_tbl_t serdes_tbl;

    for (serdes_id = 0; serdes_id < DKIT_SERDES_ID_MAX; serdes_id++)
    {
        sal_udelay(10);
        for (serdes_mode = CTC_DKIT_SERDES_TX; serdes_mode <= CTC_DKIT_SERDES_PLLB; serdes_mode++)
        {
            sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_wr_t));

            for (addr_offset = 0; addr_offset < 0x40; addr_offset++)
            {
                serdes_para.lchip = lchip;
                serdes_para.serdes_id = serdes_id;
                serdes_para.type = serdes_mode;
                serdes_para.addr_offset = addr_offset;
                ret = ret ? ret : ctc_dt2_dkit_misc_read_serdes(&serdes_para);
                serdes_tbl.serdes_id = serdes_id;
                serdes_tbl.serdes_mode = serdes_mode;
                serdes_tbl.offset = addr_offset;
                serdes_tbl.data = serdes_para.data;

                if (CTC_DKIT_SERDES_IS_HSS28G(serdes_id))
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, CsMacroPrtReq0_t + id, c[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
                else
                {
                    id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
                    ret = ret ? ret : ctc_usw_dkits_dump_tbl2data(lchip, HsMacroPrtReq0_t + id, h[id]++, (uint32*)&serdes_tbl, NULL, p_f);
                }
            }
        }
    }

    return ret;
}

STATIC int32
_ctc_dt2_dkit_misc_serdes_loopback(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint8 enable = 0;
    uint8 lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    CTC_DKIT_LCHIP_CHECK(lchip);
    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
    enable = (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]);

    ctc_dkit_serdes_wr.lchip = lchip;
    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
    ctc_dkit_serdes_wr.addr_offset = 0x3F; /*Transmit Macro Test Control Register 1*/
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 1);/*TXOE*/
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    if (CTC_DKIT_SERDIS_LOOPBACK_INTERNAL == p_serdes_para->para[1])
    {
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
        ctc_dkit_serdes_wr.addr_offset = 1; /*Receiver Test Control Register*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        if (enable)
        {
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 6);/*WPLPEN*/
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 5);/*WRPMD*/
        }
        else
        {
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 6);/*WPLPEN*/
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 5);/*WRPMD*/
        }
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    }
    else
    {
        sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
        ctc_dkit_serdes_wr.lchip = lchip;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.addr_offset = 0; /*Transmit Configuration Mode Register*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        if (enable)
        {
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 8);/*RXLOOP*/
        }
        else
        {
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 8);/*RXLOOP*/
        }
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    }

    return CLI_SUCCESS;
}


STATIC int32
_ctc_dt2_dkit_misc_serdes_reset(uint8 lchip, uint16 serdes_id, bool reset)
{
    uint8 lane_id = 0;
    uint8 is_hss15g = 0;
    uint8 hss_id = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    char tbl_name[CTC_DKITS_RW_BUF_SIZE] = {0};
    char fld_name [CTC_DKITS_RW_BUF_SIZE] = {0};
    uint32 field_id_15g[8] =
    {
            HsCfg0_cfgHss15gRxAPrbsRst_f,
            HsCfg0_cfgHss15gRxBPrbsRst_f,
            HsCfg0_cfgHss15gRxCPrbsRst_f,
            HsCfg0_cfgHss15gRxDPrbsRst_f,
            HsCfg0_cfgHss15gRxEPrbsRst_f,
            HsCfg0_cfgHss15gRxFPrbsRst_f,
            HsCfg0_cfgHss15gRxGPrbsRst_f,
            HsCfg0_cfgHss15gRxHPrbsRst_f
    };

    uint32 field_id_28g[4] =
    {
        CsCfg0_cfgHss28gRxAPrbsRst_f,
        CsCfg0_cfgHss28gRxBPrbsRst_f,
        CsCfg0_cfgHss28gRxCPrbsRst_f,
        CsCfg0_cfgHss28gRxDPrbsRst_f
    };

    CTC_DKIT_LCHIP_CHECK(lchip);
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(serdes_id);
    if (is_hss15g)
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id);
        tbl_id = HsCfg0_t + hss_id;
        fld_id = field_id_15g[lane_id];
    }
    else
    {
        hss_id = CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id);
        lane_id = CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id);
        tbl_id = CsCfg0_t + hss_id;
        fld_id = field_id_28g[lane_id];
    }

    if (reset)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    cmd = DRV_IOW(tbl_id, fld_id);
    DRV_IOCTL(lchip, 0, cmd, &value);

    drv_usw_get_tbl_string_by_id(lchip, tbl_id , tbl_name);
    drv_usw_get_field_string_by_id(lchip, tbl_id, fld_id, fld_name);

    CTC_DKIT_PRINT_DEBUG("write table %s, field %s, value: %d\n", tbl_name, fld_name, value);

    return CLI_SUCCESS;
}


STATIC int32
_ctc_dt2_dkit_misc_do_serdes_prbs(ctc_dkit_serdes_ctl_para_t* p_serdes_para, bool* pass)
{
#define TIMEOUT 20
    uint32 count = 0;
    uint8 is_keep = 0;
    uint8 time_out = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    uint32 delay = 0;
    uint8 lchip = p_serdes_para->lchip;
    uint16 temp = 0;

    CTC_DKIT_LCHIP_CHECK(lchip);
    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_wr.lchip = lchip;
    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

    if ((CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
           ||(CTC_DKIT_SERDIS_CTL_DISABLE == p_serdes_para->para[0]))
    {
        /* check prbs enable status(bit3), if prbs enable, need disable first */
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.addr_offset = 1; /*Transmit Test Control Register*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        temp = ctc_dkit_serdes_wr.data & 0x0008;
        if(temp && (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0]))
        {
            return CLI_EOL;  /* prbs is enable, need disable first */
        }

         /**/
        if (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
        {
            _ctc_dt2_dkit_misc_serdes_reset(lchip, p_serdes_para->serdes_id, TRUE);
        }
        else
        {
            _ctc_dt2_dkit_misc_serdes_reset(lchip, p_serdes_para->serdes_id, FALSE);
        }


        if (9 == p_serdes_para->para[1])/*Squareware(8081)*/
        {
            if (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
            {
                ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                ctc_dkit_serdes_wr.addr_offset = 0x00;
                ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                serdes_data[p_serdes_para->serdes_id] = ctc_dkit_serdes_wr.data;

                SET_BIT_RANGE(ctc_dkit_serdes_wr.data, 2, 2, 2)/*BWSEL = 00*/
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x20;
                ctc_dkit_serdes_wr.data= 0xff00;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
                ctc_dkit_serdes_wr.addr_offset = 0x21;
                ctc_dkit_serdes_wr.data= 0xff00;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
                if (CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
                {
                    ctc_dkit_serdes_wr.addr_offset = 0x22;
                    ctc_dkit_serdes_wr.data = 0xff00;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
                    ctc_dkit_serdes_wr.addr_offset = 0x23;
                    ctc_dkit_serdes_wr.data = 0xff00;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
                }
            }
            else
            {
                ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                ctc_dkit_serdes_wr.addr_offset = 0x00;
                ctc_dkit_serdes_wr.data = serdes_data[p_serdes_para->serdes_id];
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
            }
        }


        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.addr_offset = 1; /*Transmit Test Control Register*/

        /*1. config*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);

        if (8 == p_serdes_para->para[1]) /*PRBS9*/
        {
            SET_BIT_RANGE(ctc_dkit_serdes_wr.data, 2, 11, 3)/*SPSEL = 010*/
        }
        else if (9 == p_serdes_para->para[1])/*Squareware(8081)*/
        {
            if (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
            {
                if (CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id))
                {
                    SET_BIT_RANGE(ctc_dkit_serdes_wr.data, 6, 11, 3)/*SPSEL = 110*/
                }
                else
                {
                    SET_BIT_RANGE(ctc_dkit_serdes_wr.data, 5, 11, 3)/*SPSEL = 101*/
                }
            }
            else
            {
                ctc_dkit_serdes_wr.data = 0;
            }
        }
        else
        {
            SET_BIT_RANGE(ctc_dkit_serdes_wr.data, 0, 11, 3)/*SPSEL = 000*/
            SET_BIT_RANGE(ctc_dkit_serdes_wr.data, p_serdes_para->para[1], 0, 3)/*TPSEL*/
        }

        if (CTC_DKIT_SERDIS_CTL_ENABLE == p_serdes_para->para[0])
        {
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 3);/*TPGMD*/
        }
        else
        {
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 3);/*TPGMD*/
        }
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        /*2. reset*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 4);/*PRST = 1*/
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 4);/*PRST = 0*/
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    }
    else if (CTC_DKIT_SERDIS_CTL_CEHCK == p_serdes_para->para[0])
    {
        is_keep = p_serdes_para->para[2];
        delay = p_serdes_para->para[3];
        if (delay)
        {
            sal_task_sleep(delay); /*delay before check*/
        }

        _ctc_dt2_dkit_misc_serdes_reset(lchip, p_serdes_para->serdes_id, TRUE);

        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
        ctc_dkit_serdes_wr.addr_offset = 1; /*Receiver Test Control Register*/

        /*1. config pattern and enable*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        SET_BIT_RANGE(ctc_dkit_serdes_wr.data, p_serdes_para->para[1], 0, 3)/*PATSEL*/
        DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 3);/*PCHKEN = 1*/
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        /*2. reset*/
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 4);/*PRST = 1*/
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 4);/*PRST = 0*/
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        /*3. read result*/
        while (1)
        {
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            if ((DKITS_IS_BIT_SET(ctc_dkit_serdes_wr.data, 8))||(count>=TIMEOUT)) /*SYNCST*/
            {
                break;
            }
            count++;
        }
        if (count >= TIMEOUT)
        {
            *pass = FALSE;
            time_out = 1;
        }

        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_wr.data, 9)) /*ERRST*/
        {
            *pass = TRUE;
        }
        else
        {
            *pass = FALSE;
        }
        if (!is_keep)
        {
            /*4. disable*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 3);/*PCHKEN = 0*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

            /*5. reset*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 4);/*PRST = 1*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 4);/*PRST = 0*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

            _ctc_dt2_dkit_misc_serdes_reset(lchip, p_serdes_para->serdes_id, FALSE);
        }
    }
    return time_out?CLI_ERROR:CLI_SUCCESS;
}



STATIC int32
_ctc_dt2_dkit_misc_serdes_prbs(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    int ret = CLI_SUCCESS;
    char* pattern[PATTERN_NUM] = {"PRBS7+","PRBS7-","PRBS15+","PRBS15-","PRBS23+","PRBS23-","PRBS31+","PRBS31-", "PRBS9", "Squareware"};
    char* enable = NULL;
    bool pass = FALSE;
    sal_file_t p_file = NULL;

    if (p_serdes_para->para[1] > (PATTERN_NUM-1))
    {
        return CLI_ERROR;
    }

    ret = _ctc_dt2_dkit_misc_do_serdes_prbs(p_serdes_para, &pass);
    if(CLI_EOL == ret)
    {
        CTC_DKIT_PRINT("prbs is enable, need disable first\n");
        return CLI_ERROR;
    }
    if (p_serdes_para->str)
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
        CTC_DKITS_PRINT_FILE(p_file, "\n%-12s%-6s%-10s%-9s%-9s\n", "Serdes_ID", "Dir", "Pattern", "Enable", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-12d%-6s%-10s%-9s%-9s\n", p_serdes_para->serdes_id, "RX", pattern[p_serdes_para->para[1]], "  -", enable);
    }
    if (p_file)
    {
        sal_fclose(p_file);
    }
    return CLI_SUCCESS;
}

STATIC int8
_ctc_dt2_dkit_misc_uint2int(uint8 value, uint8 signed_bit)
{
    uint8 tmp = 0;

    if (DKITS_IS_BIT_SET(value, signed_bit))
    {
        tmp = (~value) + 1;
        return -tmp;
    }
    else
    {
        return value;
    }
}

STATIC int8
_ctc_dt2_dkit_misc_int2uint(int8 value, uint8 signed_bit)
{
    uint8 tmp = 0;

    if (value < 0)
    {
        tmp = ~((uint8)value) + 1;
        DKITS_BIT_SET(tmp, signed_bit);
        return tmp;
    }
    else
    {
        return value;
    }
}

STATIC int32
_ctc_dt2_dkit_misc_serdes_eye(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
/*Receiver Internal Status Register*/
#define READY_TO_READ_E3                    13U
#define READY_TO_READ_E6_0                  14U
#define READY_TO_READ_E6_1                  15U
#define INPUT_DATA_TOO_FAST                 12U
#define DPC_COMVERGENCE_COMPLETE            9U
#define DAC_COMVERGENCE_COMPLETE            8U
#define DDC_COMVERGENCE_COMPLETE            7U
#define EYE_AMPLITUDE_ERROR                 6U
#define EYE_WIDTH_ERROR                     5U
#define DFE_TRAINING_COMPLETE               4U
#define VGA_LOCKED_FIRST                    3U
#define ROTATOR_OFFSET_CALIBRATION_COMPLETE 2U
#define DQCC_INITIAL_CALIBRATION_COMPLETE   1U
#define DC_OFFSET_CALIBRATION_COMPLETE      0U

    uint32 times = 0;
    uint32 count = 0;
    uint32 i = 0;
    uint32 err = 0;

    int32 an = 0;
    int32 ap = 0;
    int32 amin = 0;
    int32 eye_open = 0;
    int32 an_total = 0;
    int32 ap_total = 0;
    int32 amin_total = 0;
    int32 eye_open_total = 0;
    int32 eye_open_integer = 0;
    int32 eye_open_integer_total = 0;

    uint8 ber6_status = 0;
    uint8 ber3_status = 0;
    uint32 width = 0;
    uint32 az = 0;
    uint32 oes = 0;
    uint32 ols = 0;
    uint32 deduced_width = 0;
    uint32 width_total = 0;
    uint32 az_total = 0;
    uint32 oes_total = 0;
    uint32 ols_total = 0;
    uint32 deduced_width_total = 0;
    uint8 lchip = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_para;

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
    ctc_dkit_serdes_para.type = CTC_DKIT_SERDES_RX;

    /*1. set BER to 1E-3*/
    ctc_dkit_serdes_para.addr_offset = 0x1F;/*DFE Function Control Register 1*/
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
    if (CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E3 == p_serdes_para->precision)
    {
        /*FBER3*/
        DKITS_BIT_UNSET(ctc_dkit_serdes_para.data, 8);
    }
    else
    {
        /*FBER6*/
        DKITS_BIT_SET(ctc_dkit_serdes_para.data, 8);
    }
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

    /*2. check Receiver Internal Status Register*/
    ctc_dkit_serdes_para.addr_offset = 0x1E; /*Receiver Internal Status Register*/
    do
    {
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);

        err = 0;
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DC_OFFSET_CALIBRATION_COMPLETE))
        {
            err = 1;
        }
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, ROTATOR_OFFSET_CALIBRATION_COMPLETE))
        {
            err = 1;
        }
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, VGA_LOCKED_FIRST))
        {
            err = 1;
        }
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DFE_TRAINING_COMPLETE))
        {
            err = 1;
        }
        /*
    //no need check
    if (DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, EYE_WIDTH_ERROR))
    {
        err = 1;
    }
    if (DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, EYE_AMPLITUDE_ERROR))
    {
        err = 1;
    }
    */
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DDC_COMVERGENCE_COMPLETE))
        {
            err = 1;
        }
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DAC_COMVERGENCE_COMPLETE))
        {
            err = 1;
        }
        if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DPC_COMVERGENCE_COMPLETE))
        {
            err = 1;
        }
        if (DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, INPUT_DATA_TOO_FAST))
        {
            err = 1;
        }

        if (((CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E3 == p_serdes_para->precision)
           && (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, READY_TO_READ_E3)))
           || ((CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E6 == p_serdes_para->precision)
           && (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, READY_TO_READ_E6_0)
           || !DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, READY_TO_READ_E6_1))))
        {
            err = 1;
        }

        if (err)
        {
            if (count++ >= 50)
            {

                CTC_DKIT_PRINT(" Digital eye measure is not ready!!! Receiver Internal Status Reg value = 0x%x\n",
                               ctc_dkit_serdes_para.data);
                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DC_OFFSET_CALIBRATION_COMPLETE))
                {
                    CTC_DKIT_PRINT(" --DC offset calibration is not complete.\n");
                }
                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, ROTATOR_OFFSET_CALIBRATION_COMPLETE))
                {
                    CTC_DKIT_PRINT(" --Rotator offset calibration is not complete.\n");
                }
                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, VGA_LOCKED_FIRST))
                {
                    CTC_DKIT_PRINT(" --VGA locked first error.\n");
                }
                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DFE_TRAINING_COMPLETE))
                {
                    CTC_DKIT_PRINT(" --DFE training is not Complete.\n");
                }

                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DDC_COMVERGENCE_COMPLETE))
                {
                    CTC_DKIT_PRINT(" --DDC comvergence is not Complete.\n");
                }
                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DAC_COMVERGENCE_COMPLETE))
                {
                    CTC_DKIT_PRINT(" --DAC comvergence is not Complete.\n");
                }
                if (!DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, DPC_COMVERGENCE_COMPLETE))
                {
                    CTC_DKIT_PRINT(" --DPC comvergence is not Complete.\n");
                }
                if (DKITS_IS_BIT_SET(ctc_dkit_serdes_para.data, INPUT_DATA_TOO_FAST))
                {
                    CTC_DKIT_PRINT(" --Input data too fast.\n");
                }


                break;
            }
            sal_udelay(100);
        }
    }while(err);

    if ((CTC_DKIT_SERDIS_EYE_HEIGHT == p_serdes_para->para[0])
       || (CTC_DKIT_SERDIS_EYE_ALL == p_serdes_para->para[0]))
    {
        CTC_DKIT_PRINT("\n Eye height measure(%s)\n",
                          (CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E3 == p_serdes_para->precision) ? "1E-3" : "1E-6");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%s\n", "No.", "An", "Ap", "Amin", "EyeOpen");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");

        for (i = 0; i < times; i++)
        {
            /*3. put DFE into standby mode*/
            ctc_dkit_serdes_para.addr_offset = 0x8; /*DFE Control Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            DKITS_BIT_SET(ctc_dkit_serdes_para.data, 5);/*STNDBY = 1*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /*4. read AN and AP*/
            ctc_dkit_serdes_para.addr_offset = 0x12; /*Receiver DacAP and DacAN Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            an = _ctc_dt2_dkit_misc_uint2int(((ctc_dkit_serdes_para.data >> 8) & 0xFF), 7);
            ap = _ctc_dt2_dkit_misc_uint2int((ctc_dkit_serdes_para.data & 0xFF), 7);

            /*5. read AMin*/
            ctc_dkit_serdes_para.addr_offset = 0x13; /*Receiver DacAMin and DacAz Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            amin = _ctc_dt2_dkit_misc_uint2int((ctc_dkit_serdes_para.data & 0xFF), 7);

            /*6. reenable DFE*/
            ctc_dkit_serdes_para.addr_offset = 0x8; /*DFE Control Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            DKITS_BIT_UNSET(ctc_dkit_serdes_para.data, 5);/*recover the value*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /*7. eye open*/
            if (ap != 0)
            {
                eye_open = ((amin%ap) * 1000) / ap;
                eye_open_integer = amin/ap;
                if (eye_open < 0)
                {
                    eye_open = - eye_open;
                    eye_open_integer = -eye_open_integer;
                }
            }

            an_total += an;
            ap_total += ap;
            amin_total += amin;
            eye_open_total += eye_open;
            eye_open_integer_total +=eye_open_integer;

            CTC_DKIT_PRINT(" %-10d%-10d%-10d%-10d%d.%03d\n",
                           i + 1, an, ap, amin,eye_open_integer, eye_open);
        }

        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10d%-10d%-10d%d.%03d\n\n", "Avg",
                    an_total / times, ap_total / times, amin_total/times, eye_open_integer_total/times, eye_open_total/times);
    }

    if ((CTC_DKIT_SERDIS_EYE_WIDTH == p_serdes_para->para[0])
       || (CTC_DKIT_SERDIS_EYE_ALL == p_serdes_para->para[0]))
    {
        CTC_DKIT_PRINT("\n Eye width measure(%s)\n",
                          (CTC_DKIT_EYE_METRIC_PRECISION_TYPE_E3 == p_serdes_para->precision) ? "1E-3" : "1E-6");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10s%-10s%-10s%-10s%s\n", "No.", "Width", "Az", "Oes", "Ols", "DeducedWidth");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        for (i = 0; i < times; i++)
        {
            /*3. disable DDC before valid reading can be taken*/
            ctc_dkit_serdes_para.addr_offset = 0x1F; /*DFE Function Control Register 1*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            DKITS_BIT_UNSET(ctc_dkit_serdes_para.data, 13);/*FDDC = 0*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /*4. read width*/
            ctc_dkit_serdes_para.addr_offset = 0x2A; /*Digital Eye Control Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            width = (ctc_dkit_serdes_para.data >> 10) & 0x1F;

            /*5. read az*/
            ctc_dkit_serdes_para.addr_offset = 0x13; /*Receiver DacAMin and DacAz Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            az = (ctc_dkit_serdes_para.data >> 8) & 0x1F;

            /*6. read oes and ols*/
            ctc_dkit_serdes_para.addr_offset = 0x1D; /*Dynamic Data Centering (DDC) Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            oes = (ctc_dkit_serdes_para.data >> 6) & 0x1F;
            ols = ctc_dkit_serdes_para.data >> 11;

            /*7. deduce width*/
            deduced_width = oes + ols;

            /*8. enable DDC before valid reading can be taken*/
            ctc_dkit_serdes_para.addr_offset = 0x1F; /*DFE Function Control Register 1*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            DKITS_BIT_SET(ctc_dkit_serdes_para.data, 13);/*FDDC = 1*/
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            width_total += width;
            az_total += az;
            oes_total += oes;
            ols_total += ols;
            deduced_width_total += deduced_width;

            CTC_DKIT_PRINT(" %-10d%-10d%-10d%-10d%-10d%-10d\n", i + 1, width, az, oes, ols, deduced_width);
        }
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-10s%-10d%-10d%-10d%-10d%-10d\n\n", "Avg",
                       width_total/times, az_total/times,
                       oes_total/times, ols_total/times, deduced_width_total/times);
    }

    if (CTC_DKIT_SERDIS_EYE_WIDTH_SLOW == p_serdes_para->para[0])
    {
        CTC_DKIT_PRINT("\n Eye width measure(slow)\n");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-7s%-7s%-7s%-7s%-7s%-7s%-7s%s\n", "No.", "BER3", "BER6", "Width", "Az", "Oes", "Ols", "DeducedWidth");
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        for (i = 0; i < times; i++)
        {
            /*1. force AMP path ON continously*/
            ctc_dkit_serdes_para.addr_offset = 0x08;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            SET_BIT_RANGE(ctc_dkit_serdes_para.data, 0, 7, 2);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);

            /*2. set 1E-3*/
            ctc_dkit_serdes_para.addr_offset = 0x1F;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            DKITS_BIT_UNSET(ctc_dkit_serdes_para.data, 8);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            sal_task_sleep(1000);
            ctc_dkit_serdes_para.addr_offset = 0x1E;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ber3_status = (ctc_dkit_serdes_para.data >> 13)&0x1;

            /*3. set 1E-6*/
            ctc_dkit_serdes_para.addr_offset = 0x1F;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            DKITS_BIT_SET(ctc_dkit_serdes_para.data, 8);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);
            sal_task_sleep(2000);
            ctc_dkit_serdes_para.addr_offset = 0x1E;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            ber6_status = (ctc_dkit_serdes_para.data >> 14)&0x3;

            /*4. read width*/
            ctc_dkit_serdes_para.addr_offset = 0x2A;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            width = (ctc_dkit_serdes_para.data >> 10) & 0x1F;

            /*5. read az*/
            ctc_dkit_serdes_para.addr_offset = 0x13;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            az = (ctc_dkit_serdes_para.data >> 8) & 0x1F;

            /*6. read oes and ols*/
            ctc_dkit_serdes_para.addr_offset = 0x1D;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            oes = (ctc_dkit_serdes_para.data >> 6) & 0x1F;
            ols = ctc_dkit_serdes_para.data >> 11;

             /*7. return the RX to low power mode*/
            ctc_dkit_serdes_para.addr_offset = 0x08;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_para);
            SET_BIT_RANGE(ctc_dkit_serdes_para.data, 1, 7, 2);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_para);


            deduced_width = oes + ols;
            width_total += width;
            az_total += az;
            oes_total += oes;
            ols_total += ols;
            deduced_width_total += deduced_width;

            CTC_DKIT_PRINT(" %-7d%-7d%-7d%-7d%-7d%-7d%-7d%-10d\n", i + 1, ber3_status, ber6_status, width, az, oes, ols, deduced_width);
        }
        CTC_DKIT_PRINT(" ---------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-7s%-7s%-7s%-7d%-7d%-7d%-7d%-10d\n\n", "Avg","-","-",
                       width_total / times, az_total / times,
                       oes_total / times, ols_total / times, deduced_width_total / times);
    }


    return CLI_SUCCESS;
}

STATIC int32
_ctc_dt2_dkit_misc_serdes_scope(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    sal_file_t p_f = NULL;
    int8  value_int8 = 0;
    char str0[50] = {0}, str1[50] = {0}, str2[50] = {0};
    uint16 i = 0, j = 0, k = 0, n = 0, m = 0, count = 0, plot_idx = 0/*, stable = 0*/;
    uint8 value_uint8 = 0, an[101] = {0};
    int8* an_direct = NULL;
    int8* an_plot = NULL;
    int32 ret = CLI_SUCCESS;
    uint16 reg_0x8_def = 0, reg_0x1f_def = 0, reg_0x6_def = 0, reg_0x2b_def = 0, reg_0x3_def = 0;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;

    an_direct = (int8*)sal_malloc(5000 * sizeof(int8));
    if(NULL == an_direct)
    {
        return CLI_ERROR;
    }

    an_plot = (int8*)sal_malloc(5000 * sizeof(int8));
    if(NULL == an_plot)
    {
        sal_free(an_direct);
        return CLI_ERROR;
    }

    sal_memset(an_direct, 0, 5000 * sizeof(int8));
    sal_memset(an_plot, 0, 5000 * sizeof(int8));

    CTC_DKIT_LCHIP_CHECK(p_serdes_para->lchip);
    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));
    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;

    /* 1. Read the register cfg value for the future recovery */
    /* rd_reg(0x08, 0x1F, 0x06, 0x2B, 0x03) and save temporaly; */
    ctc_dkit_serdes_wr.addr_offset = 0x8;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    reg_0x8_def = ctc_dkit_serdes_wr.data;

    ctc_dkit_serdes_wr.addr_offset = 0x1F;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    reg_0x1f_def = ctc_dkit_serdes_wr.data;

    ctc_dkit_serdes_wr.addr_offset = 0x6;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    reg_0x6_def = ctc_dkit_serdes_wr.data;

    ctc_dkit_serdes_wr.addr_offset = 0x2B;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    reg_0x2b_def = ctc_dkit_serdes_wr.data;

    ctc_dkit_serdes_wr.addr_offset = 0x3;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    reg_0x3_def = ctc_dkit_serdes_wr.data;

    /* 2. Kick off an read */
    /* wr_reg(0x08[8:7,5])=0; */
    ctc_dkit_serdes_wr.addr_offset = 0x8;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 8);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 7);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 5);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    /* 3. Ensure all measure points are collected w/o rejection caused by the not-random detector */
    /* wr_reg(0x1F[2,12])=0, then wr_reg(0x08[3])=0. */
    ctc_dkit_serdes_wr.addr_offset = 0x1F;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 2);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 12);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x8;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 3);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    /* 4. note [14:10] is a signed value (-16 to +15) ?? */
    /* wr_reg(0x06[14:10])= (rd_reg(0x06[14:10]) + 8); */
    ctc_dkit_serdes_wr.addr_offset = 0x6;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    value_int8 = _ctc_dt2_dkit_misc_uint2int((ctc_dkit_serdes_wr.data >> 10), 4) + 8;
    value_uint8 = _ctc_dt2_dkit_misc_int2uint(value_int8, 4);
    (DKITS_IS_BIT_SET(value_uint8, 0)) ? DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 10) : DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 10);
    (DKITS_IS_BIT_SET(value_uint8, 1)) ? DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 11) : DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 11);
    (DKITS_IS_BIT_SET(value_uint8, 2)) ? DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 12) : DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 12);
    (DKITS_IS_BIT_SET(value_uint8, 3)) ? DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 13) : DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 13);
    (DKITS_IS_BIT_SET(value_uint8, 4)) ? DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 14) : DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 14);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    /* 5. wr_reg(0x1F[7])=0; */
    ctc_dkit_serdes_wr.addr_offset = 0x1F;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 7);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    /* 6. use PRBS7 */
    /* wr_reg(0x2B[4:3,0]) = {2'b00, 1'b1}; wr_reg(0x2E[14:0])=126; */
    ctc_dkit_serdes_wr.addr_offset = 0x2B;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 3);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 4);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 0);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x2E;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    ctc_dkit_serdes_wr.data = 126;/*bit 15 is Rsvd*/
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    /* Set amplitude rotator selector */
    /* wr_reg(0x03[5])=1; */
    ctc_dkit_serdes_wr.addr_offset = 0x03;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 5);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    /* scopemode ='001' */
    /* wr_reg(0x08[11:9]) = 3'b001; */
    ctc_dkit_serdes_wr.addr_offset = 0x08;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 9);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 10);
    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 11);
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    p_f = sal_fopen("./serdes_scope.txt", "wt");
    if (NULL == p_f)
    {
        CTC_DKIT_PRINT(" Write serdes scope file: %s failed!\n\n", "./serdes_scope.txt");
        ret = DRV_E_FATAL_EXCEP;
        goto clean;
    }

    sal_fprintf(p_f, "%-4s\t%s\n", "x", "y");

    for (i = 0; i < 16; i++)
    {
        for (j = 36; j >= 8; j = j - 4)
        {
            /* Default amplitude offset target 180 degree */
            /* wr_reg(0x03[4:0])=0x10; 10000b */
            ctc_dkit_serdes_wr.addr_offset = 0x3;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 0);
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 1);
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 2);
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 3);
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 4);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

            /* Apply */
            /* wr_reg(0x03[6])=1'b1; */
            DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 6);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

            /* Clear apply */
            DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 6);
            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

            /*stable = 0;*/
            /* loop until An stable */
            for (k = 0; k < 100; k++)
            {
                count = 0;
                do
                {   if (count++ > 50)
                    {
                        break;
                    }

                    /* Read several times (might be 10 times or more) until the An value is changing by only a small amount. */
                    ctc_dkit_serdes_wr.addr_offset = 0x8;
                    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    /* wr_reg(0x08[14])=1'b1; */
                    DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 14);
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
                    /* wr_reg(0x08[14])=1'b0; */
                    DKITS_BIT_UNSET(ctc_dkit_serdes_wr.data, 14);
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                } while (!DKITS_IS_BIT_SET(ctc_dkit_serdes_wr.data, 15));
                /* poll(0x08[15]) until it turn to be 1'b1; */

                ctc_dkit_serdes_wr.addr_offset = 0x9;
                ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                /* An = rd_reg(0x09) >>8; */
                an[k] = ctc_dkit_serdes_wr.data >> 8;

                if (0 == ((k + 1) % 10))
                {
                    for (n = 0; n < 9; n++)
                    {
                        for (m = n + 1; m < 10; m++)
                        {
                            if (((an[(((k + 1) / 10) - 1) * 10 + n] - an[(((k + 1) / 10) - 1) * 10 + m] > 3))
                               || ((an[(((k + 1) / 10) - 1) * 10 + n] - an[(((k + 1) / 10) - 1) * 10 + m] < -3)))
                            {
                                goto DROP_RST;
                            }
                        }
                    }
                    k = (sal_rand()% 10) + (((k + 1) / 10) - 1) * 10;
                    break;
                }
DROP_RST:
                continue;
            }
            /* endloop */
            an_plot[plot_idx] = _ctc_dt2_dkit_misc_uint2int(an[k], 7);
            ctc_dkit_serdes_wr.addr_offset = 0x12;
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            an_direct[plot_idx] = _ctc_dt2_dkit_misc_uint2int((ctc_dkit_serdes_wr.data >> 8) & 0xFF, 7);

            sal_sprintf(str0, "%d", k);
            sal_sprintf(str1, "%d", an_plot[plot_idx]);
            sal_sprintf(str2, "%d", an_direct[plot_idx]);

            sal_fprintf(p_f, "%-4s\t%s\n", str1, str2);
            plot_idx++;
        }

        /* wr_reg(0x2B[1])=1'b1; */
        ctc_dkit_serdes_wr.addr_offset = 0x2b;
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
        DKITS_BIT_SET(ctc_dkit_serdes_wr.data, 1);
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    }
    sal_fclose(p_f);
    CTC_DKIT_PRINT(" Output serdes scope file:%s.\n", "./serdes_scope.txt");

    /* Recover the old values for the following registers */
    ctc_dkit_serdes_wr.addr_offset = 0x3;
    ctc_dkit_serdes_wr.data = reg_0x3_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x2B;
    ctc_dkit_serdes_wr.data = reg_0x2b_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x6;
    ctc_dkit_serdes_wr.data = reg_0x6_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x1F;
    ctc_dkit_serdes_wr.data = reg_0x1f_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x8;
    ctc_dkit_serdes_wr.data = reg_0x8_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x8;
    DKITS_BIT_SET(reg_0x8_def, 0);
    ctc_dkit_serdes_wr.data = reg_0x8_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

    ctc_dkit_serdes_wr.addr_offset = 0x8;
    DKITS_BIT_UNSET(reg_0x8_def, 0);
    ctc_dkit_serdes_wr.data = reg_0x8_def;
    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

clean:
    sal_free(an_plot);
    sal_free(an_direct);

    return ret;
}

STATIC int32
_ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(uint16* tap_count, uint8 tap_id)
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
_ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(uint16* tap_count, uint8 tap_id)
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

STATIC int32
_ctc_dt2_dkit_misc_serdes_ffe_scan(void* p_para)
{
    uint8 lchip = 0;
    uint16 tap0 = 0;
    uint16 tap1 = 0;
    uint16 tap2 = 0;
    uint16 tap3 = 0;
    uint16 tap0_old = 0;
    uint16 tap1_old = 0;
    uint16 tap2_old = 0;
    uint16 tap3_old = 0;
    uint16 tap0_best = 0;
    uint16 tap1_best = 0;
    uint16 tap2_best = 0;
    uint16 tap3_best = 0;
    uint16 tap0_count[TAP_MAX_VALUE] = {0};
    uint16 tap1_count[TAP_MAX_VALUE] = {0};
    uint16 tap2_count[TAP_MAX_VALUE] = {0};
    uint16 tap3_count[TAP_MAX_VALUE] = {0};
    uint32 tap_sum_thrd = 0;
    uint8 is_hss15g = 0;
    uint8 pattern = 0;
    uint32 delay = 0;
    uint16 ffe_polarity = 0;
    bool pass = FALSE;
    sal_file_t p_file = NULL;
    sal_time_t tv;
    char* p_time_str = NULL;
    uint8 i = 0;
    int32 ret = CLI_SUCCESS;
    char* pattern_desc[PATTERN_NUM] = {"PRBS7+","PRBS7-","PRBS15+","PRBS15-","PRBS23+","PRBS23-","PRBS31+","PRBS31-", "PRBS9", "Squareware"};
    ctc_dkit_serdes_ctl_para_t serdes_para_prbs;
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;

    ctc_dkit_serdes_ctl_para_t* p_serdes_para = (ctc_dkit_serdes_ctl_para_t*)p_para;
    is_hss15g = !CTC_DKIT_SERDES_IS_HSS28G(p_serdes_para->serdes_id);
    lchip = p_serdes_para->lchip;
    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    pattern = (uint8)p_serdes_para->para[1];
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
        || (p_serdes_para->ffe.coefficient3_min > p_serdes_para->ffe.coefficient3_max))
    {
            CTC_DKIT_PRINT("Invalid ffe para!!!\n");
            return CLI_ERROR;
    }

    if ((p_serdes_para->ffe.coefficient0_max >= TAP_MAX_VALUE)
    || (p_serdes_para->ffe.coefficient1_max >= TAP_MAX_VALUE)
    || (p_serdes_para->ffe.coefficient2_max >= TAP_MAX_VALUE)
    || (p_serdes_para->ffe.coefficient3_max >= TAP_MAX_VALUE))
    {
        CTC_DKIT_PRINT("Invalid ffe para!!!\n");
        return CLI_ERROR;
    }

    if (pattern > (PATTERN_NUM-1))
    {
        CTC_DKIT_PRINT("PRBS pattern is invalid!!!\n");
        return CLI_ERROR;
    }

    if (p_serdes_para->str)
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
        /*get old ffe*/
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

        ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap0_old;
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap1_old;
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap2_old;
        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);


        CTC_DKITS_PRINT_FILE(p_file, "--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        /*for tap1*/
        for (tap0 = p_serdes_para->ffe.coefficient0_min; tap0 <= p_serdes_para->ffe.coefficient0_max; tap0++)
        {
            for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
            {
                for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
                {
                    if (tap0 + tap1 + tap2 <= tap_sum_thrd)
                    {
                        /*set ffe*/
                        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                        ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap0;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap1;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap2;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                        /*enable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                        /*check rx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                        serdes_para_prbs.para[1] = pattern;
                        serdes_para_prbs.para[3] = delay;
                        ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        if (CLI_ERROR == ret)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Not Sync");
                        }
                        else if (pass)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Pass");
                            tap0_count[tap0]++;
                            tap1_count[tap1]++;
                            tap2_count[tap2]++;
                        }
                        else
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Fail");
                        }

                        /*disable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap1-------\n");
        tap0_best = _ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(tap0_count, 0);
        tap1_best = _ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(tap1_count, 1);
        tap2_best = _ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(tap2_count, 2);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i]);
        }

        /*for tap0*/
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap0_count, 0, TAP_MAX_VALUE);
        sal_memset(tap2_count, 0, TAP_MAX_VALUE);
        tap1 = tap1_best;
        for (tap0 = p_serdes_para->ffe.coefficient0_min; tap0 <= p_serdes_para->ffe.coefficient0_max; tap0++)
        {
            for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
            {
                if (tap0 + tap1 + tap2 <= tap_sum_thrd)
                {
                    /*set ffe*/
                    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                    ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap0;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap1;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap2;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                    /*enable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                    /*check rx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                    serdes_para_prbs.para[1] = pattern;
                    serdes_para_prbs.para[3] = delay;
                    ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    if (CLI_ERROR == ret)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Not Sync");
                    }
                    else if (pass)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Pass");
                        tap0_count[tap0]++;
                        tap2_count[tap2]++;
                    }
                    else
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Fail");
                    }

                    /*disable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap0-------\n");
        tap0_best = _ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(tap0_count, 0);
        tap2_best = _ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(tap2_count, 2);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10s%-10d\n", i, tap0_count[i], "-", tap2_count[i]);
        }

        /*for tap2*/
        CTC_DKITS_PRINT_FILE(p_file, "\n--------------FFE scan----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        sal_memset(tap2_count, 0, TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
        {
            if (tap0 + tap1 + tap2 <= tap_sum_thrd)
            {
                /*set ffe*/
                ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap0;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap1;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap2;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                /*enable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                /*check rx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                serdes_para_prbs.para[1] = pattern;
                serdes_para_prbs.para[3] = delay;
                ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                if (CLI_ERROR == ret)
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Not Sync");
                }
                else if (pass)
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Pass");
                    tap2_count[tap2]++;
                }
                else
                {
                    CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, "Fail");
                }

                /*disable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-------Check best value of tap2-------\n");
        tap2_best = _ctc_dt2_dkit_misc_serdes_ffe_get_15g_best_tap(tap2_count, 2);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %5d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %5d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %5d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "----------------Count-----------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s\n","No.", "C0", "C1", "C2");
        CTC_DKITS_PRINT_FILE(p_file, "--------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10s%-10s%-10d\n", i, "-", "-", tap2_count[i]);
        }

        CTC_DKITS_PRINT_FILE(p_file, "\n--------------Scan result-------------\n");
        CTC_DKIT_PRINT( "C0: %5d\n", tap0_best);
        CTC_DKIT_PRINT( "C1: %5d\n", tap1_best);
        CTC_DKIT_PRINT( "C2: %5d\n", tap2_best);

        /*set old ffe*/
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

        ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap0_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap1_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap2_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

    }
    else /************************************ hss28g *******************************************/
    {
        /*get old ffe*/
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

        ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap0_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap1_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap2_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x0B; /*Transmit Tap 3 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap3_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        /*for tap2*/
        CTC_DKITS_PRINT_FILE(p_file, "-----------------FFE scan---------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        for (tap0 = p_serdes_para->ffe.coefficient0_min; tap0 <= p_serdes_para->ffe.coefficient0_max; tap0++)
        {
            for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
            {
                for (tap2 = p_serdes_para->ffe.coefficient2_min; tap2 <= p_serdes_para->ffe.coefficient2_max; tap2++)
                {
                    for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
                    {
                        int32 temp = 0;
                        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
                        ctc_dkit_serdes_wr.addr_offset = 0x0D; /*Transmit Polarity Register*/
                        ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                        ffe_polarity = ctc_dkit_serdes_wr.data;
                        if ((0xb != ffe_polarity) && (0x4 != ffe_polarity))
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "Polarity value error, Polarity:0x%04x\n", ffe_polarity);
                            ret = CLI_ERROR;
                            goto FFE_SCAN_ERROR;
                        }
                        temp += (!DKITS_IS_BIT_SET(ffe_polarity, 0))?tap0:(-tap0);
                        temp += (!DKITS_IS_BIT_SET(ffe_polarity, 1))?tap1:(-tap1);
                        temp += (!DKITS_IS_BIT_SET(ffe_polarity, 2))?tap2:(-tap2);
                        temp += (!DKITS_IS_BIT_SET(ffe_polarity, 3))?tap3:(-tap3);
                        if ((tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                            &&(((0xb == ffe_polarity)&&(temp>0))||((0x4 == ffe_polarity)&&(temp<0))))
                        {
                            /*set ffe*/
                            ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                            ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                            ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                            ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                            ctc_dkit_serdes_wr.data = (uint16)tap0;
                            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                            ctc_dkit_serdes_wr.data = (uint16)tap1;
                            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                            ctc_dkit_serdes_wr.data = (uint16)tap2;
                            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            ctc_dkit_serdes_wr.addr_offset = 0x0B; /*Transmit Tap 3 Coefficient Register*/
                            ctc_dkit_serdes_wr.data = (uint16)tap3;
                            ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                            DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                            /*enable tx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                            serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                            serdes_para_prbs.para[1] = pattern;
                            _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                            /*check rx*/
                            sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                            ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                            serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                            serdes_para_prbs.para[1] = pattern;
                            serdes_para_prbs.para[3] = delay;
                            ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
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
                            ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                            serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                            serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                            serdes_para_prbs.para[1] = pattern;
                            _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        }
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-----------Check best value of tap2-----------\n");

        CTC_DKITS_PRINT_FILE(p_file, "---------------------Count--------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i]);
        }

        tap0_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap0_count, 0);
        tap1_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap1_count, 1);
        tap2_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap2_count, 2);
        tap3_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %-d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %-d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %-d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %-d, count: %d\n", tap3_best, tap3_count[tap3_best]);

        CTC_DKITS_PRINT_FILE(p_file, "---------------------Count--------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i]);
        }

        /*for tap0*/
        CTC_DKITS_PRINT_FILE(p_file, "\n-----------------FFE scan---------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        sal_memset(tap0_count, 0, TAP_MAX_VALUE);
        sal_memset(tap1_count, 0, TAP_MAX_VALUE);
        sal_memset(tap3_count, 0, TAP_MAX_VALUE);
        tap2 = tap2_best;
        for (tap0 = p_serdes_para->ffe.coefficient0_min; tap0 <= p_serdes_para->ffe.coefficient0_max; tap0++)
        {
            for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
            {
                for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
                {
                    int32 temp = 0;
                    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
                    ctc_dkit_serdes_wr.addr_offset = 0x0D; /*Transmit Polarity Register*/
                    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                    ffe_polarity = ctc_dkit_serdes_wr.data;
                    if ((0xb != ffe_polarity) && (0x4 != ffe_polarity))
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "Polarity value error, Polarity:0x%04x\n", ffe_polarity);
                    }
                    temp += (!DKITS_IS_BIT_SET(ffe_polarity, 0))? tap0 : ( - tap0);
                    temp += (!DKITS_IS_BIT_SET(ffe_polarity, 1))? tap1 : ( - tap1);
                    temp += (!DKITS_IS_BIT_SET(ffe_polarity, 2))? tap2 : ( - tap2);
                    temp += (!DKITS_IS_BIT_SET(ffe_polarity, 3))? tap3 : ( - tap3);
                    if ((tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                        && (((0xb == ffe_polarity) && (temp > 0)) || ((0x4 == ffe_polarity) && (temp < 0))))
                    {
                        /*set ffe*/
                        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                        ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap0;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap1;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap2;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        ctc_dkit_serdes_wr.addr_offset = 0x0B; /*Transmit Tap 3 Coefficient Register*/
                        ctc_dkit_serdes_wr.data = (uint16)tap3;
                        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                        DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                        /*enable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                        /*check rx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                        serdes_para_prbs.para[1] = pattern;
                        serdes_para_prbs.para[3] = delay;
                        ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                        if (CLI_ERROR == ret)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Not Sync");
                        }
                        else if (pass)
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Pass");
                            tap0_count[tap0]++;
                            tap1_count[tap1]++;
                            tap3_count[tap3]++;
                        }
                        else
                        {
                            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Fail");
                        }

                        /*disable tx*/
                        sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                        serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                        serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                        serdes_para_prbs.para[1] = pattern;
                        _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    }
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-----------Check best value of tap0-----------\n");
        tap0_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap0_count, 0);
        tap1_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap1_count, 1);
        tap3_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %-d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %-d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %-d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %-d, count: %d\n", tap3_best, tap3_count[tap3_best]);

        CTC_DKITS_PRINT_FILE(p_file, "---------------------Count--------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i]);
        }


        /*for tap1*/
        CTC_DKITS_PRINT_FILE(p_file, "\n-----------------FFE scan---------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        sal_memset(tap1_count, 0, TAP_MAX_VALUE);
        sal_memset(tap3_count, 0, TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap2 = tap2_best;
        for (tap1 = p_serdes_para->ffe.coefficient1_min; tap1 <= p_serdes_para->ffe.coefficient1_max; tap1++)
        {
            for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
            {
                int32 temp = 0;
                ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
                ctc_dkit_serdes_wr.addr_offset = 0x0D; /*Transmit Polarity Register*/
                ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
                ffe_polarity = ctc_dkit_serdes_wr.data;
                if ((0xb != ffe_polarity) && (0x4 != ffe_polarity))
                {
                    CTC_DKITS_PRINT_FILE(p_file, "Polarity value error, Polarity:0x%04x\n", ffe_polarity);
                }
                temp += (!DKITS_IS_BIT_SET(ffe_polarity, 0))? tap0 : ( - tap0);
                temp += (!DKITS_IS_BIT_SET(ffe_polarity, 1))? tap1 : ( - tap1);
                temp += (!DKITS_IS_BIT_SET(ffe_polarity, 2))? tap2 : ( - tap2);
                temp += (!DKITS_IS_BIT_SET(ffe_polarity, 3))? tap3 : ( - tap3);
                if ((tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                    && (((0xb == ffe_polarity) && (temp > 0)) || ((0x4 == ffe_polarity) && (temp < 0))))
                {
                    /*set ffe*/
                    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                    ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap0;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap1;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap2;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    ctc_dkit_serdes_wr.addr_offset = 0x0B; /*Transmit Tap 3 Coefficient Register*/
                    ctc_dkit_serdes_wr.data = (uint16)tap3;
                    ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                    DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                    /*enable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                    /*check rx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                    serdes_para_prbs.para[1] = pattern;
                    serdes_para_prbs.para[3] = delay;
                    ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                    if (CLI_ERROR == ret)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Not Sync");
                    }
                    else if (pass)
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Pass");
                        tap1_count[tap1]++;
                        tap3_count[tap3]++;
                    }
                    else
                    {
                        CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10s\n", tap0, tap1, tap2, tap3, "Fail");
                    }

                    /*disable tx*/
                    sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                    ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                    serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                    serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                    serdes_para_prbs.para[1] = pattern;
                    _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
                }
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-----------Check best value of tap1-----------\n");
        tap1_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap1_count, 1);
        tap3_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %-d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %-d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %-d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %-d, count: %d\n", tap3_best, tap3_count[tap3_best]);

        CTC_DKITS_PRINT_FILE(p_file, "---------------------Count--------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i]);
        }



        /*for tap3*/
        CTC_DKITS_PRINT_FILE(p_file, "\n-----------------FFE scan---------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "C0", "C1", "C2", "C3", "Result");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        sal_memset(tap3_count, 0, TAP_MAX_VALUE);
        tap0 = tap0_best;
        tap1 = tap1_best;
        tap2 = tap2_best;
        for (tap3 = p_serdes_para->ffe.coefficient3_min; tap3 <= p_serdes_para->ffe.coefficient3_max; tap3++)
        {
            int32 temp = 0;
            ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
            ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
            ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;
            ctc_dkit_serdes_wr.addr_offset = 0x0D; /*Transmit Polarity Register*/
            ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
            ffe_polarity = ctc_dkit_serdes_wr.data;
            if ((0xb != ffe_polarity) && (0x4 != ffe_polarity))
            {
                CTC_DKITS_PRINT_FILE(p_file, "Polarity value error, Polarity:0x%04x\n", ffe_polarity);
            }
            temp += (!DKITS_IS_BIT_SET(ffe_polarity, 0))? tap0 : ( - tap0);
            temp += (!DKITS_IS_BIT_SET(ffe_polarity, 1))? tap1 : ( - tap1);
            temp += (!DKITS_IS_BIT_SET(ffe_polarity, 2))? tap2 : ( - tap2);
            temp += (!DKITS_IS_BIT_SET(ffe_polarity, 3))? tap3 : ( - tap3);
            if ((tap0 + tap1 + tap2 + tap3 <= tap_sum_thrd)
                && (((0xb == ffe_polarity) && (temp > 0)) || ((0x4 == ffe_polarity) && (temp < 0))))
            {
                /*set ffe*/
                ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

                ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap0;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap1;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap2;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                ctc_dkit_serdes_wr.addr_offset = 0x0B; /*Transmit Tap 3 Coefficient Register*/
                ctc_dkit_serdes_wr.data = (uint16)tap3;
                ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

                DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);

                /*enable tx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);

                /*check rx*/
                sal_memset(&serdes_para_prbs, 0, sizeof(ctc_dkit_serdes_ctl_para_t));
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                serdes_para_prbs.serdes_id = p_serdes_para->para[0];
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
                serdes_para_prbs.para[1] = pattern;
                serdes_para_prbs.para[3] = delay;
                ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
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
                ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
                serdes_para_prbs.serdes_id = p_serdes_para->serdes_id;
                serdes_para_prbs.para[0] = CTC_DKIT_SERDIS_CTL_DISABLE;
                serdes_para_prbs.para[1] = pattern;
                _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para_prbs, &pass);
            }
        }

        CTC_DKITS_PRINT_FILE(p_file, "-----------Check best value of tap3-----------\n");
        tap3_best = _ctc_dt2_dkit_misc_serdes_ffe_get_28g_best_tap(tap3_count, 3);
        CTC_DKITS_PRINT_FILE(p_file, "C0: %-d, count: %d\n", tap0_best, tap0_count[tap0_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C1: %-d, count: %d\n", tap1_best, tap1_count[tap1_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C2: %-d, count: %d\n", tap2_best, tap2_count[tap2_best]);
        CTC_DKITS_PRINT_FILE(p_file, "C3: %-d, count: %d\n", tap3_best, tap3_count[tap3_best]);

        CTC_DKITS_PRINT_FILE(p_file, "---------------------Count--------------------\n");
        CTC_DKITS_PRINT_FILE(p_file, "%-10s%-10s%-10s%-10s%-10s\n", "No.", "C0", "C1", "C2", "C3");
        CTC_DKITS_PRINT_FILE(p_file, "----------------------------------------------\n");
        for (i = 0; i < TAP_MAX_VALUE; i++)
        {
            CTC_DKITS_PRINT_FILE(p_file, "%-10d%-10d%-10d%-10d%-10d\n", i, tap0_count[i], tap1_count[i], tap2_count[i], tap3_count[i]);
        }

        CTC_DKITS_PRINT_FILE(p_file, "\n-----------------Scan result------------------\n");
        CTC_DKIT_PRINT( "C0: %5d\n", tap0_best);
        CTC_DKIT_PRINT( "C1: %5d\n", tap1_best);
        CTC_DKIT_PRINT( "C2: %5d\n", tap2_best);
        CTC_DKIT_PRINT( "C3: %5d\n", tap3_best);

        /*set old ffe*/
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
        ctc_dkit_serdes_wr.lchip = p_serdes_para->lchip;
        ctc_dkit_serdes_wr.serdes_id = p_serdes_para->serdes_id;

        ctc_dkit_serdes_wr.addr_offset = 0x08; /*Transmit Tap 0 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap0_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x09; /*Transmit Tap 1 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap1_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x0A; /*Transmit Tap 2 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap2_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        ctc_dkit_serdes_wr.addr_offset = 0x0B; /*Transmit Tap 3 Coefficient Register*/
        ctc_dkit_serdes_wr.data = (uint16)tap3_old;
        ctc_dt2_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);

        DKITS_SERDES_TAP_RELOAD(ctc_dkit_serdes_wr);
    }

FFE_SCAN_ERROR:
    if (p_file)
    {
        sal_fclose(p_file);
    }

    return ret;
}

STATIC int32
_ctc_dt2_dkit_misc_serdes_status(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
    uint32 hs_mon[] =
    {
        HsMon0_t,
        HsMon1_t,
        HsMon2_t
    };
    uint32 cs_mon[] =
    {
        CsMon0_t,
        CsMon1_t,
        CsMon2_t,
        CsMon3_t
    };
    uint32 hs_sigdet[]  =
    {
        HsMon0_monHss15gRxASigdet_f,
        HsMon0_monHss15gRxBSigdet_f,
        HsMon0_monHss15gRxCSigdet_f,
        HsMon0_monHss15gRxDSigdet_f,
        HsMon0_monHss15gRxESigdet_f,
        HsMon0_monHss15gRxFSigdet_f,
        HsMon0_monHss15gRxGSigdet_f,
        HsMon0_monHss15gRxHSigdet_f
    };
    uint32 cs_sigdet[]  =
    {
        CsMon0_monHss28gRxASigdet_f,
        CsMon0_monHss28gRxBSigdet_f,
        CsMon0_monHss28gRxCSigdet_f,
        CsMon0_monHss28gRxDSigdet_f
    };
    char* pll_select = NULL;
    uint8 lane_id = 0;
    uint8 hss_id = 0;
    uint8 is_hss15g = 0;
    uint32 value = 0;
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
    ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
    ctc_dkit_serdes_wr.addr_offset = 0;
    ctc_dt2_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);

    CTC_DKIT_PRINT("\n------Serdes%d Hss%d Lane%d Status-----\n", p_serdes_para->serdes_id, hss_id, lane_id);
    tbl_id = is_hss15g ? hs_mon[hss_id] : cs_mon[hss_id];
    if (0 == ((ctc_dkit_serdes_wr.data >> 6)&0x3))
    {
        pll_select = "PLLA";
        fld_id = is_hss15g? HsMon0_monHss15gHssPllLockA_f : CsMon0_monHss28gHssPllLockA_f;
        cmd = DRV_IOR(tbl_id, fld_id);
        DRV_IOCTL(lchip, 0, cmd, &value);
        pll_ready = value?"YES":"NO";
    }
    else if (1 == ((ctc_dkit_serdes_wr.data >> 6)&0x3))
    {
        fld_id = is_hss15g? HsMon0_monHss15gHssPllLockB_f : CsMon0_monHss28gHssPllLockB_f;
        pll_select = "PLLB";
        cmd = DRV_IOR(tbl_id, fld_id);
        DRV_IOCTL(lchip, 0, cmd, &value);
        pll_ready = value?"YES":"NO";
    }
    else
    {
        pll_select = "NONE";
    }
    CTC_DKIT_PRINT("%-20s:%s\n", "PLL Select", pll_select);
    CTC_DKIT_PRINT("%-20s:%s\n", "PLL Ready", pll_ready);

    fld_id = is_hss15g? hs_sigdet[lane_id] : cs_sigdet[lane_id];
    cmd = DRV_IOR(tbl_id, fld_id);
    DRV_IOCTL(lchip, 0, cmd, &value);
    CTC_DKIT_PRINT("%-20s:%d\n", "Sigdet", value);

    CTC_DKIT_PRINT("------------------------------------\n");


    return CLI_SUCCESS;
}

int32
_ctc_dt2_dkit_misc_serdes_polr_check(ctc_dkit_serdes_ctl_para_t* p_serdes_para)
{
	int ret = CLI_SUCCESS;
	bool pass = 0;
    ctc_dkit_serdes_ctl_para_t serdes_para;

    sal_memset(&serdes_para, 0, sizeof(ctc_dkit_serdes_ctl_para_t));

    serdes_para.serdes_id = p_serdes_para->serdes_id;
    serdes_para.para[0] = CTC_DKIT_SERDIS_CTL_ENABLE;
    serdes_para.para[1] = 6;  /* PRBS pattern 6 */
    _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para, &pass);

    serdes_para.serdes_id = p_serdes_para->para[0];
    serdes_para.para[0] = CTC_DKIT_SERDIS_CTL_CEHCK;
    serdes_para.para[1] = 6;  /* PRBS pattern 6 */
    ret = _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para, &pass);
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
    _ctc_dt2_dkit_misc_do_serdes_prbs(&serdes_para, &pass);

    return CLI_SUCCESS;
}

int32
ctc_dt2_dkit_misc_serdes_ctl(void* p_para)
{
    ctc_dkit_serdes_ctl_para_t* p_serdes_para = (ctc_dkit_serdes_ctl_para_t*)p_para;

    DKITS_PTR_VALID_CHECK(p_serdes_para);
    CTC_DKIT_LCHIP_CHECK(p_serdes_para->lchip);
    DRV_INIT_CHECK(p_serdes_para->lchip);

    switch (p_serdes_para->type)
    {
        case CTC_DKIT_SERDIS_CTL_LOOPBACK:
            _ctc_dt2_dkit_misc_serdes_loopback(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_PRBS:
            _ctc_dt2_dkit_misc_serdes_prbs(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_EYE:
            _ctc_dt2_dkit_misc_serdes_eye(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_EYE_SCOPE:
            _ctc_dt2_dkit_misc_serdes_scope(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_FFE:
            _ctc_dt2_dkit_misc_serdes_ffe_scan(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_STATUS:
            _ctc_dt2_dkit_misc_serdes_status(p_serdes_para);
            break;
        case CTC_DKIT_SERDIS_CTL_POLR_CHECK:
            _ctc_dt2_dkit_misc_serdes_polr_check(p_serdes_para);
            break;
        default:
            break;
    }

    return CLI_SUCCESS;
}

