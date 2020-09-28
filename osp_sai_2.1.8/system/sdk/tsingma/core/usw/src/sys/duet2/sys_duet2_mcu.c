/**
 @file sys_duet2_mcu.c

 @date 2018-09-12

 @version v1.0
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#include "sys_usw_common.h"
#include "sys_usw_datapath.h"
#include "sys_usw_mac.h"
#include "sys_usw_mcu.h"
#include "sys_duet2_mcu.h"

#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"

/****************************************************************************
 *
* Function
*
*****************************************************************************/
int32
sys_duet2_mcu_show_debug_info(uint8 lchip)
{
    uint32 i = 0;
    uint32 j = 0;
    uint32 port_info[4] = {0};
    uint32 tmp_addr32 = 0;
    uint32 sigdet_info = 0;
    uint32 link_info = 0;
    uint32 mcu_glb_on = 0;
    uint32 mcu_port_dbg_on32 = 0;
    uint32 mcu_port_func_on32 = 0;
    sys_usw_mcu_port_attr_t *p_mcu_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, SYS_USW_MCU_UT_GLB_ON_BASE_ADDR, &mcu_glb_on));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "!!! MCU is %s now !!!\n", mcu_glb_on?"ON":"OFF");

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-8s%-5s%-8s%-8s%-8s%-5s%-12s%-8s%-6s%-13s\n",
                      "GPort", "MAC", "MAC-en", "UD-en", "SerDes", "Mode", "SerDes-Num", "AN-en", "Link", "MCU-port-on");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------\n");


    for (i = 0; i < 64; i++)
    {
        tmp_addr32 = SYS_USW_MCU_UT_PORT_BASE_ADDR(i) + SYS_USW_MCU_UT_PORT_SIGDET_OFST;
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr32, &sigdet_info));
        tmp_addr32 = SYS_USW_MCU_UT_PORT_BASE_ADDR(i) + SYS_USW_MCU_UT_PORT_LINK_STA_OFST;
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr32, &link_info));
        tmp_addr32 = SYS_USW_MCU_UT_PORT_BASE_ADDR(i) + SYS_USW_MCU_UT_PORT_DBG_ON_ACK_OFST;
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr32, &mcu_port_dbg_on32));
        tmp_addr32 = SYS_USW_MCU_UT_PORT_BASE_ADDR(i) + SYS_USW_MCU_UT_PORT_ON_ACK_OFST;
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr32, &mcu_port_func_on32));

        p_mcu_attr = (sys_usw_mcu_port_attr_t*)mem_malloc(MEM_PORT_MODULE, sizeof(sys_usw_mcu_port_attr_t));                    
        if (NULL == p_mcu_attr)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_mcu_attr, 0, sizeof(sys_usw_mcu_port_attr_t));
        
        tmp_addr32 = SYS_USW_MCU_UT_PORT_DS_BASE_ADDR + SYS_USW_MCU_PORT_ALLOC_BYTE*i;
        for (j = 0; j < 4; j++)
        {
            CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr32+j*4, &port_info[j]));
        }
        p_mcu_attr->mac_en       = port_info[0] & 0xff;
        p_mcu_attr->mac_id       = (port_info[0] >> 8) & 0xff;
        p_mcu_attr->serdes_id    = (port_info[0] >> 16) & 0xff;
        p_mcu_attr->serdes_id2   = (port_info[0] >> 24) & 0xff;

        p_mcu_attr->serdes_num   = port_info[1] & 0xff;
        p_mcu_attr->serdes_mode  = (port_info[1] >> 8) & 0xff;
        p_mcu_attr->flag         = (port_info[1] >> 16) & 0xff;
        p_mcu_attr->unidir_en    = (port_info[1] >> 24) & 0xff;

        p_mcu_attr->an_en        = (port_info[2] >> 8) & 0xff;

        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-8d%-5d%-8d%-8d%-8d%-5d%-12d%-8d%-6d%-13d\n", i, 
            p_mcu_attr->mac_id, p_mcu_attr->mac_en, p_mcu_attr->unidir_en, p_mcu_attr->serdes_id, p_mcu_attr->serdes_mode,
            p_mcu_attr->serdes_num, p_mcu_attr->an_en, link_info, mcu_port_func_on32);

        if (p_mcu_attr)
        {
            mem_free(p_mcu_attr);
            p_mcu_attr = NULL;
        }
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------\n");
    return CTC_E_NONE;
}

