/****************************************************************************
* $Id$
*  SG8348C board init functions
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Matthew Qi
* Date          : 2016-04-08
* Reason        : First Create.
* Revision      : R1.1
* Author        : liangf
* Date          : 2018-08-15
* Reason     : fix bug48521
****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal_common.h"
#include "lcm_specific.h"
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "lcm_debug.h"
#include "lcm_error.h"
#include "lcm_log.h"
#include "lcm_card.h"
#include "ctc_pci.h"
#include "ctc_spi.h"
#include "ctc_i2c.h"
#include "phy_api.h"
#include "fiber_api.h"
#include "sensor_api.h"
#include "gpio_api.h"
#include "fan_api.h"
#include "power_api.h"
#include "eeprom_api.h"
#include "led_api.h"
#include "ltc2991_api.h"
#include "ltc4151_api.h"
#include "lcm_mgt.h"
#include "ctc_api.h"
#include "ctc_chip.h"
#include "glb_distribute_system_define.h"
#include "glb_if_define.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
/*
 * CPU I2C slave device
 */ 

/* Thermal sensor */
#define SG8348C_SENSOR_NUM                      0x01
#define SG8348C_ADDR_SENSOR                     0x48

/* EEPROM */
#define SG8348C_EEPROM_NUM                      0x01
#define SG8348C_ADDR_EEPROM                     0x57

/* Power supply */
#define SG8348C_PSU_MODULE_MAX                  0x0
/* Fan */
#define SG8348C_FAN_MODULE_MAX                  0x0

/* Voltage monitor: LTC2991 and LTC4151*/
#define SG8348C_LTC2991_MON_NUM               0x01
#define SG8348C_LTC4151_MON_NUM               0x01

#define SG8348C_VOLT_CURR_MON_NUM          (SG8348C_LTC2991_MON_NUM + SG8348C_LTC4151_MON_NUM)
/*addr for ltc2991*/
#define SG8348C_ADDR0_VOLT_CURR_MON         0x4c
/*addr for ltc4151*/
#define SG8348C_ADDR1_VOLT_CURR_MON         0x67

/* I2C to GPIO: PCA9505 */
#define SG8348C_GPIO_CHIP_NUM                   0x03
/* PCA9505 */
#define SG8348C_ADDR0_GPIO                      0x20
/* PCA9535 */
#define SG8348C_ADDR1_GPIO                      0x21

#define SG8348C_ADDR2_GPIO                      0x27

/* Port number property */
/* 
 * 8 MG Ports(FE):   VSC8512
 * 48 GE Ports:      VSC8512
 * 16 SFP+ Ports:    No PHY
 * 20 Serdes Ports:  No PHY
 */
#define SG8348C_PANEL_PORT_NUM_MAX        (8+48+16)
#define SG8348C_PANEL_PORT_NUM            (8+48+16)
#define SG8348C_FIBER_PORT_NUM            (8+48+16)
 
#define SG8348C_PHY_PORT_NUM                    (8+48)
#define SG8348C_PHY_MGMT_PORT_NUM         8

/* MAC LED */ ///TODO: 
#define SG8348C_SLICE0_LED_MAC_NUM         16
#define SG8348C_LED_MAC_NUM                     16
#define SG8348C_LED_TBL_NUM                      4
 
/* PHY */
#define SG8348C_MIN_INTERNAL_PHY_ADDR              0  /* should be adapted per board */
#define SG8348C_LAST_INTERNAL_PHY_ADDR            11
#define SG8348C_PORT_NUM_PER_PHY_CHIP             12
#define SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP             4

#define SG8348C_PHY_INT_BITS_SUM                   1

/****************************************************************************
*
* Global and Declarations
*
****************************************************************************/
static const glb_card_t         *glb_card;
static lcm_card_port_t          *glb_lcm_sg8348c_port             = NULL;
static fiber_port_info_t        *glb_lcm_sg8348c_fiber            = NULL;

extern int32 lcm_card_init_callback(card_init_callback_t func);

lcm_card_port_t lcm_sg8348c_port_max[SG8348C_PANEL_PORT_NUM_MAX + SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP] =
{ 
    /* CTC8096 MDC/MDIO BUS 3 */
    { 0,   0 + GG_SLICE1_BASE,   0 + GG_SLICE1_BASE,  0x0b },    /* Port  1 <-> SerDes 48 */
    { 0,   1 + GG_SLICE1_BASE,   1 + GG_SLICE1_BASE,  0x0a },    /* Port  2 <-> SerDes 49 */
    { 0,   2 + GG_SLICE1_BASE,   2 + GG_SLICE1_BASE,  0x09 },    /* Port  3 <-> SerDes 50 */    
    { 0,   3 + GG_SLICE1_BASE,   3 + GG_SLICE1_BASE,  0x08 },    /* Port  4 <-> SerDes 51 */
    { 0,   4 + GG_SLICE1_BASE,   4 + GG_SLICE1_BASE,  0x07 },    /* Port  5 <-> SerDes 52 */
    { 0,   5 + GG_SLICE1_BASE,   5 + GG_SLICE1_BASE,  0x06 },    /* Port  6 <-> SerDes 53 */
    { 0,   6 + GG_SLICE1_BASE,   6 + GG_SLICE1_BASE,  0x05 },    /* Port  7 <-> SerDes 54 */
    { 0,   7 + GG_SLICE1_BASE,   7 + GG_SLICE1_BASE,  0x04 },    /* Port  8 <-> SerDes 55 */

    /* CTC8096 MDC/MDIO BUS 0 */
    { 0,  32 + GG_SLICE1_BASE,  32 + GG_SLICE1_BASE,  0x0b },    /* Port  9 <-> SerDes 80 */
    { 0,  33 + GG_SLICE1_BASE,  33 + GG_SLICE1_BASE,  0x0a },    /* Port 10 <-> SerDes 81 */
    { 0,  34 + GG_SLICE1_BASE,  34 + GG_SLICE1_BASE,  0x09 },    /* Port 11 <-> SerDes 82 */    
    { 0,  35 + GG_SLICE1_BASE,  35 + GG_SLICE1_BASE,  0x08 },    /* Port 12 <-> SerDes 83 */
    { 0,  40 + GG_SLICE1_BASE,  40 + GG_SLICE1_BASE,  0x07 },    /* Port 13 <-> SerDes 84 */
    { 0,  41 + GG_SLICE1_BASE,  41 + GG_SLICE1_BASE,  0x06 },    /* Port 14 <-> SerDes 85 */
    { 0,  42 + GG_SLICE1_BASE,  42 + GG_SLICE1_BASE,  0x05 },    /* Port 15 <-> SerDes 86 */
    { 0,  43 + GG_SLICE1_BASE,  43 + GG_SLICE1_BASE,  0x04 },    /* Port 16 <-> SerDes 87 */
    { 0,  36 + GG_SLICE1_BASE,  36 + GG_SLICE1_BASE,  0x03 },    /* Port 17 <-> SerDes 88 */
    { 0,  37 + GG_SLICE1_BASE,  37 + GG_SLICE1_BASE,  0x02 },    /* Port 18 <-> SerDes 89 */
    { 0,  38 + GG_SLICE1_BASE,  38 + GG_SLICE1_BASE,  0x01 },    /* Port 19 <-> SerDes 90 */    
    { 0,  39 + GG_SLICE1_BASE,  39 + GG_SLICE1_BASE,  0x00 },    /* Port 20 <-> SerDes 91 */
                    
    { 0,  47 + GG_SLICE1_BASE,  47 + GG_SLICE1_BASE,  0x17 },    /* Port 21 <-> SerDes 95 */
    { 0,  46 + GG_SLICE1_BASE,  46 + GG_SLICE1_BASE,  0x16 },    /* Port 22 <-> SerDes 94 */
    { 0,  45 + GG_SLICE1_BASE,  45 + GG_SLICE1_BASE,  0x15 },    /* Port 23 <-> SerDes 93 */
    { 0,  44 + GG_SLICE1_BASE,  44 + GG_SLICE1_BASE,  0x14 },    /* Port 24 <-> SerDes 92 */
    { 0,  44                 ,  44                 ,  0x13 },    /* Port 25 <-> SerDes 44 */
    { 0,  45                 ,  45                 ,  0x12 },    /* Port 26 <-> SerDes 45 */
    { 0,  46                 ,  46                 ,  0x11 },    /* Port 27 <-> SerDes 46 */    
    { 0,  47                 ,  47                 ,  0x10 },    /* Port 28 <-> SerDes 47 */
    { 0,  39                 ,  39                 ,  0x0f },    /* Port 29 <-> SerDes 43 */
    { 0,  38                 ,  38                 ,  0x0e },    /* Port 30 <-> SerDes 42 */
    { 0,  37                 ,  37                 ,  0x0d },    /* Port 31 <-> SerDes 41 */
    { 0,  36                 ,  36                 ,  0x0c },    /* Port 32 <-> SerDes 40 */

    /* CTC8096 MDC/MDIO BUS 2 */
    { 0,  43                 ,  43                 ,  0x0b },    /* Port 33 <-> SerDes 39 */
    { 0,  42                 ,  42                 ,  0x0a },    /* Port 34 <-> SerDes 38 */
    { 0,  41                 ,  41                 ,  0x09 },    /* Port 35 <-> SerDes 37 */    
    { 0,  40                 ,  40                 ,  0x08 },    /* Port 36 <-> SerDes 36 */
    { 0,  35                 ,  35                 ,  0x07 },    /* Port 37 <-> SerDes 35 */
    { 0,  34                 ,  34                 ,  0x06 },    /* Port 38 <-> SerDes 34 */
    { 0,  33                 ,  33                 ,  0x05 },    /* Port 39 <-> SerDes 33 */
    { 0,  32                 ,  32                 ,  0x04 },    /* Port 40 <-> SerDes 32 */
    { 0,  31                 ,  31                 ,  0x03 },    /* Port 41 <-> SerDes 31 */
    { 0,  30                 ,  30                 ,  0x02 },    /* Port 42 <-> SerDes 30 */
    { 0,  29                 ,  29                 ,  0x01 },    /* Port 43 <-> SerDes 29 */    
    { 0,  28                 ,  28                 ,  0x00 },    /* Port 44 <-> SerDes 28 */
                    
    { 0,  27                 ,  27                 ,  0x17 },    /* Port 45 <-> SerDes 27 */
    { 0,  26                 ,  26                 ,  0x16 },    /* Port 46 <-> SerDes 26 */
    { 0,  25                 ,  25                 ,  0x15 },    /* Port 47 <-> SerDes 25 */
    { 0,  24                 ,  24                 ,  0x14 },    /* Port 48 <-> SerDes 24 */
    { 0,  23                 ,  23                 ,  0x13 },    /* Port 49 <-> SerDes 23 */
    { 0,  22                 ,  22                 ,  0x12 },    /* Port 50 <-> SerDes 22 */
    { 0,  21                 ,  21                 ,  0x11 },    /* Port 51 <-> SerDes 21 */    
    { 0,  20                 ,  20                 ,  0x10 },    /* Port 52 <-> SerDes 20 */
    { 0,  19                 ,  19                 ,  0x0f },    /* Port 53 <-> SerDes 19 */
    { 0,  18                 ,  18                 ,  0x0e },    /* Port 54 <-> SerDes 18 */
    { 0,  17                 ,  17                 ,  0x0d },    /* Port 55 <-> SerDes 17 */
    { 0,  16                 ,  16                 ,  0x0c },    /* Port 56 <-> SerDes 16 */

    /* Fiber/SFP+ port, NO PHY */
    { 0,   0                 ,   0                 ,       },    /* Port 57 <-> SerDes  0 */
    { 0,   1                 ,   1                 ,       },    /* Port 58 <-> SerDes  1 */
    { 0,   2                 ,   2                 ,       },    /* Port 59 <-> SerDes  2 */    
    { 0,   3                 ,   3                 ,       },    /* Port 60 <-> SerDes  3 */
    { 0,   4                 ,   4                 ,       },    /* Port 61 <-> SerDes  4 */
    { 0,   5                 ,   5                 ,       },    /* Port 62 <-> SerDes  5 */
    { 0,   6                 ,   6                 ,       },    /* Port 63 <-> SerDes  6 */
    { 0,   7                 ,   7                 ,       },    /* Port 64 <-> SerDes  7 */
                     
    { 0,   8                 ,   8                 ,       },    /* Port 65 <-> SerDes  8 */
    { 0,   9                 ,   9                 ,       },    /* Port 66 <-> SerDes  9 */
    { 0,  10                 ,  10                 ,       },    /* Port 67 <-> SerDes 10 */    
    { 0,  11                 ,  11                 ,       },    /* Port 68 <-> SerDes 11 */
    { 0,  12                 ,  12                 ,       },    /* Port 69 <-> SerDes 12 */
    { 0,  13                 ,  13                 ,       },    /* Port 70 <-> SerDes 13 */
    { 0,  14                 ,  14                 ,       },    /* Port 71 <-> SerDes 14 */
    { 0,  15                 ,  15                 ,       },    /* Port 72 <-> SerDes 15 */ 
};

fiber_port_info_t lcm_sg8348c_fiber_max[SG8348C_FIBER_PORT_NUM] =
{
    /* 8T */
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},

    /* 48T */
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},

    /*
     * Present/Los chip: PCA9505  0x20(chip 0)
     * Enable chip: PCA9535       0x21(chip 1)
     */
    /* fiber_flg+fiber access mode+fbid+busid+btmp+enable_chip+enable_No+present_chip+present_No+los_chip+los_No */       
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0, 11}},  {.en_gpio={1,  0}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0, 10}},  {.en_gpio={1,  1}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   9}},  {.en_gpio={1,  2}}},  
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   8}},  {.en_gpio={1,  3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   3}},  {.en_gpio={1,  4}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   2}},  {.en_gpio={1,  5}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   1}},  {.en_gpio={1,  6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   0}},  {.en_gpio={1,  7}}},

    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   4}},  {.en_gpio={1,  8}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   5}},  {.en_gpio={1,  9}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   6}},  {.en_gpio={1, 10}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0,   7}},  {.en_gpio={1, 11}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0, 15}},  {.en_gpio={1, 12}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0, 14}},  {.en_gpio={1, 13}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0, 13}},  {.en_gpio={1, 14}}}, 
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,  E_FIBER_EN_GPIO, 0, {.asic={0, 12}},  {.en_gpio={1, 15}}},
};

/* Modified by liuht for bug 34540, 2015-09-09 */
mac_led_api_para_t lcm_sg8348c_mac_led_default_entry[SG8348C_LED_MAC_NUM]=
{
    /* panel port 57~72 */
    { 15, 0, 0, LED_MODE_1_FORCE_OFF},
    { 14, 0, 0, LED_MODE_1_FORCE_OFF},
    { 13, 0, 0, LED_MODE_1_FORCE_OFF},
    { 12, 0, 0, LED_MODE_1_FORCE_OFF},
    { 11, 0, 0, LED_MODE_1_FORCE_OFF},
    { 10, 0, 0, LED_MODE_1_FORCE_OFF},
    { 9, 0, 0, LED_MODE_1_FORCE_OFF},
    { 8, 0, 0, LED_MODE_1_FORCE_OFF},

    {7, 0, 0, LED_MODE_1_FORCE_OFF},
    {6, 0, 0, LED_MODE_1_FORCE_OFF},
    {5, 0, 0, LED_MODE_1_FORCE_OFF},
    {4, 0, 0, LED_MODE_1_FORCE_OFF},
    {3, 0, 0, LED_MODE_1_FORCE_OFF},
    {2, 0, 0, LED_MODE_1_FORCE_OFF},
    {1, 0, 0, LED_MODE_1_FORCE_OFF},
    {0, 0, 0, LED_MODE_1_FORCE_OFF},
};

lcm_card_serdes_ffe_t 
lcm_sg8348c_serdes_ffe_max[SG8348C_PANEL_PORT_NUM_MAX]=
{
    {48,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff)},  /* Port  1 <-> SerDes 48 */
    {49,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  2 <-> SerDes 49 */
    {54,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  3 <-> SerDes 54 */    
    {50,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  4 <-> SerDes 50 */
    {51,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  5 <-> SerDes 51 */
    {55,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  6 <-> SerDes 55 */
    {52,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  7 <-> SerDes 52 */
    {53,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  8 <-> SerDes 53 */
       
    {80,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port  9 <-> SerDes 80 */
    {81,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 10 <-> SerDes 81 */
    {82,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 11 <-> SerDes 82 */    
    {83,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 12 <-> SerDes 83 */
    {84,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 13 <-> SerDes 84 */
    {85,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 14 <-> SerDes 85 */
    {86,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 15 <-> SerDes 86 */
    {87,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 16 <-> SerDes 87 */
    {88,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 17 <-> SerDes 88 */
    {89,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 18 <-> SerDes 89 */
    {90,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 19 <-> SerDes 90 */    
    {91,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 20 <-> SerDes 91 */
       
    {95,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 21 <-> SerDes 95 */
    {94,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 22 <-> SerDes 94 */
    {93,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 23 <-> SerDes 93 */
    {92,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 24 <-> SerDes 92 */
    {44,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 25 <-> SerDes 44 */
    {45,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 26 <-> SerDes 45 */
    {46,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 27 <-> SerDes 46 */    
    {47,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 28 <-> SerDes 47 */
    {43,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 29 <-> SerDes 43 */
    {42,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 30 <-> SerDes 42 */
    {41,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 31 <-> SerDes 41 */
    {40,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 32 <-> SerDes 40 */
       
    {39,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 33 <-> SerDes 39 */
    {38,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 34 <-> SerDes 38 */
    {37,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 35 <-> SerDes 37 */    
    {36,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 36 <-> SerDes 36 */
    {35,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 37 <-> SerDes 35 */
    {34,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 38 <-> SerDes 34 */
    {33,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 39 <-> SerDes 33 */
    {32,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 40 <-> SerDes 32 */
    {31,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 41 <-> SerDes 31 */
    {30,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 42 <-> SerDes 30 */
    {29,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 43 <-> SerDes 29 */    
    {28,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 44 <-> SerDes 28 */
       
    {27,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 45 <-> SerDes 27 */
    {26,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 46 <-> SerDes 26 */
    {25,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 47 <-> SerDes 25 */
    {24,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 48 <-> SerDes 24 */
    {23,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 49 <-> SerDes 23 */
    {22,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 50 <-> SerDes 22 */
    {21,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 51 <-> SerDes 21 */    
    {20,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 52 <-> SerDes 20 */
    {19,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 53 <-> SerDes 19 */
    {18,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 54 <-> SerDes 18 */
    {17,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 55 <-> SerDes 17 */
    {16,  0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 56 <-> SerDes 16 */
       
    { 0, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 57 <-> SerDes  0 */
    { 1, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 58 <-> SerDes  1 */
    { 2, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 59 <-> SerDes  2 */    
    { 3, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 60 <-> SerDes  3 */
    { 4, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 61 <-> SerDes  4 */
    { 5, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 62 <-> SerDes  5 */
    { 6, 0, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 63 <-> SerDes  6 */
    { 7, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 64 <-> SerDes  7 */
    
     /*fix bug40611, adjust FFE, liangf, 2016-09-05*/          
    { 8, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 65 <-> SerDes  8 */
    { 9, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 66 <-> SerDes  9 */
    {10, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 67 <-> SerDes 10 */    
    {11, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 68 <-> SerDes 11 */
    {12, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 69 <-> SerDes 12 */
    {13, 1, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 70 <-> SerDes 13 */
    {14, 2, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 71 <-> SerDes 14 */
    {15, 2, 0xff, COEFF1(0xff, 0xff, 0xff, 0xff) },    /* Port 72 <-> SerDes 15 */
 };

glb_port_range_t lcm_sg8348c_phy_chip_port_range[SG8348C_PHY_INT_BITS_SUM] =
{
    { 1,  56},
};


/****************************************************************************
 *
* Function
*
****************************************************************************/
static int32
lcm_init_sg8348c_init_port_table(glb_card_t* p_card)
{
    uint8 port_id, panel_port_no;
    uint8 pos;

    /* SG8348C board no port num change */
    for(port_id=0; port_id<p_card->port_num; port_id++)
    {
        panel_port_no = p_card->pp_port[port_id]->panel_port_no;
        pos = panel_port_no - 1;
        sal_memcpy(&glb_lcm_sg8348c_port[port_id], &lcm_sg8348c_port_max[pos], sizeof(lcm_card_port_t));
    }

    return LCM_E_SUCCESS;
}

static int32
lcm_init_sg8348c_init_fiber_table(glb_card_t* p_card)
{
    uint8 port_id, panel_port_no, panel_subport_no;
    uint8 fiber_channel;

    for(port_id=0; port_id<p_card->port_num; port_id++)
    {
        panel_port_no = p_card->pp_port[port_id]->panel_port_no;
        panel_subport_no = p_card->pp_port[port_id]->panel_sub_port_no;
        if(panel_subport_no)
        {
            fiber_channel = panel_subport_no;
        }
        else
        {
            fiber_channel = 0;
        }
        sal_memcpy(&glb_lcm_sg8348c_fiber[port_id], &lcm_sg8348c_fiber_max[panel_port_no-1], sizeof(fiber_port_info_t));
        glb_lcm_sg8348c_fiber[port_id].fiber_channel = fiber_channel;
        glb_lcm_sg8348c_fiber[port_id].fiber_id = port_id;
    }
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_panel_port(glb_card_t* p_card)
{
    uint8 ppt_id;

    LCM_LOG_BRINGUP("Lcm init panel port begin.");

    /* 1, Allocate panel port memory */
    p_card->pp_ppt = (glb_panel_port_t** )XCALLOC(CTCLIB_MEM_LCM_PORT, 
        sizeof(glb_panel_port_t* )* p_card->panel_port_num);
    if (!p_card->pp_ppt)
    {
        LCM_LOG_ERR("Allocate pointer to global panel ports fail.");
        return LCM_E_NO_MEMORY;
    }

    /* 2, init panel port split/merge related data structure */
    for(ppt_id = 0; ppt_id < p_card->panel_port_num; ppt_id++)
    {
        p_card->pp_ppt[ppt_id] = (glb_panel_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(glb_panel_port_t));
        if(!p_card->pp_ppt[ppt_id])
        {
            LCM_LOG_ERR("Allocate global panel ports fail.");
            return LCM_E_NO_MEMORY;
        }
        
        p_card->pp_ppt[ppt_id]->ppt_idx = ppt_id + 1;
        p_card->pp_ppt[ppt_id]->pslt_id = p_card->phy_slot_no;
        p_card->pp_ppt[ppt_id]->ppt_numchg_type = GLB_PPT_NUMCHG_TYPE_NONE;
        p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
        if (ppt_id < (8+48))
        {   
            /* For bug38848, the first 8 ports with max speed 100M, qicx, 2016-06-02 */
            if (ppt_id < 8)
            {
                p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
                p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_100M;
                p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_100M;
            }
            else
            {
                p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
                p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_1G;
                p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_1G;
            }
        }
        else
        { 
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_10G | GLB_SUPPORT_SPEED_1G;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_10G;
        }
    }
    LCM_LOG_BRINGUP("Lcm init panel port end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_port(glb_card_t* p_card)
{
    int32 port_id = 0;
    uint32 ppt_idx;
    uint16 logic_port_idx;
    lcm_card_port_panel_mapping_t port_panel_mapping[256];

    LCM_LOG_BRINGUP("Lcm init port begin.");

    /* 1, init panel port */
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_panel_port(p_card));

    /* 2, read '/mnt/flash/startup-config.conf' file, get port split/merge info */
    LCM_IF_ERROR_RETURN(lcm_common_parse_port_numchg_info(p_card));

    if(p_card->port_num != SG8348C_PANEL_PORT_NUM)
    {
        p_card->split_flag = 1;
    }
    else
    {
        p_card->split_flag = 0;
    }

    /* 3, get panel_port/sub_port mapping */
    LCM_IF_ERROR_RETURN(lcm_common_ppt_map_port(p_card, port_panel_mapping));
    
    /* 4, integrate port info */
    p_card->pp_port = (glb_port_t** )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(glb_port_t* )* p_card->port_num);
    if (!p_card->pp_port)
    {
        LCM_LOG_ERR("Allocate pointer to global ports fail.");
        return LCM_E_NO_MEMORY;
    }

    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        p_card->pp_port[port_id] = (glb_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(glb_port_t));
        if(!p_card->pp_port[port_id])
        {
            LCM_LOG_ERR("Allocate global ports fail.");
            return LCM_E_NO_MEMORY;
        }
        p_card->pp_port[port_id]->panel_slot_no = port_panel_mapping[port_id].panel_slot_no;
        p_card->pp_port[port_id]->panel_port_no = port_panel_mapping[port_id].panel_port_no;
        p_card->pp_port[port_id]->panel_sub_port_no = port_panel_mapping[port_id].panel_subport_no;
    }

    /* 5, allocate memory for port/fiber/serdes data structure */
    glb_lcm_sg8348c_port = (lcm_card_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(lcm_card_port_t)* p_card->port_num);
    if (!glb_lcm_sg8348c_port)
    {
        LCM_LOG_ERR("Allocate pointer to lcm_card_port_t fail.");
        return LCM_E_NO_MEMORY;
    }

    glb_lcm_sg8348c_fiber = (fiber_port_info_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(fiber_port_info_t)*p_card->port_num);
    if (!glb_lcm_sg8348c_fiber)
    {
        LCM_LOG_ERR("Allocate pointer to fiber_port_info_t fail.");
        return LCM_E_NO_MEMORY;
    }

    /* 6, init current running port/fiber/serdes info */
    LCM_IF_ERROR_RETURN(lcm_init_sg8348c_init_port_table(p_card));
    LCM_IF_ERROR_RETURN(lcm_init_sg8348c_init_fiber_table(p_card));

    /* 7, init port properties */
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        logic_port_idx = glb_lcm_sg8348c_port[port_id].logic_port_idx;
        ppt_idx = p_card->pp_port[port_id]->panel_port_no;

        p_card->pp_port[port_id]->p_fiber = NULL;
        p_card->pp_port[port_id]->create_done = 1;
        p_card->pp_port[port_id]->is_combo = 0;
        p_card->pp_port[port_id]->port_idx = port_id + 1;
        p_card->pp_port[port_id]->logic_slot_idx = p_card->logic_slot_no;
        p_card->pp_port[port_id]->glb_chip_idx = p_card->phy_slot_no;
        p_card->pp_port[port_id]->port_cfg.loopback = GLB_LB_NONE;
        p_card->pp_port[port_id]->port_cfg.enable = 0;
        p_card->pp_port[port_id]->port_cfg.duplex = GLB_DUPLEX_AUTO;
        p_card->pp_port[port_id]->port_cfg.speed = GLB_SPEED_AUTO;
        p_card->pp_port[port_id]->port_cfg.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        p_card->pp_port[port_id]->port_cfg.flowctrl.recv = GLB_FLOWCTRL_DISABLE;

        p_card->pp_port[port_id]->port_status.link_up = GLB_LINK_DOWN;
        p_card->pp_port[port_id]->port_status.duplex = GLB_DUPLEX_AUTO;
        p_card->pp_port[port_id]->port_status.speed = GLB_SPEED_AUTO;
        p_card->pp_port[port_id]->port_status.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        p_card->pp_port[port_id]->port_status.flowctrl.recv = GLB_FLOWCTRL_DISABLE;
        p_card->pp_port[port_id]->poe_support = NOT_POE_PORT;
        p_card->pp_port[port_id]->is_combo_to = 0;
        p_card->pp_port[port_id]->lp_support = 1;  /* bug44010, support serdes loopback, qicx, 2017-05-15 */

        if (port_id < (8+48))
        {
            /* For bug38848, the first 8 ports with max speed 100M, qicx, 2016-06-02 */
            if (port_id < 8)
                p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_100BASE_TX;
            else
                p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_1000BASE_T;
            
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_COPPER;
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_UNSUPPORT;
            p_card->pp_port[port_id]->unidir_support = 0;
        }
        else
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_SFP_ONE_LED;
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
            p_card->pp_port[port_id]->unidir_support = 1;
        }

        p_card->pp_port[port_id]->port_speed_ability = p_card->pp_ppt[ppt_idx-1]->ppt_speed_ability;
        p_card->pp_port[port_id]->port_speed_real = p_card->pp_ppt[ppt_idx-1]->ppt_speed_real;
        p_card->pp_port[port_id]->mux_chip_id = 0;
        p_card->pp_port[port_id]->eee_support = 0;
        p_card->pp_port[port_id]->logic_port_idx = logic_port_idx;
        p_card->pp_port[port_id]->local_chip_idx = glb_lcm_sg8348c_port[port_id].chip_idx;
        p_card->pp_port[port_id]->mac_idx = glb_lcm_sg8348c_port[port_id].mac_idx;
        p_card->pp_port[port_id]->chip_serdes_id = glb_lcm_sg8348c_port[port_id].serdes_id;
        p_card->pp_port[port_id]->g_port_index = (p_card->pp_port[port_id]->glb_chip_idx << 8) | logic_port_idx;
        p_card->pp_port[port_id]->ctl_id = (logic_port_idx >= GG_SLICE1_BASE)?1:0;
        if(p_card->pp_port[port_id]->ctl_id == 1)
        {
            logic_port_idx = logic_port_idx - GG_SLICE1_BASE;
        }
        p_card->pp_port[port_id]->port_led_mac = (logic_port_idx >= 40)?
                (logic_port_idx+8):logic_port_idx;        
    }

    /* 8, create file '/tmp/ctcos_port_info' */
    LCM_IF_ERROR_RETURN(lcm_common_save_port_info_file(p_card));
    
    LCM_LOG_BRINGUP("Lcm init port end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_eeprom_info(glb_card_t* p_card)
{
    LCM_LOG_BRINGUP("Lcm init eeprom info begin.");

    p_card->p_eeprom_alloc = (eeprom_info_t *)XCALLOC(MTYPE_BUFFER_DATA, sizeof(eeprom_info_t)*EEPROM_MAX);
    if(NULL == p_card->p_eeprom_alloc)
    {
        LCM_LOG_ERR("alloc p_eeprom_alloc array fail.");
        return LCM_E_INVALID_PTR;
    }
/*system modified by liangf for bug 34292, support ls2f, 2015-07-06*/    
#ifdef _CTC_LOONGSON_  
    p_card->p_eeprom_alloc[EEPROM_BOOTCMD].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_BOOTCMD].base_addr = 0x800;
    
    p_card->p_eeprom_alloc[EEPROM_0_MANU_TEST].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_0_MANU_TEST].base_addr = 0x1900;

    p_card->p_eeprom_alloc[EEPROM_TCAM_PROFILE].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_TCAM_PROFILE].base_addr = 0x1260;
    
    p_card->p_eeprom_alloc[EEPROM_SERIALNO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_SERIALNO].base_addr = 0x1220;

    /*add by guhg,2015-11-19,
    0x55 mean reboot command ,0xaa mean sensor high tempture,default 0x00*/
    p_card->p_eeprom_alloc[EEPROM_REBOOT_INFO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_REBOOT_INFO].base_addr = 0x1268;
    
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
    p_card->p_eeprom_alloc[EEPROM_SMARTCFG].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_SMARTCFG].base_addr = 0x1265;
#endif /* !HAVE_SMARTCFG */

    p_card->p_eeprom_alloc[EEPROM_MGMT_CFG].eeprom_idx = 0;
    /*fix bug38531, liangf, 2016-06-01*/
    p_card->p_eeprom_alloc[EEPROM_MGMT_CFG].base_addr = 0x1100;

    p_card->p_eeprom_alloc[EEPROM_OEM_INFO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_OEM_INFO].base_addr = 0x1300;


    p_card->p_eeprom_alloc[EEPROM_PORT_INFO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_PORT_INFO].base_addr = 0x1400;
    p_card->p_eeprom_alloc[EEPROM_PHY_SERIES_LED].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_PHY_SERIES_LED].base_addr = 0x1015;
#endif    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_fiber(void)
{
    int32 ret = 0;
    
    LCM_LOG_BRINGUP("Lcm init fiber begin.");
    ret = fiber_init(glb_card->port_num,  /* fiber num */
                     glb_card->port_num, /* port num */
                     glb_lcm_sg8348c_fiber);

    LCM_LOG_BRINGUP("Lcm init fiber end.");

    return ret;
}

static int32
_lcm_init_sg8348c_sensor(glb_card_t* p_card)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[SG8348C_SENSOR_NUM];
    sensor_chip_t sensor_chip[SG8348C_SENSOR_NUM];
    void *p_data[SG8348C_SENSOR_NUM];

    LCM_LOG_BRINGUP("Lcm init sensor begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));

    i2c_gen[0].addr = SG8348C_ADDR_SENSOR;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_1;
    sensor_chip[0].io_type = SENSOR_I2C;
    i2c_gen[0].alen = LM77_OFFSET_WIDTH;
    sensor_chip[0].chip_type = SENSOR_LM77;
    sensor_chip[0].pos = SENSOR_BEFORE_CHIP;

    for(i=0; i<SG8348C_SENSOR_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = sensor_init(p_data, sensor_chip, SG8348C_SENSOR_NUM);

    if(ret < 0)
    {
        LCM_LOG_ERR("Temperature sensor init fail.");
        return LCM_E_INIT_FAILED;
    }

    {
        for(i=0; i<SG8348C_SENSOR_NUM; i++)
        {
           sensor_dev_init(i);
        }
    }
    LCM_LOG_BRINGUP("Lcm init sensor end.");
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_sg8348c_cfg_gpio(uint32 chip_id) ///TODO: need review by myself
{
    uint8 reg, val;

    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module begin.");
    switch (chip_id)
    {
    case 0:    /* PCA9505 */
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0xff;   /* input */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;   /* input */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0xff;   /* input */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0xff;   /* input */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
#if 0
        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK3;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
 #endif 
    case 1:    /* PCA9535 */
        reg = PCA9535_DIR_CTRL_REG_BANK0;
        val = 0x0;   /* output */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9535_DIR_CTRL_REG_BANK1;
        val = 0x0;   /* output */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);

        /* tx enable */
        reg = PCA9535_OUTPUT_PORT_REG_BANK0;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9535_OUTPUT_PORT_REG_BANK1;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        break;
    case 2:    /* PCA9535 */
        /*release PPU MAC LED*/
        reg = PCA9535_OUTPUT_PORT_REG_BANK1;
        val = 0x40;   /* output */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        
        reg = PCA9535_DIR_CTRL_REG_BANK1;
        val = 0x3f; /* bit 0/1/2/3/4/5 as input, 6/7 as output  */
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);

    default:
        break;
    }
    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module end.");
    
    return RESULT_OK;
}

static int32
_lcm_init_sg8348c_gpio(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[SG8348C_GPIO_CHIP_NUM];
    gpio_chip_t gpio_chip[SG8348C_GPIO_CHIP_NUM];
    void *p_data[SG8348C_GPIO_CHIP_NUM];

    LCM_LOG_BRINGUP("Lcm init i2c to gpio module begin.");
    
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));

    i2c_gen[0].addr = SG8348C_ADDR0_GPIO;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[0].alen = PCA95X5_OFFSET_WIDTH;
    gpio_chip[0].io_type = GPIO_I2C;    
    gpio_chip[0].chip_type = GPIO_PCA9505;
    gpio_chip[0].scan_group_bitmap = 0x0f; /*group 0,1,2,3*/

    i2c_gen[1].addr = SG8348C_ADDR1_GPIO;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].bridge_flag = 0;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[1].alen = PCA95X5_OFFSET_WIDTH;
    gpio_chip[1].io_type = GPIO_I2C;    
    gpio_chip[1].chip_type = GPIO_PCA9535;
    gpio_chip[1].scan_group_bitmap = 0x3; /*group 0,1 for TX en/dis*/

    i2c_gen[2].addr = SG8348C_ADDR2_GPIO;
    i2c_gen[2].i2c_type = E_I2C_CPM;
    i2c_gen[2].bridge_flag = 0;
    i2c_gen[2].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[2].alen = PCA95X5_OFFSET_WIDTH;
    gpio_chip[2].io_type = GPIO_I2C;    
    gpio_chip[2].chip_type = GPIO_PCA9535;
    gpio_chip[2].scan_group_bitmap = 0x0;  /*use only for ppu0_led_reset*/
    
    for(i=0; i<SG8348C_GPIO_CHIP_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = gpio_init(p_data, gpio_chip, SG8348C_GPIO_CHIP_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Gpio device init fail.");
        return LCM_E_INIT_FAILED;
    }

    for(i=0; i<SG8348C_GPIO_CHIP_NUM; i++)
    {
        _lcm_sg8348c_cfg_gpio(i);
    }
    LCM_LOG_BRINGUP("Lcm init i2c to gpio module end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_volt_mon_pin(glb_card_t* p_card)
{
#if 0
    uint8 svb_status = 0;
    float32 svb_volt[16] ={0.98125,0.97500,0.96875,0.96250,
                                      0.95625,0.95000,0.94375,0.93750 ,
                                      0.93125,0.92500,0.91875,0.91250,
                                      0.90625,0.90000 ,0.89375,0.88750
                                    };
    /*read svb from I2C-GPIO 0x27, liangf, 2016-05-26*/
    /*step1. configure direction, 0x27 IO1 0-3 INPUT(value:1)*/
    gpio_reg_read(2, PCA9535_DIR_CTRL_REG_BANK1, &svb_status, 1);
    svb_status |= 0x0f;
    gpio_reg_write(2, PCA9535_DIR_CTRL_REG_BANK1, &svb_status, 1);
    /*step2. read svb value*/
    gpio_reg_read(2, PCA9535_INPUT_PORT_REG_BANK1, &svb_status, 1);
    svb_status = svb_status & 0xf;
    
    /*******************************the first chip  LTC 2991********************************************/
    p_card->ltc_chip_type[0] = GLB_LTC_CHIP_LTC2991;
     /*init PIN0*/
    p_card->volt_curr_cfg[0][0].flag = GLB_LTC_PIN_VOLT ;
    p_card->volt_curr_cfg[0][0].volt_split_factor = 11.0;
    p_card->volt_curr_cfg[0][0].low = 12.0*0.9;
    p_card->volt_curr_cfg[0][0].high = 12.0*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][0].name, "12V0", sizeof("12V0"));

    /*init PIN1*/
    p_card->volt_curr_cfg[0][1].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][1].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[0][1].low = svb_volt[svb_status]*0.95;
    p_card->volt_curr_cfg[0][1].high = svb_volt[svb_status]*1.05;
    sal_memcpy(p_card->volt_curr_cfg[0][1].name, "PPU_CORE", sizeof("PPU_CORE"));

    /*init PIN2*/
    p_card->volt_curr_cfg[0][2].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][2].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[0][2].low = 1.15*0.9;
    p_card->volt_curr_cfg[0][2].high = 1.15*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][2].name, "1V15", sizeof("1V15"));

    /*init PIN3*/
    p_card->volt_curr_cfg[0][3].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][3].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[0][3].low = 2.5*0.9;
    p_card->volt_curr_cfg[0][3].high = 2.5*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][3].name, "CPU_2V5", sizeof("CPU_2V5"));
    
    /*init PIN4*/
    p_card->volt_curr_cfg[0][4].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][4].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[0][4].low = 1.5*0.9;
    p_card->volt_curr_cfg[0][4].high = 1.5*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][4].name, "CPU_DDR_1V5", sizeof("CPU_DDR_1V5"));
    
    /*init PIN5*/
    p_card->volt_curr_cfg[0][5].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][5].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[0][5].low = 1.8*0.9;
    p_card->volt_curr_cfg[0][5].high = 1.8*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][5].name, "1V8", sizeof("1V8"));
    
    /*init PIN6*/
    p_card->volt_curr_cfg[0][6].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][6].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[0][6].low = 1.2*0.9;
    p_card->volt_curr_cfg[0][6].high = 1.2*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][6].name, "1V2", sizeof("1V2"));
    
    /*init PIN7*/
    p_card->volt_curr_cfg[0][7].flag = GLB_LTC_PIN_VOLT;
    p_card->volt_curr_cfg[0][7].volt_split_factor = 1.1;
    p_card->volt_curr_cfg[0][7].low = 3.3*0.9;
    p_card->volt_curr_cfg[0][7].high = 3.3*1.1;
    sal_memcpy(p_card->volt_curr_cfg[0][7].name, "3V3", sizeof("3V3"));

    /*******************************the second chip  LTC 4151********************************************/
    p_card->ltc_chip_type[1] = GLB_LTC_CHIP_LTC_4151;
    /*init PIN0*/
    p_card->volt_curr_cfg[1][0].flag = GLB_LTC_PIN_VOLT | GLB_LTC_PIN_VOLT_POWER;
    p_card->volt_curr_cfg[1][0].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[1][0].low = 12.0*0.9;
    p_card->volt_curr_cfg[1][0].high = 12.0*1.1;
    sal_memcpy(p_card->volt_curr_cfg[1][0].name, "12V0", sizeof("12V0"));

    /*init PIN1*/
    /*flag GLB_LTC_PIN_FLAG_OVER mean the pin is the lastest one*/
    p_card->ltc_rsense[1][1] = 0.5; /*used for calculate system current,unit: m¦¸*/
    p_card->volt_curr_cfg[1][1].flag = GLB_LTC_PIN_CURR | GLB_LTC_PIN_CURR_POWER | GLB_LTC_PIN_FLAG_OVER;
    p_card->volt_curr_cfg[1][1].volt_split_factor = 1.0;
    p_card->volt_curr_cfg[1][1].low = 12.0*0.9;
    p_card->volt_curr_cfg[1][1].high = 12.0*1.1;
    sal_memcpy(p_card->volt_curr_cfg[1][1].name, "12V0", sizeof("12V0"));
#endif    
    return 0;
}


static int32
_lcm_init_sg8348c_volt_curr_mon(glb_board_type_t board_type)
{  
#if 0
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[SG8348C_VOLT_CURR_MON_NUM];
    ltc2991_chip_t ltc2991_chip[SG8348C_LTC2991_MON_NUM];
    ltc4151_chip_t ltc4151_chip[SG8348C_LTC4151_MON_NUM];
    
    void *p_data[SG8348C_VOLT_CURR_MON_NUM];

    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
   //need to add array contain addr of volt-curr
    i2c_gen[0].addr = SG8348C_ADDR0_VOLT_CURR_MON;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = LTC2991_OFFSET_WIDTH;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[0].bridge_flag = 0;
    ltc2991_chip[0].chip_type = LTC2991_CMS;
    ltc2991_chip[0].io_type = LTC2991_I2C;
    p_data[0] = &i2c_gen[0];


    i2c_gen[1].addr = SG8348C_ADDR1_VOLT_CURR_MON;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].alen = LTC4151_OFFSET_WIDTH;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[1].bridge_flag = 0;
    ltc4151_chip[0].chip_type = LTC4151_CDD;
    ltc4151_chip[0].io_type = LTC4151_I2C;
    p_data[1] = &i2c_gen[1];
    ret = ltc2991_init(p_data, ltc2991_chip, SG8348C_LTC2991_MON_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Ltc2991 init fail.");
        return LCM_E_INIT_FAILED;
    }    

    for(i=0; i<SG8348C_LTC2991_MON_NUM; i++)
    {
        ret = ltc2991_dev_init(board_type, i);
        if(ret < 0)
        {
            LCM_LOG_ERR("Ltc2991 dev init fail.");
            return LCM_E_INIT_FAILED;
        }
    }

    ret = ltc4151_init(p_data+1, ltc4151_chip, SG8348C_LTC4151_MON_NUM);

    if(ret < 0)
    {
        LCM_LOG_ERR("Ltc4151 init fail.");
        return LCM_E_INIT_FAILED;
    }    
    for(i=0; i<SG8348C_LTC4151_MON_NUM; i++)
    {
        ltc4151_dev_init(i);
        if(ret < 0)
        {
            LCM_LOG_ERR("Ltc4151 dev init fail.");
            return LCM_E_INIT_FAILED;
        }
    }
#endif
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_eeprom(void)
{
    int32 ret = 0;
    i2c_gen_t i2c_gen[SG8348C_EEPROM_NUM];

    LCM_LOG_BRINGUP("Lcm init eeprom begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));

    i2c_gen[0].addr = SG8348C_ADDR_EEPROM;

    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = EEPROM_OFFSET_WIDTH;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_1;

    ret = eeprom_init(i2c_gen, SG8348C_EEPROM_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm EEPROM init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_BRINGUP("Lcm init eeprom end.");
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_phy(void)
{
    uint16 port_id, val;
    phy_info_t phyinfo;
    phy_handle_t** pphdl = NULL;
    glb_port_t* p_port;
    int32 ret;
    uint8 phy_led_type = GLB_PHY_LED_6;

    LCM_LOG_BRINGUP("Lcm init phy begin.");

    sal_memset(&phyinfo, 0, sizeof(phy_info_t));
    pphdl = (phy_handle_t**)LCM_CALLOC(CTCLIB_MEM_LCM_MODULE,
        sizeof(phy_handle_t*)*(glb_card->port_num + SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP));

    if(NULL == pphdl)
    {
        LCM_LOG_ERR("LCM phy no memory.");
        return LCM_E_NO_MEMORY;
    }

    for(port_id = 0; port_id < (glb_card->port_num + SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP); port_id++)
    {
        p_port = glb_card->pp_port[port_id];
        if(port_id >= SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP)
            p_port = glb_card->pp_port[port_id - SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP];

        if (port_id < SG8348C_PHY_PORT_NUM + SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP)
        {
            phyinfo.phy_device_type = PORT_PHY_VSC8512;
            phyinfo.phy_manage_info.speed = GLB_SPEED_AUTO;
            phyinfo.phy_manage_info.duplex = GLB_DUPLEX_AUTO;
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_RJ45;
            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_SGMII;
            /*fix bug38171, liangf, 2016-05-17*/
            phyinfo.vct_support = 1;
            
            phyinfo.led_type = phy_led_type; /*read from eeprom,must according to enum glb_phy_led_type_e in glb_hw_define.h*/
            /* added by tongzb, for config led freq, 2017-01-06 */
            phyinfo.led_freq = GLB_PHY_LED_FREQ_2P5;
            if(port_id < SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP)
                phyinfo.phy_addr = SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP-1-port_id;
            else
                phyinfo.phy_addr = lcm_sg8348c_port_max[port_id - SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP].phy_addr;
            /*
            if(port_id < SG8348C_PHY_MGMT_PORT_NUM)
                phyinfo.led_type = GLB_PHY_LED_9;
            else
                phyinfo.led_type = GLB_PHY_LED_8;*/
            phyinfo.mdio_bus = ASIC_GG_PHY_1G_TYPE;
            if (port_id < 8 + SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP)
                phyinfo.base_addr = 3;
            else if (port_id < (8+24+SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP))
                phyinfo.base_addr = 2;
            else
                phyinfo.base_addr = 0;

            if (port_id < 8+SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP)
            {
                if(port_id == 0x03)
                    phyinfo.phy_manage_info.phy_init_seq_flag = GLB_PRE_INIT_FLAG;
                else if(port_id == 0x0b)                    
                    phyinfo.phy_manage_info.phy_init_seq_flag = GLB_POST_INIT_FLAG;
                else
                    phyinfo.phy_manage_info.phy_init_seq_flag = GLB_NO_INIT_FLAG;
            }
            else
            {
                 if(SG8348C_LAST_INTERNAL_PHY_ADDR == (port_id % SG8348C_PORT_NUM_PER_PHY_CHIP))
                {
                    phyinfo.phy_manage_info.phy_init_seq_flag = GLB_PRE_INIT_FLAG|GLB_POST_INIT_FLAG;
                }
                else
                {
                    phyinfo.phy_manage_info.phy_init_seq_flag = GLB_NO_INIT_FLAG;
                }
            }                
        }
        else
        {
            phyinfo.phy_manage_info.speed = GLB_SPEED_10G;
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_SFP_PLUS;
            phyinfo.phy_device_type = PORT_PHY_NULL;
            phyinfo.phy_manage_info.duplex = GLB_DUPLEX_AUTO;
            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_SERDES;  
   
            if (lcm_stack_is_enable())
            {
                phyinfo.port_info.port_id = GLB_TRANS_PORTID_TO_GPORT(
                        lcm_stack_self_slotid(), p_port->logic_port_idx);            
            }
            else
            {
                phyinfo.port_info.port_id = p_port->logic_port_idx;
            }
            phyinfo.port_info.lchip = p_port->local_chip_idx;
            phyinfo.port_info.serdes_id = p_port->chip_serdes_id;
            phyinfo.port_info.serdes_num = 1;   /* bug44010, support serdes loopback, qicx, 2017-05-15 */
        }

        /* Modified by liuht for access port id in phy handle for bug 25808 */	
        if (port_id >= SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP)	
            phyinfo.port_num = port_id -SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP;

        /*********************************************************************
         * Default:
         * PHY_WORK_MODE_NORMAL GLB_LB_NONE GLB_SPEED_AUTO
         * GLB_DUPLEX_AUTO
         ********************************************************************/
        phyinfo.phy_manage_info.mode = PHY_WORK_MODE_NORMAL;
        phyinfo.phy_manage_info.lb_mode = GLB_LB_NONE;
        phyinfo.phy_manage_info.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        phyinfo.phy_manage_info.flowctrl.recv= GLB_FLOWCTRL_DISABLE;
        phyinfo.phy_manage_info.flowctrl_ability.asymmetric_pause = 0;
        phyinfo.phy_manage_info.flowctrl_ability.symmetric_pause = 0;

        phyinfo.phy_stat_flag.duplex = phyinfo.phy_manage_info.duplex;
        phyinfo.phy_stat_flag.speed = phyinfo.phy_manage_info.speed;
        phyinfo.phy_stat_flag.link_up = GLB_LINK_DOWN;
        phyinfo.phy_stat_flag.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        phyinfo.phy_stat_flag.flowctrl.recv = GLB_FLOWCTRL_DISABLE;

        pphdl[port_id] = phy_dev_register(&phyinfo);
        if(NULL ==  pphdl[port_id])
        {
            LCM_LOG_ERR("Register phy handle failed\n");
            return LCM_E_INIT_FAILED;
        }
    }

    LCM_LOG_BRINGUP("Lcm ready to init phy.");
    ret = phy_init(pphdl, (glb_card->port_num | (SG8348C_NOT_USE_PORT_NUM_FIRST_PHY_CHIP << 16)));
    if(ret)
    {
        return LCM_E_INIT_FAILED;
    }

    for(port_id = 0; port_id < SG8348C_PHY_MGMT_PORT_NUM; port_id++)
    {
        /*remove the port 1000M capability*/
        gephy_reg_read(port_id, 0x9, &val);
        val &=0xfcff;
        gephy_reg_write(port_id, 0x9, val);
    }
    LCM_LOG_BRINGUP("Lcm init phy end.");
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_sg8348c_i2c(void)
{
    int32 ret;

    LCM_LOG_BRINGUP("Lcm init i2c bus begin.");

    ret = i2c_open(E_I2C_CPM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Init i2c bus failed.");
        return LCM_E_HW_CLKGEN_GEN;
    }
    LCM_LOG_BRINGUP("Lcm init i2c bus end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_sg8348c_led(void)
{
#if 0
    int32 ret = 0;
    led_info_t led_info[E_MAX_LED];

    LCM_LOG_BRINGUP("Lcm init LED begin.");
    sal_memset(led_info, 0, sizeof(led_info_t)*E_MAX_LED);

    /* init sys led */
    led_info[E_SYS_LED].exist = 0;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].val = 0x5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].val = 0xb;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].val = 0x5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].val = 0x7;
    /* End of Merge */
    led_info[E_FAN_LED].exist = 0;
    led_info[E_FAN_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ALARM].val = 0x2;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;
    led_info[E_PSU1_LED].exist = 0;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ALARM].val = 0x2;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;
    led_info[E_PSU2_LED].exist = 0;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_PSU2_LED;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_PSU2_LED;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ALARM].val = 0x2;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_PSU2_LED;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;	
    led_info[E_IND_LED].exist = 1;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].op_reg = EPLD_IND_LED;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].val = 0x1;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].op_reg = EPLD_IND_LED;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].val = 0x0;
	
    ret = led_init(led_info);
    if(ret < 0)
    {
        LCM_LOG_ERR("Init LEDs failed.");
        return LCM_E_HW_LED_GEN;
    }
    LCM_LOG_BRINGUP("Lcm init LED end.");
#endif
    return LCM_E_SUCCESS;
}



static int32
_lcm_init_sg8348c_ctc_chip_led(void)
{
    uint8 table_id = 0;
    uint8 table_num = 1;
    mac_led_info_t mac_led_info;

    LCM_LOG_BRINGUP("Lcm init MAC LED begin.");
 
    mac_led_info.mac_led_api_para = (mac_led_api_para_t **)XCALLOC(CTCLIB_MEM_LCM_MODULE, 
        sizeof(mac_led_api_para_t*)*table_num);
    if(NULL == mac_led_info.mac_led_api_para)
    {
        LCM_LOG_ERR("Alloc mac_led_api_para_t array fail.");
        return LCM_E_NO_MEMORY;
    }
    
    for(table_id = 0; table_id < table_num; table_id++)
    {
        mac_led_info.mac_led_api_para[table_id] = lcm_sg8348c_mac_led_default_entry;
    }

    mac_led_info.table_num = table_num;
    mac_led_info.mac_num = SG8348C_LED_MAC_NUM;
    mac_led_info.slice0_mac_num = SG8348C_SLICE0_LED_MAC_NUM;
    mac_led_info.polarity = 1;
    mac_led_info_register(&mac_led_info);
    mac_led_init();

    XFREE(CTCLIB_MEM_LCM_MODULE, mac_led_info.mac_led_api_para);

    LCM_LOG_BRINGUP("Lcm init MAC LED end.");
    
    return LCM_E_SUCCESS;
}

int32
lcm_sg8348c_get_sum_ffe_cfg(lcm_chip_serdes_ffe_t* serdes_ffe)
{
    uint8 serdes_id;
    uint8 index;

    serdes_id = serdes_ffe->serdes_id;
    
    /*FR4*/
    serdes_ffe->board_material = glb_card->board_material;
    for(index = 0; index < glb_card->port_num; index++)
    {
        if(serdes_id == lcm_sg8348c_serdes_ffe_max[index].serdes_id)
        {
            if(0xff == lcm_sg8348c_serdes_ffe_max[index].coeff1.c0)  
            {
                serdes_ffe->mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;  /*typical*/
                serdes_ffe->trace_len = lcm_sg8348c_serdes_ffe_max[index].trace_len;                    
            }
            else
            {
                serdes_ffe->mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;  /*user-defined*/
                serdes_ffe->c0 = lcm_sg8348c_serdes_ffe_max[index].coeff1.c0;
                serdes_ffe->c1 = lcm_sg8348c_serdes_ffe_max[index].coeff1.c1;
                serdes_ffe->c2 = lcm_sg8348c_serdes_ffe_max[index].coeff1.c2;
                serdes_ffe->c3 = lcm_sg8348c_serdes_ffe_max[index].coeff1.c3;
            }
            return 0;
        }  
    }    

    LCM_LOG_ERR("Lcm get sum ffe cfg error, doesn't find ffe param.");
    return -1;
}


int32
lcm_init_sg8348c_serdes(void)
{
    ctc_chip_serdes_ffe_t serdes_ffe;
    uint8 index;
    int32 ret = 0;
    
    LCM_LOG_BRINGUP("Lcm init serdes begin.");
    
    /*SG8348C is FR4 */
    /* 1. init user-defined ffe */
    serdes_ffe.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;             /* user-defined */
    for(index = 0; index < glb_card->port_num; index++)
    {
        if (0xff == lcm_sg8348c_serdes_ffe_max[index].coeff1.c0)
            continue;
        
        serdes_ffe.serdes_id = lcm_sg8348c_serdes_ffe_max[index].serdes_id;
        serdes_ffe.coefficient[0] = lcm_sg8348c_serdes_ffe_max[index].coeff1.c0;
        serdes_ffe.coefficient[1] = lcm_sg8348c_serdes_ffe_max[index].coeff1.c1;
        serdes_ffe.coefficient[2] = lcm_sg8348c_serdes_ffe_max[index].coeff1.c2;
        serdes_ffe.coefficient[3] = lcm_sg8348c_serdes_ffe_max[index].coeff1.c3;
        ret = ctc_chip_set_property(0, CTC_CHIP_PROP_SERDES_FFE, (void *)&serdes_ffe);
        if (ret < 0)
        {
            LCM_LOG_ERR("LCM set serdes ffe parameter failed.\n");
            return LCM_E_INIT_FAILED;
        }
    }
    
    /* 2. init typical ffe */
    serdes_ffe.mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;             /* typical */
    serdes_ffe.board_material = glb_card->board_material;   /* FR4 */
    for(index = 0; index < glb_card->port_num; index++)
    {
        if (0xff != lcm_sg8348c_serdes_ffe_max[index].coeff1.c0)
            continue;
        
        serdes_ffe.serdes_id = lcm_sg8348c_serdes_ffe_max[index].serdes_id;
        serdes_ffe.trace_len = lcm_sg8348c_serdes_ffe_max[index].trace_len;
        ret = ctc_chip_set_property(0, CTC_CHIP_PROP_SERDES_FFE, (void *)&serdes_ffe);
        if (ret < 0)
        {
            LCM_LOG_ERR("LCM set serdes ffe parameter failed.\n");
            return LCM_E_INIT_FAILED;
        }  
    }

    LCM_LOG_BRINGUP("Lcm init serdes end.");
    
    return ret;
}

int32
lcm_init_sg8348c_cb(void)
{
    LCM_LOG_BRINGUP("Lcm init callback begin.");
    /* LCM_IF_ERROR_RETURN(lcm_init_sg8348c_ctc_gpio_init()); */
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_fiber());
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_phy());
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_ctc_chip_led());
    LCM_IF_ERROR_RETURN(lcm_init_sg8348c_serdes());
    LCM_LOG_BRINGUP("Lcm init callback end.");

    return LCM_E_SUCCESS;
}
extern int32 lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func);
int32
lcm_init_sg8348c(glb_card_t* p_card)
{ 
    LCM_LOG_BRINGUP("Lcm init board begin.");
    glb_card = p_card;
    p_card->asic_chip_num = 1;
    p_card->chip_sensor_num = 1;

    lcm_common_get_slot_no(&p_card->logic_slot_no, &p_card->phy_slot_no);
    p_card->logic_slot_no = 1;
    p_card->phy_slot_no = 0;

    p_card->panel_port_num = SG8348C_PANEL_PORT_NUM;
    p_card->l2switch_num = 0;
    //p_card->volt_curr_mon_num = SG8348C_VOLT_CURR_MON_NUM;
    p_card->sensor_num = SG8348C_SENSOR_NUM;
    if(p_card->chip_sensor_num+p_card->sensor_num > MAX_TMPR_SENSOR_NUM)
    {
        LCM_LOG_ERR("Sensor structure is too small to store sensor info on board.\n");        
        return LCM_E_NO_MEMORY;
    }
    p_card->fan_module_num = SG8348C_FAN_MODULE_MAX;
    p_card->psu_module_num = SG8348C_PSU_MODULE_MAX;
    p_card->volt_curr_mon_num = SG8348C_VOLT_CURR_MON_NUM;

    p_card->phy_interrupt_mode = GLB_PHY_INTERRUPT_MODE_NULL;   /*get phy interrupt info from EPLD*/
    p_card->phy_int_bit_num = SG8348C_PHY_INT_BITS_SUM;
    p_card->phy_intr_en = 1;
    p_card->p_port_range = lcm_sg8348c_phy_chip_port_range;
    p_card->cpu_type = GLB_CPU_LS2H;
    p_card->phy_chip_num = 5;
    p_card->tmpr_limit.board_low = ODM_DEFAULT_LOW_TMPR;
    p_card->tmpr_limit.board_high = ODM_DEFAULT_HIGH_TMPR;
    p_card->tmpr_limit.board_crit = ODM_DEFAULT_CRIT_TMPR;
    
    /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    p_card->ptp_en = 0;
    p_card->bootrom_type = E_BOOTROM_TYPE_4M;
    p_card->board_material = 0; //all use FR4

    sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                DATAPATH_PROFILE_PATH, SG8348C_NOBKPORT_DATAPATH_NAME);
    
    lcm_common_init_cpu_info();
    ctc_hw_fd_init();  /* For dpll interrupt */
    ctc_phy_fd_init(); /* For phy interrupt */
    ctc_sys_led_fd_init();
    
    LCM_LOG_BRINGUP("card product type %x, board type %x.", p_card->board_type.series, p_card->board_type.type);
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_eeprom_info(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_led());
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_i2c());
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_eeprom());
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_port(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_sensor(p_card));    
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_gpio());
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_volt_mon_pin(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_sg8348c_volt_curr_mon(p_card->board_type));

    LCM_IF_ERROR_RETURN(lcm_common_parse_bootup_diag(p_card));


    lcm_card_init_callback(lcm_init_sg8348c_cb);
    lcm_port_init_ffe_cfg_callback(lcm_sg8348c_get_sum_ffe_cfg);

    LCM_LOG_BRINGUP("Lcm init board end.");

    return LCM_E_SUCCESS;
}

