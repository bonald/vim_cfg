/****************************************************************************
* $Id$
*  The header file of the epld api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Jian Zhu
* Date          : 2008-07-21 
* Reason        : First Create.
****************************************************************************/
#ifndef __EPLD_API_H__
#define __EPLD_API_H__
#include "epld_drv.h"
#include "glb_hw_define.h"

#define MAX_EPLD_NUM 2
#define CTC_EPLD_FIRST 0

/* EPLD register list */
#define CTC_EPLD_BASE 0x300

#define EPLD_SPI_CLK_BIT      0x2000                  /*PD18*/
#define EPLD_SPI_DO_BIT       0x800                  /*PD20*/
#define EPLD_SPI_DI_BIT       0x1000                   /*PD19*/
#define EPLD_SPI_CS_BIT       0x100                   /*PC23*/

/********* common register ***********/
/* HW should make sure meaning of these register at all product series */

#define CTC_EPLD_CPU_MODE               0x00
#define CTC_EPLD_BOARD_SLOT             0x01
#define CTC_EPLD_BOARD_TYPE_VERSION     0x02
#define CTC_EPLD_EPLD_VERSION           0x03
#define CTC_EPLD_FPGA_TYPE              0x04
#define CTC_EPLD_TCAM_SRAM_TYPE         0x06

#define CTC_EPLD_ACT_STATUS             0x0A
#define CTC_EPLD_RUN_STATUS             0x0B

#define CTC_EPLD_SFP_PRESENT            0x13
#define CTC_EPLD_LIU_ONLINE_STATUS1     0x1A
#define CTC_EPLD_LIU_ONLINE_STATUS2     0x1B
#define CTC_EPLD_LIU_RESET1             0x12
#define CTC_EPLD_LIU_RESET2             0x13

/* Only used for EX3000 */
#define CTC_EPLD_LC_SYSTEM_STATUS     0x09

#define CTC_EPLD_BAY0_RST     0x38
#define CTC_EPLD_BAY1_RST     0x3C
#define CTC_EPLD_PHY_RST      0x40
#define CTC_EPLD_PARTS_RST    0x44

#define RESET_BAY          0x00
#define RESET_TCAM         0x01
#define RELEASE_CORE_RESET      0x02
#define RELEASE_TCAM_RESET      0x03

typedef enum
{
    BITWIDTH_BYTE = 1,
    BITWIDTH_WORD = 2,
    BITWIDTH_DWORD = 4,
    BITWIDTH_LONG = 8,
    BITWIDTH_MAX
} reg_width_e;

typedef struct epld_psu_status_s
{
    int8 psu_fan;
    int8 psu_mode;
    int8 psu_work_status;
    int8 psu_absent;
    int8 psu_shutdown;
    int8 psu_alert; /* modified by liuht for bug 24525,2013-10-24 */
}epld_psu_status_t;

typedef enum
{
    BUZZER_NORMAL,          /*no alarm*/
    BUZZER_L_ITVL_ALARM,    /*alarm for long interval*/
    BUZZER_S_ITVL_ALARM,    /*alarm for short interval*/
    BUZZER_ALARM,           /*alarm always*/
}buzzer_alarm_type_e;

enum epld_reg
{
    EPLD_CPU_MODE = 0,
    EPLD_BOARD_SLOT,
    EPLD_BOARD_TYPE,
    EPLD_BOARD_VERSION,
    EPLD_BOARD_SERIES,
    EPLD_EPLD_VERSION,
    EPLD_FPGA_TYPE,
    EPLD_SRAM_TYPE,
    EPLD_TCAM_TYPE,
    EPLD_PHY_RST,
    /*added by liuht for e350 48+12,2013-11-05*/
    EPLD_QSFP_RST,
    EPLD_L2SWITCH_RST,
    EPLD_SYS_CTRL_RESET_BOARD,    
    EPLD_SYS_CTRL_WATCHDOG_FEED,//20
    EPLD_SYS_CTRL_RESET_FLASH,
    EPLD_SYS_CTRL_PROTECT_FLASH,
    EPLD_SYS_LED_SWITCH,
    EPLD_SYS_LED_ACT_STAND,
    EPLD_SYS_LED_ALARM,
    EPLD_SYS_LED_RUN,
    EPLD_SYS_LED_SYS,
    EPLD_SYS_STAT_BUZZER,
    EPLD_ACT_STAT_STAT,
    EPLD_ACT_STAT_IN,
    EPLD_ACT_STAT_OUT,
    EPLD_RUN_STAT_ACT_CLR,
    EPLD_RUN_STAT_RUN_OUT_CTRL,
    EPLD_RUN_STAT_RST_OUT_CTRL,
    EPLD_TEST,
    EPLD_SFP_PRESENT1,    
    EPLD_SFP_PRESENT2,    
    EPLD_SFP_PRESENT3,    
    EPLD_SFP_PRESENT4,    
    EPLD_SFP_PRESENT5,
    EPLD_SFP_PRESENT6,
    EPLD_SFP_PRESENT7,
    EPLD_SFP_DISABLE1,
    EPLD_SFP_DISABLE2,
    EPLD_SFP_DISABLE3,
    EPLD_SFP_DISABLE4,
    EPLD_SFP_DISABLE5,
    EPLD_SFP_DISABLE6,
    EPLD_SFP_DISABLE7,
    /* Added by qicx for GB Demo: 2012-11-28 */
    EPLD_SFP_LOS1,
    EPLD_SFP_LOS2,
    EPLD_SFP_LOS3,
    EPLD_SFP_LOS4,
    EPLD_SFP_LOS5,
    EPLD_SFP_LOS6,

    EPLD_PSU_WORK_STATUS,
    EPLD_PSU_FAN_STATUS,
    EPLD_PSU_TYPE,
    EPLD_PSU_PRESENT_STATUS, 
    EPLD_PSU_SHUTDOWN,
    EPLD_PSU_POE_WORK_STATUS,
    EPLD_PSU_ALERT_STATUS,  /* added by qicx for E350, 2013-06-05 */
	EPLD_PSU_BRIDGE_RST,

    EPLD_FAN_STATUS,
    EPLD_FAN_POWER_SWITCH,
    EPLD_FAN_MODULE0_TYPE,
    EPLD_FAN_MODULE0_POWER_CTRL,
    EPLD_FAN_MODULE0_PRESENT,
    EPLD_FAN_MODULE0_STATUS,
    EPLD_FAN_MODULE0_SET_SPEED,  /* Added by liuht for bug26671, 2014-02-14 */
    EPLD_FAN_MODULE0_GET_SPEED1_HIGH,
    EPLD_FAN_MODULE0_GET_SPEED1_LOW,
    EPLD_FAN_MODULE0_GET_SPEED2_HIGH,
    EPLD_FAN_MODULE0_GET_SPEED2_LOW,
    EPLD_FAN_MODULE0_GET_SPEED3_HIGH,
    EPLD_FAN_MODULE0_GET_SPEED3_LOW,  /* Added by liuht for bug26671, 2014-02-14 */
    EPLD_FAN_MODULE0_VER,

    EPLD_EXT_SLOT_PRESENT,
    EPLD_EXT_SLOT_SHUTDOWN,
    EPLD_EXT_SLOT_TYPE,
    EPLD_EXT_SLOT_VER,
    EPLD_EXT_SLOT_VER_SWT,

    EPLD_SYS_CF_RESET,
    EPLD_SYS_FLASH_RESET,
    EPLD_SYS_VSC7395_RESET,
    EPLD_SYS_KSZ8041_RESET,

    EPLD_ALARM_SLOT_1_4, //for 6 slot chassis 3-6, for 14 slot chassis 1-4
    EPLD_ALARM_SLOT_5_8, //for 14 slot chassis 5,6,9,10
    EPLD_ALARM_SLOT_9_12,//for 14 slot chassis, 11-14

    EPLD_RUN_SLOT_1_4, //for 6 slot chassis 3-6, for 14 slot chassis 1-4
    EPLD_RUN_SLOT_5_8, //for 14 slot chassis 5,6,9,10
    EPLD_RUN_SLOT_9_12,//for 14 slot chassis, 11-14


    EPLD_PWR2_CTRL_OUT,
    EPLD_PWR1_CTRL_OUT,

    EPLD_ACT_INT_STATUS,
    EPLD_OTHER_SUP_STATUS,
    EPLD_ACT_OUTPUT_STATUS,

    EPLD_LC_PRESENT_INT_CLEAR,
    EPLD_HOT_SWAP_INT_CLEAR,
    EPLD_ACT_CLEAR_CTL,
    EPLD_RUN_STATUS,

    EPLD_FAN_CTRL_OUT,
    EPLD_CF_IDE_LED_N,
    EPLD_BUZZER,
    EPLD_PWR_ON_CTRL_SUP,
    EPLD_LC_PWR_ON_SLOT_A,
    EPLD_LC_PWR_ON_SLOT_B,

    EPLD_VOLTAGE_MONITOR_V25_MIN,
    EPLD_VOLTAGE_MONITOR_V18_MIN,
    EPLD_VOLTAGE_MONITOR_V12_MIN,
    EPLD_VOLTAGE_MONITOR_V25_MAX,
    EPLD_VOLTAGE_MONITOR_V18_MAX,
    EPLD_VOLTAGE_MONITOR_V12_MAX,

    EPLD_WATCH_DOG_EPLD_FEED,
    EPLD_SYS_RESET,
    EPLD_WATCH_DOG_TYPE,
    EPLD_SYS_BOOT_UP_STATUS,

    EPLD_LC_PRESENT_SLOT_A,
    EPLD_LC_PRESENT_SLOT_B,
    
    EPLD_REPLICATOR_MUX,
    EPLD_REPLICATOR_LPB_EN,
    EPLD_REPLICATOR_LINE_PRE_CTL,
    EPLD_REPLICATOR_SWITCH_PRE_CTL,

    /*HUMBER DEMO BOARD*/
    EPLD_HUMBER_PCI_RST_N,
    EPLD_DL_RST,
    EPLD_SU_RST,
    EPLD_RST_B,
    EPLD_S_RST_L,
    EPLD_C_RST_L,
    EPLD_VSC7395_RST,
    EPLD_AD9517_RST,
    EPLD_I2C_BRIDGE_RST,
    EPLD_GG_I2C_BRIDGE_RST,
    EPLD_ASIC_I2C_BRIDGE_RST,
    EPLD_XGPHY_RST,
    EPLD_XGPHY_RST1,
    EPLD_XGPHY_RST2,
    EPLD_CPUPHY_RST,
    EPLD_XGPHY_SMI_SWITCH,
    EPLD_XG_CARD_TYPE,

    /* GreatBelt Demo slot card */
    EPLD_GB_PCIE_RST,
    EPLD_GB_POR_RST,
    EPLD_GB_HARD_RST,

    /*add bu guhg,2015-12-01,for EPLD simulate i2c bus*/
    EPLD_I2C_CTRL,
    EPLD_I2C_STATUS,
    EPLD_I2C_ADDR,
    EPLD_I2C_WR_DATA_H,
    EPLD_I2C_WR_DATA_L,
    EPLD_I2C_RD_DATA_H,
    EPLD_I2C_RD_DATA_L,
    
    EPLD_EPLD_STATUS_LED,
    EPLD_SYNCE_STATUS_LED,
    
    EPLD_SLOT0_TYPE,
    EPLD_SLOT1_TYPE,
    EPLD_SLOT2_TYPE,
    
    EPLD_SLOT_PHY_RST,
    EPLD_SI5325_RST,
    EPLD_SI5325_RST_3,
    EPLD_SI5317_RST,
    EPLD_SDPLL_RST,
    EPLD_BIT1_RST,
    EPLD_BIT2_RST,
    EPLD_VSC3308_MB_RST,
    EPLD_VSC3308_SLOT_RST,
    EPLD_VSC8211_RST1,
    EPLD_VSC8211_RST2,
    EPLD_PEX8604_RST,
    EPLD_SERIAL_LED_RST,
    EPLD_USBPHY_RST,
    
    EPLD_SDPLL_INT_MASK,
    EPLD_VSC8211_INT1_MASK,
    EPLD_VSC8211_INT2_MASK,
    EPLD_SLOT0_PHY_INT,
    EPLD_SLOT0_PHY_INT_MASK,
    EPLD_SLOT1_PHY_INT,
    EPLD_SLOT1_PHY_INT_MASK,
    EPLD_SLOT2_PHY_INT,
    EPLD_SLOT2_PHY_INT_MASK,
    
    EPLD_CPU_SMI_MUX_SEL,
    EPLD_XFP_POW_DOWN,
    
    /*added by jcao */
    EPLD_PHY_INT_MASK1,
    EPLD_PHY_INT_MASK2,
    EPLD_XGPHY_INT_MASK, /*modified by jqiu for bug14876. 2011-05-06 */
    EPLD_DPLL_INT_MASK,
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-08 */
    EPLD_RST_FOAM_GLB,
    EPLD_RST_FOAM_PCI,
#endif /*FOAM_SUPPORT*/
    EPLD_FAN_LED,
    EPLD_PSU1_LED,
    EPLD_PSU2_LED,
    EPLD_POE_RST,
    EPLD_POE_DISABLE_PORT,   
    EPLD_POE_INT_MASK,
    EPLD_POE_PRESENT,
    EPLD_POE_READY,

    /* added by qicx for NID, 2013-11-22 */
    EPLD_LED_TEST,
    EPLD_LED_OAM,
    EPLD_LED_MAJ,
    EPLD_LED_CRI,

    EPLD_SLOT_I2C_CHANNEL_ENABLE,
    EPLD_SFP_I2C_CHANNEL_ENABLE,
#ifdef GB_DEMO_BOARD
    EPLD_PORT_SET_TOD_MOD, /*add by wangl for tod interface*/
#endif
#ifdef PTN_722_BOARD
    EPLD_PORT_SET_TOD_MOD_PTN722,
    EPLD_SLOT_RCVCLK_SEL_PTN722,
#endif

    EPLD_FAST_LINK1,
    EPLD_FAST_LINK2,
    EPLD_FAST_LINK3,
    EPLD_FAST_LINK4,
    EPLD_FAST_LINK5,
    EPLD_FAST_LINK6,
    EPLD_FAST_LINK_MASK1,
    EPLD_FAST_LINK_MASK2,
    EPLD_FAST_LINK_MASK3,
    EPLD_FAST_LINK_MASK4,
    EPLD_FAST_LINK_MASK5,
    EPLD_FAST_LINK_MASK6,
    
    EPLD_POWER_INTR_MASK, /* Added by liuht for bug 27035, 2014-03-10 */
    EPLD_RESET_INT_MASK,    
    EPLD_PORT_LED_RST,
    EPLD_GB_INT_MASK,
    EPLD_DUBLIN_PORT1_LED,
    EPLD_DUBLIN_PORT2_LED,
    EPLD_DUBLIN_PORT3_LED,
    EPLD_DUBLIN_PORT4_LED,

    EPLD_REBOOT_POWER_RST,  /* Added by liuht for bug27036, 2014-02-27 */
    EPLD_REBOOT_MANUAL_OTHER_RST,  /* Added by liuht for bug27036, 2014-02-27 */
    EPLD_PORT_FLF_STAT,
    EPLD_PORT1_FLF_MASK,
    EPLD_PORT2_FLF_MASK,
    EPLD_PORT3_FLF_MASK,
    EPLD_PORT4_FLF_MASK,

    EPLD_STACKING_LED,
    EPLD_GPIO_RST,
    EPLD_GPIO_EN,
    EPLD_POWER_DOWN,
    EPLD_IND_LED,
    EPLD_SVB_STATUS, /* for svb status, needed by power the ASIC */
    EPLD_MUX1_RST,
    EPLD_MUX2_RST,
    EPLD_FAN_EN,
    EPLD_INT_STAT1,/* for phy intr */
    EPLD_INT_STAT2,
    EPLD_I2C_SLAVE,
    EPLD_I2C_MASTER,
    EPLD_I2C_EN,
    EPLD_TXLNK_STAT,
    EPLD_PORT_SPLIT,
    EPLD_DPLL_CS,
    EPLD_DPLL_RST,
    EPLD_PPU_INT_MASK,
    EPLD_QSFP_INT_MASK,
    EPLD_FIBER_I2C_SCL_SWITCH1,/* for new fiber control */
    EPLD_FIBER_I2C_SCL_SWITCH2,/* for new fiber control */

    EPLD_PHY_COMMBO_STAT0,/* for hf board */
    EPLD_XG_SPEED,/* for hf board */
    EPLD_BYPASS,/* for hf board */
    EPLD_RCV_CLK_SEL_REG, /* Added by qianj for T65c, 2019-04-26 */

    EPLD_REG_MAX_NUM //This must less than EPLD_REG_DESC_MAXNUM
};

typedef enum
{
    EPLD_UPGRADE_MODE_VME,
    EPLD_UPGRADE_MODE_JAM,
    EPLD_UPGRADE_MODE_NULL,
}epld_upgrade_mode_t;

typedef enum
{
    EPLD_EXT_SLOT_SWT_NEW_VERSION,
    EPLD_EXT_SLOT_SWT_OLD_VERSION,    
}epld_ext_slot_switch_version_e;

typedef struct epld_hw_info_s
{
    glb_board_type_t    board_type;                         /* board type */
    int16         valid;              
    int16         epld_start_version;                 /* start version of epld */
    int16         epld_end_version;                   /* end version of epld */
    int16         epld_reg_index;                     /* index to the reg struct */
} epld_hw_info_t;

typedef struct epld_para_s{
    uint32 addr;    /* the address in epld to be access */
    uint32 val;        /* the value W/R from the filed of the address*/
} epld_para_t;

typedef struct epld_diag_result_s{
    uint8 rd_fail;
    uint8 wr_fail;
    uint8 cmp_fail;
} epld_diag_result_t;

epld_info_t *epld_get_info(glb_board_type_t *board_type, uint8 epld_version);
int32 epld_set_reg_desc(struct epld_reg_s * reg_desc, uint8 reg_offset, int32 start_bit,
                              int32 end_bit, int32 item_bitwidth);
int32 epld_reset_chipset(uint32 type, int32 flag);
int32 epld_get_fiber_present(uint32 port_id,uint32 *fiber_present);
int32 epld_get_fiber_los(uint32 chip_no, uint32 *los);
int32 epld_set_sfp_enable(uint32 chip_no, int32 enable);
int32 epld_read(uint8 idx, epld_para_t *ppara);
int32 epld_write(uint8 idx, epld_para_t *ppara);
int32 epld_item_read(uint8 idx, uint32 op_reg, int32* value);
int32 epld_item_write(uint8 idx, uint32 op_reg, int32 value);
int32 epld_item_get_width(uint8 idx, uint32 op_reg);
int32 epld_init(epld_info_t* p_epld_info);
uintptr_t epld_localbus_addr_get(void);

/* add 'is_gbdemo' flag for some features that gbdemo board owns only, added by qicx, 2013-07-08 */
int32 epld_cfg_phy_interrupt_mask(int32 mask_en, uint8 is_gbdemo);
int32 epld_cfg_poe_interrupt_mask(int32 mask_en);
int32 epld_cfg_dpll_interrupt_mask(int32 mask_en);
int32 epld_get_fan_type(uint8 fan_module, uint8* type, uint8 epld_index);
int32 epld_get_fan_present(uint8 fan_module, uint8* present, uint8 epld_index);
int32 epld_set_fan_power(uint8 fan_module, uint8 power_ctrl, uint8 epld_index);
int32 epld_get_fan_status(uint8 fan_module, uint8* status, uint8 epld_index);
/* Added by liuht for bug26671, 2014-02-14 */
int32 epld_get_fan_speed(uint8 fan_module, uint8* speed, uint8 epld_index, uint8 reg);
int32 epld_set_fan_speed(uint8 fan_module, uint8 speed, uint8 epld_index, uint8 reg);
int32 epld_set_psu_shutdown(uint8 psu_idx, uint8 epld_index, uint8 shutdown);
int32 epld_get_psu_status(uint8 psu_idx, void * p_psu_status, uint8 epld_index, uint8 psu_type);
int32 epld_get_poe_psu_status(uint8* psu_status);
int32 epld_get_poe_present(uint8* present);
int32 epld_update(char* src_file, uint8 epld_type);
void epld_update_reg_info(epld_info_t * p_epld_info);

int32 epld_cfg_phy_switch_smi(uint8 idx, uint8 chnl_id, uint8 enable);
int32 epld_get_slot_card_status(uint8 epld_idx, uint8* slot_card_type);
int32 epld_cfg_lc_power_on(uint8 idx, uint8 lc_id, uint8 enable);

int32 epld_check_power_reboot();
int32 epld_check_manual_reboot();
int32 epld_check_other_reboot();
/* added by tongzb for bug 40719, 2016-09-12 */
int32 epld_get_reboot_info(void);
int32 epld_reset_reboot_info(void);
int32 epld_reset_reboot_flags();
/* Added by liuht for bug 27657, 2014-03-25 */
int32 epld_cfg_power_interrupt_mask(int32 mask_en);
int32 epld_shim_read(uint8 chip, uint8 fpga_idx,uint32  addr, uint32 *value);
int32 epld_sup_read(uint8 chip, uint32  addr, uint32 *value);
int32 epld_shim_write(uint8 chip, uint8 fpga_idx,uint32  addr, uint32 value);
int32 epld_sup_write(uint8 chip, uint32  addr, uint32 value);
int32 epld_shim_test(uint32 loop);
int32 epld_sup_test(uint32 loop);
int32 epld_sup_shim_test(uint32 loop);
int32 epld_ddr_test(uint32 loop,uint8 fpga_idx);
int32 epld_download_fpga(uint8 board_idx, uint8 fpga_idx,uint8 count);

int32 epld_ddr_read(uint8 chip, uint8 fpga_idx,uint32  addr, uint32 *value);
int32 epld_ddr_write(uint8 chip, uint8 fpga_idx,uint32  addr, uint32 value);

int32 epld_get_txlink_status(uint32 chip_no, int *status);
int32 epld_set_port_split_flag(uint32 chip_no, int split);
int32 epld_bootup_diag_and_report(uint32 idx);
int32 epld_item_write_mask(uint8 idx, uint32 op_reg, int32 value, int32 mask);

#endif /* !__EPLD_API_H__ */
