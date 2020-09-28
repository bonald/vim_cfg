/**
 @file sys_usw_mcu.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-9

 @version v2.0

*/

#ifndef _SYS_USW_MCU_H
#define _SYS_USW_MCU_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"


#define SYS_USW_DATA_MEM_ENTRY_NUM    15360
#define SYS_USW_PROG_MEM_ENTRY_NUM    16384
#define SYS_USW_DATA_MEM_BASE_ADDR0   0x000a0000
#define SYS_USW_PROG_MEM_BASE_ADDR0   0x00090000
#define SYS_USW_DATA_MEM_BASE_ADDR1   0x00060000
#define SYS_USW_PROG_MEM_BASE_ADDR1   0x00050000

/* #1, MCU global info */
#define SYS_USW_MCU_GLB_INFO_BASE_ADDR  (SYS_USW_DATA_MEM_BASE_ADDR0 + 0x0)
#define SYS_USW_MCU_VER_BASE_ADDR       (SYS_USW_MCU_GLB_INFO_BASE_ADDR + 0x0)    /* 4 Bytes */
#define SYS_USW_MCU_GLB_ON_BASE_ADDR    (SYS_USW_MCU_GLB_INFO_BASE_ADDR + 0x4)    /* 4 Bytes */

/* #2, MCU port data structure */
#define SYS_USW_MCU_PORT_DS_BASE_ADDR   (SYS_USW_DATA_MEM_BASE_ADDR0 + 1*0x400)
#define SYS_USW_MCU_PORT_ALLOC_BYTE     16

/* #3, MCU port other cfg/status */
#define SYS_USW_MCU_PORT_OTHER_BASE_ADDR   (SYS_USW_DATA_MEM_BASE_ADDR0 + 2*0x400)
#define SYS_USW_MCU_PORT_ON_BASE_ADDR      (SYS_USW_MCU_PORT_OTHER_BASE_ADDR + 0x0)   /* 64 Bytes, 1 Byte per port */
#define SYS_USW_MCU_PORT_LOG_BASE_ADDR     (SYS_USW_MCU_PORT_OTHER_BASE_ADDR + 0x40)  /* 256 Bytes, 4 Bytes per port */

/* #4, MCU global info dump unitest */
#define SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR   (SYS_USW_DATA_MEM_BASE_ADDR0 + 6*0x400)
//#define SYS_USW_MCU_UT_VER_BASE_ADDR        (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x0)
#define SYS_USW_MCU_UT_GLB_ON_BASE_ADDR     (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x0)
#define SYS_USW_MCU_UT_RUNLOOP_BASE_ADDR    (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x4)
#define SYS_USW_MCU_UT_15GFAIL_BASE_ADDR    (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x8)
#define SYS_USW_MCU_UT_28GFAIL_BASE_ADDR    (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0xc)
#define SYS_USW_MCU_UT_LOCK_TIMES           (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x10)
#define SYS_USW_MCU_UT_WADN_RUN_TEST        (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x14)
#define SYS_USW_MCU_UT_WAUP_RUN_TEST        (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x18)
#define SYS_USW_MCU_UT_LOCK_FAIL_TIMES      (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x1c)
#define SYS_USW_MCU_UT_UNLOCK_TIMES         (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x20)
#define SYS_USW_MCU_UT_UNLOCK_FAIL_TIMES    (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x24)
#define SYS_USW_MCU_WA_UP_PORT              (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x2c)
#define SYS_USW_MCU_WA_DN_PORT              (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x30)
#define SYS_USW_MCU_CURR_ROUND              (SYS_USW_MCU_UT_GLB_INFO_BASE_ADDR + 0x34)

/* #5, MCU port data structure dump unitest */
#define SYS_USW_MCU_UT_PORT_DS_BASE_ADDR    (SYS_USW_DATA_MEM_BASE_ADDR0 + 7*0x400)

/* #6, MCU port unittest */
#define SYS_USW_MCU_UT_BASE_ADDR            (SYS_USW_DATA_MEM_BASE_ADDR0 + 8*0x400)
#define SYS_USW_MCU_UT_PORT_BASE_ADDR(x)    (SYS_USW_MCU_UT_BASE_ADDR + 256*x)  /* 256 Bytes per port */
#define SYS_USW_MCU_UT_PORT_ON_ACK_OFST      0x0    /* 4 Bytes per port */
#define SYS_USW_MCU_UT_PORT_DBG_ON_OFST      0x4    /* 4 Bytes per port */
#define SYS_USW_MCU_UT_PORT_DBG_ON_ACK_OFST  0x8    /* 4 Bytes per port */
#define SYS_USW_MCU_UT_PORT_SIGDET_OFST      0xc    /* 4 Bytes per port */
#define SYS_USW_MCU_UT_PORT_LINK_STA_OFST    0x10   /* 4 Bytes per port */

#define SYS_USW_MCU_UT_PORT_CODEERR_ADDR_OFST   0x14   /* 20 Bytes per port, see _sys_usw_get_code_err_tbl_addr() */
#define SYS_USW_MCU_UT_PORT_LINK_ADDR_OFST      0x28   /* 4 Bytes per port */
#define SYS_USW_MCU_UT_PORT_PCS_ADDR_OFST       0x2c   /* 12 Bytes per port (addr + val + val)*/
#define SYS_USW_MCU_UT_PORT_FEC_ADDR_OFST       0x38   /* 12 Bytes per port (addr + val + val)*/
#define SYS_USW_MCU_UT_PORT_NETTX_CRFEDIT1      0x44   /* 4 Bytes per port */
#define SYS_USW_MCU_UT_PORT_NETTX_CRFEDIT2      0x48   /* 4 Bytes per port */

#define SYS_USW_MCU_UT_PORT_IGN_FT_OFST         0x4c   /* 12 Bytes per port(addr + val + val) */

/* #7, MCU WA test */
#define SYS_USW_MCU_WA_BASE_ADDR            (SYS_USW_DATA_MEM_BASE_ADDR0 + 24*0x400)
#define SYS_USW_MCU_WA_ROUND1_BASE_ADDR      SYS_USW_MCU_WA_BASE_ADDR


/*#################################################### TM start ##################################################*/
 /*TM macros*/
#define SYS_TSINGMA_MCU_MAX_NUM 4
#define SYS_TSINGMA_MCU_MAX_PORT_PER_CORE   32
#define SYS_TSINGMA_MCU_MAX_SERDES_PER_CORE 8
#define SYS_TSINGMA_MCU_WORD_BYTE           4
#define SYS_TSINGMA_MCU_PORT_ALLOC_BYTE     (4*SYS_TSINGMA_MCU_WORD_BYTE)
#define SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE   (8*SYS_TSINGMA_MCU_WORD_BYTE)
 /*#define SYS_TSINGMA_MCU_DATA_MEM_LEN        0x700        //bytes*/
#define SYS_TSINGMA_MCU_SUP_PROG_MEM_0_ADDR 0x004d0000   /*McuSupProgMem0*/
#define SYS_TSINGMA_MCU_SUP_PROG_MEM_1_ADDR 0x00550000   /*McuSupProgMem1*/
#define SYS_TSINGMA_MCU_SUP_PROG_MEM_2_ADDR 0x005d0000   /*McuSupProgMem2*/
#define SYS_TSINGMA_MCU_SUP_PROG_MEM_3_ADDR 0x002d0000   /*McuSupProgMem3*/
#define SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR 0x004e0000   /*McuSupDataMem0*/
#define SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR 0x00560000   /*McuSupDataMem1*/
#define SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR 0x005e0000   /*McuSupDataMem2*/
#define SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR 0x002e0000   /*McuSupDataMem3*/
#define SYS_TSINGMA_MCU_DATA_MEM_LEN        0xc800       /*bytes*/

/*
                           Tsingma MCU data memory structure
+----------------------------+---------------------------+----------------------------+
|    MCU core global info    |  port info (max 32 port)  | serdes info (max 8 serdes) |
+-------------------------------------------------------------------------------------+
+        1024 bytes          +       512 bytes           +         256 bytes          +
base addr                    0x400                       0x600                        0x700
*/
#define SYS_TSINGMA_MCU_0_CORE_ID_ADDR          (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x0)        /* MCU core id, 4 Bytes */
#define SYS_TSINGMA_MCU_0_VERSION_BASE_ADDR     (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x4)        /* MCU version info, 4 Bytes */
#define SYS_TSINGMA_MCU_0_SWITCH_CTL_ADDR       (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x8)        /* MCU core switch control, 4 Byte */
#define SYS_TSINGMA_MCU_0_SWITCH_STAT_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0xc)        /* MCU core switch status, 4 Byte */
#define SYS_TSINGMA_MCU_0_QM_CHOICE_ADDR        (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x10)       /* MCU port mac qm choice, g_qm_choice, 4 Byte */
#define SYS_TSINGMA_MCU_0_SW_MODE_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x20)       /* Software mode control, refer to mcu_sw_mode_t */
#define SYS_TSINGMA_MCU_0_TEST_MODE_CTL_ADDR    (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x24)       /* test mode control, written by CPU, read only for MCU */
#define SYS_TSINGMA_MCU_0_TEST_MODE_STAT_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x28)       /* test mode status, refer to mcu_test_mode_stat_t */
#define SYS_TSINGMA_MCU_0_TEST_LOCK_SUCC_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x2c)       /* test mode, lock success counter */
#define SYS_TSINGMA_MCU_0_TEST_LOCK_FAIL_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x30)       /* test mode, lock fail counter */
#define SYS_TSINGMA_MCU_0_TEST_MCU_LOCK_CNT     (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x34)       /* test mode, mcu lock number */
#define SYS_TSINGMA_MCU_0_DELAY_CYCLE_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x38)       /* Delay cycle */
#define SYS_TSINGMA_MCU_0_DBG_LOG_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x3c)       /* Debug log control */
#define SYS_TSINGMA_MCU_0_ISCAN_MODE_CTL        (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x40)       /* fast iscan mode control, default 0 (center height) */
#define SYS_TSINGMA_MCU_0_INIT_FFE_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x44)       /* LT init FFE: flag[7:0], txmg[15:8], adv[23:16], dly[31:24]*/
#define SYS_TSINGMA_MCU_0_RX_EQ_LT_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x48)       /* RX adjust after LT, 0-no RX eq, 1-RX eq after LT */
#define SYS_TSINGMA_MCU_0_ISCAN_MIN_VAL_THD     (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x4c)       /* minimun iscan fine value, any value less than that is equal to 0 */
#define SYS_TSINGMA_MCU_0_DFE_OPR_IND           (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 0x50)       /* DFE operation indicator, 0-no operation, 1-DFE enable, 2-DFE unhold */
#define SYS_TSINGMA_MCU_0_PORT_INFO_BASE_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_0_ADDR + 1*0x400)    /* port info address, 16 bytes/port */
#define SYS_TSINGMA_MCU_0_SERDES_INFO_BASE_ADDR (SYS_TSINGMA_MCU_0_PORT_INFO_BASE_ADDR + 0x200)    /* serdes info address, 32 bytes/serdes */

#define SYS_TSINGMA_MCU_1_CORE_ID_ADDR          (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x0)        /* MCU core id, 4 Bytes */
#define SYS_TSINGMA_MCU_1_VERSION_BASE_ADDR     (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x4)        /* MCU version info, 4 Bytes */
#define SYS_TSINGMA_MCU_1_SWITCH_CTL_ADDR       (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x8)        /* MCU core switch control, 4 Byte */
#define SYS_TSINGMA_MCU_1_SWITCH_STAT_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0xc)        /* MCU core switch status, 4 Byte */
#define SYS_TSINGMA_MCU_1_QM_CHOICE_ADDR        (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x10)       /* MCU port mac qm choice, g_qm_choice, 4 Byte */
#define SYS_TSINGMA_MCU_1_SW_MODE_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x20)       /* Software mode control, refer to mcu_sw_mode_t */
#define SYS_TSINGMA_MCU_1_TEST_MODE_CTL_ADDR    (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x24)       /* test mode control, written by CPU, read only for MCU */
#define SYS_TSINGMA_MCU_1_TEST_MODE_STAT_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x28)       /* test mode status, refer to mcu_test_mode_stat_t */
#define SYS_TSINGMA_MCU_1_TEST_LOCK_SUCC_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x2c)       /* test mode, lock success counter */
#define SYS_TSINGMA_MCU_1_TEST_LOCK_FAIL_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x30)       /* test mode, lock fail counter */
#define SYS_TSINGMA_MCU_1_TEST_MCU_LOCK_CNT     (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x34)       /* test mode, mcu lock number */
#define SYS_TSINGMA_MCU_1_DELAY_CYCLE_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x38)       /* Delay cycle */
#define SYS_TSINGMA_MCU_1_DBG_LOG_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x3c)       /* Debug log control */
#define SYS_TSINGMA_MCU_1_ISCAN_MODE_CTL        (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x40)       /* fast iscan mode control, default 0 (center height) */
#define SYS_TSINGMA_MCU_1_INIT_FFE_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x44)       /* LT init FFE: flag[7:0], txmg[15:8], adv[23:16], dly[31:24]*/
#define SYS_TSINGMA_MCU_1_RX_EQ_LT_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x48)       /* RX adjust after LT, 0-no RX eq, 1-RX eq after LT */
#define SYS_TSINGMA_MCU_1_ISCAN_MIN_VAL_THD     (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x4c)       /* minimun iscan fine value, any value less than that is equal to 0 */
#define SYS_TSINGMA_MCU_1_DFE_OPR_IND           (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 0x50)       /* DFE operation indicator, 0-no operation, 1-DFE enable, 2-DFE unhold */
#define SYS_TSINGMA_MCU_1_PORT_INFO_BASE_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_1_ADDR + 1*0x400)    /* port info address, 16 bytes/port */
#define SYS_TSINGMA_MCU_1_SERDES_INFO_BASE_ADDR (SYS_TSINGMA_MCU_1_PORT_INFO_BASE_ADDR + 0x200)    /* serdes info address, 32 bytes/serdes */

#define SYS_TSINGMA_MCU_2_CORE_ID_ADDR          (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x0)        /* MCU core id, 4 Bytes */
#define SYS_TSINGMA_MCU_2_VERSION_BASE_ADDR     (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x4)        /* MCU version info, 4 Bytes */
#define SYS_TSINGMA_MCU_2_SWITCH_CTL_ADDR       (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x8)        /* MCU core switch control, 4 Byte */
#define SYS_TSINGMA_MCU_2_SWITCH_STAT_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0xc)        /* MCU core switch status, 4 Byte */
#define SYS_TSINGMA_MCU_2_QM_CHOICE_ADDR        (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x10)       /* MCU port mac qm choice, g_qm_choice, 4 Byte */
#define SYS_TSINGMA_MCU_2_SW_MODE_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x20)       /* Software mode control, refer to mcu_sw_mode_t */
#define SYS_TSINGMA_MCU_2_TEST_MODE_CTL_ADDR    (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x24)       /* test mode control, written by CPU, read only for MCU */
#define SYS_TSINGMA_MCU_2_TEST_MODE_STAT_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x28)       /* test mode status, refer to mcu_test_mode_stat_t */
#define SYS_TSINGMA_MCU_2_TEST_LOCK_SUCC_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x2c)       /* test mode, lock success counter */
#define SYS_TSINGMA_MCU_2_TEST_LOCK_FAIL_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x30)       /* test mode, lock fail counter */
#define SYS_TSINGMA_MCU_2_TEST_MCU_LOCK_CNT     (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x34)       /* test mode, mcu lock number */
#define SYS_TSINGMA_MCU_2_DELAY_CYCLE_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x38)       /* Delay cycle */
#define SYS_TSINGMA_MCU_2_DBG_LOG_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x3c)       /* Debug log control */
#define SYS_TSINGMA_MCU_2_ISCAN_MODE_CTL        (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x40)       /* fast iscan mode control, default 0 (center height) */
#define SYS_TSINGMA_MCU_2_INIT_FFE_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x44)       /* LT init FFE: flag[7:0], txmg[15:8], adv[23:16], dly[31:24]*/
#define SYS_TSINGMA_MCU_2_RX_EQ_LT_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x48)       /* RX adjust after LT, 0-no RX eq, 1-RX eq after LT */
#define SYS_TSINGMA_MCU_2_ISCAN_MIN_VAL_THD     (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x4c)       /* minimun iscan fine value, any value less than that is equal to 0 */
#define SYS_TSINGMA_MCU_2_DFE_OPR_IND           (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 0x50)       /* DFE operation indicator, 0-no operation, 1-DFE enable, 2-DFE unhold */
#define SYS_TSINGMA_MCU_2_PORT_INFO_BASE_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_2_ADDR + 1*0x400)    /* port info address, 16 bytes/port */
#define SYS_TSINGMA_MCU_2_SERDES_INFO_BASE_ADDR (SYS_TSINGMA_MCU_2_PORT_INFO_BASE_ADDR + 0x200)    /* serdes info address, 32 bytes/serdes */

#define SYS_TSINGMA_MCU_3_CORE_ID_ADDR          (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x0)        /* MCU core id, 4 Bytes */
#define SYS_TSINGMA_MCU_3_VERSION_BASE_ADDR     (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x4)        /* MCU version info, 4 Bytes */
#define SYS_TSINGMA_MCU_3_SWITCH_CTL_ADDR       (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x8)        /* MCU core switch control, 4 Byte */
#define SYS_TSINGMA_MCU_3_SWITCH_STAT_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0xc)        /* MCU core switch status, 4 Byte */
#define SYS_TSINGMA_MCU_3_QM_CHOICE_ADDR        (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x10)       /* MCU port mac qm choice, g_qm_choice, 4 Byte */
#define SYS_TSINGMA_MCU_3_SW_MODE_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x20)       /* Software mode control, refer to mcu_sw_mode_t */
#define SYS_TSINGMA_MCU_3_TEST_MODE_CTL_ADDR    (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x24)       /* test mode control, written by CPU, read only for MCU */
#define SYS_TSINGMA_MCU_3_TEST_MODE_STAT_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x28)       /* test mode status, refer to mcu_test_mode_stat_t */
#define SYS_TSINGMA_MCU_3_TEST_LOCK_SUCC_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x2c)       /* test mode, lock success counter */
#define SYS_TSINGMA_MCU_3_TEST_LOCK_FAIL_CNT    (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x30)       /* test mode, lock fail counter */
#define SYS_TSINGMA_MCU_3_TEST_MCU_LOCK_CNT     (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x34)       /* test mode, mcu lock number */
#define SYS_TSINGMA_MCU_3_DELAY_CYCLE_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x38)       /* Delay cycle */
#define SYS_TSINGMA_MCU_3_DBG_LOG_CTL_ADDR      (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x3c)       /* Debug log control */
#define SYS_TSINGMA_MCU_3_ISCAN_MODE_CTL        (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x40)       /* fast iscan mode control, default 0 (center height) */
#define SYS_TSINGMA_MCU_3_INIT_FFE_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x44)       /* LT init FFE: flag[7:0], txmg[15:8], adv[23:16], dly[31:24]*/
#define SYS_TSINGMA_MCU_3_RX_EQ_LT_ADDR         (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 0x48)       /* RX eq before LT flag, 0-no RX eq, 1-RX eq before LT */
#define SYS_TSINGMA_MCU_3_PORT_INFO_BASE_ADDR   (SYS_TSINGMA_MCU_SUP_DATA_MEM_3_ADDR + 1*0x400)    /* port info address, 16 bytes/port */
#define SYS_TSINGMA_MCU_3_SERDES_INFO_BASE_ADDR (SYS_TSINGMA_MCU_3_PORT_INFO_BASE_ADDR + 0x200)    /* serdes info address, 32 bytes/serdes */


struct  sys_usw_mcu_port_attr_s
{
    uint8 mac_en;            /* mac enable config */
    uint8 mac_id;
    uint8 serdes_id;
    uint8 serdes_id2;    /* if 50G and lane swap, serdes_id2 is the second serdes ID, otherwise, this is 0 */
    
    uint8 serdes_num;    /*serdes num of port*/
    uint8 serdes_mode;      /*refer to ctc_chip_serdes_mode_t*/
    uint8 flag;       /* for D2 50G, if serdes lane 0/1 and 2/3 form 50G port, flag is eq to 0;
                                     if serdes lane 2/1 and 0/3 form 50G port, flag is eq to 1; */
    uint8 unidir_en;   /* unidir en config */

    uint8 rsv1;    /* make sure here reserved! */
    uint8 an_en;
    uint8 rsv[6];
};
typedef struct  sys_usw_mcu_port_attr_s sys_usw_mcu_port_attr_t;


extern int32 sys_usw_mcu_chip_read(uint8 lchip, uint32 offset, uint32* p_value);
extern int32 sys_usw_mcu_chip_write(uint8 lchip, uint32 offset, uint32 value);
extern int32 sys_usw_mac_mcu_init(uint8 lchip);
extern int32 sys_usw_mac_set_mcu_enable(uint8 lchip, uint32 enable);
extern int32 sys_usw_mac_get_mcu_enable(uint8 lchip, uint32* enable);
extern int32 sys_usw_mac_set_mcu_port_enable(uint8 lchip, uint32 gport, uint32 enable);
extern int32 sys_usw_mac_get_mcu_port_enable(uint8 lchip, uint32 gport, uint32* enable);

#ifdef __cplusplus
}
#endif

#endif
