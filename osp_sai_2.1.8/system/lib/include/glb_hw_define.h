/****************************************************************************
* $Id$
*  Centec hardware related MACRO and ENUM, Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Kun Cao
* Date          : 2008-11-14 14:15
* Reason        : First Create.
****************************************************************************/
#ifndef _GLB_HW_DEFINE_H_
#define _GLB_HW_DEFINE_H_
#include "sal.h"
#include "glb_phy_define.h"
#include "glb_eeprom_define.h"
#include "glb_sysenv_define.h"
#include "glb_const.h"

#define GLB_BOARD_TYPE_STRING         "board_type"
#define GLB_PRODUCT_SERIES_STRING     "product_series"
#define GLB_BOARD_MATERIAL_STRING     "board_material"
//#define GLB_SPEED_MODE_STRING         "speed_mode"
#define GLB_EPLD_VERSION_STRING       "epld_ver"
#define GLB_EPLD_DATE_STRING          "epld_date"
#define GLB_EPLD_TIME_STRING          "epld_time"
#define GLB_FPGA_VERSION_STRING       "fpga_ver"
#define GLB_SW_VERSION_STRING         "sw_ver"
#define GLB_HW_VERSION_STRING         "hw_ver"
#define GLB_HW_FLASH_SIZE             "flash_size"
#define GLB_HW_DRAM_SIZE              "sdram_size"
#define GLB_HW_ECC_SUPPORT            "ecc"
#define GLB_HW_CPU_POSITION           "cpu_position"
#define GLB_HW_CPU_CARD_VER           "cpu_card_ver"
#define GLB_HW_CPU_CARD_TYPE          "cpu_card_type"
#define GLB_SERIES_NUM                "serialno"
#define GLB_ROOT_MAC                  "rmac"
#define GLB_SYS_MAC                   "sysmac"
#define GLB_BOOTROM_VER               "bootrom"
#define GLB_PLATFORM_TYPE             "platform_type"

/* fix bug51663, support 65dv1, qianj, 2019-04-16 */
#define GLB_CUSTOM_HW_VERSION_STRING       "custom_hw_ver"
#define GLB_CUSTOM_HW_LEVEL_STRING         "custom_hw_level"
#define GLB_CUSTOM_SW_LEVEL_STRING         "custom_sw_level"
#define GLB_CUSTOM_DAUG_CARD_NAME          "daughter_card_name"

/********************************** define the MAX value **************************************************/
#define MAX_PORT_INFO_LEN           256
#define M_FULLPATH_MAX_LEN          256
#define M_FULLPATH_DEPTH_MAX        32
#define DATAPATH_LEN_MAX            128
#define MAX_EEPROM_SWVER_LEN    29 
/* Modfied by liuht for bug42588, 2017-0329 */
#define REBOOT_TIMES_MAX 50    

/********************************** define the path value **************************************************/
#define GLB_INTR_HUMBER_FATAL_CHIP_DEV_NAME             "/dev/ctc_asic_fatal"
#define GLB_INTR_HUMBER_NORMAL_CHIP0_DEV_NAME           "/dev/ctc_asic_normal"
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
#define GLB_INTR_FOAM_NORMAL_DEV_NAME                   "/dev/ctc_foam_normal"
#endif /*FOAM_SUPPORT*/
#define GLB_STARTUP_CONFIG_FILE_PATH                    "/mnt/flash/startup-config.conf"
#define GLB_FACTORY_CONFIG_FILE_PATH     "/mnt/flash/boot/.factory-config.conf"
#define REBOOT_RECORD_FILE          "/mnt/flash/reboot-info/reboot_record.info"


/********************************** define the board type **************************************************/
#define GLB_BOARD_TYPE_UNKNOWN              0xff

#define GLB_BOARD_GREATBELT_DEMO            0x1
#define GLB_BOARD_E350_48T4X2Q              0x1
#define GLB_BOARD_E350_48T4XG               0x2
#define GLB_BOARD_E350_24T4XG               0x4
#define GLB_BOARD_E350_PHICOMM_8T12XG       0x3
#define GLB_BOARD_E350_8T12XG               0x5
/* actus nid board */
#define GLB_BOARD_ACTUS_NID                 0x1

#define GLB_BOARD_E350_8T4S12XG             0x6
#define GLB_BOARD_E350_8TS12X               0x7   // TODO: Which is gotten from EEPROM
#define GLB_BOARD_E350_24X                  0x8
#define GLB_BOARD_E350_MT_8T12XG            0x9
#define GLB_BOARD_E350_MTRJ_8T12XG          0xa
/* ptn722 board */
#define GLB_BOARD_PTN722                    0x1
#define GLB_BOARD_E350_PEGATRON             0x10
#define GLB_BOARD_V350_48P4X            0x13
/* added by tongzb for support HF 2018-09-18*/ 
#define GLB_BOARD_E350_HF_12T4S8X       0x14
#define GLB_BOARD_E350_65DV1            0x15
#define GLB_BOARD_V350_24P4X               0x19


/* E580 board */
#define GLB_BOARD_E580_24Q                  1
#define GLB_BOARD_E580_48X2Q4Z              2
#define GLB_BOARD_E580_48X6Q                3
#define GLB_BOARD_E580_32X2Q                4
#define GLB_BOARD_E580_32X                  5
#define GLB_BOARD_E580_48XT6Q               6
#define GLB_BOARD_E580_WQT48XT4Q            9
#define GLB_BOARD_SG8348C                   1
#define GLB_BOARD_T65C_8T64X6Q      5

/* E550 board */
#define GLB_BOARD_E550_24X8Y2C              1
#define GLB_BOARD_E550_48T4Y2Q              2
#define GLB_BOARD_E550_28M4Y2C              3
#define GLB_BOARD_E550_48M4Y2C              4
#define GLB_BOARD_E550_24T16Y               5
#define GLB_BOARD_E550_24T16X               6

/* E530 board */
#define GLB_BOARD_E530_24X2C_D              1
#define GLB_BOARD_E530_48T4X2Q_D              2
#define GLB_BOARD_E530_48M4Y2D              3
#define GLB_BOARD_E530_48X                  4
#define GLB_BOARD_E530_24X2C              5
#define GLB_BOARD_E530_48T4X              6
#define GLB_BOARD_E530_48S4X              7
#define GLB_BOARD_E530_24X2Q              8
#define GLB_BOARD_E530_48P4X              9

/*Emulation board*/
#define GLB_BOARD_TM2_EMU                  1

/*P580 board*/
/*phicomm board*/
#define GLB_BOARD_P580_48X6Q                1
#define GLB_BOARD_P580_HW_48X2Q4Z           2
/*fix bug38778, support Magotan board, liangf, 2016-05-30*/
#define GLB_BOARD_MAGOTAN_48X6Q             3
#define GLB_BOARD_P580_32X6Q       4

#define E330_EXT_VER_UNKNOWN 0x0
#define E330_EXT_XGCARD_VER_1 0x3
#define E330_EXT_XGCARD_VER_2 0x2
/********************************** define the port property**************************************************/
/* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
#define MAX_GB_PORT  60
/* end of qicx added */

#define GG_SLICE1_BASE 0x8000

#define GLB_SLOT_NUM_MIN                1
#define GLB_SLOT_NUM_MAX_6SLOT          6
#define GLB_SLOT_NUM_MAX_14SLOT         14

#define GLB_CARD_PORT_NUM_MAX           128

#define GLB_ATCA6_MIN_SUP_SLOT				1
#define GLB_ATCA6_MAX_SUP_SLOT				2
#define GLB_ATCA6_MIN_LC_LOGIC_SLOT		3
#define GLB_ATCA6_MAX_LC_LOGIC_SLOT		6

#define GLB_ATCA14_MIN_SUP_SLOT				7
#define GLB_ATCA14_MAX_SUP_SLOT				8
#define GLB_ATCA14_MIN_LC_LOGIC_SLOT	3
#define GLB_ATCA14_MAX_LC_LOGIC_SLOT	14

#define GLB_MAX_SUP_NUM					2
#define GLB_MAX_LC_NUM					12
#define GLB_MAX_PORT_NUM_PER_BOARD		52

#define DAU_CARD_NAME  12
#define CUSTOM_HW_VERSION  12
#define CUSTOM_SW_VERSION  64
#define GLB_MAX_PORT_NUM_24S_BOARD    24
#define MAX_PSE_NUM             4
#define MAX_POE_PORT_NUM        48
#define DEFAULT_POE_ENALBE   1/* according to POE APPNOTE */
#define DEFAULT_POE_POWER_MAX    320 /* according to POE APPNOTE */
#define DEFAULT_POE_AT_POWER    30000 /* according to POE APPNOTE */
#define DEFAULT_POE_AF_POWER    15400 /* according to POE APPNOTE */
#define DEFAULT_POE_POH_POWER    95000 /* according to POE APPNOTE */
#define GLB_POE_HIGH_PRI    1
#define GLB_POE_LOW_PRI    0

/* for support bypass on HF board */
#define MAX_BYPASS_NUM        8

#define GLB_INVALID_PORT_NO				-1
#define GLB_INVALID_SLOT_NO				-1

#define GLB_MAX_HUMBER_NUM_E800_14SLOT		12
#define GLB_MAX_HUMBER_NUM_E800_6SLOT		4

#define GLB_GREATBELT_DEMO_SLOT_48T_MAX_PORT       48
#define GLB_GREATBELT_DEMO_SLOT_24SFP_MAX_PORT     24
#define GLB_GREATBELT_DEMO_SLOT_4XG_MAX_PORT        4
#define GLB_GREATBELT_DEMO_8XG_MAX_PORT             8

#define GLB_E350_48T4X2Q_48T_MAX_PORT                      48
#define GLB_E350_48T4X2Q_4XG_MAX_PORT                       4
#define GLB_E350_48T4X2Q_2QSFPP_MAX_PORT                    8
/* Added by liuht to support 24T4X board, 2013-11-18 */
#define GLB_E350_24T4X_24T_MAX_PORT                      24

#define GLB_E350_8T12X_8T_MAX_PORT                            8
#define GLB_E350_8T12X_12XG_MAX_PORT                       12

#define GLB_E350_8TS12X_8TS_MAX_PORT                            8
#define GLB_E350_8TS12X_12XG_MAX_PORT                       12

#define GLB_E350_24X_MAX_PORT                       24
#define GLB_FIBER_INFO_INVALID_FLAG                 0xffff
#define GLB_QSFP_CHANNEL_NUM                       4

#define GLB_E580_24Q_MAX_PORT                     24
#define GLB_E580_48X6Q_MAX_PORT                   54
#define GLB_E580_32X2Q_MAX_PORT                   34
#define GLB_E580_32X_MAX_PORT                     32

/*bug25835, transfer speed ability to nsm.*/
/* Modified by liuht for bug 36196, 2015-11-25 */
/* speed sequence must be 100M/1G/10G/100G/2.5G/5G/other speed,
 * this sequence is consistent with the sequence of interface name
 * which is saveed in eeprom. The first for 100M interface name, the
 * second for 1G interface name and so on.
 */
#define GLB_SUPPORT_SPEED_100M    1<<0
#define GLB_SUPPORT_SPEED_1G      1<<1
#define GLB_SUPPORT_SPEED_10G     1<<2
#define GLB_SUPPORT_SPEED_40G     1<<3
#define GLB_SUPPORT_SPEED_100G    1<<4
#define GLB_SUPPORT_SPEED_2_5G    1<<5
#define GLB_SUPPORT_SPEED_5G      1<<6
#define GLB_SUPPORT_SPEED_25G      1<<7
#define GLB_SUPPORT_SPEED_50G      1<<8
#define GLB_SUPPORT_SPEED_MASK    0x1ff

/******* 
* bit: 31    30    29    28    27    26-25    24     23-15    14-0
*   combo   EEE   POE    lp   undir  MS_bit  tunable  speed  phy_type
*******/
/* for bug 49052, when add bit, need to adjust speed bits */
#define GLB_TUNABLE_BIT     24  /* added by shil for bug 44553, 2017-07-21 */
#define GLB_MASTER_SLAVE_BIT  25  /* used for master-slave mode include 2 bits */
#define GLB_UNIDIR_BIT        27
#define GLB_LP_BIT            28  /*bug25520, support port loopback attribute*/
#define GLB_POE_BIT           29
#define GLB_EEE_BIT           30  /* support eee function for bug 28298, 2014-04-21 */
#define GLB_COMBO_BIT         31
#define GLB_SPEED_ABILITY_BIT 15  /*speed ability, when add new speed,decrease it,relate to 100M/1000M/2.5G/5G/10G/25G
/50G/40G/100G */
#define GLB_PORT_PHY_TYPE(PORT) \
    ((PORT->eee_support<<GLB_EEE_BIT)|(PORT->lp_support<<GLB_LP_BIT)|(PORT->poe_support<<GLB_POE_BIT) \
    |(PORT->port_speed_ability<<GLB_SPEED_ABILITY_BIT)|(PORT->is_combo<<GLB_COMBO_BIT)\
    |(PORT->master_slave<<GLB_MASTER_SLAVE_BIT)|(PORT->unidir_support<<GLB_UNIDIR_BIT)|(PORT->tunable_support<<GLB_TUNABLE_BIT))
#define GLB_PORT_PHY_TYPE_MASK \
    ((1<<GLB_COMBO_BIT)|(1<<GLB_EEE_BIT)|(1<<GLB_POE_BIT)|(1<<GLB_LP_BIT)|(1<<GLB_UNIDIR_BIT) \
    |(3<<GLB_MASTER_SLAVE_BIT) |(1 << GLB_TUNABLE_BIT) |(GLB_SUPPORT_SPEED_MASK<<GLB_SPEED_ABILITY_BIT))

/* Modified by liuht for bug 29005, 2014-06-17 */
#define GLB_LINK_CHANGE       0
#define GLB_SPEED_CHANGE      1
#define GLB_DUPLEX_CHANGE     2
#define GLB_MEDIA_CHANGE      3
#define GLB_FLOWCTRL_CHANGE   4
#define GLB_EEE_CHANGE        5
#define GLB_PORT_STATUS_CHANGE(PORT) \
    ((PORT->link_change<<GLB_LINK_CHANGE)|(PORT->speed_change<<GLB_SPEED_CHANGE)|(PORT->duplex_change<<GLB_DUPLEX_CHANGE) \
    |(PORT->media_change<<GLB_MEDIA_CHANGE)|(PORT->flowctrl_change<<GLB_FLOWCTRL_CHANGE)|(PORT->eee_status_change<<GLB_EEE_CHANGE))
/* End of liuht modified */


/********************************** define the device property*************************************************/
#define MAX_SWVER_LEN           256
/*fix bug29801, cr9899, liangf, 2014-08-28*/
#define MAX_SERIAL_NO_LEN           31
#define MAX_BOOTROM_VER_LEN         32
#define MAX_EEPROM_OEM_INFO_LEN      256
#define MAX_SYSENV_OEM_INFO_LEN      256
/* Added by liuht for bug 27788, 2014-03-24 */
#define MAX_HARDWARE_VER_LEN 16
#define MAX_LED_STATUS_LEN   16
#define MAX_VCM_CHANNEL_LEN           20

#define GLB_EEPROM_SW_VER_OFFSET 0x1502

/* added by tongzb for bug 43937 */
#define GLB_FIBER_DEFAULT_FREQ      19340
#define GLB_FIBER_FREQ_NO_0CONFIG       0

/* for bug 53288, added by tongzb, 2019-10-14, for board have no epld */
#define GLB_EPLD_STATUS_ABSENT      0xff
#define GLB_EPLD_STATUS_PRESENT     0 /* default status */

#define DEFAULT_LOW_TMPR    5
#define DEFAULT_HIGH_TMPR   65
#define DEFAULT_CRIT_TMPR   80
//#define DEFAULT_LOW_TMPR    5
//#define DEFAULT_HIGH_TMPR   35
//#define DEFAULT_CRIT_TMPR   65

/*Modify ODM default value liangf, 2017-02-21*/
#define ODM_DEFAULT_LOW_TMPR    -40
#define ODM_DEFAULT_HIGH_TMPR    75
#define ODM_DEFAULT_CRIT_TMPR    90

#define MIN_LOW_TMPR        -45
#define MAX_LOW_TMPR        50

#define MIN_HIGH_TMPR       50
#define MAX_HIGH_TMPR       85

#define MIN_CRIT_TMPR       55
#define MAX_CRIT_TMPR       100

/*fiber transceiver, liangf*/
//#define FIBER_COMPLIANCE_CODE_LEN 8
//#define FIBER_VENDOR_NAME_LEN 16
//#define FIBER_VENDOR_OUI_LEN   3
//#define FIBER_VENDOR_PN_LEN   16
//#define FIBER_VENDOR_REV_LEN   4
//#define FIBER_VENDOR_SN_LEN   16
//#define FIBER_TRANS_TYPE_LEN  32
//#define FIBER_DIAG_INFO_LEN  128
//#define FIBER_VAL_TYPE   4  /*high/low alarm, high/low warn*/
//#define FIBER_VAL_TYPE2  4  /*real value, channel num max is 4*/
#define MAX_TMPR_SENSOR_NUM     6 /*include sensor on board and sensor in switch chip*/
#define MAX_VOLT_CURR_NUM         3
#define MAX_VOLT_CURR_CHANNEL_NUM  8

#define MAX_EEPROM_OEM_INFO_LEN      256
#define MAX_VCM_MODULE_NUM      8   /* max vcm module num per board */
#define MAX_VCM_CHANNEL_NUM     9   /* max channel per module */

#define MAX_PSU_NUM             3
#define MAX_FAN_TRAY_NUM        2
#define MAX_FAN_IN_ONE_TRAY     8


/*Fix bug 15511. zhaow 2011-08-01. for fibermodule DDM.*/
#define TRANSCEIVER_RX_POWER_HIGH 9
#define TRANSCEIVER_RX_POWER_LOW     8
#define TRANSCEIVER_TEMP_HIGH        7
#define TRANSCEIVER_TEMP_LOW         6
#define TRANSCEIVER_VCC_HIGH         5
#define TRANSCEIVER_VCC_LOW          4
#define TRANSCEIVER_BIAS_HIGH     3
#define TRANSCEIVER_BIAS_LOW      2
#define TRANSCEIVER_TX_POWER_HIGH    1
#define TRANSCEIVER_TX_POWER_LOW     0

/*ON GB system fan speed adjust depend on board tempr*/
#define BOARD_TMPR_FULL  80
#define BOARD_TMPR_HIGH  65
#define BOARD_TMPR_LOW  50
#define BOARD_TMPR_HYST 2//hysteresis threshold about temperature value for speed adjust and critical warn.

/*On GG sytem fan speed adjust depend on GG chip tempr. low, high, full.*/
#ifdef _CTC_LOONGSON_
#define GG_CHIP_TMPR_LOW_ALARM        -40
#else
#define GG_CHIP_TMPR_LOW_ALARM        -10
#endif
#define GG_CHIP_TMPR_LOW              80
#define GG_CHIP_TMPR_HIGH             90
#define GG_CHIP_TMPR_ALARM            100
#define GG_CHIP_TMPR_CRITICAL         110
#define GG_CHIP_TMPR_HYST             2//hysteresis threshold about temperature value for speed adjust and critical warn.
#define GG_CHIP_TMPR_CRITICAL_HYST    8 //re-poweron hysteresis threshold after chip tmpr reach critical
#define GLB_PHYPORT_FLAG_COMBO      0x01
#define GLB_PHYPORT_FLAG_LOOPBACK   0x02
#define GLB_PHYPORT_FLAG_EEE        0x04
#define GLB_PHYPORT_FLAG_POE        0x08
#define GLB_PHYPORT_FLAG_MASTER     0x10
#define GLB_PHYPORT_FLAG_SLAVE      0x20
#define GLB_PHYPORT_FLAG_UNIDIR     0x40

/* GG cut_through speed */
#define CUT_THROUGH_FORWARDING_SPEED_10G_40G_100G  1
#define CUT_THROUGH_FORWARDING_SPEED_1G_10G_100G   2
#define CUT_THROUGH_FORWARDING_SPEED_1G_10G_40G    3

/********************************** define other property*************************************************/
#define GLB_VCT_PAIR_ACCURACY 3
#define GLB_VCT_PAIR_NORMAL_ACCURACY    5
#define GLB_VCT_PAIR_OPEN_SHORT_ACCURACY    8
#define GLB_VCT_PAIR_CROSS_ACCURACY        10

#define GLB_VCT_DETECT_TIME  3000


/********************************** define struct and enum*************************************************/
/* EPLD board type Defines */
enum glb_board_series_e
{
    GLB_SERIES_GREATBELT_DEMO = 0xc,
    GLB_SERIES_E580 = 0x10,
    GLB_SERIES_P580 = 0x11,     /*phicomm series no*/
    GLB_SERIES_E550 = 0x12,
    GLB_SERIES_E530 = 0x13,  /* temporary product id */
    GLB_SERIES_E350 = 0x1,
    GLB_SERIES_ACTUS = 0xb1,  /* ACTUS NID board type, temp value */
    GLB_SERIES_SG8348C = 0x82, /* SG8348C */
    GLB_SERIES_XGSW = 0x83,   /* XGSW-EA-10 */
    GLB_SERIES_VPX=0x84,/* RZVP-SW3 */
    GLB_SERIES_PTN722 = 0x87,
    GLB_SERIES_SGG = 0xa0,  /*SG8348B*/
    GLB_SERIES_EMU = 0xfe,
    GLB_SERIES_MAX
};
typedef enum glb_board_series_e glb_board_series_t;

enum reboot_type
{
    REBOOT_UNSUPPORT,
    REBOOT_EPLD,
    REBOOT_EPLD_EEPROM,
    REBOOT_FILE_EPLD,
    REBOOT_MAX
};

enum fan_speed_rate_s
{
    FAN_SPEED_00=00,
    FAN_SPEED_10=10,
    FAN_SPEED_20=20,
    FAN_SPEED_30=30,
    FAN_SPEED_40=40,
    FAN_SPEED_50=50,
    FAN_SPEED_60=60,
    FAN_SPEED_70=70,
    FAN_SPEED_80=80,
    FAN_SPEED_90=90,
    FAN_SPEED_100=100
};

#define DEFAULT_FAN_RATE_BOTTOM    FAN_SPEED_40
#define DEFAULT_FAN_RATE_LOW       FAN_SPEED_60
#define DEFAULT_FAN_RATE_HIGH      FAN_SPEED_80
#define DEFAULT_FAN_RATE_FULL      FAN_SPEED_100
typedef enum
{
    GLB_REBOOT_OTHER,
    GLB_REBOOT_COLD,
    GLB_REBOOT_WARM,
    GLB_REBOOT_TYPE_MAX
} glb_reboot_type_t;

typedef enum
{
    GLB_WARMBOOT_NORMAL,    /* hardware reinit, software reinit, ASIC reinit */
    GLB_WARMBOOT_KERNEL,    /* hardware not reinit, kernel ASIC and process reinit */
    GLB_WARMBOOT_FAST,      /* hardware not reinit, kernel not reinit, ASIC reinit, process reinit */
    GLB_WARMBOOT_HITLESS,   /* hardware not reinit, kernel not reinit, ASIC not reinit, process reinit */    
    GLB_WARMBOOT_TYPE_MAX
} glb_warmboot_type_t;

typedef enum
{
    GLB_TIME_SYNC_NONE,
    GLB_TIME_SYNC_SYSTIME,
    GLB_TIME_SYNC_PTP,
    GLB_TIME_SYNC_TYPE_MAX
} glb_time_sync_type_t;

enum glb_asic_fatal_type_e
{
    GLB_ASIC_FATAL_INTR_RESET,
    GLB_ASIC_FATAL_INTR_RESET_MODULE,
    GLB_ASIC_FATAL_INTR_PARITY,
    GLB_ASIC_FATAL_INTR_LOG,
    GLB_ASIC_FATAL_INTR_MAX
};
typedef enum glb_asic_fatal_type_e glb_asic_fatal_type_t;

enum glb_reset_type_e
{
    GLB_RESET_TCAM_CORE,
    GLB_RESET_TCAM_POWER,
    GLB_RESET_GLB,
    GLB_RESET_DLLOCK,
    GLB_RESET_PCI,
    GLB_RESET_SUP,
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-08 */
    GLB_RESET_FOAM_GLB,
    GLB_RESET_FOAM_PCI,
#endif /*FOAM_SUPPORT*/

    GLB_RESET_RESET_TYPE_MAX
};
typedef enum glb_reset_type_e glb_reset_type_t;


typedef enum
{
    E_SYS_LED = 0,
    E_RUN_LED,
    E_ALARM_LED,
    E_FAN_LED,
    E_PSU1_LED,
    E_PSU2_LED,
    E_POE_LED,
    E_IND_LED,
    E_MAX_LED,
}glb_led_type_t;

typedef enum
{
    E_SPEED_1_10G = 0,
    E_SPEED_1_25G = 1,
    E_SPEED_10_25G = 2,
}glb_speed_mode_t;

typedef enum
{
    SYS_EVENT_RUN,
    SYS_EVENT_TEMP,
    SYS_EVENT_FAN,
    SYS_EVENT_PSU,
    SYS_EVENT_MAX,
}E_SYS_EVENT_T;

typedef enum
{
    SYS_LED_NORMAL,
    SYS_LED_ALARM,    
}E_SYS_LED_T;

typedef enum
{
    E_LED_SYS_INIT = 0,
    E_LED_SYS_RUN,
    E_LED_SYS_ABNORMAL,
    E_LED_SYS_ALARM,
    E_LED_SYS_NO_ALARM,
    E_LED_SYS_ABSENT,
    E_LED_SYS_OFF,
    E_LED_SYS_ON,
    E_LED_STAT_MAX
}glb_led_stat_t;

struct glb_board_type_s
{
    uint8 series;
    uint8 type;
};
typedef struct glb_board_type_s glb_board_type_t;

enum glb_epld_type_e
{
    GLB_EPLD_TYPE_JAM = 0,
    GLB_EPLD_TYPE_VME,
    GLB_EPLD_TYPE_MAX
};
typedef enum glb_epld_type_e glb_epld_type_t;

enum glb_bootrom_type_e
{
    E_BOOTROM_TYPE_UNSUPPORT,
    E_BOOTROM_TYPE_4M,   /* For board 48T4XG */
    E_BOOTROM_TYPE_512K, /* For board 8t12x,8ts12x,8t4s12x,24x,mt8t12x */
    E_BOOTROM_TYPE_32M,
    E_BOOTROM_TYPE_MAX
};
typedef enum glb_bootrom_type_e glb_bootrom_type_t;

enum glb_chassis_type_e
{
    GLB_CHASSIS_2_SLOT,
    GLB_CHASSIS_6_SLOT,
    GLB_CHASSIS_14_SLOT,
    GLB_CHASSIS_PIZZA_BOX,
    GLB_CHASSIS_TYPE_MAX
};
typedef enum glb_chassis_type_e glb_chassis_type_t;

enum glb_supcard_state_e
{
    GLB_SUPCARD_STATE_INIT,
    GLB_SUPCARD_STATE_ACTIVE,
    GLB_SUPCARD_STATE_STANDBY,
    GLB_SUPCARD_STATE_SMOOTH,
    GLB_SUPCARD_STATE_INVALID
};
typedef enum glb_supcard_state_e glb_supcard_state_t;
enum glb_card_fun_type_e
{
    GLB_ACTIVE_SUP_CARD,
    GLB_STANDBY_SUP_CARD,
    GLB_FWD_LINE_CARD,
    GLB_BOARD_FUN_TYPE_MAX
};
typedef enum glb_card_fun_type_e glb_card_fun_type_t;
struct glb_phy_hdl_s
{

};
typedef struct glb_phy_hdl_s glb_phy_hdl_t;

struct glb_mac_hdl_s
{

};
typedef struct glb_mac_hdl_s glb_mac_hdl_t;

struct glb_fiber_s
{

};
typedef struct glb_fiber_s glb_fiber_t;

enum glb_card_state_e
{
    BOARD_ABSENT = 0,
    BOARD_PRESENT,
    BOARD_FIRST_CREATED,
    BOARD_TYPE_CONFLICT,
    BOARD_HW_INSUFFICIENT,
    BOARD_READY,
    BOARD_STATE_MAX
} ;
typedef enum glb_card_state_e glb_card_state_t;

#define GLB_CARD_STATE_MAX BOARD_STATE_MAX

enum glb_port_state_e
{
    PORT_ABSENT = 0,
    PORT_PRESENT,
    PORT_FIRST_CREATED,
    PORT_STATE_MAX
};
typedef enum glb_port_state_e glb_port_state_t;

enum glb_chip_stats_e
{
    LC_CHIP_STM_UNINITED,
    LC_CHIP_INITING,
    LC_CHIP_INIT_DONE,
    LC_CHIP_INIT_FAIL,
    LC_CHIP_SYNC_DONE,
    LC_CHIP_INIT_ABSENT,
};
typedef enum glb_chip_stats_e glb_chip_stats_t;

enum glb_tcam_type_e
{
    GLB_TCAM_NULL = 0,
    /*NL device*/
    GLB_TCAM_NL5512,
    GLB_TCAM_NL6512,
    GLB_TCAM_NL9K,
    /*IDT device*/
    GLB_TCAM_IDT75K72100,
    GLB_TCAM_IDT75S10010A,
    GLB_TCAM_IDT75S10005A,
    MAX_GLB_TCAM_TYPE,
};
typedef enum glb_tcam_type_e glb_tcam_type_t;

enum glb_sram_type_e
{
    GLB_SRAM_NULL,
    GLB_SRAM_18MBIT,
    GLB_SRAM_36MBIT,
    GLB_SRAM_72MBIT,
    GLB_SRAM_TYPE_MAX,
};
typedef enum glb_sram_type_e glb_sram_type_t;

enum phy_interrupt_mode_e
{
    GLB_PHY_INTERRUPT_MODE_NULL,
    GLB_PHY_INTERRUPT_MODE_FPGA,
    GLB_PHY_INTERRUPT_MODE_EPLD,  /* mask/unmask int by epld , get int stat by ioctl */
    GLB_PHY_INTERRUPT_MODE_CPU,   /* mask/unmask int by ioctl, get int stat by epld */
    GLB_PHY_INTERRUPT_MODE_MAX,
};
typedef enum phy_interrupt_mode_e phy_interrupt_mode_t;

enum poe_interrupt_mode_e
{
    GLB_POE_INTERRUPT_MODE_NULL,
    GLB_POE_INTERRUPT_MODE_FPGA,
    GLB_POE_INTERRUPT_MODE_EPLD,
    GLB_POE_INTERRUPT_MODE_MAX,
};
typedef enum poe_interrupt_mode_e poe_interrupt_mode_t;

struct card_stats_s
{
    struct timeval  attach_time;
    uint32        attach_count;
    struct timeval  ready_time;
    uint32        ready_count;
    struct timeval  absent_time;
    uint32        absent_count;
};
typedef struct card_stats_s card_stats_t;

struct glb_humber_dev_s
{
    uint8 pbqdr_sram_type;    /* packet buffer qdr */
    uint8 piqdr_sram_type;    /* packet info qdr */
    uint8 tbqdr_sram_type;    /* table info qdr */
    uint8 ddr_sram_type;      /*external sram type*/
    uint8 tcam_type;          /*external tcam type*/

};
typedef struct glb_humber_dev_s glb_humber_dev_t;

struct glb_chip_dev_s
{
    uint8 glb_chip_type;
    glb_chip_stats_t chip_states;
    void *p_priv_info;
};
typedef struct glb_chip_dev_s glb_chip_dev_t;

struct glb_port_range_s
{
    uint8 port_start;
    uint8 port_end;
};
typedef struct glb_port_range_s glb_port_range_t;

typedef struct
{
    int32   low;
    int32   high;
    int32   crit;
#ifdef CNOS_OS    
    int32   chip_low_alarm;
    int32   chip_low;
    int32   chip_high;
    int32   chip_alarm;
    int32   chip_crit;
#endif
} tmpr_cfg_t;

typedef enum
{
    GLB_PIN_NOT_USE          =  0,                 /*0 mean not use*/
    GLB_PIN_VOLT             = 1 << 0,         /*mean pin voltage*/
    GLB_PIN_CURR             = 1 << 1,        /*mean pin current*/
    GLB_PIN_VOLT_POWER       = 1 << 2,              /*mean volt for calculate system power*/
    GLB_PIN_CURR_POWER       = 1 << 3,             /*mean  curent for calculate system power*/
    /* for bmr464/max20743 */
    GLB_PIN_VOLT_OUT        = 1 << 4,            /* mean out volt */
    GLB_PIN_VOLT_IN         = 1 << 5,           /* mean in volt */
    GLB_PIN_CURR_OUT        = 1 << 6,            /* mean out curr */
    GLB_PIN_CURR_IN         = 1 << 7,           /* mean in curr */
    GLB_PIN_TMPR            = 1 << 8,           /* mean tmpr */

    GLB_PIN_FLAG_OVER       = 1 << 16,            /*mean the the pin is lastest one,used to judge the receive data is complete*/
}vcm_channel_flag_t;

typedef struct
{
    vcm_channel_flag_t  flag;
    uint8     curr_depend_pin; /* maasure current : (V1-v2)/R, this pin is V2 pin, start index is 0 */
    float32   volt_split_factor;  /*the measure value * volt_split_factor = real value*/
    float32   low;                    /*low alarm*/
    float32   high;                   /*high alarm*/
    uint8     name[MAX_VCM_CHANNEL_LEN+1];    /*the name of measure pin*/
}vcm_channel_cfg_t;

typedef struct
{
    vcm_channel_cfg_t channel_cfg[MAX_VCM_CHANNEL_NUM];
} vcm_cfg_t;

struct vcm_status_s
{
    uint8 vcm_type;
    uint8 vcm_channel_nums;
    float32 volt[MAX_VCM_CHANNEL_NUM];
    float32 curr[MAX_VCM_CHANNEL_NUM];
    float32 tmpr[MAX_VCM_CHANNEL_NUM];
};
typedef struct vcm_status_s  vcm_status_t;

struct tmpr_status_s
{
    uint8   pos; /*Sensor position*/
    int32   tmpr_val; /*real time value*/
};
typedef struct tmpr_status_s tmpr_status_t;

typedef enum
{
    E330_NM_NULL,
    E330_NM_2SFP_PLUS,
    E330_NM_4SFP_PLUS,
    E330_NM_4SFP,
    E350_65DV1_4SFP_8T,
    E350_65DV1_6SFP_PLUS,    
    E330_NM_UNSUPPORT,
}E330_daughter_card_e;

typedef enum
{
    GLB_SLOT_CARD_UNSUPPORT = 0x0,  /* slot card is unsupported */
    GLB_SLOT_CARD_16RJ45_VITESSE = 0x1,
    GLB_SLOT_CARD_16RJ45_LSI = 0x2,
    GLB_SLOT_CARD_16RJ45_RELTEK = 0x3,
    GLB_SLOT_CARD_8SFP = 0x4,
    GLB_SLOT_CARD_2SFPP_SFI = 0x5,
    GLB_SLOT_CARD_2SFPP_XAUI = 0x7,
    GLB_SLOT_CARD_10G_KR = 0x8,
    GLB_SLOT_CARD_NULL = 0xf,    /* slot card is absent */
}GreatBelt_Demo_slot_card_type_e;

typedef enum
{
    GLB_SLOT_CARD_VER_1 = 0,
    GLB_SLOT_CARD_MAX,
    GLB_SLOT_CARD_VER_UNSUPPORT,
}GreatBelt_Demo_slot_card_ver_e;

typedef enum
{
    CPU_SMI_VSC8211_SEL,
    CPU_SMI_SLOT0_PHY_SEL,
    CPU_SMI_SLOT1_PHY_SEL,
    CPU_SMI_SLOT2_PHY_SEL,
}GreatBelt_Demo_cpu_smi_mux_sel_e;

enum glb_slot_num_e
{
    GLB_SLOT_NULL = -1,
    GLB_SLOT_0 = 0,
    GLB_SLOT_1 = 1,
    GLB_SLOT_2 = 2,
    GB_VTSS8211 = 3,  /* CPU SMI access vtss8211(connect to GB) */
    GLB_SLOT_MAX = 10,
};
typedef enum glb_slot_num_e glb_slot_num_t;

enum datapath_mode_e
{
    DATAPATH_MODE_1G,
    DATAPATH_MODE_10G,
};
typedef enum datapath_mode_e datapath_mode_t;

/* Added by qicx for bug 21474: 2012-11-27 */
typedef enum
{
    GLB_I2C_IDX_0 = 0,
    GLB_I2C_IDX_1,
}glb_i2c_idx_e;

typedef enum
{
    PSU_SHOW_FULL_STATUS = 0,
    PSU_SHOW_GOOD_SIG,
    PSU_SHOW_HF,/* for board HF */
    PSU_SHOW_MAX
} psu_type_e;

enum glb_psu_status_e
{
    PSU_OK,
    PSU_FAIL
};

enum glb_psu_absent_e
{
    PSU_PRESENT,
    PSU_ABSENT
};

enum glb_psu_alert_e
{
    PSU_NORMAL,
    PSU_ALERT
};

typedef enum
{
    PSU_DC,
    PSU_AC,
    PSU_DC_POE,
    PSU_UNKNOWN
}psu_mode_e;

struct psu_pmbus_status_s
{
    uint8 support_pmbus;
    
    uint8 status_byte;
    uint16 status_word;
    uint8 status_vout;
    uint8 status_iout;
    uint8 status_input;
    uint8 status_temperature;
    uint8 status_cml;
    uint8 status_other;
    uint8 status_mfr_specific;
    uint8 status_fans_1_2;
    uint8 status_fans_3_4;
    
};
typedef struct psu_pmbus_status_s psu_pmbus_status_t;

struct psu_status_s
{
    uint8 psu_fan; /*1 mean fail, 0 mean ok*/
    /* modified by liuht for bug 24525,2013-10-24 */
    uint8 psu_mode; /* 1 mean AC, 0 mean DC*/
    uint8 psu_work_status; /*1 mean fail, 0 mean ok. fix bug 14696.*/
    uint8 psu_absent;/*1 mean absent, 0 mean present*/
    uint8 shutdown;/*1 mean shutdown, 0 mean enable*/
    uint8 psu_alert; /*1 mean alert, 0 mean no alert, added by liuht for bug 24525,2013-10-24*/
    uint8 psu_type;
    uint8 psu_health_status_change;
    psu_pmbus_status_t psu_pmbus_status;
    uint8 fixed_status; /* add by chenjr for bug 53516. 1 mean power don't support alert signal*/
};
typedef struct psu_status_s psu_status_t;



typedef enum
{
    SENSOR_CPU,         /*Sensor around CPU */
    SENSOR_BEFORE_CHIP, /*sensor between  switch chip and front panel*/
    SENSOR_BEHIND_CHIP, /*sensor between  switch chip and back panel*/
    SENSOR_FAN, /*Sensor around fan*/
    SENSOR_PSU, /*Sensor around psu*/
    SENSOR_CHIP0, /*internal sensor of switch chip 0*/
    SENSOR_CHIP1, /*internal sensor of switch chip 1*/
    SENSOR_POS_MAX,
}sensor_position_e;

typedef struct
{
    uint16 bottom_speed_rate;
    uint16 low_speed_rate;
    uint16 high_speed_rate;
    uint16 full_speed_rate;
}fan_consumer_speed_cfg_t;

/*Fix bug 14895. jqiu 2011-05-17. add this for auto adjust fan speed depend on temperature. HW spec define this.*/
/*Fix bug 17254. jqiu 2011-12-07. adjust fan speed and temperature threshold.*/
enum glb_fan_speed_level
{
    FAN_SPEED_BOTTOM,
    FAN_SPEED_LOW,
    FAN_SPEED_HIGH,
    FAN_SPEED_FULL,
    FAN_SPEED_LEVEL_MAX,
};

enum glb_fan_status_e
{
    FAN_ABSENT,
    FAN_INIT,
    FAN_PRESENT,
};

enum glb_fan_speed_e
{
    FAN_SPEED_FORCE = 0,
    FAN_SPEED_SW, /* adjust speed by software */
    FAN_SPEED_HW, /* adjust speed by hardware */
    FAN_SPEED_MAX
};

/*This struct describe the fan module information.*/
struct fan_status_s
{
    uint8 num; /* fan number in one fan tray*/
    uint8 speed_adjust; /*whether fan support speed adjust, 0 mean no, 1 mean yes.*/
    uint8 present; /*fan tray present status, see glb_fan_status_e*/
    uint8 status; /*every fan work status in one fan tray, each bit show one fan status. 0 mean fail, 1 mean ok.*/
    uint8 speed_level; /*config speed level*/
    uint8 speed_rate; /*speed rate by percent */
};
typedef struct fan_status_s fan_status_t;

enum glb_phy_led_type_e
{
    GLB_PHY_LED_DEFAULT,             /* 1G phy:port has one led, copper.*/
    GLB_PHY_LED_1,                   /* 1G phy:port has one led, fiber; 10G phy: led1-link,led2-act*/
    GLB_PHY_LED_2,                   /* 1G phy:port has two led, fiber; 10G phy: led1-rx link and act,led2-same to led1*/
    GLB_PHY_LED_3,                   /* 1G phy: special for E330-48T combo port. Fix bug 14704. resolve crc problem. 1G
                                               fiber led no orange, only green.*/
    GLB_PHY_LED_4,                   /* 1G phy:combo port has two led, fiber only has green, no orange, other same to
                                               upper LED default mode. so for fiber, 100M & 1000M led mode are same*/
    GLB_PHY_LED_5,                   /* 1G phy serial led and phy just light led0 for linkup.*/
    GLB_PHY_LED_6,                   /* 1G phy parallel led, port has one led */
    GLB_PHY_LED_7,                   /* 1G phy:combo port has three leds, one for copper, two for fiber.
                                                Copper led is dual-color, amber for 10/100M, green for 1000M;
                                                Fiber leds are all single color, amber for 100BASE-FX link up/activity,
                                                      green for 1000BASE-FX link up/activity */
    GLB_PHY_LED_8,                /*two led 1G copper phy,green for 1000,amber for 100/10*/
    GLB_PHY_LED_9,                /*two led 1G copper phy,green not use ,amber for 100/10*/
    GLB_PHY_LED_10,                    /* AQPHY one led */
    GLB_PHY_LED_11,                    /* AQPHY two leds */
    GLB_PHY_LED_12,             /* two led 1G copper phy,green for 100/10,amber for 1000, reverse to GLB_PHY_LED_8*/
    GLB_PHY_LED_16,               /* 88e1680 1G copper phy, 2 led, use dual mode led, mode 4 behaveor */
    GLB_PHY_LED_MAX,
};
typedef enum glb_phy_led_type_e glb_phy_led_type_t;

enum glb_phy_led_freq_e
{
    GLB_PHY_LED_FREQ_DEFAULT,               /* led freq default 5hz*/
    GLB_PHY_LED_FREQ_2P5,                   /* led freq default 2.5hz*/
    GLB_PHY_LED_FREQ_5,                     /* led freq default 5hz*/
    GLB_PHY_LED_FREQ_10,                    /* led freq default 10hz*/
    GLB_PHY_LED_FREQ_20,                    /* led freq default 20hz*/
    GLB_PHY_LED_FREQ_MAX,
};
typedef enum glb_phy_led_freq_e glb_phy_led_freq_t;

struct firmware_ver_s{
    uint8 major_num;
    uint8 minor_num;
    uint8 build_id;
    uint8 provision_id;
    long platform_id;
    long ver_id;
};
typedef struct firmware_ver_s firmware_ver_t;

enum glb_remote_pair_e
{
    GLB_REMOTE_PAIR_ABSENT = 0,    /* remote pair is absent */
    GLB_REMOTE_PAIR_PRESENT,       /* remote pair is present */
    GLB_REMOTE_PAIR_MAX
};
typedef enum glb_remote_pair_e glb_remote_pair_t;

enum glb_vct_status_e
{
    GLB_VCT_STATUS_CLOSED = 0,     /* VCT Mod is not start */
    GLB_VCT_STATUS_RUNNING,        /* VCT Mod is running   */
    GLB_VCT_STATUS_FINISHED,       /* VCT Mod is finished  */
    GLB_VCT_STATUS_MAX,

};
typedef enum glb_vct_status_e glb_vct_status_t;

enum glb_pair_state_e
{
    GLB_PAIR_STATE_NORMAL = 0x0,                /* Correctly terminated pair */
    GLB_PAIR_STATE_ABNORMAL_OPEN = 0x1,         /* Open pair */
    GLB_PAIR_STATE_ABNORMAL_SHORTED = 0x2,      /* Short pair */
    GLB_PAIR_STATE_ABNORMAL_TERMINATION = 0x4,  /* Abnormal termination */
    GLB_PAIR_STATE_CROSS_PAIR_SHORT_A = 0x8,    /* Cross-pair short to pair A */
    GLB_PAIR_STATE_CROSS_PAIR_SHORT_B = 0x9,    /* Cross-pair short to pair B */
    GLB_PAIR_STATE_CROSS_PAIR_SHORT_C = 0xa,    /* Cross-pair short to pair C */
    GLB_PAIR_STATE_CROSS_PAIR_SHORT_D = 0xb,    /* Cross-pair short to pair D */
    GLB_PAIR_STATE_ABNORMAL_CROSS_PAIR_A = 0xc, /* Abnormal cross-pair coupling, pair A */
    GLB_PAIR_STATE_ABNORMAL_CROSS_PAIR_B = 0xd, /* Abnormal cross-pair coupling, pair B */
    GLB_PAIR_STATE_ABNORMAL_CROSS_PAIR_C = 0xe, /* Abnormal cross-pair coupling, pair C */
    GLB_PAIR_STATE_ABNORMAL_CROSS_PAIR_D = 0xf, /* Abnormal cross-pair coupling, pair D */
    GLB_PAIR_STATE_LOW_IMPEDANCE  = 0x10,       /* The pair has lower than expected impedance */
    GLB_PAIR_STATE_HIGH_IMPEDANCE = 0x11,       /* The pair has higher than expected impedance */
    GLB_PAIR_STATE_MAX
};
typedef enum glb_pair_state_e glb_pair_state_t;


struct glb_pair_info_s
{
    uint16 pair_length;
    uint16 pair_accuracy;
    glb_pair_state_t pair_status;
};
typedef struct glb_pair_info_s glb_pair_info_t;

struct glb_port_cable_info_s
{
    glb_pair_info_t pair_A;
    glb_pair_info_t pair_B;
    glb_pair_info_t pair_C;
    glb_pair_info_t pair_D;
    glb_remote_pair_t remote_pair;
    uint16 cable_speed;
    uint32 port_link_up;    /* if non-zero, link is up; otherwise link is down */
    uint32 port_enable;     /* if non-zero,  is phy is noshutdown; otherwise phy is shutdown */
    glb_vct_status_t status_change;
};
typedef struct glb_port_cable_info_s glb_port_cable_info_t;

enum glb_cpu_type_e
{
    GLB_CPU_PPC_MPC8247 = 0,               /* POWERPC:MPC8247 */
    GLB_CPU_MIPS_CN5010,                   /* MIPS:CN5010 */
    GLB_CPU_PPC_P1020,                     /* POWERPC:P1020 */
    GLB_CPU_PPC_P1010,                     /* POWERPC:P1010 */
    GLB_CPU_PPC_P1014,                     /* POWERPC:P1014 */
    GLB_CPU_PPC_MPC8308,                   /* POWERPC:MPC8308 */
    GLB_CPU_LS2F,                          /* MIPS:LS2F*/
    GLB_CPU_LS2H,                          /* MIPS:LS2H*/
    GLB_CPU_HI3535,                        /* ARM:HI3535*/
    GLB_CPU_X86_64,                         /* Intel:X86_64 */
    GLB_CPU_LS1023A,                        /* ARM64:LS1023A*/
    GLB_CPU_MAX,
};
typedef enum glb_cpu_type_e glb_cpu_type_t;

typedef enum
{
    GLB_MUX_CHANNEL_NONE = 0,
    GLB_MUX_CHANNEL0,
    GLB_MUX_CHANNEL1,
    GLB_MUX_CHANNEL1_MAX,
}glb_mux_channel_e;
typedef enum glb_mux_channel_e glb_mux_channel_t;

enum glb_port_media_type_s
{
    GLB_PORT_MEDIA_TYPE_DEFAULT,
    GLB_PORT_MEDIA_TYPE_SFP,
    GLB_PORT_MEDIA_TYPE_QSFP,
    GLB_PORT_MEDIA_TYPE_MAX
};
typedef enum glb_port_media_type_s glb_port_media_type_t;

/* Added by liuht for bug 34540, 2015-09-09 */
enum glb_port_led_mode_change_s
{
    GLB_PORT_LED_MODE_CHANGE_UNSUPPORT,    /* Port unsupport led mode change */
    GLB_PORT_LED_MODE_CHANGE_SFP_ONE_LED,  /* SFP+ Port with one led per port */
    GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED,  /* SFP+ Port with two led per port */
    GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED_LOWSPEED,  /* SFP Port with two led per port,and the highest speed is 1G */
    GLB_PORT_LED_MODE_CHANGE_QSFP_ONE_LED, /* QSFP+ Port with one led per port */
    GLB_PORT_LED_MODE_CHANGE_QSFP_TWO_LED, /* QSFP+ Port with two led per port */
    GLB_PORT_LED_MODE_CHANGE_QSFP28_NO_SPLIT,       /* QSFP28 Port no split */
    GLB_PORT_LED_MODE_CHANGE_QSFP28_SPLIT,          /* QSFP28 Port split */
    GLB_PORT_LED_MODE_CHANGE_PHY_TWO_LED,          /* PHY with two led per port*/
    GLB_PORT_LED_MODE_CHANGE_MAX
};
typedef enum glb_port_led_mode_change_s glb_port_led_mode_change_t;

/* Added by liuht for bug27036, 2014-02-27 */
typedef struct{
    char reboot_type[32];
    char reboot_time[32];
}reboot_info_t;

/*Fix bug32802, store fiber info in handle, move all fiber operation into one thread.*/
typedef struct fiber_running_info_s{
    uint8 present; //fiber_present_state_t
    uint8 tx_enable; //cfg tx_enable
    uint8 is_coper;
}fiber_running_info_t;

typedef enum glb_port_poe_e
{
    NOT_POE_PORT = 0,
    POE_PORT
} glb_port_poe_t;

typedef enum glb_poe_mode_e
{
    POE_AF = 0,
    POE_AT = 1,
    POE_POH = 2
} glb_poe_mode_t;

typedef enum glb_poe_pri_s
{
    POE_CRIT = 1,
    POE_HIGH = 2,
    POE_LOW = 3
} glb_poe_pri_t;

typedef struct glb_poe_cfg_s{
    uint8 enable;
    uint8 force_power;
    uint8 mode;/* af/at/poh */
    uint8 priority;/* critical,high,low */
    uint32 power_limit;
}glb_poe_cfg_t; ///TODO:/* this use in common.proto on CNOS, so change it carefully */

/* for support poe with PD69200/PD69208 */
typedef struct glb_poe_sys_info_s{
    uint8 pse_id; /* power source equipment */
    glb_poe_cfg_t sys_cfg; /* global poe cfg for all pse */
    uint32 status;/* pse device status */
    uint32 cur_consump;   /* system total real time comsumption power (watt)*/
    uint32 aver_consump;
    uint32 peak_consump;
    uint16 pse_channel_num;/* one channel for one pd, one pse have some channels*/
    uint16 const_power_limit;/* the max power for one pse base on the power supply(W) */
}glb_poe_sys_info_t;

typedef struct glb_poe_port_info_s{
    uint8 pse_id;/* power source equipment */
    uint8 pse_mgmt_id;/* poe manage id */
    uint16 pse_channel_id;
    uint32 port_idx;

    glb_poe_cfg_t channel_cfg; /* poe cfg for each interface */
    uint8 class;
    uint32 status;
    uint32 cur_consump; /* system total real time comsumption power (milli-watt)*/
    uint32 aver_consump; 
    uint32 peak_consump;
    uint32 current;
    uint32 voltage;
    uint32 temper;/* temperature per port*/
}glb_poe_port_info_t;


typedef struct glb_card_s glb_card_t;
struct glb_port_s
{
    uint8 port_idx;                 /* idx of port, start from NO.1 */
    uint16 mac_idx;                  /* mac index of ASIC chip */
    uint16 logic_port_idx;           /* logic port of ASIC chip, local phy port */
    uint8 mux_enable;               /* mux enable flag */
    uint8 logic_slot_idx;           /* logic slot id */
    uint8 glb_chip_idx;             /* global chip id */
    uint8 local_chip_idx;           /* local chip id */
    uint8 is_combo;                 /* is a combo port or not. src */
    uint8 is_combo_to;              /* is this port combo to another port.  bug 14685.*/
    uint8 combo_port_idx;           /* which port does this port is combo with? For bug 14685*/
    uint8 port_speed_ability;           /* port support speed ability bug25835*/
    uint8 panel_slot_no;  /*slot number show on panel, for e330-48t, NM-4SFP, NM-2SFP+, NM-4SFP+, For bug 14686*/
    uint8 panel_port_no;  /*port number show on panel, for e330-48t, NM-4SFP, NM-2SFP+, NM-4SFP+, For bug 14686*/
    uint8 poe_support;              /* support poe or not */
    uint8 eee_support;              /* support EEE or not for bug 28298, 2014-04-21 */
    uint8 lp_support;               /* support loopback or not*/
    glb_phy_type_t phy_type;        /* phy type */
    glb_lb_phy_t lp_type;           /* lp type*/
    glb_port_state_t port_state;    /* port state */
    glb_phy_state_t port_status;    /* port physical state */
    glb_port_cable_info_t port_cable_status;   /* port cable state */
    void* p_fiber;                  /* fiber module information and handler */
    uint8 fiber_triger;             /* fiber triger when insert or remove*/
    uint8 fiber_access_err_cnt;     /* fiber module access error count*/
    /*modified by xgu for bug 13493, 2011-01-11*/
    uint8 create_done;              /* port created in hsrv and hagt*/
                                    /* 1 stand for current configure to port ,
                                      0 stand for not created*/
    /* field entered by users */
    glb_port_cfg_t port_cfg;
    glb_card_t* p_host_card;  /* board information of port */
    uint8 mac[GLB_ETH_ADDR_LEN];
    int8 chip_serdes_id;
    uint8 master_slave;

    /* Modified by liuht for bug26853, 2013-01-20 */
    /* Support bootup diag for line card */
    uint16 g_port_index; /* global port id */
    uint8 support_led_mode_change;
    glb_port_led_mode_change_t port_led_mode_change;
    uint8 port_led_mac;
    uint8 port_led_reverse;/* for some led mode , just reverse the mode , not the display of each led */
    uint8 ctl_id; /* GG slice id */
    uint8 mac_ctl_id; /* D2 only, which has one slice but two bus to control mac led */
    /* modified by yaom for 10G remote fault 20140410 */
    uint8 remote_fault_status;  /* remote fault status of the port */
    uint8 force_tx_status;      /* force tx status of the port */

    int8 panel_sub_port_no; /* negtive value represent port merge
                               non-negtive value represent port split */
    uint8 port_numchg_type;
    uint8 port_speed_real;
    uint8 support_media_switch;
    int8 mux_chip_id;
    uint8 port_media_type;
    uint8 unidir_support;
    uint8 tunable_support; /* added by shil for bug 44553, 2017-07-21*/
    uint32 fiber_freq;/* added by tongzb for bug 43937 */
    uint8 serdes_mode;
    fiber_running_info_t run_info;

    /* modified by liwh for bug 35914 support portextend, 2015-10-30 */
    uint16 port_num;       /* 802.1br cascade port num */
    uint16 slot_num;      /* 802.1br cascade port base */
    uint16 vlan_base;      /* 802.1br cascade port base */
    uint16 port_base;
    /* liwh end */
    uint8 speed_mode_cfg_en;/* On duet2 25G port do not support 1G/10G/25G at same time */
    glb_speed_mode_t speed_mode; /* for duet2 25G port */
    glb_speed_mode_t default_speed_mode; /* for duet2 25G port, fixed, this will never change after init */
    uint16 phy_addr;                  /* phy_addr for mido scan */
    uint16 mdio_bus_addr;             /* mdio_bus_addr for mido scan */
    uint8 bypass;   /* for HF board bypass function */
    uint8 bypass_slot;/* for HF board */
    glb_poe_port_info_t poe_info;
};
typedef struct glb_port_s glb_port_t;

enum glb_ppt_numchg_op_e
{
    GLB_PPT_OP_NONE,  /* do not support port split/merge */
    GLB_PPT_OP_SPLIT,
    GLB_PPT_OP_MERGE,
    GLB_PPT_OP_MAX,
};
typedef enum glb_ppt_numchg_op_e glb_ppt_numchg_op_t;

enum glb_ppt_op_ability_e
{
    GLB_PPT_OP_SPLIT_NONE = 0,
    GLB_PPT_OP_SPLIT_10G = (1<<0), 
    GLB_PPT_OP_SPLIT_40G = (1<<1), 
    GLB_PPT_OP_SPLIT_25G = (1<<2), 
    GLB_PPT_OP_SPLIT_50G = (1<<3), 
};
typedef enum glb_ppt_op_ability_e glb_ppt_op_ability_t;

enum glb_ppt_numchg_type_e
{
    GLB_PPT_NUMCHG_TYPE_NONE = 0,
    GLB_PPT_NUMCHG_TYPE_10G = 1,
    GLB_PPT_NUMCHG_TYPE_40G = 2,
    GLB_PPT_NUMCHG_TYPE_1G = 3,
    GLB_PPT_NUMCHG_TYPE_25G = 4,
    GLB_PPT_NUMCHG_TYPE_50G = 5,
    GLB_PPT_NUMCHG_TYPE_MAX
};
typedef enum glb_ppt_numchg_type_e glb_ppt_numchg_type_t;

struct glb_panel_port_s
{
    uint8                    ppt_idx;       /* panel port index, start from 1 */
    uint8                    pslt_id;       /* panel slot id, start from 0, i.e. eth-'0'-1, slot id is the quoted guy */
    glb_ppt_numchg_op_t      op;            /* panel port support merge or split */
    glb_ppt_op_ability_t     op_ability;    /* panel port merge or split ability */
    glb_ppt_numchg_type_t    ppt_numchg_type;
    uint8                    ppt_numchg_num;
    uint16                    ppt_speed_ability;
    uint16                    ppt_speed_real;
    uint8                     disable_speed_change;
};
typedef struct glb_panel_port_s glb_panel_port_t;

struct glb_tmpr_limit_s
{
    int32   board_low;
    int32   board_high;
    int32   board_crit;
};
typedef struct glb_tmpr_limit_s glb_tmpr_limit_t;

enum glb_psu_led_mode_e
{
    GLB_PSU_LED_MODE_MULTI,  /* one led for one psu, this the default mode */
    GLB_PSU_LED_MODE_ONE,/* one led for all psu */
};
typedef enum glb_psu_led_mode_e glb_psu_led_mode_t;

struct glb_card_s
{
    glb_board_type_t board_type;
    glb_board_type_t config_board_type;
    glb_card_state_t card_state;
    glb_card_fun_type_t card_fun_type;   /* working mode */
    char serial_no[MAX_SERIAL_NO_LEN+1];
    uint8 sysmac[GLB_ETH_ADDR_LEN+1];
    char bootrom_ver[MAX_BOOTROM_VER_LEN+1];
    /*Jqiu modify for universal datapath. 2011-02-22*/
    char cfg_profile_name[DATAPATH_LEN_MAX+1]; /*store datapath profile name*/
    char cfg_profile_name2[DATAPATH_LEN_MAX+1]; /*store datapath profile name*/
    uint8 hw_ver;
    char sw_ver[MAX_SWVER_LEN+1];
    uint8 epld_ver;
    uint32 epld_date;
    uint16 epld_time;
    uint8 epld_status;
    uint8 fpga_ver;
    uint32 flash_size;/* the CPU subsystem flash size */
    uint32 dram_size; /* the CPU subsystem dram size */
    uint8 tcam_type;
    uint8 sram_type;
    uint8 phy_chip_num;
    uint8 phy_int_bit_num; /*total bits num from epld register.*/
    uint8 asic_chip_num;
    uint8 chip_sensor_num;
    uint8 l2switch_num;
    uint8 sensor_num;
    uint8 fan_module_num;
    uint8 fan_module_ver; /*store fan module version*/
    uint8 psu_module_num;
    glb_psu_led_mode_t psu_led_mode;
    uint8 vcm_module_num;   /* the num of vcm module */
    uint8 poe_module_num;   /* the num of poe module, same as pse_num */
    uint8 volt_curr_mon_num;   /*the num of ltc module to monitor volt and curr*/
    uint8 daughter_card_type;
    uint8 daughter_card_ver; /*store daughter card version*/
    /* Added by qianj for crsc, 2019-07-09,begin */
    char daughter_card_name[DAU_CARD_NAME]; 
    char custom_hw_ver[CUSTOM_HW_VERSION];
    char custom_sw_ver[CUSTOM_SW_VERSION];
    /* Added by qianj for crsc, 2019-07-09,end */

    uint8 board_material; /* FR4=0, M4=1, M6=2*/
    uint16 logic_slot_no;
    uint16 phy_slot_no;
    uint16 port_num;
    uint16 panel_port_num;
    uint16 bootup_diag_level;
    card_stats_t stats;
    glb_port_range_t* p_port_range;  /* the array which can get the port range by phy chip index */
    glb_chip_dev_t* p_asic_chip;
    glb_port_t** pp_port;
    glb_panel_port_t** pp_ppt;
    void* p_sess;   /* line card connection session with CHSM */    
    /* non-network port PHY */
    /* thermal sensor */
    tmpr_cfg_t          tmpr_cfg;
    glb_tmpr_limit_t    tmpr_limit;
    tmpr_status_t       tmpr_status[MAX_TMPR_SENSOR_NUM];
    /*psu*/
    psu_status_t        psu_status[MAX_PSU_NUM];    
    /*fan*/
    fan_status_t        fan_status[MAX_FAN_TRAY_NUM];
    /*poe*/
    glb_poe_sys_info_t  poe_sys_info[MAX_PSE_NUM];
    /* vcm */
    vcm_cfg_t   vcm_cfg[MAX_VCM_MODULE_NUM];
    vcm_status_t vcm_status[MAX_VCM_MODULE_NUM];
    /* eeprom and sysenv */
    /* if use_sysenv_api != 0, p_sysenv_alloc should init and sysenv driver should init, system use sysenv api */
    /* if use_sysenv_api == 0, p_eeprom_alloc should init and eeprom driver should init, system use eeprom api */
    uint8                use_sysenv_api;
    eeprom_info_t* p_eeprom_alloc;    /* eeprom info  */
    sysenv_info_t* p_sysenv_alloc;      /* sysenv info */
    void *p_board_priv;   /* supervisor card or line card specific attributes. SYSMAC */
    uint8 phy_interrupt_mode;/* where we can get phy interrupt info, use phy_interrupt_mode_t*/
    uint8 dpll_interrupt_mode;/* where we can get dpll interrupt info, use phy_interrupt_mode_t*/
    uint8 poe_interrupt_mode;
    uint8 ptp_quanta;
    uint8 watchdog_supported;
    glb_cpu_type_t cpu_type;
    uint8 slot_card_type;
    uint8 slot_card_ver;
    uint8 datapath_mode;
    uint8 is_gbdemo; /* flag for some features that gbdemo board owns only, added by qicx, 2013-07-08 */
    uint8 ptp_en; /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    /* Added by liuht for bug27036, 2014-02-27 */
    uint8 support_reboot_info;
    uint8 reboot_times;
    reboot_info_t *reboot_info;

    int gb_irq[4]; /* add for get dynamic-allocated irq numbers for GB chip, qicx, 2013-07-27 */
    uint8 nid_1g_xg_mode_flag; /* flag for configure nid box to 1g mode or xg mode
                                  default value: 0 means 1g mode;
                                  value 1 means xg mode */
    /* Added by liuht for bug 27788, 2014-03-24 */
    char hardware_ver[MAX_HARDWARE_VER_LEN + 1];
    int uptime_day;
    int uptime_hour;
    int uptime_min;
    uint8 epld_type;
    uint8 bootrom_type;
    uint8 split_flag; /* set if any port split */
    uint8 platform_type; /* 0 normal platform, 1 onie platform */
    uint8 dpll_num;     /* fix bug42763, XGSW-EA-10 dpll interrupt, liangf, 2017-05-17  */
    uint8 phy_intr_en;

    /*added by hanf for save profile type from eeprom*/
    uint32 profile_type;
    uint8 do_phy_bootup_diag;
    int8 support_bootmtd;/* added by tongzb, -1 not support, 0 support but not enable, 1 support and enable */
};

typedef enum
{
    E_DIAG_LEVEL_NO = 1,
    E_DIAG_LEVEL_MINIMAL,
    E_DIAG_LEVEL_COMPLETE,
    E_DIAG_LEVEL_MAX,
} diag_bootup_level_t;

#endif /* !_GLB_HW_DEFINE_H_ */
