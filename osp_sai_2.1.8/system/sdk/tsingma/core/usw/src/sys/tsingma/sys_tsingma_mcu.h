/**
 @file sys_tsingma_mcu.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2018-03-19

 @version v1.0

*/

#ifndef _SYS_TSINGMA_MCU_H
#define _SYS_TSINGMA_MCU_H
#ifdef __cplusplus
extern "C" {
#endif

/*
MCU_SET_U8_FIELD: Set given byte of a u32 value. Used for writting operation.
MCU_GET_U8_FIELD: Get given byte of a u32 value. Used for reading operation.
byte_n:    Byte number of u32_val, ranges 0~3.
u32_val:   U32 value to be operated.
u32 value: One byte of value. In SET operation it stands for value to be set, 
           while In GET it stands for the container of requested value.
*/
#define MCU_SET_U8_FIELD(byte_n, u32_val, u8_val) ((u32_val) = ((~(0xff << (8*(byte_n)))) & (u32_val)) | ((u8_val) << (8*(byte_n))))
#define MCU_GET_U8_FIELD(byte_n, u32_val, u8_val) ((u8_val) = (uint8)(((u32_val)>>(8*(byte_n))) & 0xff))
#define MCU_28G_GET_UINT32(p_base) ((uint32)((*(p_base))<<24) + (uint32)((*(p_base+1))<<16) + (uint32)((*(p_base+2))<<8) + (uint32)(*(p_base+3)))
#define MCU_28G_GET_UINT16(p_base) ((uint16)((*(p_base))<<8) + (uint16)(*(p_base+1)))

struct  tsingma_mcu_port_attr_s
{
    uint8 port_valid_flag;  /* 1 - port is valid (devided from mac_en)  0 - none mode port */
    uint8 mac_id;           /* port mac id */
    uint8 mac_en;           /* mac enable config */
    uint8 link_stat;        /* 1 - port link up  0 - port link down */
    
    uint8 serdes_id;        /* port serdes id, for multi-serdes port, it indicates the first serdes lane */
    uint8 serdes_id2;       /* if 50G and lane swap, serdes_id2 is the second serdes ID, otherwise, this is 0 */
    uint8 serdes_num;       /* serdes num of port */
    uint8 serdes_mode;      /* refer to ctc_chip_serdes_mode_t */
    
    uint8 flag;             /* for D2 50G, if serdes lane 0/1 and 2/3 form 50G port, flag is eq to 0; 
                               if serdes lane 2/1 and 0/3 form 50G port, flag is eq to 1; */
    uint8 tbl_id;           /* mac table id */
    uint8 rsv[6];
};
typedef struct  tsingma_mcu_port_attr_s tsingma_mcu_port_attr_t;

struct  tsingma_mcu_serdes_attr_s
{
    uint8  inner_serdes_id;  /* inner serdes id, 0~7 */
    uint8  port_pos;         /* port info position in data mem */
    uint8  valid_flag;       /* 1 - this serdes lane info is valid  0 - this is an idle serdes lane */
    uint8  lt_stage;         /* GUC link training stage, refer to mcu_lt_stage_t */
    
    uint8  train_remote_en;  /* 1 - enable train remote  0 - disable train remote */
    uint8  train_local_en;   /* 1 - enable train local  0 - disable train local */
    uint8  loc_done_flag;    /* train local all done flag (not train ok)  1 - all actions of train local are done */
    uint8  coeff_stat;       /* current LP coefficient status value (this lane is LP) */
    
    uint8  cur_loc_state;    /* current LD coefficient update flow state (this lane is LD) */
    uint8  prv_loc_state;    /* previous LD coefficient update flow state (this lane is LD) */
    uint8  rmt_rcv_cnt;      /* LP receiving UpdateReq info counter (this lane is LP) */
    uint8  rmt_rcv_info;     /* LP current received UpdateReq info content (this lane is LP) */

    uint8  coeff_val;        /* coefficient_adj or coefficient_dly */
    uint8  inc_flag;         /* increase/decrease of coeff_val flag */
    uint8  max_iscan_val;    /* max eyescan value found in c-1 or c+1 adjust flow */
    uint8  max_iscan_coeff;  /* max eyescan value corresponding coeff_val */
    
    uint8  txmargin;         /* (FOR EMU) pcs2pma_txmargin value */
    uint8  pcs_tap_dly;      /* (FOR EMU) pcs_tap_dly value */
    uint8  pcs_tap_adv;      /* (FOR EMU) pcs_tap_adv value */
    uint8  lp_status0;       /* LP coefficient status (this lane is LD) */
    
    uint32 loc_adj_value;    /* train local adjust value */

    uint8  cur_iscan_val;    /* current eyescan value */
    uint8  intr_lt_stop;     /* record train OK/Fail intr (refer to mcu_lt_intr_train_stop_t), then processed in main loop */
    uint8  ageing_cnt;       /* frame state ageing counter  just used in state FRAME_AGENT_STAT_RPT_WAIT */
    uint8  lt_reset_flag;    /* LT reset flag, set 1 by CPU when LT disable happens, then cleared by MCU */

    uint8  rsv[4];
};
typedef struct  tsingma_mcu_serdes_attr_s tsingma_mcu_serdes_attr_t;


enum tsingma_mcu_sw_mode_e
{
    IDLE_MODE   = 0,               //run nothing but heartbeat
    TEST_MODE   = 1,               //run test function to validate basic I/O
    NORMAL_MODE = 2,               //run normal function to do LT
    SW_MODE_BUTT
};
typedef enum tsingma_mcu_sw_mode_e tsingma_mcu_sw_mode_t;

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"

extern int32 sys_tsingma_mcu_init(uint8 lchip);
extern int32 sys_tsingma_mcu_write_glb_info(uint8 lchip, uint8 mcu_num);
extern int32 sys_tsingma_mcu_get_enable(uint8 lchip, uint32* enable, uint8 mcu_num);
extern int32 sys_tsingma_mcu_set_port_link_stat(uint8 lchip, uint16 lport, uint32 enable);
extern uint32 sys_tsingma_mcu_get_port_pos(uint8 lchip, uint8 core_id, uint8 tbl_id);
extern int32 sys_tsingma_mcu_write_port_info(uint8 lchip, uint16 lport, uint8 spec_core_id);
extern uint8 sys_tsingma_mcu_get_core_id_by_tbl_id(uint8 lchip, uint8 tbl_id);
extern int32 sys_tsingma_mcu_write_serdes_info(uint8 lchip, uint8 inner_serdes_id, uint8 core_id);
extern int32 sys_tsingma_mcu_clear_port_info(uint8 lchip, uint16 lport);
extern int32 sys_tsingma_mcu_show_debug_info(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif
