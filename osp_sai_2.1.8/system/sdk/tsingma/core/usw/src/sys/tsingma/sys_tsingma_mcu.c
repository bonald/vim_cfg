/**
 @file sys_tsingma_mcu.c

 @author  Copyright (C) 2018 Centec Networks Inc.  All rights reserved.

 @date 2018-03-19

 @version v1.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#include "sys_usw_common.h"
#include "sys_usw_datapath.h"
#include "sys_tsingma_datapath.h"
#include "sys_usw_mac.h"
#include "sys_tsingma_mac.h"
#include "sys_usw_mcu.h"
#include "sys_tsingma_mcu.h"

#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"
#include "sys_tsingma_hss12g_firmware.inc"

extern sys_datapath_master_t* p_usw_datapath_master[];



extern int32 drv_usw_mcu_tbl_lock(uint8 lchip, tbls_id_t tbl_id, uint32 op);
extern int32 drv_usw_mcu_tbl_unlock(uint8 lchip, tbls_id_t tbl_id, uint32 op);
extern int32 drv_usw_mcu_lock(uint8 lchip, uint32 lock_id, uint8 mcu_id);
extern int32 drv_usw_mcu_unlock(uint8 lchip, uint32 lock_id, uint8 mcu_id);
/****************************************************************************
 *
* Functions
*
****************************************************************************/
uint8
sys_tsingma_mcu_get_core_id_by_tbl_id(uint8 lchip, uint8 tbl_id)
{
    uint8 cnt;
    uint8 core_0_field[] = {0,1,2,3, 4,5,6,7, 16,17,18,19, 20,21,22,23, 8,9,10,11, 36,37,38,39, 40,41,42,43, 44,45,46,47};
    uint8 core_1_field[] = {24,25,26,27, 52,53,54,55, 56,57,58,59, 60,61,62,63, 12,13,14,15};
    uint8 core_2_field[] = {28,29,30,31, 32,33,34,35};
    uint8 core_3_field[] = {48,49,50,51, 64,65,66,67};

    if(2 == p_usw_datapath_master[lchip]->qm_choice.txqm3)
    {
        core_2_field[4] = 68;
        core_2_field[5] = 69;
        core_2_field[6] = 70;
        core_2_field[7] = 71;
    }

    for(cnt = 0; cnt < 32; cnt++)
    {
        if(tbl_id == core_0_field[cnt])
        {
            return 0;
        }
    }
    for(cnt = 0; cnt < 20; cnt++)
    {
        if(tbl_id == core_1_field[cnt])
        {
            return 1;
        }
    }
    for(cnt = 0; cnt < 8; cnt++)
    {
        if(tbl_id == core_2_field[cnt])
        {
            return 2;
        }
    }
    for(cnt = 0; cnt < 8; cnt++)
    {
        if(tbl_id == core_3_field[cnt])
        {
            return 3;
        }
    }
    return 0xff;
}

uint32
sys_tsingma_mcu_get_port_pos(uint8 lchip, uint8 core_id, uint8 tbl_id)
{
    uint8 core_0_field[] = {0,1,2,3, 4,5,6,7, 16,17,18,19, 20,21,22,23, 8,9,10,11, 36,37,38,39, 40,41,42,43, 44,45,46,47};
    uint8 core_1_field[] = {24,25,26,27, 52,53,54,55, 56,57,58,59, 60,61,62,63, 12,13,14,15};
    uint8 core_2_field[] = {28,29,30,31, 32,33,34,35};
    uint8 core_3_field[] = {48,49,50,51, 64,65,66,67};
    uint8 *core_field = NULL;
    uint8 cnt;
    uint8 max_cnt;

    if(NULL == p_usw_datapath_master[lchip])
    {
        return SYS_TSINGMA_MCU_MAX_PORT_PER_CORE;
    }

    if(1 == p_usw_datapath_master[lchip]->qm_choice.muxmode)
    {
        for(cnt = 4; cnt < 8; cnt++)
        {
            core_2_field[cnt] = 64+cnt;
        }
    }
    
    switch(core_id)
    {
        case 0:
            core_field = core_0_field;
            max_cnt = sizeof(core_0_field)/sizeof(uint8);
            break;
        case 1:
            core_field = core_1_field;
            max_cnt = sizeof(core_1_field)/sizeof(uint8);
            break;
        case 2:
            core_field = core_2_field;
            max_cnt = sizeof(core_2_field)/sizeof(uint8);
            break;
        case 3:
            core_field = core_3_field;
            max_cnt = sizeof(core_3_field)/sizeof(uint8);
            break;
        default:
            return SYS_TSINGMA_MCU_MAX_PORT_PER_CORE;
    }

    for(cnt = 0; cnt < max_cnt; cnt++)
    {
        if(tbl_id == core_field[cnt])
        {
            return cnt;
        }
    }
    return SYS_TSINGMA_MCU_MAX_PORT_PER_CORE;
}

int32
sys_tsingma_mcu_write_glb_info(uint8 lchip, uint8 core_id)
{
    uint32 glb_ctl_addr[7][3] = {
        {SYS_TSINGMA_MCU_0_CORE_ID_ADDR,      SYS_TSINGMA_MCU_1_CORE_ID_ADDR,      SYS_TSINGMA_MCU_2_CORE_ID_ADDR},
        {SYS_TSINGMA_MCU_0_SWITCH_CTL_ADDR,   SYS_TSINGMA_MCU_1_SWITCH_CTL_ADDR,   SYS_TSINGMA_MCU_2_SWITCH_CTL_ADDR},
        {SYS_TSINGMA_MCU_0_QM_CHOICE_ADDR,    SYS_TSINGMA_MCU_1_QM_CHOICE_ADDR,    SYS_TSINGMA_MCU_2_QM_CHOICE_ADDR},
        {SYS_TSINGMA_MCU_0_DBG_LOG_CTL_ADDR,  SYS_TSINGMA_MCU_1_DBG_LOG_CTL_ADDR,  SYS_TSINGMA_MCU_2_DBG_LOG_CTL_ADDR},
        {SYS_TSINGMA_MCU_0_RX_EQ_LT_ADDR,     SYS_TSINGMA_MCU_1_RX_EQ_LT_ADDR,     SYS_TSINGMA_MCU_2_RX_EQ_LT_ADDR},
        {SYS_TSINGMA_MCU_0_ISCAN_MIN_VAL_THD, SYS_TSINGMA_MCU_1_ISCAN_MIN_VAL_THD, SYS_TSINGMA_MCU_2_ISCAN_MIN_VAL_THD},
        {SYS_TSINGMA_MCU_0_DFE_OPR_IND,       SYS_TSINGMA_MCU_1_DFE_OPR_IND,       SYS_TSINGMA_MCU_2_DFE_OPR_IND}
    };
    uint32 value[7] = {
        0,  //core id (default 0, need change)
        1,  //switch set 1
        0,  //qm choice (default 0, need change)
        2,  //record fatal log
        1,  //RX eq after LT
        25, //minimun iscan fine value
        1   //DFE disable during LT
    };
    uint8 step = 0;
    LCHIP_CHECK(lchip);
    
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }
    if(2 < core_id)
    {
        return CTC_E_INVALID_PARAM;
    }

    value[0] = core_id;
    value[2] = p_usw_datapath_master[lchip]->qm_choice.muxmode;
    value[2] = ((value[2])<<8)|(p_usw_datapath_master[lchip]->qm_choice.qmmode);
    value[2] = ((value[2])<<8)|(p_usw_datapath_master[lchip]->qm_choice.txqm3);
    value[2] = ((value[2])<<8)|(p_usw_datapath_master[lchip]->qm_choice.txqm1);

    for(step = 0; step < 7; step++)
    {
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, glb_ctl_addr[step][core_id], value[step]));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_get_enable(uint8 lchip, uint32* enable, uint8 core_id)
{
    uint32 value = 0;
    uint32 mcu_switch_stat_addr = 0;
    
    LCHIP_CHECK(lchip);
    
    switch(core_id)
    {
        case 0:
            mcu_switch_stat_addr = SYS_TSINGMA_MCU_0_SWITCH_STAT_ADDR;
            break;
        case 1:
            mcu_switch_stat_addr = SYS_TSINGMA_MCU_1_SWITCH_STAT_ADDR;
            break;
        case 2:
            mcu_switch_stat_addr = SYS_TSINGMA_MCU_2_SWITCH_STAT_ADDR;
            break;
        case 3:
        default:
            mcu_switch_stat_addr = SYS_TSINGMA_MCU_3_SWITCH_STAT_ADDR;
            break;
    }
    CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, mcu_switch_stat_addr, &value));
    *enable = (uint32)(value & 0xff);

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_check_run(uint8 lchip, uint8 core_id)
{
    uint8 max_time = 50;
    uint8 time = 0;
    uint32 run_stat = 0;
    
    for(; time < max_time; time++)
    {
        sys_tsingma_mcu_get_enable(lchip, &run_stat, core_id);
        if(0 != run_stat)
        {
            return TRUE;
        }
        sal_task_sleep(20);
    }

    return FALSE;
}

int32
sys_tsingma_mcu_get_port_addr_by_tblid(uint8 lchip, uint8 core_id, uint8 tbl_id, uint32* p_port_addr)
{
    uint32 port_addr;
    uint32 port_pos = sys_tsingma_mcu_get_port_pos(lchip, core_id, tbl_id);
    
    if(SYS_TSINGMA_MCU_MAX_PORT_PER_CORE == port_pos)
    {
        return CTC_E_INVALID_PARAM;
    }
    if(NULL == p_port_addr)
    {
        return CTC_E_INVALID_PTR;
    }
    switch(core_id)
    {
        case 0:
            port_addr = SYS_TSINGMA_MCU_0_PORT_INFO_BASE_ADDR;
            break;
        case 1:
            port_addr = SYS_TSINGMA_MCU_1_PORT_INFO_BASE_ADDR;
            break;
        case 2:
            port_addr = SYS_TSINGMA_MCU_2_PORT_INFO_BASE_ADDR;
            break;
        case 3:
            port_addr = SYS_TSINGMA_MCU_3_PORT_INFO_BASE_ADDR;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    port_addr += (port_pos*SYS_TSINGMA_MCU_PORT_ALLOC_BYTE);
    *p_port_addr = port_addr;
    
    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_get_port_addr_by_lport(uint8 lchip, uint16 lport, uint32* p_port_addr)
{
    uint8  serdes_num      = 0;
    uint16 chip_port       = 0;
    uint16 serdes_id       = 0;
    uint32 slice_id        = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(SYS_DMPS_NETWORK_PORT != port_attr->port_type)
    {
        return CTC_E_INVALID_PARAM;
    }

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id * SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    CTC_ERROR_RETURN(sys_tsingma_mcu_get_port_addr_by_tblid(lchip, serdes_id/8, port_attr->tbl_id, p_port_addr));
    
    return CTC_E_NONE;
}

/*sys_usw_mac_set_mcu_port_enable*/
int32
sys_tsingma_mcu_set_port_link_stat(uint8 lchip, uint16 lport, uint32 up_flag)
{
    uint32 port_info = 0;
    uint32 port_addr = 0;
    
    if(1 == SDK_WORK_PLATFORM)
    {
        return CTC_E_NONE;
    }

    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_tsingma_mcu_get_port_addr_by_lport(lchip, lport, &port_addr));
    
    CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, port_addr, &port_info));
    /*tsingma_mcu_port_attr_t.link_stat*/
    if(up_flag)
    {
        MCU_SET_U8_FIELD(3, port_info, 1);
    }
    else
    {
        MCU_SET_U8_FIELD(3, port_info, 1);
    }
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, port_addr, port_info));

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_get_init_info(uint8 core_id, uint32 *prog_mem_addr, uint32 *data_mem_addr, uint32 *mcu_proc_len, uint32 **p_tsingma_mcu_proc)
{
    uint32 tmp_prog_mem_addr;
    uint32 tmp_data_mem_addr;
    uint32 tmp_mcu_proc_len;
    uint32 *p_tmp_tsingma_mcu_proc;
    
    switch(core_id)
    {
        case 0:
            tmp_prog_mem_addr = SYS_TSINGMA_MCU_SUP_PROG_MEM_0_ADDR;
            tmp_data_mem_addr = SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR;
            break;
        case 1:
            tmp_prog_mem_addr = SYS_TSINGMA_MCU_SUP_PROG_MEM_1_ADDR;
            tmp_data_mem_addr = SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR;
            break;
        case 2:
            tmp_prog_mem_addr = SYS_TSINGMA_MCU_SUP_PROG_MEM_2_ADDR;
            tmp_data_mem_addr = SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR;
            break;
        case 3:
            tmp_prog_mem_addr = SYS_TSINGMA_MCU_SUP_PROG_MEM_3_ADDR;
            tmp_data_mem_addr = SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR;
            break;
        default:
            return CTC_E_INIT_FAIL;
    }
    p_tmp_tsingma_mcu_proc = g_tsingma_mcu_firmware;
    tmp_mcu_proc_len = sizeof(g_tsingma_mcu_firmware)/sizeof(g_tsingma_mcu_firmware[0]);
    
    if(prog_mem_addr)       *prog_mem_addr      = tmp_prog_mem_addr;
    if(data_mem_addr)       *data_mem_addr      = tmp_data_mem_addr;
    if(mcu_proc_len)        *mcu_proc_len       = tmp_mcu_proc_len;
    if(p_tsingma_mcu_proc)  *p_tsingma_mcu_proc = p_tmp_tsingma_mcu_proc;
    
    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_clear_data_mem(uint8 lchip, uint32 data_mem_addr)
{
    uint32 cnt;
    uint32 max_cnt = SYS_TSINGMA_MCU_DATA_MEM_LEN;
    
//    max_cnt = 0x400;


    for (cnt = 0; cnt < max_cnt; cnt++)
    {
        sys_usw_mcu_chip_write(lchip, (data_mem_addr + cnt), 0);
    }

    
    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_download_program(uint8 lchip, uint32 *p_tsingma_mcu_proc, uint32 prog_mem_addr, uint32 mcu_proc_len)
{
    uint32 cnt;

    
    for (cnt = 0; cnt < mcu_proc_len + 1; cnt++)
    {
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (prog_mem_addr + cnt*4), p_tsingma_mcu_proc[cnt]));
    }

    
    return CTC_E_NONE;
}


int32
sys_tsingma_mcu_clear_port_info(uint8 lchip, uint16 lport)
{
    uint8  cnt;
    uint8  core_id           = 0;
    uint8  slice_id          = 0;
    uint32 port_pos          = 0;
    uint32 port_info_addr    = 0;
    uint32 mcu_port_data[4]  = {0};  /*16 bytes per port*/
    sys_datapath_lport_attr_t* port_attr  = NULL;
    
    /* 1. read info */
     /*sal_printf("sys_tsingma_mac_mcu_clear_port_info: lport %d\n", lport);*/
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if((NULL == port_attr) || (SYS_DMPS_NETWORK_PORT != port_attr->port_type))
    {
        return CTC_E_NONE;
    }
    core_id = sys_tsingma_mcu_get_core_id_by_tbl_id(lchip, port_attr->tbl_id);
     /*sal_printf("sys_tsingma_mac_mcu_clear_port_info: tbl_id %d, core_id %d\n", port_attr->tbl_id, core_id);    */
    switch(core_id)
    {
        case 0:
            port_info_addr = SYS_TSINGMA_MCU_0_PORT_INFO_BASE_ADDR;
            break;
        case 1:
            port_info_addr = SYS_TSINGMA_MCU_1_PORT_INFO_BASE_ADDR;
            break;
        case 2:
            port_info_addr = SYS_TSINGMA_MCU_2_PORT_INFO_BASE_ADDR;
            break;
        case 3:
        default:
            port_info_addr = SYS_TSINGMA_MCU_3_PORT_INFO_BASE_ADDR;
            break;
    }
    
    /* 2. write 0 */
    port_pos = sys_tsingma_mcu_get_port_pos(lchip, core_id, port_attr->tbl_id);
    port_info_addr += (port_pos*SYS_TSINGMA_MCU_PORT_ALLOC_BYTE);
    if(SYS_TSINGMA_MCU_MAX_PORT_PER_CORE == port_pos)
    {
        return CTC_E_INVALID_PARAM;
    }
    for(cnt = 0; cnt < 4; cnt++)
    {
        sys_usw_mcu_chip_write(lchip, port_info_addr, mcu_port_data[cnt]);
    }

    return CTC_E_NONE;
}

/*if core id is specified (spec_core_id != 0xff), but current lport not belong to it, return.
if caller not specify core id, any lport will be processed based on its own core*/
int32
sys_tsingma_mcu_write_port_info(uint8 lchip, uint16 lport, uint8 spec_core_id)
{
    uint8  cnt;
    uint8  num               = 0;
    uint8  core_id           = 0;
    uint8  slice_id          = 0;
     /*int32  ret               = 0;*/
    uint16 chip_port         = 0;
    uint16 serdes_id         = 0;
    uint32 port_pos          = 0;
    uint32 port_info_addr    = 0;
    sys_datapath_lport_attr_t* port_attr  = NULL;
    uint32 mcu_port_data[4]  = {0};
    
    /* 1. read info */
     /*sal_printf("sys_tsingma_mac_mcu_write_port_info: lport %d, drop_flag %d, spec_core_id %d\n", lport, drop_flag, spec_core_id);*/
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if((NULL == port_attr) || (SYS_DMPS_NETWORK_PORT != port_attr->port_type))
    {
        return CTC_E_NONE;
    }
    core_id = sys_tsingma_mcu_get_core_id_by_tbl_id(lchip, port_attr->tbl_id);
     /*sal_printf("sys_tsingma_mac_mcu_write_port_info: tbl_id %d, core_id %d\n", port_attr->tbl_id, core_id);    */
    if(core_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }
    if((spec_core_id != 0xff) && (spec_core_id != core_id))
    {
        return CTC_E_NONE;
    }
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
     /*if (CTC_E_NONE != ret)   //no serdes info means this port is invalid, will write all 0*/
     /*{*/
     /*    return CTC_E_NONE;*/
     /*}*/
     /*sal_printf("sys_tsingma_mac_mcu_write_port_info: chip_port %d, serdes_id %d, num %d, core_id %d\n", chip_port, serdes_id, num, core_id);*/
    
    switch(core_id)
    {
        case 0:
            port_info_addr = SYS_TSINGMA_MCU_0_PORT_INFO_BASE_ADDR;
            break;
        case 1:
            port_info_addr = SYS_TSINGMA_MCU_1_PORT_INFO_BASE_ADDR;
            break;
        case 2:
            port_info_addr = SYS_TSINGMA_MCU_2_PORT_INFO_BASE_ADDR;
            break;
        case 3:
        default:
            port_info_addr = SYS_TSINGMA_MCU_3_PORT_INFO_BASE_ADDR;
            break;
    }
    
    /* 2. collect info */
    MCU_SET_U8_FIELD(0, mcu_port_data[0], 1); //tsingma_mcu_port_attr_t.port_valid_flag
    MCU_SET_U8_FIELD(1, mcu_port_data[0], port_attr->mac_id); //tsingma_mcu_port_attr_t.mac_id
    MCU_SET_U8_FIELD(2, mcu_port_data[0], 0); //tsingma_mcu_port_attr_t.mac_en
    MCU_SET_U8_FIELD(3, mcu_port_data[0], 0); //tsingma_mcu_port_attr_t.link_stat
    
    MCU_SET_U8_FIELD(0, mcu_port_data[1], serdes_id); //tsingma_mcu_port_attr_t.serdes_id
    if ((CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode) && (1 == port_attr->flag))
    {
        if (0 == port_attr->mac_id % 4)
        {
            MCU_SET_U8_FIELD(1, mcu_port_data[1], serdes_id + 1); //tsingma_mcu_port_attr_t.serdes_id2
        }
        else
        {
            MCU_SET_U8_FIELD(1, mcu_port_data[1], serdes_id + 3); //tsingma_mcu_port_attr_t.serdes_id2
        }
    }
    MCU_SET_U8_FIELD(2, mcu_port_data[1], num); //tsingma_mcu_port_attr_t.serdes_num
    MCU_SET_U8_FIELD(3, mcu_port_data[1], port_attr->pcs_mode); //tsingma_mcu_port_attr_t.serdes_mode
    
    MCU_SET_U8_FIELD(0, mcu_port_data[2], port_attr->flag); //tsingma_mcu_port_attr_t.flag
    MCU_SET_U8_FIELD(1, mcu_port_data[2], port_attr->tbl_id); //tsingma_mcu_port_attr_t.tbl_id

    /* 3. write info */
    port_pos = sys_tsingma_mcu_get_port_pos(lchip, core_id, port_attr->tbl_id);
    port_info_addr += (port_pos*SYS_TSINGMA_MCU_PORT_ALLOC_BYTE);
    if(SYS_TSINGMA_MCU_MAX_PORT_PER_CORE == port_pos)
    {
        return CTC_E_INVALID_PARAM;
    }
    for(cnt = 0; cnt < 4; cnt++)
    {
        port_info_addr += (cnt*SYS_TSINGMA_MCU_WORD_BYTE);
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, port_info_addr, mcu_port_data[cnt]));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_write_serdes_info(uint8 lchip, uint8 inner_serdes_id, uint8 core_id)
{
    uint32 gport = 0;
    uint16 lport = 0;
    uint16 serdes_id = 8 * core_id + inner_serdes_id;
    uint32 serdes_info_addr = inner_serdes_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE;
    uint32 serdes_info = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    
    if(inner_serdes_id >= 8)
    {
        return CTC_E_INVALID_PARAM;
    }
    
    CTC_ERROR_RETURN(sys_usw_datapath_get_gport_with_serdes(lchip, serdes_id, &gport));
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if((port_attr->port_type != SYS_DMPS_NETWORK_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch(core_id)
    {
        case 0:
            serdes_info_addr += SYS_TSINGMA_MCU_0_SERDES_INFO_BASE_ADDR;
            break;
        case 1:
            serdes_info_addr += SYS_TSINGMA_MCU_1_SERDES_INFO_BASE_ADDR;
            break;
        case 2:
            serdes_info_addr += SYS_TSINGMA_MCU_2_SERDES_INFO_BASE_ADDR;
            break;
        case 3:
            serdes_info_addr += SYS_TSINGMA_MCU_3_SERDES_INFO_BASE_ADDR;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    MCU_SET_U8_FIELD(0, serdes_info, inner_serdes_id); //tsingma_mcu_serdes_attr_t.inner_serdes_id
    MCU_SET_U8_FIELD(1, serdes_info, sys_tsingma_mcu_get_port_pos(lchip, core_id, port_attr->tbl_id)); //tsingma_mcu_serdes_attr_t.port_pos
    MCU_SET_U8_FIELD(2, serdes_info, 1); //tsingma_mcu_serdes_attr_t.valid_flag
    MCU_SET_U8_FIELD(3, serdes_info, 0);
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, serdes_info_addr, serdes_info));

    return CTC_E_NONE;
}

/*
Set enable/disable of IEEE 802.3 clause 72 link training.
Two directions - train local and train remote are set enable/disable together.
*/
int32
sys_tsingma_mcu_set_cl72_enable(uint8 lchip, uint8 hss_id, uint8 lane_id, bool enable)
{
    uint32 serdes_info_addr = 0;
    uint32 serdes_info_data = 0;
    uint32 mode_ctl_addr    = 0;
    uint32 mode_ctl_val     = 0;
    uint32 intr_val  = 0;
    uint32 intr_addr = 0;
    uint8  rst_flag = 0;
    uint32 time_cnt = 10000;

    if(lane_id >= 8)
    {
        return CTC_E_INVALID_PARAM;
    }

    switch(hss_id)
    {
        case 0:
            serdes_info_addr = SYS_TSINGMA_MCU_0_SERDES_INFO_BASE_ADDR + lane_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE + 4;
            mode_ctl_addr    = SYS_TSINGMA_MCU_0_SW_MODE_CTL_ADDR;
            intr_addr = 0x00483340;
            break;
        case 1:
            serdes_info_addr = SYS_TSINGMA_MCU_1_SERDES_INFO_BASE_ADDR + lane_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE + 4;
            mode_ctl_addr    = SYS_TSINGMA_MCU_1_SW_MODE_CTL_ADDR;
            intr_addr = 0x00501340;
            break;
        case 2:
            serdes_info_addr = SYS_TSINGMA_MCU_2_SERDES_INFO_BASE_ADDR + lane_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE + 4;
            mode_ctl_addr    = SYS_TSINGMA_MCU_2_SW_MODE_CTL_ADDR;
            intr_addr = 0x00584340;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /*1. mask/unmask LT mcu interrupt*/
    if(enable)
    {
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, mode_ctl_addr, &mode_ctl_val));
        if(NORMAL_MODE != mode_ctl_val)
        {
            CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, mode_ctl_addr, NORMAL_MODE));
        }

        /*1.1 enable local FFE & iscan*/
        /*CTC_ERROR_RETURN(sys_tsingma_mcu_set_tx_adj_enable(lchip, hss_id, lane_id, TRUE));
        CTC_ERROR_RETURN(sys_tsingma_mcu_set_lt_initial_value(lchip, hss_id, lane_id));*/
        /*CTC_ERROR_RETURN(sys_tsingma_mcu_set_iscan_enable(lchip, hss_id, lane_id, TRUE));*/

        /*1.2 unmask interrupt*/
        intr_addr += SYS_TSINGMA_MCU_WORD_BYTE*4*3; //entry 3: unmask intr
        if(5 > lane_id)
        {
            intr_addr += 2*SYS_TSINGMA_MCU_WORD_BYTE; //word 2 for lane 0~4
            intr_val   = (uint32)(0x38 << lane_id * 6);
        }
        else
        {
            intr_addr += 3*SYS_TSINGMA_MCU_WORD_BYTE; //word 3 for lane 5~7
            intr_val   = (uint32)(0xe << (lane_id-5) * 6);
        }
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, intr_addr, intr_val));
    }
    else
    {
        /*mask interrupt*/
        intr_addr += SYS_TSINGMA_MCU_WORD_BYTE*4*2; //entry 2: mask intr
        if(5 > lane_id)
        {
            intr_addr += 2*SYS_TSINGMA_MCU_WORD_BYTE; //word 2 for lane 0~4
            intr_val   = (uint32)(0x38 << lane_id * 6);
        }
        else
        {
            intr_addr += 3*SYS_TSINGMA_MCU_WORD_BYTE; //word 3 for lane 5~7
            intr_val   = (uint32)(0xe << (lane_id-5) * 6);
        }
        CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, intr_addr, intr_val));
    }
    
    /*2. set train_remote_en and train_local_en
         for disable scene, first set lt_reset_flag as 1 */
    if(FALSE == enable)
    {
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, serdes_info_addr, &serdes_info_data));
        if(0 != (serdes_info_data & 0xffff)) //if train_remote_en and train_local_en is 0, do not set reset flag
        {
            CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, serdes_info_addr+0x14, &serdes_info_data));
            MCU_SET_U8_FIELD(3, serdes_info_data, 1);         //tsingma_mcu_serdes_attr_t.lt_reset_flag
            CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, serdes_info_addr+0x14, serdes_info_data));
        }
    }
    CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, serdes_info_addr, &serdes_info_data));
    MCU_SET_U8_FIELD(0, serdes_info_data, (uint8)enable); //tsingma_mcu_serdes_attr_t.train_remote_en
    MCU_SET_U8_FIELD(1, serdes_info_data, (uint8)enable); //tsingma_mcu_serdes_attr_t.train_local_en
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, serdes_info_addr, serdes_info_data));

    /*if disable, waiting for MCU clear restart flag*/
    if(FALSE == enable)
    {
        while(--time_cnt)
        {
            CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, serdes_info_addr+0x14, &serdes_info_data));
            MCU_GET_U8_FIELD(3, serdes_info_data, rst_flag);
            if(0 == rst_flag)
            {
                break;
            }
        }
        if(!time_cnt)
        {
            SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MCU restart timeout!\n");
            return CTC_E_HW_TIME_OUT;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_init_single_core(uint8 lchip, uint8 core_id)
{
    uint8  lport;
    uint8  inner_serdes_id;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    uint32 *p_tsingma_mcu_proc = NULL;
    uint32 mcu_proc_len = 0;
    uint32 prog_mem_addr = 0;
    uint32 data_mem_addr = 0;

    /*HSS28G process*/
    if(((0 == SDK_WORK_PLATFORM) && (0 == SDK_WORK_ENV)) && (3 == core_id))
    {
        return CTC_E_NONE;
    }

    /*HSS12G process*/
    tbl_id = McuSupInterruptNormal0_t + core_id;
    fld_id = McuSupInterruptNormal0_mcuSupDataMemEccError_f;
    tmp_val32 = 1;
    cmd = DRV_IOW(tbl_id, fld_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 2, cmd, &tmp_val32)); ///TODO: Is this entry right?

    tbl_id = McuSupRstCtl0_t + core_id;
    fld_id = McuSupRstCtl0_mcuSoftRst_f;
    cmd = DRV_IOR(tbl_id, fld_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_val32));
    if (!tmp_val32)
    {
        return CTC_E_NONE;
    }
    
    CTC_ERROR_RETURN(sys_tsingma_mcu_get_init_info(core_id, &prog_mem_addr, &data_mem_addr, &mcu_proc_len, &p_tsingma_mcu_proc));

    CTC_ERROR_RETURN(sys_tsingma_mcu_clear_data_mem(lchip, data_mem_addr));

    CTC_ERROR_RETURN(sys_tsingma_mcu_download_program(lchip, p_tsingma_mcu_proc, prog_mem_addr, mcu_proc_len));
    
    for(lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
    {
        sys_tsingma_mcu_write_port_info(lchip, lport, core_id);
    }
    for(inner_serdes_id = 0; inner_serdes_id < 8; inner_serdes_id++)
    {
        sys_tsingma_mcu_write_serdes_info(lchip, inner_serdes_id, core_id);
    }

    /* write mcu global info & sw enable */
    CTC_ERROR_RETURN(sys_tsingma_mcu_write_glb_info(lchip, core_id));
 
    tbl_id = McuSupRstCtl0_t + core_id;
    fld_id = McuSupRstCtl0_mcuSoftRst_f;
    tmp_val32 = 0;
    cmd = DRV_IOW(tbl_id, fld_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_val32));

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_init(uint8 lchip)
{
    uint8 core_id;
    sys_datapath_hss_attribute_t *p_hss = NULL;

    for(core_id = 0; core_id < SYS_TSINGMA_MCU_MAX_NUM; core_id++)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_get_hss_info(lchip, core_id, &p_hss));
        if (NULL == p_hss)
        {
            continue;;
        }
        if(CTC_E_NONE != sys_tsingma_mcu_init_single_core(lchip, core_id))
        {
            sal_printf("sys_tsingma_mac_mcu_init: MCU init fail! core_id %d\n", core_id);
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mcu_show_debug_info(uint8 lchip)
{
    uint8  core_id = 0;
    uint32 base_addr[SYS_TSINGMA_MCU_MAX_NUM]        = {SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR, 
                                                        SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR, 
                                                        SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR, 
                                                        SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR};
    uint32 port_info_addr[SYS_TSINGMA_MCU_MAX_NUM]   = {SYS_TSINGMA_MCU_0_PORT_INFO_BASE_ADDR,
                                                        SYS_TSINGMA_MCU_1_PORT_INFO_BASE_ADDR,
                                                        SYS_TSINGMA_MCU_2_PORT_INFO_BASE_ADDR,
                                                        SYS_TSINGMA_MCU_3_PORT_INFO_BASE_ADDR};
    uint32 serdes_info_addr[SYS_TSINGMA_MCU_MAX_NUM] = {SYS_TSINGMA_MCU_0_SERDES_INFO_BASE_ADDR,
                                                        SYS_TSINGMA_MCU_1_SERDES_INFO_BASE_ADDR,
                                                        SYS_TSINGMA_MCU_2_SERDES_INFO_BASE_ADDR,
                                                        SYS_TSINGMA_MCU_3_SERDES_INFO_BASE_ADDR};
    
    sys_datapath_hss_attribute_t *p_hss = NULL;
    
    uint32 mode_ctl = 0;
    //uint32 port_pos;
    uint32 serdes_pos;
    uint32 tmp_addr = 0;
    uint32 port_info[4] = {0};
    uint32 serdes_info[8] = {0};
    uint8  word_cnt = 0;
    uint32 addr = 0;
    char*  item[11] = {"Core ID", "Switch Controller", "Switch status", "QM choice", "Current Heartbeat", "Interrupt counter", 
                       "Software mode", "Test mode controller", "Test mode status", "Test lock success", "Test lock fail"};
    uint8  item_idx;
    //tsingma_mcu_port_attr_t port = {0};
    tsingma_mcu_serdes_attr_t serdes = {0};

    for(core_id = 0; core_id < (SYS_TSINGMA_MCU_MAX_NUM-1); core_id++)
    {
        /*if(0 != core_id)
        {
            continue;
        }*/
        CTC_ERROR_RETURN(sys_usw_datapath_get_hss_info(lchip, core_id, &p_hss));
        if (NULL == p_hss)
        {
            continue;;
        }
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n----------------------------------------- MCU %u info -----------------------------------------\n", core_id);
        for(item_idx = 0; item_idx < 11; item_idx++)
        {
            addr = ((0 == item_idx) ? (item_idx) : ((4<item_idx) ? (item_idx+2) : (item_idx+1))) * SYS_TSINGMA_MCU_WORD_BYTE
                   + base_addr[core_id];
            CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, addr, &mode_ctl));
            if(3 == item_idx)
            {
                continue;
            }
            if((6 == item_idx) && (TEST_MODE != mode_ctl))
            {
                break;
            }
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s: %u\n", item[item_idx], mode_ctl);
        }
#if 0
        /*port info*/
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nPort info: \n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-8s%-5s%-8s%-8s%-5s%-12s%-6s%-9s\n", "GPort", "MAC", "MAC-en", "SerDes", "Mode", "SerDes-Num", 
                        "Link", "TableId");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------\n");

        for (port_pos = 0; port_pos < SYS_TSINGMA_MCU_MAX_PORT_PER_CORE; port_pos++)
        {
            tmp_addr = port_info_addr[core_id]+port_pos*SYS_TSINGMA_MCU_PORT_ALLOC_BYTE;
            for(word_cnt = 0; word_cnt < 4; word_cnt++)
            {
                CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr, &port_info[word_cnt]));
                tmp_addr += SYS_TSINGMA_MCU_WORD_BYTE;
            }
            MCU_GET_U8_FIELD(0, port_info[0], port.port_valid_flag);
            if(0 == port.port_valid_flag)
            {
                continue;
            }
            MCU_GET_U8_FIELD(1, port_info[0], port.mac_id);
            MCU_GET_U8_FIELD(2, port_info[0], port.mac_en);
            MCU_GET_U8_FIELD(3, port_info[0], port.link_stat);
            MCU_GET_U8_FIELD(0, port_info[1], port.serdes_id);
            MCU_GET_U8_FIELD(1, port_info[1], port.serdes_id2);
            MCU_GET_U8_FIELD(2, port_info[1], port.serdes_num);
            MCU_GET_U8_FIELD(3, port_info[1], port.serdes_mode);
            MCU_GET_U8_FIELD(0, port_info[2], port.flag);
            MCU_GET_U8_FIELD(1, port_info[2], port.tbl_id);
            
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-8d%-5d%-8d%-8d%-5d%-12d%-6d%-9d\n", port.mac_id, port.mac_id, port.mac_en, 
                            port.serdes_id, port.serdes_mode, port.serdes_num, port.link_stat, port.tbl_id);
        }
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------\n\n");
#endif
        /*LT info*/
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Link training info: \n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------------\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-5s%-4s%-6s%-4s%-4s%-5s%-4s%-4s%-5s%-5s| %-5s%-5s%-4s%-4s%-5s%-5s%-6s%-5s%-4s%-4s\n", "Lane", "Stg", "CfVal", 
                        "Frm", "Age", "txmg", "dly", "adv", "CfUp", "CfSt",   "EnLT", "LdDn", "Inc", "ISC", "MISC", "ISCC", "LpSt0", "Stop", "Rst", "REn");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------------\n");

        for (serdes_pos = 0; serdes_pos < SYS_TSINGMA_MCU_MAX_SERDES_PER_CORE; serdes_pos++)
        {
            tmp_addr = serdes_info_addr[core_id]+serdes_pos*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE;
            for(word_cnt = 0; word_cnt < 8; word_cnt++)
            {
                CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, tmp_addr, &serdes_info[word_cnt]));
                tmp_addr += SYS_TSINGMA_MCU_WORD_BYTE;
                /*sal_printf("0x%x\n", serdes_info[word_cnt]);*/
            }
            MCU_GET_U8_FIELD(2, serdes_info[0], serdes.valid_flag);
            if(0 == serdes.valid_flag)
            {
                continue;
            }
            MCU_GET_U8_FIELD(0, serdes_info[0], serdes.inner_serdes_id);
            MCU_GET_U8_FIELD(3, serdes_info[0], serdes.lt_stage);
            MCU_GET_U8_FIELD(0, serdes_info[1], serdes.train_remote_en);
            MCU_GET_U8_FIELD(1, serdes_info[1], serdes.train_local_en);
            MCU_GET_U8_FIELD(2, serdes_info[1], serdes.loc_done_flag);
            MCU_GET_U8_FIELD(3, serdes_info[1], serdes.coeff_stat);
            MCU_GET_U8_FIELD(0, serdes_info[2], serdes.cur_loc_state);
            /*MCU_GET_U8_FIELD(2, serdes_info[2], serdes.rmt_rcv_cnt);
            MCU_GET_U8_FIELD(3, serdes_info[2], serdes.rmt_rcv_info);*/
            MCU_GET_U8_FIELD(0, serdes_info[3], serdes.coeff_val);
            MCU_GET_U8_FIELD(1, serdes_info[3], serdes.inc_flag);
            MCU_GET_U8_FIELD(2, serdes_info[3], serdes.max_iscan_val);
            MCU_GET_U8_FIELD(3, serdes_info[3], serdes.max_iscan_coeff);
            MCU_GET_U8_FIELD(0, serdes_info[4], serdes.txmargin);
            MCU_GET_U8_FIELD(1, serdes_info[4], serdes.pcs_tap_dly);
            MCU_GET_U8_FIELD(2, serdes_info[4], serdes.pcs_tap_adv);
            MCU_GET_U8_FIELD(3, serdes_info[4], serdes.lp_status0);
            serdes.loc_adj_value = serdes_info[5];
            MCU_GET_U8_FIELD(0, serdes_info[6], serdes.cur_iscan_val);
            MCU_GET_U8_FIELD(1, serdes_info[6], serdes.intr_lt_stop);
            MCU_GET_U8_FIELD(2, serdes_info[6], serdes.ageing_cnt);
            MCU_GET_U8_FIELD(3, serdes_info[6], serdes.lt_reset_flag);
            CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, (port_info_addr[core_id] + (serdes.port_pos)*SYS_TSINGMA_MCU_PORT_ALLOC_BYTE), 
                                                   &(port_info[0])));
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-5d%-4d%-6d%-4d%-4d%-5d%-4d%-4d0x%-3x0x%-3x| %-5d%-5d%-4d%-4d%-5d%-5d0x%-4x%-5d%-4d%-4d\n", serdes.inner_serdes_id, 
                            serdes.lt_stage, serdes.coeff_val, serdes.cur_loc_state, serdes.ageing_cnt, serdes.txmargin, serdes.pcs_tap_dly, serdes.pcs_tap_adv, 
                            serdes.loc_adj_value, serdes.coeff_stat,  serdes.train_local_en, serdes.loc_done_flag, serdes.inc_flag, serdes.cur_iscan_val, serdes.max_iscan_val, 
                            serdes.max_iscan_coeff, serdes.lp_status0, serdes.intr_lt_stop, serdes.lt_reset_flag, serdes.train_remote_en);
        }
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------------\n\n");
        
    }
    return CTC_E_NONE;
}


#ifdef _TSINGMA_MCU_DEBUG_
void sys_tsingma_dump_mcu_dbg_log(uint8 core_id)
{
    sal_file_t mcu_dbg_log = NULL;
    uint8  intr_flag = TRUE;
    uint32 mem_addr;
    uint32 value = 0;
    uint32 max_mem_addr;
    char   file_name[20];
    uint8  prv_err_code  = 0;
    uint32 mem_start_addr[SYS_TSINGMA_MCU_MAX_NUM] = {SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR+0x700,
                                                      SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR+0x700,
                                                      SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR+0x700,
                                                      SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR+0x700};
    
    sal_snprintf(file_name, 20, "./mcu_%u_dbg_log.csv", core_id);
    mcu_dbg_log = sal_fopen(file_name, "w+");
    if(NULL == mcu_dbg_log)
    {
        return;
    }

    max_mem_addr = 0xc100 + mem_start_addr[core_id];
    
    sys_usw_mcu_chip_read(0, mem_start_addr[core_id], &value);
    sal_fprintf(mcu_dbg_log, "Current interrupt log address: 0x%08x\n", value);
    sys_usw_mcu_chip_read(0, mem_start_addr[core_id]+0x2000, &value);
    sal_fprintf(mcu_dbg_log, "Current main loop log address: 0x%08x\n", value);
        
    sal_fprintf(mcu_dbg_log, "MCU ID,Intr flag,Address,Code,Heartbeat,Value,Key info\n");
    for(mem_addr = mem_start_addr[core_id]+SYS_TSINGMA_MCU_WORD_BYTE; mem_addr < max_mem_addr; mem_addr += SYS_TSINGMA_MCU_WORD_BYTE)
    {
        if(0x2000 == mem_addr - mem_start_addr[core_id])
        {
            intr_flag = FALSE;
            continue;
        }
        sys_usw_mcu_chip_read(0, mem_addr, &value);
        sal_fprintf(mcu_dbg_log, "%u,%u,0x%08x,%u,%u,%u,", 
                    core_id, intr_flag, mem_addr, 
                    (uint8)value, (uint16)(value>>8), (uint8)(value>>24));
        /*immportant info print*/
        switch((uint8)value)
        {
            case 15:
            case 16:
                sal_fprintf(mcu_dbg_log, "lane %u,", (uint8)(value>>24));
                break;
            case 6:
                if(5 == (uint8)(value>>24))
                {
                    sal_fprintf(mcu_dbg_log, "RX ready,");
                }
                break;
            case 27:
            case 28:
                if((uint8)value != prv_err_code)
                {
                    switch((uint8)(value>>24))
                    {
                        case 1:
                            sal_fprintf(mcu_dbg_log, "-1 inc,");
                            break;
                        case 2:
                            sal_fprintf(mcu_dbg_log, "-1 dec,");
                            break;
                        case 3:
                            sal_fprintf(mcu_dbg_log, "-1 unknow,");
                            break;
                        case 4:
                            sal_fprintf(mcu_dbg_log, "0 inc,");
                            break;
                        case 8:
                            sal_fprintf(mcu_dbg_log, "0 dec,");
                            break;
                        case 12:
                            sal_fprintf(mcu_dbg_log, "0 unknow,");
                            break;
                        case 16:
                            sal_fprintf(mcu_dbg_log, "+1 inc,");
                            break;
                        case 32:
                            sal_fprintf(mcu_dbg_log, "+1 dec,");
                            break;
                        case 48:
                            sal_fprintf(mcu_dbg_log, "+1 unknow,");
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    if(((28 == (uint8)value) && (1 == (uint16)(value>>8))) || 
                       ((27 == (uint8)value) && (16 == (uint16)(value>>8))))
                    {
                        sal_fprintf(mcu_dbg_log, "init,");
                    }
                    else if(((28 == (uint8)value) && (2 == (uint16)(value>>8))) || 
                            ((27 == (uint8)value) && (32 == (uint16)(value>>8))))
                    {
                        sal_fprintf(mcu_dbg_log, "preset,");
                    }
                }
                break;
            case 89:
            case 13:
                if((uint8)value != prv_err_code)
                {
                    switch((uint8)(value>>24))
                    {
                        case 1:
                            sal_fprintf(mcu_dbg_log, "-1 upd,");
                            break;
                        case 2:
                            sal_fprintf(mcu_dbg_log, "-1 min,");
                            break;
                        case 3:
                            sal_fprintf(mcu_dbg_log, "-1 max,");
                            break;
                        case 4:
                            sal_fprintf(mcu_dbg_log, "0 upd,");
                            break;
                        case 8:
                            sal_fprintf(mcu_dbg_log, "0 min,");
                            break;
                        case 12:
                            sal_fprintf(mcu_dbg_log, "0 max,");
                            break;
                        case 16:
                            sal_fprintf(mcu_dbg_log, "+1 upd,");
                            break;
                        case 32:
                            sal_fprintf(mcu_dbg_log, "+1 min,");
                            break;
                        case 48:
                            sal_fprintf(mcu_dbg_log, "+1 max,");
                            break;
                        case 21:
                            sal_fprintf(mcu_dbg_log, "all upd,");
                            break;
                        case 63:
                            sal_fprintf(mcu_dbg_log, "all max,");
                            break;
                        default:
                            break;
                    }
                }
                break;
            case 19:
                sal_fprintf(mcu_dbg_log, "RX ready,");
                break;
            default:
                sal_fprintf(mcu_dbg_log, "omit,");
                break;
        }

        sal_fprintf(mcu_dbg_log, "\n");

        prv_err_code  = (uint8)value;
    }
    
    sal_fclose(mcu_dbg_log);
}
#endif



