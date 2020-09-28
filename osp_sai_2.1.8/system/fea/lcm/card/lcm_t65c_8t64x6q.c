/****************************************************************************
* $Id$
*  T65C_8T64X6Q board init functions
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : qianj
* Date          : 2019-05-30
* Reason        : First Create.
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
#include "epld_api.h"
#include "phy_api.h"
#include "fiber_api.h"
#include "sensor_api.h"
#include "gpio_api.h"
#include "fan_api.h"
#include "power_api.h"
#include "eeprom_api.h"
#include "led_api.h"
#include "ltc2991_api.h"
#include "lcm_mgt.h"
#include "ctc_api.h"
#include "ctc_chip.h"
#include "glb_distribute_system_define.h"
#include "glb_if_define.h"
#include "clock_api.h"
#include "ctc_sync_ether.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
 
/*
 * CPU I2C slave device
 */

/* Thermal sensor */
#define T65C_8T64X6Q_SENSOR_NUM    0x03
/* Sensor 0/1 on motherboard */
#define T65C_8T64X6Q_ADDR0_SENSOR                    0x48
#define T65C_8T64X6Q_ADDR1_SENSOR                    0x4a
/* Sensor 2 on daughterboard */
#define T65C_8T64X6Q_ADDR2_SENSOR                    0x49

/* EEPROM */
#define T65C_8T64X6Q_EEPROM_NUM                      0x03
#define T65C_8T64X6Q_ADDR_MASTER_EEPROM_0              0x57
#define T65C_8T64X6Q_ADDR_MASTER_EEPROM_1              0x56
/* eeprom 2 on daughterboard */
#define T65C_8T64X6Q_ADDR_DAU_EEPROM                 0x57

/* I2C bridge */
#define T65C_8T64X6Q_I2C_BRIDGE_ADDR_0      0x70 // under i2c0 bus of cpu 

/*
 * CPU I2C slave device, after i2c bridge
 */ 

/* Power supply */
#define T65C_8T64X6Q_PSU_MODULE_MAX                  0x2
#define T65C_8T64X6Q_ADDR0_PSU                       0x70
#define T65C_8T64X6Q_ADDR1_PSU                       0x70

/* Fan */
#define T65C_8T64X6Q_FAN_MODULE_MAX                  0x2
/* Fan controller0, controll fan 0/1/2/3 */
#define T65C_8T64X6Q_ADDR0_FAN                       0x2f
/* Fan controller1, controll fan 4/5/6/7 */
#define T65C_8T64X6Q_ADDR1_FAN                       0x2c

/* I2C to GPIO: PCA9505 */
#define T65C_8T64X6Q_GPIO_CHIP_NUM                   0x05
/* gpio 0/1/2 on motherboard */
#define T65C_8T64X6Q_ADDR0_GPIO                      0x20
#define T65C_8T64X6Q_ADDR1_GPIO                      0x20
#define T65C_8T64X6Q_ADDR2_GPIO                      0x20
/* gpio 3/4 on daughterboard */
#define T65C_8T64X6Q_ADDR3_GPIO                      0x20
#define T65C_8T64X6Q_ADDR4_GPIO                      0x20

/*dpll*/
#define T65C_8T64X6Q_AD9559_NUM                      1

/* Port number property */
/* 
 * 8T
 * 64 XFI
 * 6Q 
 */
#define T65C_8T64X6Q_8T 8
#define T65C_8T64X6Q_64X 64
#define T65C_8T64X6Q_6Q 6
#define T65C_8T64X6Q_8T64X (T65C_8T64X6Q_8T + T65C_8T64X6Q_64X)

#define T65C_8T64X6Q_PANEL_PORT_NUM_MAX (T65C_8T64X6Q_8T + T65C_8T64X6Q_64X + T65C_8T64X6Q_6Q * 4)
#define T65C_8T64X6Q_PANEL_PORT_NUM     (T65C_8T64X6Q_8T + T65C_8T64X6Q_64X + T65C_8T64X6Q_6Q)
#define T65C_8T64X6Q_FIBER_PORT_NUM     (T65C_8T64X6Q_64X + T65C_8T64X6Q_6Q)

/*PHY*/
#define T65C_8T64X6Q_VSC8512_PHY_NUM   1  
#define T65C_8T64X6Q_PHY_INT_BITS_SUM   1
#define T65C_8T64X6Q_PHY_FIRST_ADDR          3
#define T65C_8T64X6Q_PHY_LAST_ADDR           4

#define T65C_8T64X6Q_SLICE0_LED_MAC_NUM              40
#define T65C_8T64X6Q_LED_MAC_NUM                     T65C_8T64X6Q_FIBER_PORT_NUM 
#define T65C_8T64X6Q_LED_TBL_NUM                     1

#define AD9559_SOFT_IO_UPDATE           0x0005
#define AD9559_IRQ_CLEARING                 0x0a05

/****************************************************************************
*
* Global and Declarations
*
****************************************************************************/
static const glb_card_t         *glb_card;
static lcm_card_port_t          *glb_lcm_t65c_8t64x6q_port             = NULL;
static fiber_port_info_t        *glb_lcm_t65c_8t64x6q_fiber            = NULL;

extern int32 lcm_card_init_callback(card_init_callback_t func);

lcm_card_port_t lcm_t65c_8t64x6q_port_max[T65C_8T64X6Q_PANEL_PORT_NUM_MAX] =
{
     /* 8T*/
    { 0,   5,    5,   5,   },    /* Port  1 <-> SerDes  5 */
    { 0,   4,    4,   4,   },    /* Port  2 <-> SerDes  4 */
    { 0,   1,    1,   1,   },    /* Port  3 <-> SerDes  1 */
    { 0,   0,    0,   0,   },    /* Port  4 <-> SerDes  0 */
    { 0,   7,    7,   7,   },    /* Port  5 <-> SerDes  7 */
    { 0,   6,    6,   6,   },    /* Port  6 <-> SerDes  6 */
    { 0,   3,    3,   3,   },    /* Port  7 <-> SerDes  3 */
    { 0,   2,    2,   2,   },    /* Port  8 <-> SerDes  2 */
    /* 64 XFI*/
    { 0,   16,   16,       },    /* Port  9 <-> SerDes  16 */
    { 0,   17,   17,       },    /* Port  10 <-> SerDes  17 */
    { 0,   8,     8,       },    /* Port  11 <-> SerDes  8 */
    { 0,   9,     9,       },    /* Port  12 <-> SerDes  9 */
    { 0,   18,   18,       },    /* Port  13 <-> SerDes  18 */
    { 0,   19,   19,       },    /* Port  14 <-> SerDes  19 */
    { 0,   10,   10,       },    /* Port  15 <-> SerDes  10 */
    { 0,   11,   11,       },    /* Port  16 <-> SerDes  11 */
    { 0,   20,   20,       },    /* Port  17 <-> SerDes  20 */
    { 0,   21,   21,       },    /* Port  18 <-> SerDes  21 */
    { 0,   12,   12,       },    /* Port  19 <-> SerDes  12 */
    { 0,   13,   13,       },    /* Port  20 <-> SerDes  13 */
    { 0,   22,   22,       },    /* Port  21 <-> SerDes  22 */
    { 0,   23,   23,       },    /* Port  22 <-> SerDes  23 */
    { 0,   14,   14,       },    /* Port  23 <-> SerDes  14 */
    { 0,   15,   15,       },    /* Port  24 <-> SerDes  15 */
    { 0,   24,   24,       },    /* Port  25 <-> SerDes  24 */
    { 0,   25,   25,       },    /* Port  26 <-> SerDes  25 */
    { 0,   34,   34,       },    /* Port  27 <-> SerDes  34 */
    { 0,   35,   35,       },    /* Port  28 <-> SerDes  35 */
    { 0,   26,   26,       },    /* Port  29 <-> SerDes  26 */
    { 0,   27,   27,       },    /* Port  30 <-> SerDes  27 */

    { 0,   40,   40,       },    /* Port  31 <-> SerDes  36 */
    { 0,   41,   41,       },    /* Port  32 <-> SerDes  37 */

    { 0,   28,   28,       },    /* Port  33 <-> SerDes  28 */
    { 0,   29,   29,       },    /* Port  34 <-> SerDes  29 */

    { 0,   42,   42,       },    /* Port  35 <-> SerDes  38 */
    { 0,   43,   43,       },    /* Port  36 <-> SerDes  39 */

    { 0,   30,   30,       },    /* Port  37 <-> SerDes  30 */
    { 0,   31,   31,       },    /* Port  38 <-> SerDes  31 */

    { 0,   36,   36,       },    /* Port  39 <-> SerDes  40 */
    { 0,   37,   37,       },    /* Port  40 <-> SerDes  41 */
    { 0,   32,   32,       },    /* Port  41 <-> SerDes  32 */
    { 0,   33,   33,       },    /* Port  42 <-> SerDes  33 */

    { 0,   38,   38,       },    /* Port  43 <-> SerDes  42 */
    { 0,   39,   39,       },    /* Port  44 <-> SerDes  43 */

    { 0,   46 + GG_SLICE1_BASE,   46 + GG_SLICE1_BASE,       },    /* Port  45 <-> SerDes  94 */
    { 0,   47 + GG_SLICE1_BASE,   47 + GG_SLICE1_BASE,       },    /* Port  46 <-> SerDes  95 */
    { 0,   47 ,  47,       },                                      /* Port  47 <-> SerDes  47 */
    { 0,   46 ,  46,       },                                      /* Port  48 <-> SerDes  46 */

    { 0,   39 + GG_SLICE1_BASE,   39 + GG_SLICE1_BASE,       },    /* Port  49 <-> SerDes  91 */
    { 0,   38 + GG_SLICE1_BASE,   38 + GG_SLICE1_BASE,       },    /* Port  50 <-> SerDes  90 */
    { 0,   45,   45,       },                                      /* Port  51 <-> SerDes  45 */
    { 0,   44,   44,       },                                      /* Port  52 <-> SerDes  44 */
    { 0,   37 + GG_SLICE1_BASE,   37 + GG_SLICE1_BASE,       },    /* Port  53 <-> SerDes  89 */
    { 0,   36 + GG_SLICE1_BASE,   36 + GG_SLICE1_BASE,       },    /* Port  54 <-> SerDes  88 */
    { 0,   44 + GG_SLICE1_BASE,   44 + GG_SLICE1_BASE,       },    /* Port  55 <-> SerDes  92 */
    { 0,   45 + GG_SLICE1_BASE,   45 + GG_SLICE1_BASE,       },    /* Port  56 <-> SerDes  93 */

    { 0,   43 + GG_SLICE1_BASE,   43 + GG_SLICE1_BASE,       },    /* Port  57 <-> SerDes  87 */
    { 0,   42 + GG_SLICE1_BASE,   42 + GG_SLICE1_BASE,       },    /* Port  58 <-> SerDes  86 */

    { 0,   27 + GG_SLICE1_BASE,   27 + GG_SLICE1_BASE,       },    /* Port  59 <-> SerDes  75 */
    { 0,   26 + GG_SLICE1_BASE,   26 + GG_SLICE1_BASE,       },    /* Port  60 <-> SerDes  74 */
    { 0,   41 + GG_SLICE1_BASE,   41 + GG_SLICE1_BASE,       },    /* Port  61 <-> SerDes  85 */
    { 0,   40 + GG_SLICE1_BASE,   40 + GG_SLICE1_BASE,       },    /* Port  62 <-> SerDes  84 */
    { 0,   25 + GG_SLICE1_BASE,   25 + GG_SLICE1_BASE,       },    /* Port  63 <-> SerDes  73 */
    { 0,   24 + GG_SLICE1_BASE,   24 + GG_SLICE1_BASE,       },    /* Port  64 <-> SerDes  72 */
    { 0,   35 + GG_SLICE1_BASE,   35 + GG_SLICE1_BASE,       },    /* Port  65 <-> SerDes  83 */
    { 0,   34 + GG_SLICE1_BASE,   34 + GG_SLICE1_BASE,       },    /* Port  66 <-> SerDes  82 */
    { 0,   33 + GG_SLICE1_BASE,   33 + GG_SLICE1_BASE,       },    /* Port  67 <-> SerDes  81 */
    { 0,   32 + GG_SLICE1_BASE,   32 + GG_SLICE1_BASE,       },    /* Port  68 <-> SerDes  80 */
    { 0,   31 + GG_SLICE1_BASE,   31 + GG_SLICE1_BASE,       },    /* Port  69 <-> SerDes  79 */
    { 0,   30 + GG_SLICE1_BASE,   30 + GG_SLICE1_BASE,       },    /* Port  70 <-> SerDes  78 */
    { 0,   29 + GG_SLICE1_BASE,   29 + GG_SLICE1_BASE,       },    /* Port  71 <-> SerDes  77 */
    { 0,   28 + GG_SLICE1_BASE,   28 + GG_SLICE1_BASE,       },    /* Port  72 <-> SerDes  76 */
    /* 6Q */
     /*Q1*/
    { 0,   19 + GG_SLICE1_BASE,   19 + GG_SLICE1_BASE,       },    /* Port  73 <-> SerDes  67 */
    { 0,   18 + GG_SLICE1_BASE,   18 + GG_SLICE1_BASE,       },    /* Port  73 <-> SerDes  66 */
    { 0,   17 + GG_SLICE1_BASE,   17 + GG_SLICE1_BASE,       },    /* Port  73 <-> SerDes  65 */
    { 0,   16 + GG_SLICE1_BASE,   16 + GG_SLICE1_BASE,       },    /* Port  73 <-> SerDes  64 */
     /*Q2*/
    { 0,   23 + GG_SLICE1_BASE,   23 + GG_SLICE1_BASE,       },    /* Port  74 <-> SerDes  71 */
    { 0,   22 + GG_SLICE1_BASE,   22 + GG_SLICE1_BASE,       },    /* Port  74 <-> SerDes  70 */
    { 0,   21 + GG_SLICE1_BASE,   21 + GG_SLICE1_BASE,       },    /* Port  74 <-> SerDes  69 */
    { 0,   20 + GG_SLICE1_BASE,   20 + GG_SLICE1_BASE,       },    /* Port  74 <-> SerDes  68 */
     /*Q3*/
    { 0,   11 + GG_SLICE1_BASE,   11 + GG_SLICE1_BASE,       },    /* Port  75 <-> SerDes  59 */
    { 0,   10 + GG_SLICE1_BASE,   10 + GG_SLICE1_BASE,       },    /* Port  75 <-> SerDes  58 */
    { 0,    9 + GG_SLICE1_BASE,    9 + GG_SLICE1_BASE,       },    /* Port  75 <-> SerDes  57 */
    { 0,    8 + GG_SLICE1_BASE,    8 + GG_SLICE1_BASE,       },    /* Port  75 <-> SerDes  56 */
    /*Q4*/
    { 0,   15 + GG_SLICE1_BASE,   15 + GG_SLICE1_BASE,       },    /* Port  76 <-> SerDes  63 */
    { 0,   14 + GG_SLICE1_BASE,   14 + GG_SLICE1_BASE,       },    /* Port  76 <-> SerDes  62 */
    { 0,   13 + GG_SLICE1_BASE,   13 + GG_SLICE1_BASE,       },    /* Port  76 <-> SerDes  61 */
    { 0,   12 + GG_SLICE1_BASE,   12 + GG_SLICE1_BASE,       },    /* Port  76 <-> SerDes  60 */
    /*Q5*/
    { 0,    3 + GG_SLICE1_BASE,    3 + GG_SLICE1_BASE,       },    /* Port  77 <-> SerDes  51 */
    { 0,    2 + GG_SLICE1_BASE,    2 + GG_SLICE1_BASE,       },    /* Port  77 <-> SerDes  50 */
    { 0,    1 + GG_SLICE1_BASE,    1 + GG_SLICE1_BASE,       },    /* Port  77 <-> SerDes  49 */
    { 0,    0 + GG_SLICE1_BASE,    0 + GG_SLICE1_BASE,       },    /* Port  77 <-> SerDes  48 */
    /*Q6*/
    { 0,    7 + GG_SLICE1_BASE,    7 + GG_SLICE1_BASE,       },    /* Port  78 <-> SerDes  55 */
    { 0,    6 + GG_SLICE1_BASE,    6 + GG_SLICE1_BASE,       },    /* Port  78 <-> SerDes  54 */
    { 0,    5 + GG_SLICE1_BASE,    5 + GG_SLICE1_BASE,       },    /* Port  78 <-> SerDes  53 */
    { 0,    4 + GG_SLICE1_BASE,    4 + GG_SLICE1_BASE,       },    /* Port  78 <-> SerDes  52 */
};

fiber_port_info_t lcm_t65c_8t64x6q_fiber_max[T65C_8T64X6Q_PANEL_PORT_NUM] =
{
    /* fiber_flg+fiber access mode+fbid+busid+btmp+enable_chip+enable_No+present_chip+present_No+los_chip+los_No */
    /* panel port 1~8, 8T */
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    /* panel port 9~72, 64x */
    //i2c 1
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,11}},{.en_gpio={3,18}},0}, //port 9
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,10}},{.en_gpio={3,19}},1}, //port 10
    //i2c 0
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,11}},{.en_gpio={0,16}},2}, //port 11
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,10}},{.en_gpio={0,17}},3}, //port 12
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 1}},{.en_gpio={3,20}},4}, //port 13
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 3}},{.en_gpio={3,21}},5}, //port 14
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 1}},{.en_gpio={0,18}},6}, //port 15
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 3}},{.en_gpio={0,19}},7}, //port 16
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 0}},{.en_gpio={3,22}},8}, //port 17
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 2}},{.en_gpio={3,23}},9}, //port 18
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 0}},{.en_gpio={0,20}},10}, //port 19
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 2}},{.en_gpio={0,21}},11}, //port 20
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 8}},{.en_gpio={3,24}},12}, //port 21
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 9}},{.en_gpio={3,25}},13}, //port 22
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 8}},{.en_gpio={0,22}},14}, //port 23
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 9}},{.en_gpio={0,23}},15}, //port 24
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 7}},{.en_gpio={3,26}},16}, //port 25
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 6}},{.en_gpio={3,27}},17}, //port 26
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 7}},{.en_gpio={0,24}},18}, //port 27
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 6}},{.en_gpio={0,25}},19}, //port 28
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 5}},{.en_gpio={3,28}},20}, //port 29
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0, 4}},{.en_gpio={3,29}},21}, //port 30
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 5}},{.en_gpio={0,26}},22}, //port 31
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0, 4}},{.en_gpio={0,27}},23}, //port 32
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,12}},{.en_gpio={3,30}},24}, //port 33
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,15}},{.en_gpio={3,31}},25}, //port 34
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,12}},{.en_gpio={0,28}},26}, //port 35
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,13}},{.en_gpio={0,29}},27}, //port 36
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,14}},{.en_gpio={3,32}},28}, //port 37
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,13}},{.en_gpio={3,33}},29}, //port 38
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,14}},{.en_gpio={0,30}},30}, //port 39
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,15}},{.en_gpio={0,31}},31}, //port 40
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,19}},{.en_gpio={3,34}},32}, //port 41
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,20}},{.en_gpio={3,35}},33}, //port 42
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,24}},{.en_gpio={1,16}},34}, //port 43
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,19}},{.en_gpio={1,17}},35}, //port 44
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,18}},{.en_gpio={4,18}},36}, //port 45
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,17}},{.en_gpio={4,19}},37}, //port 46
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,18}},{.en_gpio={1,18}},38}, //port 47
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,25}},{.en_gpio={1,19}},39}, //port 48
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,16}},{.en_gpio={4,20}},40}, //port 49
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,21}},{.en_gpio={4,21}},41}, //port 50
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,17}},{.en_gpio={1,20}},42}, //port 51
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,26}},{.en_gpio={1,21}},43}, //port 52
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,22}},{.en_gpio={4,22}},44}, //port 53
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,28}},{.en_gpio={4,23}},45}, //port 54
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,22}},{.en_gpio={1,22}},46}, //port 55
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,27}},{.en_gpio={1,23}},47}, //port 56
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,26}},{.en_gpio={4,24}},48}, //port 57
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,27}},{.en_gpio={4,25}},49}, //port 58
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,21}},{.en_gpio={1,24}},50}, //port 59
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,28}},{.en_gpio={1,25}},51}, //port 60
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,24}},{.en_gpio={4,26}},52}, //port 61
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,25}},{.en_gpio={4,27}},53}, //port 62
    //i2c 0
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,16}},{.en_gpio={1,26}},54}, //port 63
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,20}},{.en_gpio={1,27}},55}, //port 64
    //i2c 1
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,36}},{.en_gpio={4,28}},56}, //port 65
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,37}},{.en_gpio={4,29}},57}, //port 66
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,35}},{.en_gpio={4,30}},58}, //port 67
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,38}},{.en_gpio={4,31}},59}, //port 68
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,34}},{.en_gpio={4,32}},60}, //port 69
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,33}},{.en_gpio={4,33}},61}, //port 70
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,32}},{.en_gpio={4,34}},62}, //port 71
	{E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,1,{.asic={0,39}},{.en_gpio={4,35}},63}, //port 72
	/* panel port 73~78, 6q */
    //i2c 0
	{E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,32}},{.en_gpio={2,16}},64}, //port 73
	{E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,33}},{.en_gpio={2,17}},65}, //port 74
	{E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,34}},{.en_gpio={2,18}},66}, //port 75
	{E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,35}},{.en_gpio={2,19}},67}, //port 76
	{E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,36}},{.en_gpio={2,20}},68}, //port 77
	{E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,37}},{.en_gpio={2,21}},69}, //port 78
};

glb_port_range_t lcm_t65c_8t64x6q_phy_chip_port_range[T65C_8T64X6Q_PHY_INT_BITS_SUM] =
{
    { 1, 8},
};

mac_led_api_para_t lcm_t65c_8t64x6q_mac_led_default_entry[T65C_8T64X6Q_LED_MAC_NUM]=
{
     /* slice 0*/
    {  8, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  11 <-> SerDes  8 */
    {  9, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  12 <-> SerDes  9 */
    { 10, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  15 <-> SerDes  10 */
    { 11, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  16 <-> SerDes  11 */
    { 12, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  19 <-> SerDes  12 */
    { 13, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  20 <-> SerDes  13 */
    { 14, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  23 <-> SerDes  14 */
    { 15, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  24 <-> SerDes  15 */
    { 16, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  9 <-> SerDes  16 */
    { 17, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  10 <-> SerDes  17 */
        
    { 18, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  13 <-> SerDes  18 */
    { 19, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  14 <-> SerDes  19 */
    { 20, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  17 <-> SerDes  20 */
    { 21, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  18 <-> SerDes  21 */
    { 22, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  21 <-> SerDes  22 */
    { 23, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  22 <-> SerDes  23 */
    { 24, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  25 <-> SerDes  24 */
    { 25, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  26 <-> SerDes  25 */
    { 26, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  29 <-> SerDes  26 */
    { 27, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  30 <-> SerDes  27 */
        
    { 28, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  33 <-> SerDes  28 */
    { 29, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  34 <-> SerDes  29 */
    { 30, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  37 <-> SerDes  30 */
    { 31, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  38 <-> SerDes  31 */
    { 32, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  41 <-> SerDes  32 */
    { 33, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  42 <-> SerDes  33 */
    { 34, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  27 <-> SerDes  34 */
    { 35, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  28 <-> SerDes  35 */
    { 48, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  31 <-> SerDes  36 */
    { 49, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  32 <-> SerDes  37 */
        
    { 50, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  35 <-> SerDes  38 */
    { 51, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  36 <-> SerDes  39 */
    { 36, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  39 <-> SerDes  40 */
    { 37, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  40 <-> SerDes  41 */
    { 38, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  43 <-> SerDes  42 */
    { 39, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  44 <-> SerDes  43 */
    { 52, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  52 <-> SerDes  44 */
    { 53, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  51 <-> SerDes  45 */
    { 54, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  48 <-> SerDes  46 */
    { 55, 0, 0, LED_MODE_2_FORCE_OFF, 0},    /* Port  47 <-> SerDes  47 */
        
     /* slice 1*/
    /*Q5*/
    {  0, 0, 1, LED_MODE_1_FORCE_OFF, 0},    /* Port  77 <-> SerDes  48 */
    /*Q6*/
    {  4, 0, 1, LED_MODE_1_FORCE_OFF, 0},    /* Port  78 <-> SerDes  52 */
     /*Q3*/
    {  8, 0, 1, LED_MODE_1_FORCE_OFF, 0},    /* Port  75 <-> SerDes  56 */
    /*Q4*/
    { 12, 0, 1, LED_MODE_1_FORCE_OFF, 0},    /* Port  76 <-> SerDes  60 */
     /*Q1*/
    { 16, 0, 1, LED_MODE_1_FORCE_OFF, 0},    /* Port  73 <-> SerDes  64 */
     /*Q2*/
    { 20, 0, 1, LED_MODE_1_FORCE_OFF, 0},    /* Port  74 <-> SerDes  68 */

    { 24, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  64 <-> SerDes  72 */
    { 25, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  63 <-> SerDes  73 */
    { 26, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  60 <-> SerDes  74 */
    { 27, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  59 <-> SerDes  75 */
    { 28, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  72 <-> SerDes  76 */
    { 29, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  71 <-> SerDes  77 */
    { 30, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  70 <-> SerDes  78 */
    { 31, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  69 <-> SerDes  79 */
    { 32, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  68 <-> SerDes  80 */
    { 33, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  67 <-> SerDes  81 */
    { 34, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  66 <-> SerDes  82 */
    { 35, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  65 <-> SerDes  83 */
    { 48, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  62 <-> SerDes  84 */
    { 49, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  61 <-> SerDes  85 */
    { 50, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  58 <-> SerDes  86 */
    { 51, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  57 <-> SerDes  87 */
    { 36, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  54 <-> SerDes  88 */
    { 37, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  53 <-> SerDes  89 */
    { 38, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  50 <-> SerDes  90 */
    { 39, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  49 <-> SerDes  91 */
    { 52, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  55 <-> SerDes  92 */
    { 53, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  56 <-> SerDes  93 */
    { 54, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  45 <-> SerDes  94 */
    { 55, 0, 1, LED_MODE_2_FORCE_OFF, 0},    /* Port  46 <-> SerDes  95 */
};

lcm_card_serdes_ffe_t 
lcm_t65c_8t64x6q_serdes_ffe_max[T65C_8T64X6Q_PANEL_PORT_NUM_MAX]=
{
     /* 8T*/
    { 5,   2,   },    /* Port  1 <-> SerDes  5 */
    { 4,   2,   },    /* Port  2 <-> SerDes  4 */
    { 1,   2,   },    /* Port  3 <-> SerDes  1 */
    { 0,   2,   },    /* Port  4 <-> SerDes  0 */
    { 7,   2,   },    /* Port  5 <-> SerDes  7 */
    { 6,   2,   },    /* Port  6 <-> SerDes  6 */
    { 3,   1,   },    /* Port  7 <-> SerDes  3 */
    { 2,   1,   },    /* Port  8 <-> SerDes  2 */
    /* 64 XFI*/
    { 16,   2,       },    /* Port  9 <-> SerDes  16 */
    { 17,   2,       },    /* Port  10 <-> SerDes  17 */
    { 8,    1,       },    /* Port  11 <-> SerDes  8 */
    { 9,    2,       },    /* Port  12 <-> SerDes  9 */
    { 18,   2,       },    /* Port  13 <-> SerDes  18 */
    { 19,   2,       },    /* Port  14 <-> SerDes  19 */
    { 10,   1,       },    /* Port  15 <-> SerDes  10 */
    { 11,   1,       },    /* Port  16 <-> SerDes  11 */
    { 20,   2,       },    /* Port  17 <-> SerDes  20 */
    { 21,   2,       },    /* Port  18 <-> SerDes  21 */
    { 12,   1,       },    /* Port  19 <-> SerDes  12 */
    { 13,   1,       },    /* Port  20 <-> SerDes  13 */
    { 22,   1,       },    /* Port  21 <-> SerDes  22 */
    { 23,   2,       },    /* Port  22 <-> SerDes  23 */
    { 14,   1,       },    /* Port  23 <-> SerDes  14 */
    { 15,   1,       },    /* Port  24 <-> SerDes  15 */
    { 24,   1,       },    /* Port  25 <-> SerDes  24 */
    { 25,   1,       },    /* Port  26 <-> SerDes  25 */
    { 34,   1,       },    /* Port  27 <-> SerDes  34 */
    { 35,   1,       },    /* Port  28 <-> SerDes  35 */
    { 26,   2,       },    /* Port  29 <-> SerDes  26 */
    { 27,   2,       },    /* Port  30 <-> SerDes  27 */

    { 36,   0,       },    /* Port  31 <-> SerDes  36 */
    { 37,   1,       },    /* Port  32 <-> SerDes  37 */

    { 28,   2,       },    /* Port  33 <-> SerDes  28 */
    { 29,   2,       },    /* Port  34 <-> SerDes  29 */

    { 38,   1,       },    /* Port  35 <-> SerDes  38 */
    { 39,   1,       },    /* Port  36 <-> SerDes  39 */

    { 30,   2,       },    /* Port  37 <-> SerDes  30 */
    { 31,   2,       },    /* Port  38 <-> SerDes  31 */

    { 40,   1,       },    /* Port  39 <-> SerDes  40 */
    { 41,   1,       },    /* Port  40 <-> SerDes  41 */
    { 32,   2,       },    /* Port  41 <-> SerDes  32 */
    { 33,   2,       },    /* Port  42 <-> SerDes  33 */

    { 42,   1,       },    /* Port  43 <-> SerDes  42 */
    { 43,   1,       },    /* Port  44 <-> SerDes  43 */

    { 94,   1,       },    /* Port  45 <-> SerDes  94 */
    { 95,   1,       },    /* Port  46 <-> SerDes  95 */
    { 47,   0,       },    /* Port  47 <-> SerDes  47 */
    { 46,   0,       },    /* Port  48 <-> SerDes  46 */

    { 91,   1,       },    /* Port  49 <-> SerDes  91 */
    { 90,   1,       },    /* Port  50 <-> SerDes  90 */
    { 45,   0,       },    /* Port  51 <-> SerDes  45 */
    { 44,   0,       },    /* Port  52 <-> SerDes  44 */
    { 89,   1,       },    /* Port  53 <-> SerDes  89 */
    { 88,   1,       },    /* Port  54 <-> SerDes  88 */
    { 92,   0,       },    /* Port  55 <-> SerDes  92 */
    { 93,   1,       },    /* Port  56 <-> SerDes  93 */

    { 87,   1,       },    /* Port  57 <-> SerDes  87 */
    { 86,   1,       },    /* Port  58 <-> SerDes  86 */

    { 75,   1,       },    /* Port  59 <-> SerDes  75 */
    { 74,   1,       },    /* Port  60 <-> SerDes  74 */
    { 85,   1,       },    /* Port  61 <-> SerDes  85 */
    { 84,   1,       },    /* Port  62 <-> SerDes  84 */
    { 73,   1,       },    /* Port  63 <-> SerDes  73 */
    { 72,   1,       },    /* Port  64 <-> SerDes  72 */
    { 83,   1,       },    /* Port  65 <-> SerDes  83 */
    { 82,   1,       },    /* Port  66 <-> SerDes  82 */
    { 81,   1,       },    /* Port  67 <-> SerDes  81 */
    { 80,   2,       },    /* Port  68 <-> SerDes  80 */
    { 79,   2,       },    /* Port  69 <-> SerDes  79 */
    { 78,   2,       },    /* Port  70 <-> SerDes  78 */
    { 77,   2,       },    /* Port  71 <-> SerDes  77 */
    { 76,   2,       },    /* Port  72 <-> SerDes  76 */
    /* 6Q */
     /*Q1*/
    { 67,   1,       },    /* Port  73 <-> SerDes  67 */
    { 66,   1,       },    /* Port  73 <-> SerDes  66 */
    { 65,   1,       },    /* Port  73 <-> SerDes  65 */
    { 64,   1,       },    /* Port  73 <-> SerDes  64 */
     /*Q2*/
    { 71,   1,       },    /* Port  74 <-> SerDes  71 */
    { 70,   1,       },    /* Port  74 <-> SerDes  70 */
    { 69,   1,       },    /* Port  74 <-> SerDes  69 */
    { 68,   1,       },    /* Port  74 <-> SerDes  68 */
     /*Q3*/
    { 59,   1,       },    /* Port  75 <-> SerDes  59 */
    { 58,   1,       },    /* Port  75 <-> SerDes  58 */
    { 57,   1,       },    /* Port  75 <-> SerDes  57 */
    { 56,   1,       },    /* Port  75 <-> SerDes  56 */
    /*Q4*/
    { 63,   1,       },    /* Port  76 <-> SerDes  63 */
    { 62,   1,       },    /* Port  76 <-> SerDes  62 */
    { 61,   1,       },    /* Port  76 <-> SerDes  61 */
    { 60,   1,       },    /* Port  76 <-> SerDes  60 */
    /*Q5*/
    { 51,    1,       },    /* Port  77 <-> SerDes  51 */
    { 50,    1,       },    /* Port  77 <-> SerDes  50 */
    { 49,    1,       },    /* Port  77 <-> SerDes  49 */
    { 48,    2,       },    /* Port  77 <-> SerDes  48 */
    /*Q6*/
    { 55,    1,       },    /* Port  78 <-> SerDes  55 */
    { 54,    1,       },    /* Port  78 <-> SerDes  54 */
    { 53,    1,       },    /* Port  78 <-> SerDes  53 */
    { 52,    1,       },    /* Port  78 <-> SerDes  52 */
};
/****************************************************************************
 *
* Function
*
****************************************************************************/
static int32
lcm_init_t65c_8t64x6q_init_port_table(glb_card_t* p_card)
{
    uint8 port_id, panel_port_no, panel_subport_no;
    uint8 pos;

    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        panel_port_no = p_card->pp_port[port_id]->panel_port_no;
        panel_subport_no = p_card->pp_port[port_id]->panel_sub_port_no;
        if (port_id < T65C_8T64X6Q_8T64X)  /* no port num change */
        {
            pos = panel_port_no - 1;
        }
        else
        {
            if(panel_subport_no)  /* port split */
            {
                pos = T65C_8T64X6Q_8T64X + (panel_port_no - T65C_8T64X6Q_8T64X - 1) * 4 + panel_subport_no - 1;
            }
            else
            {
               pos = T65C_8T64X6Q_8T64X + (panel_port_no - T65C_8T64X6Q_8T64X - 1) * 4 + 3;
            }
        }
        sal_memcpy(&glb_lcm_t65c_8t64x6q_port[port_id], &lcm_t65c_8t64x6q_port_max[pos], sizeof(lcm_card_port_t));
        glb_lcm_t65c_8t64x6q_port[port_id].serdes_id = lcm_t65c_8t64x6q_serdes_ffe_max[pos].serdes_id;
    }

    return LCM_E_SUCCESS;
}

static int32
lcm_init_t65c_8t64x6q_init_fiber_table(glb_card_t* p_card)
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
        sal_memcpy(&glb_lcm_t65c_8t64x6q_fiber[port_id], &lcm_t65c_8t64x6q_fiber_max[panel_port_no-1], sizeof(fiber_port_info_t));
        glb_lcm_t65c_8t64x6q_fiber[port_id].fiber_channel = fiber_channel;
    }
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_panel_port(glb_card_t* p_card)
{
    uint8 ppt_id;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init panel port begin.");
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
       
        if (ppt_id < T65C_8T64X6Q_8T)
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_1G | GLB_SUPPORT_SPEED_100M;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_1G;
        }
        else if (ppt_id < T65C_8T64X6Q_8T + T65C_8T64X6Q_64X)
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_10G | GLB_SUPPORT_SPEED_1G;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_10G;
        }
        else
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_SPLIT;
            p_card->pp_ppt[ppt_id]->op_ability = GLB_PPT_OP_SPLIT_10G;
            p_card->pp_ppt[ppt_id]->ppt_numchg_num = 1;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_40G;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_40G;
        }
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init panel port end.");
    LCM_LOG_BRINGUP("Lcm init panel port end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_port(glb_card_t* p_card)
{
    int32 port_id = 0;
    uint32 ppt_idx;
    uint16 logic_port_idx;
    lcm_card_port_panel_mapping_t port_panel_mapping[256];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init port begin.");
    LCM_LOG_BRINGUP("Lcm init port begin.");
     /* 1, init panel port */
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_panel_port(p_card));
 
    /* 2, read '/mnt/flash/startup-config.conf' file, get port split/merge info */
    LCM_IF_ERROR_RETURN(lcm_common_parse_port_numchg_info(p_card));
    if(p_card->port_num != T65C_8T64X6Q_PANEL_PORT_NUM)
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
    glb_lcm_t65c_8t64x6q_port = (lcm_card_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(lcm_card_port_t)* p_card->port_num);
    if (!glb_lcm_t65c_8t64x6q_port)
    {
        LCM_LOG_ERR("Allocate pointer to lcm_card_port_t fail.");
        return LCM_E_NO_MEMORY;
    }
 
    glb_lcm_t65c_8t64x6q_fiber = (fiber_port_info_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(fiber_port_info_t)*p_card->port_num);
    if (!glb_lcm_t65c_8t64x6q_fiber)
    {
        LCM_LOG_ERR("Allocate pointer to fiber_port_info_t fail.");
        return LCM_E_NO_MEMORY;
    }
 
    /* 6, init current running port/fiber info */
    LCM_IF_ERROR_RETURN(lcm_init_t65c_8t64x6q_init_port_table(p_card));
    LCM_IF_ERROR_RETURN(lcm_init_t65c_8t64x6q_init_fiber_table(p_card));
 
    /* 7, init port properties */
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        logic_port_idx = glb_lcm_t65c_8t64x6q_port[port_id].logic_port_idx;
        ppt_idx = p_card->pp_port[port_id]->panel_port_no;

        p_card->pp_port[port_id]->p_fiber = NULL;
        p_card->pp_port[port_id]->create_done = 1;
        p_card->pp_port[port_id]->is_combo = 0;
        p_card->pp_port[port_id]->port_idx = port_id + 1;
        p_card->pp_port[port_id]->logic_slot_idx = p_card->logic_slot_no;
        p_card->pp_port[port_id]->glb_chip_idx = p_card->phy_slot_no;
        p_card->pp_port[port_id]->port_cfg.loopback = GLB_LB_NONE;
        p_card->pp_port[port_id]->port_cfg.enable = 0;
        p_card->pp_port[port_id]->port_cfg.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        p_card->pp_port[port_id]->port_cfg.flowctrl.recv = GLB_FLOWCTRL_DISABLE;

        p_card->pp_port[port_id]->port_status.link_up = GLB_LINK_DOWN;
        p_card->pp_port[port_id]->port_status.duplex = GLB_DUPLEX_AUTO;
        p_card->pp_port[port_id]->port_status.speed = GLB_SPEED_AUTO;
        p_card->pp_port[port_id]->port_status.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        p_card->pp_port[port_id]->port_status.flowctrl.recv = GLB_FLOWCTRL_DISABLE;
        p_card->pp_port[port_id]->poe_support = NOT_POE_PORT;
        p_card->pp_port[port_id]->is_combo_to = 0;

        p_card->pp_port[port_id]->port_speed_ability = p_card->pp_ppt[ppt_idx-1]->ppt_speed_ability;
        p_card->pp_port[port_id]->port_speed_real = p_card->pp_ppt[ppt_idx-1]->ppt_speed_real;
        lcm_common_get_defaultspeed_from_speedability(p_card->pp_port[port_id]->port_speed_ability, 
            &(p_card->pp_port[port_id]->port_cfg.speed), &(p_card->pp_port[port_id]->port_cfg.duplex));
        p_card->pp_port[port_id]->mux_chip_id = 0;
        p_card->pp_port[port_id]->lp_support = 1;  /* added by tongzb for bug 44368, support loopback on CNOS */
        p_card->pp_port[port_id]->eee_support = 0;
        p_card->pp_port[port_id]->logic_port_idx = logic_port_idx;
        p_card->pp_port[port_id]->local_chip_idx = glb_lcm_t65c_8t64x6q_port[port_id].chip_idx;
        p_card->pp_port[port_id]->mac_idx = glb_lcm_t65c_8t64x6q_port[port_id].mac_idx;
        p_card->pp_port[port_id]->chip_serdes_id = glb_lcm_t65c_8t64x6q_port[port_id].serdes_id;
        p_card->pp_port[port_id]->g_port_index = (p_card->pp_port[port_id]->glb_chip_idx << 8) | logic_port_idx;
        /* Modified by liuht for bug 34540, 2015-09-09 */
        if(ppt_idx <= T65C_8T64X6Q_8T)
        {
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_1000BASE_T;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_COPPER;
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_UNSUPPORT;
            p_card->pp_port[port_id]->unidir_support = 0; 
        }
        else if(ppt_idx <= T65C_8T64X6Q_8T64X)
        {            
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED;
            p_card->pp_port[port_id]->unidir_support = 1;
        }
        else
        {
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_QSFP_ONE_LED;
            p_card->pp_port[port_id]->unidir_support = 1;
        }
        p_card->pp_port[port_id]->ctl_id = (logic_port_idx >= GG_SLICE1_BASE)?1:0;
        if(p_card->pp_port[port_id]->ctl_id==1)
        {
            logic_port_idx = logic_port_idx -GG_SLICE1_BASE;
        }
        p_card->pp_port[port_id]->port_led_mac = (logic_port_idx >= 40)?
            (logic_port_idx+8):logic_port_idx;
     }

    /* 8, create file '/tmp/ctcos_port_info' */
    LCM_IF_ERROR_RETURN(lcm_common_save_port_info_file(p_card));
     
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init port end.");
    LCM_LOG_BRINGUP("Lcm init port end.");

    return LCM_E_SUCCESS;
}


static int32_t
_lcm_reg_t65c_8t64x6q_epld(uint8 hw_ver, epld_info_t * p_epld_info)
{
    p_epld_info->base_addr = epld_localbus_addr_get();
    p_epld_info->epld_bus_type = EPLD_LOCALBUS_TYPE;

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_EPLD_VERSION]),          0x1, 0, 7, 8);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SYS_LED_SYS]),           0x2, 4, 7, 4);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_FAN_LED]),               0x3, 2, 3, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU1_LED]),              0x3, 4, 5, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU2_LED]),              0x3, 6, 7, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_IND_LED]),               0x3, 0, 1, 2);
    /*add by guhg,20151230,for bug36814*/
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SVB_STATUS]),          0x6, 0, 3, 4);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_PRESENT_STATUS]),    0x16, 4, 5, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_WORK_STATUS]),       0x16, 6, 7, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_ALERT_STATUS]),      0x16, 2, 3, 2);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_REBOOT_POWER_RST]),         0xf6, 0, 7, 8);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_REBOOT_MANUAL_OTHER_RST]),  0xf7, 0, 7, 8);	

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_POWER_DOWN]),               0x23, 0, 1, 2);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PHY_INT_MASK1]),               0x0b, 0, 0, 1);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_DPLL_INT_MASK]),               0x0d, 0, 0, 1);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PHY_RST]),               0x08, 3, 3, 1);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_RCV_CLK_SEL_REG]),          0x13, 0, 1, 2);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_TEST]),                 0x7f, 0, 7, 8);

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_epld(glb_card_t* p_card, uint8 epld_version)
{
    epld_info_t *p_epld_info = NULL;
    int32 ret = 0;
    LCM_LOG_DEBUG(LCM_CARD,"Lcm init epld begin.");
    LCM_LOG_BRINGUP("Lcm init epld begin.");

    p_epld_info = epld_get_info(&p_card->board_type, epld_version);
    if (!p_epld_info)
    {
        LCM_LOG_ERR("Get EPLD info fail.");
        return LCM_E_INVALID_PTR;
    }

    _lcm_reg_t65c_8t64x6q_epld(p_card->hw_ver, p_epld_info);
    ret = epld_init(p_epld_info);
    if (0 != ret)
    {
        LCM_LOG_ERR("EPLD Init fail.");
        return LCM_E_INIT_FAILED;
    }
    /* Fix bug29772, cr9895, qicx, 2014-08-26 */
    p_card->support_reboot_info = 1;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init epld end.");
    LCM_LOG_BRINGUP("Lcm init epld end.");

    return LCM_E_SUCCESS;
}


static int32
_lcm_init_t65c_8t64x6q_eeprom_info(glb_card_t* p_card)
{
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init eeprom info.");
    LCM_LOG_BRINGUP("Lcm init eeprom info.");
    p_card->p_eeprom_alloc = (eeprom_info_t *)XCALLOC(MTYPE_BUFFER_DATA, sizeof(eeprom_info_t)*EEPROM_MAX);
    if(NULL == p_card->p_eeprom_alloc)
    {
        LCM_LOG_ERR("alloc p_eeprom_alloc array fail.");
        return LCM_E_INVALID_PTR;
    }

    p_card->p_eeprom_alloc[EEPROM_0_MANU_TEST].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_0_MANU_TEST].base_addr = 0x1000;

    p_card->p_eeprom_alloc[EEPROM_TCAM_PROFILE].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_TCAM_PROFILE].base_addr = 0x1001;

#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
    p_card->p_eeprom_alloc[EEPROM_SMARTCFG].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_SMARTCFG].base_addr = 0x1007;
#endif /* !HAVE_SMARTCFG */

    p_card->p_eeprom_alloc[EEPROM_REBOOT_INFO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_REBOOT_INFO].base_addr = 0x100a;

    p_card->p_eeprom_alloc[EEPROM_MGMT_CFG].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_MGMT_CFG].base_addr = 0x1100;

    p_card->p_eeprom_alloc[EEPROM_OEM_INFO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_OEM_INFO].base_addr = 0x1300;


    p_card->p_eeprom_alloc[EEPROM_PORT_INFO].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_PORT_INFO].base_addr = 0x1400;

    p_card->p_eeprom_alloc[EEPROM_SW_VER].eeprom_idx = 0;
    p_card->p_eeprom_alloc[EEPROM_SW_VER].base_addr = 0x1502; 

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_fiber(void)
{
    int32 ret = 0;
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fiber begin.");
    LCM_LOG_BRINGUP( "Lcm init fiber begin.");
    ret = fiber_init(T65C_8T64X6Q_FIBER_PORT_NUM,  /* fiber num */
                     glb_card->port_num, /* port num */
                     glb_lcm_t65c_8t64x6q_fiber);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fiber end.");
    LCM_LOG_BRINGUP("Lcm init fiber end.");
    return ret;
}

static int32
_lcm_init_t65c_8t64x6q_sensor(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[T65C_8T64X6Q_SENSOR_NUM];
    sensor_chip_t sensor_chip[T65C_8T64X6Q_SENSOR_NUM];
    void *p_data[T65C_8T64X6Q_SENSOR_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sensor begin.");
    LCM_LOG_BRINGUP("Lcm init sensor begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(sensor_chip, 0, sizeof(sensor_chip));

    /* sensor0: on motherboard, before GG chip */
    i2c_gen[0].addr = T65C_8T64X6Q_ADDR0_SENSOR;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_0;
    sensor_chip[0].io_type = SENSOR_I2C;
    i2c_gen[0].alen = LM77_OFFSET_WIDTH;
    sensor_chip[0].chip_type = SENSOR_LM77;
    sensor_chip[0].pos = SENSOR_BEFORE_CHIP;

    /* sensor1: on motherboard, behind GG chip */
    i2c_gen[1].addr = T65C_8T64X6Q_ADDR1_SENSOR;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].bridge_flag = 0;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_0;
    sensor_chip[1].io_type = SENSOR_I2C;
    i2c_gen[1].alen = LM77_OFFSET_WIDTH;
    sensor_chip[1].chip_type = SENSOR_LM77;
    sensor_chip[1].pos = SENSOR_BEHIND_CHIP;

    /* sensor2: on motherboard, near fan module */
    i2c_gen[2].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[2].p_br.bridge_addr = T65C_8T64X6Q_I2C_BRIDGE_ADDR_0;
    i2c_gen[2].p_br.channel = 2;
    i2c_gen[2].addr = T65C_8T64X6Q_ADDR2_SENSOR;
    i2c_gen[2].i2c_type = E_I2C_CPM;
    i2c_gen[2].bridge_flag = 1;
    i2c_gen[2].i2c_bus_idx = GLB_I2C_IDX_0;
    sensor_chip[2].io_type = SENSOR_I2C;
    i2c_gen[2].alen = LM77_OFFSET_WIDTH;
    sensor_chip[2].chip_type = SENSOR_LM77;
    sensor_chip[2].pos = SENSOR_BEFORE_CHIP;
    
    for(i=0; i<T65C_8T64X6Q_SENSOR_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = sensor_init(p_data, sensor_chip, T65C_8T64X6Q_SENSOR_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Temperature sensor init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(i=0; i<T65C_8T64X6Q_SENSOR_NUM; i++)
    {
        sensor_dev_init(i);
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sensor end.");
    LCM_LOG_BRINGUP("Lcm init sensor end.");
    return LCM_E_SUCCESS;
}

static int32
_lcm_t65c_8t64x6q_cfg_gpio(uint32 chip_id)
{
    uint8 reg, val;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm cfg i2c to gpio module begin.");
    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module begin.");
    switch (chip_id)
    {
    case 0:    
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK3;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        /* tx enable */
        reg = PCA9505_OUTPUT_PORT_REG_BANK2;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK3;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        break;
        
    case 1:
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0xf0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK3;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* tx enable */
        reg = PCA9505_OUTPUT_PORT_REG_BANK2;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK3;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        break;
    case 2:
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0xc0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0xc0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK4;
        val = 0xc0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK3;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* tx enable */
        reg = PCA9505_OUTPUT_PORT_REG_BANK2;
        val = 0x3f; //6QSFP work in low power mode
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        reg = PCA9505_OUTPUT_PORT_REG_BANK3;
        val = 0x0; //6QSFP mode change
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        reg = PCA9505_OUTPUT_PORT_REG_BANK4;
        val = 0x3f; //6QSFP reset
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        break;
    case 3:
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0x03;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK4;
        val = 0xf0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK3;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* tx enable */
        reg = PCA9505_OUTPUT_PORT_REG_BANK2;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK3;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK4;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        break;
    case 4:
        /* for QSFP+ */
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0x03;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK4;
        val = 0xf0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK0;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK3;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* release reset */
        reg = PCA9505_OUTPUT_PORT_REG_BANK2;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK3;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK4;
        val = 0x0;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        break;
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm cfg i2c to gpio module end.");
    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module end.");
    return RESULT_OK;
}

static int32
_lcm_init_t65c_8t64x6q_gpio(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[T65C_8T64X6Q_GPIO_CHIP_NUM];
    gpio_chip_t gpio_chip[T65C_8T64X6Q_GPIO_CHIP_NUM];
    void *p_data[T65C_8T64X6Q_GPIO_CHIP_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init i2c to gpio module begin.");
    LCM_LOG_BRINGUP("Lcm init gpio begin.");

    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    /* gpio 0/1/2 on motherboard */
    i2c_gen[0].slave_bitmap = 30;
    i2c_gen[0].lchip= 0;
    i2c_gen[0].addr = T65C_8T64X6Q_ADDR0_GPIO;
    i2c_gen[0].i2c_type = E_I2C_GG;             
    i2c_gen[0].alen = PCA9505_OFFSET_WIDTH;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_0;
    gpio_chip[0].io_type = GPIO_I2C;    
    gpio_chip[0].chip_type = GPIO_PCA9505;
    gpio_chip[0].scan_group_bitmap = 0x03; /*group 0,1*/

    i2c_gen[1].slave_bitmap = 31;
    i2c_gen[1].lchip= 0;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_0;
    i2c_gen[1].addr = T65C_8T64X6Q_ADDR1_GPIO;
    i2c_gen[1].i2c_type = E_I2C_GG;             
    i2c_gen[1].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[1].io_type = GPIO_I2C;    
    gpio_chip[1].chip_type = GPIO_PCA9505;
    gpio_chip[1].scan_group_bitmap = 0x03; /*group 0,1*/

    i2c_gen[2].slave_bitmap = 38;
    i2c_gen[2].lchip= 0;
    i2c_gen[2].i2c_bus_idx = GLB_I2C_IDX_0;
    i2c_gen[2].addr = T65C_8T64X6Q_ADDR2_GPIO;
    i2c_gen[2].i2c_type = E_I2C_GG;             
    i2c_gen[2].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[2].io_type = GPIO_I2C;    
    gpio_chip[2].chip_type = GPIO_PCA9505;
    gpio_chip[2].scan_group_bitmap = 0x01; /*group 0*/
/* gpio 3/4 on daughterboard */
    i2c_gen[3].slave_bitmap = 30;
    i2c_gen[3].lchip= 0;
    i2c_gen[3].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[3].addr = T65C_8T64X6Q_ADDR3_GPIO;
    i2c_gen[3].i2c_type = E_I2C_GG;             
    i2c_gen[3].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[3].io_type = GPIO_I2C;    
    gpio_chip[3].chip_type = GPIO_PCA9505;
    gpio_chip[3].scan_group_bitmap = 0x07; /*group 0,1,2*/

    i2c_gen[4].slave_bitmap = 31;
    i2c_gen[4].lchip= 0;
    i2c_gen[4].i2c_bus_idx = GLB_I2C_IDX_1;
    i2c_gen[4].addr = T65C_8T64X6Q_ADDR4_GPIO;
    i2c_gen[4].i2c_type = E_I2C_GG;             
    i2c_gen[4].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[4].io_type = GPIO_I2C;    
    gpio_chip[4].chip_type = GPIO_PCA9505;
    gpio_chip[4].scan_group_bitmap = 0x07; /*group 0,1,2*/

    for(i=0; i<T65C_8T64X6Q_GPIO_CHIP_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = gpio_init(p_data, gpio_chip, T65C_8T64X6Q_GPIO_CHIP_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Gpio device init fail.");
        return LCM_E_INIT_FAILED;
    }

    for(i=0; i<T65C_8T64X6Q_GPIO_CHIP_NUM; i++)
    {
        _lcm_t65c_8t64x6q_cfg_gpio(i);
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init i2c to gpio module end.");
    LCM_LOG_BRINGUP("Lcm init gpio end.");

    return LCM_E_SUCCESS;
}

/* Added by liuht for bug26671, 2014-02-14 */
static int32
_lcm_init_t65c_8t64x6q_fan(void)
{
    int32 ret = 0;
    uint8 fan_module_idx;
    i2c_gen_t i2c_gen[T65C_8T64X6Q_FAN_MODULE_MAX];
    fan_chip_t fan_chip[T65C_8T64X6Q_FAN_MODULE_MAX];
    void *p_data[T65C_8T64X6Q_FAN_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module begin.");
    LCM_LOG_BRINGUP("Lcm init fan module begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(fan_chip, 0, sizeof(fan_chip)); 
    
    i2c_gen[0].addr = T65C_8T64X6Q_ADDR0_FAN;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = ADT7470_OFFSET_WIDTH;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_0;
    fan_chip[0].chip_type = FAN_ADT7470;
    fan_chip[0].io_type = E_FAN_I2C;
    fan_chip[0].support_hot_swap = 0;  /* stable on GB mother board, do not support hot swap */
    p_data[0] = &i2c_gen[0];
    fan_chip[0].fan_num = 4;
    fan_chip[0].speed_adjust = 1;
    fan_chip[0].rpm = 9000;    /*Revolutions per minites, bug29436, qicx, 2014-08-08*/
    fan_chip[0].pwm_map_table = NULL;/* use the default table */
    fan_chip[0].pwm.bootom_rate = DEFAULT_FAN_RATE_BOTTOM; 
    fan_chip[0].pwm.low_rate    = DEFAULT_FAN_RATE_LOW;
    fan_chip[0].pwm.high_rate   = DEFAULT_FAN_RATE_HIGH;
    fan_chip[0].pwm.full_rate   = DEFAULT_FAN_RATE_FULL;

    i2c_gen[1].addr = T65C_8T64X6Q_ADDR1_FAN;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].alen = ADT7470_OFFSET_WIDTH;
    i2c_gen[1].bridge_flag = 0;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_0;
    fan_chip[1].chip_type = FAN_ADT7470;
    fan_chip[1].io_type = E_FAN_I2C;
    fan_chip[1].support_hot_swap = 0;  /* stable on GB mother board, do not support hot swap */
    p_data[1] = &i2c_gen[1];
    fan_chip[1].fan_num = 4;
    fan_chip[1].speed_adjust = 1;
    fan_chip[1].rpm = 9000;    /*Revolutions per minites, bug29436, qicx, 2014-08-08*/
    fan_chip[1].pwm_map_table = NULL;/* use the default table */
    fan_chip[1].pwm.bootom_rate = DEFAULT_FAN_RATE_BOTTOM; 
    fan_chip[1].pwm.low_rate    = DEFAULT_FAN_RATE_LOW;
    fan_chip[1].pwm.high_rate   = DEFAULT_FAN_RATE_HIGH;
    fan_chip[1].pwm.full_rate   = DEFAULT_FAN_RATE_FULL;

    ret = fan_init(p_data, fan_chip, T65C_8T64X6Q_FAN_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Fan driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(fan_module_idx=0; fan_module_idx<T65C_8T64X6Q_FAN_MODULE_MAX; fan_module_idx++)
    {
        fan_dev_init(fan_module_idx);
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module end.");
    LCM_LOG_BRINGUP("Lcm init fan module end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_psu(void)
{
    int32 ret;
    uint8 epld_idx[T65C_8T64X6Q_PSU_MODULE_MAX];
    i2c_gen_t i2c_gen[T65C_8T64X6Q_PSU_MODULE_MAX];
    psu_chip_t psu_chip[T65C_8T64X6Q_PSU_MODULE_MAX];
    psu_private_t p_data[T65C_8T64X6Q_PSU_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init psu module begin.");
    LCM_LOG_BRINGUP("Lcm init psu module begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(p_data, 0, sizeof(p_data));
    sal_memset(psu_chip, 0, sizeof(psu_chip));
    
    epld_idx[0] = 0; 
    i2c_gen[0].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[0].p_br.bridge_addr = T65C_8T64X6Q_I2C_BRIDGE_ADDR_0;
    i2c_gen[0].p_br.channel = 0;
    i2c_gen[0].addr = T65C_8T64X6Q_ADDR0_PSU;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = PSU_I2C_OFFSET_WIDTH;
    i2c_gen[0].bridge_flag = 1;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_0;
    psu_chip[0].chip_type = PSU_I2C_EPLD;
    psu_chip[0].io_type = PSU_IO_I2C_EPLD;
    psu_chip[0].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[0].psu_mode_type = (1<<PSU_MODE_TYPE_REG_38_26_0d)|(1<<PSU_MODE_TYPE_REG_50_50_08)
                            |(1<<PSU_MODE_TYPE_REG_5b_9a_0a); //same as 48x6q
    psu_chip[0].support_pmbus = 1;
    p_data[0].p_data_i2c= &i2c_gen[0];
    p_data[0].p_data_epld= &epld_idx[0];

    epld_idx[1] = 0;
    i2c_gen[1].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[1].p_br.bridge_addr = T65C_8T64X6Q_I2C_BRIDGE_ADDR_0;
    i2c_gen[1].p_br.channel = 1;
    i2c_gen[1].addr = T65C_8T64X6Q_ADDR1_PSU;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].alen = PSU_I2C_OFFSET_WIDTH;
    i2c_gen[1].bridge_flag = 1;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_0;
    psu_chip[1].chip_type = PSU_I2C_EPLD;
    psu_chip[1].io_type = PSU_IO_I2C_EPLD;
    psu_chip[1].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[1].psu_mode_type = (1<<PSU_MODE_TYPE_REG_38_26_0d)|(1<<PSU_MODE_TYPE_REG_50_50_08)
                            |(1<<PSU_MODE_TYPE_REG_5b_9a_0a);
    psu_chip[1].support_pmbus = 1;
    p_data[1].p_data_i2c= &i2c_gen[1];
    p_data[1].p_data_epld= &epld_idx[1];
    
    ret = psu_init(p_data, psu_chip, T65C_8T64X6Q_PSU_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Psu driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init psu module end.");
    LCM_LOG_BRINGUP("Lcm int psu end.");    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_eeprom(void)
{
    int32 ret = 0;
    i2c_gen_t i2c_gen[T65C_8T64X6Q_EEPROM_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init eeprom begin.");
    LCM_LOG_BRINGUP("Lcm init eeprom begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    /*eeprom0 0x57 on mother card*/
    i2c_gen[0].addr = T65C_8T64X6Q_ADDR_MASTER_EEPROM_0;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = EEPROM_OFFSET_WIDTH;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_0;
    /*eeprom1 0x56 on mother card*/
    i2c_gen[1].addr = T65C_8T64X6Q_ADDR_MASTER_EEPROM_1;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].alen = EEPROM_OFFSET_WIDTH;
    i2c_gen[1].bridge_flag = 0;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX_0;
    /*eeprom2 on daughter card*/
    /*bridge info,begin*/
    i2c_gen[2].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[2].p_br.bridge_addr = T65C_8T64X6Q_I2C_BRIDGE_ADDR_0;
    i2c_gen[2].p_br.channel = 2;
    /*bridge info,end*/
    i2c_gen[2].addr = T65C_8T64X6Q_ADDR_DAU_EEPROM;
    i2c_gen[2].i2c_type = E_I2C_CPM;
    i2c_gen[2].alen = EEPROM_OFFSET_WIDTH;
    i2c_gen[2].bridge_flag = 1;
    i2c_gen[2].i2c_bus_idx = GLB_I2C_IDX_0;

    ret = eeprom_init(i2c_gen, T65C_8T64X6Q_EEPROM_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm EEPROM init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init eeprom end.");
    LCM_LOG_BRINGUP("Lcm init eeprom end.");
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_phy(void)
{    
    uint16 port_id;
    phy_info_t phyinfo;
    phy_handle_t** pphdl = NULL;
    glb_port_t* p_port;
    int32 ret = 0;
    uint8 phy_led_type = GLB_PHY_LED_8;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init phy begin.");
    LCM_LOG_BRINGUP("Lcm init phy begin.");
    /* release GE phy */

    epld_item_write(0, EPLD_PHY_RST, 0x1);
    sal_delay(1);
    sal_memset(&phyinfo, 0, sizeof(phy_info_t));
    pphdl = (phy_handle_t**)LCM_CALLOC(CTCLIB_MEM_LCM_MODULE,
        sizeof(phy_handle_t*)*(glb_card->port_num));

    if(NULL == pphdl)
    {
        LCM_LOG_ERR("LCM phy no memory.");
        return LCM_E_NO_MEMORY;
    }
    for(port_id = 0; port_id < (glb_card->port_num); port_id++)
    {
        p_port = glb_card->pp_port[port_id];
        if (port_id < T65C_8T64X6Q_8T)
        {
            phyinfo.phy_device_type = PORT_PHY_VSC8512;
            phyinfo.phy_manage_info.speed = GLB_SPEED_AUTO;
            phyinfo.phy_manage_info.duplex = GLB_DUPLEX_AUTO;
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_RJ45;
            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_SGMII; 

            phyinfo.led_type = phy_led_type;
            phyinfo.led_freq = GLB_PHY_LED_FREQ_2P5;
            
            phyinfo.vct_support = 1;
            phyinfo.phy_addr = lcm_t65c_8t64x6q_port_max[port_id].phy_addr;
            phyinfo.mdio_bus = ASIC_GG_PHY_1G_TYPE;

            phyinfo.base_addr = 0; //smi_id
     

            if(port_id == T65C_8T64X6Q_PHY_FIRST_ADDR)
            {
                phyinfo.phy_manage_info.phy_init_seq_flag = GLB_PRE_INIT_FLAG;
            }
            else if(port_id == T65C_8T64X6Q_PHY_LAST_ADDR)
            {
                phyinfo.phy_manage_info.phy_init_seq_flag = GLB_POST_INIT_FLAG;
            }
            else
            {
                phyinfo.phy_manage_info.phy_init_seq_flag = GLB_NO_INIT_FLAG;                        
            }
        }
        else
        {
            phyinfo.phy_device_type = PORT_PHY_NULL;
            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_SERDES;

            lcm_common_get_defaultspeed_from_speedability(p_port->port_speed_ability, 
                &(phyinfo.phy_manage_info.speed), &(phyinfo.phy_manage_info.duplex));
            if(port_id < T65C_8T64X6Q_8T64X)
            {     
                phyinfo.phy_manage_info.media_type = GLB_MEDIA_SFP_PLUS;
            }
            else
            {     
                phyinfo.phy_manage_info.media_type = GLB_MEDIA_QSFP_PLUS;
            }     
            if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)
            {
                phyinfo.port_info.serdes_num = 4;
                /*40G BASE-KR4 supports auto-negotiation*/
                phyinfo.phy_manage_info.cl73_speed_ability = CTC_PORT_CL73_40GBASE_KR4;                
                if ((phyinfo.phy_manage_info.speed==GLB_SPEED_AUTO)||(phyinfo.phy_manage_info.duplex==GLB_DUPLEX_AUTO))
                {
                    phyinfo.phy_manage_info.cl73_enable = 1;
                }
                else
                {
                    phyinfo.phy_manage_info.cl73_enable = 0;
                }
            }
            else
            {
                phyinfo.port_info.serdes_num = 1;
                phyinfo.phy_manage_info.cl73_enable = 0;
            }
            if (lcm_stack_is_enable())
            {
                phyinfo.port_info.port_id = GLB_TRANS_PORTID_TO_GPORT(
                    lcm_stack_self_slotid(), p_port->logic_port_idx);  
            }
            else
            {
                phyinfo.port_info.port_id = p_port->logic_port_idx;
            }
            phyinfo.port_info.lchip = glb_card->pp_port[port_id]->local_chip_idx;
            phyinfo.port_info.serdes_id = glb_card->pp_port[port_id]->chip_serdes_id;
        }

        phyinfo.port_num = port_id;


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

    LCM_LOG_DEBUG(LCM_CARD, "Lcm ready to init phy.");
    LCM_LOG_BRINGUP("Lcm ready to init phy.");
    ret = phy_init(pphdl, (glb_card->port_num));
    if(ret)
    {
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init phy end.");
    LCM_LOG_BRINGUP("Lcm init phy end.");
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_t65c_8t64x6q_i2c(void)
{
    int32 ret;

    sal_task_sleep(100);
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init i2c bus begin.");
    LCM_LOG_BRINGUP("Lcm init i2c bus begin.");
    
    ret = i2c_open(E_I2C_CPM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Init i2c bus failed.");
        return LCM_E_HW_CLKGEN_GEN;
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init i2c bus end.");
    LCM_LOG_BRINGUP("Lcm init i2c bus end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_led(void)
{
    int32 ret = 0;
    led_info_t led_info[E_MAX_LED];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init LED begin.");
    LCM_LOG_BRINGUP("Lcm init LED begin.");
    sal_memset(led_info, 0, sizeof(led_info_t)*E_MAX_LED);

    /* init sys led */
    led_info[E_SYS_LED].exist = 1;
    led_info[E_SYS_LED].led_ctl = E_LED_EPLD0;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].val = 0x5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].val = 0xb;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].val = 0x5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].val = 0x7;
    /* End of Merge */
    led_info[E_FAN_LED].exist = 1;
    led_info[E_FAN_LED].led_ctl = E_LED_EPLD0;
    led_info[E_FAN_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_NO_ALARM].val = 1; /*light green led*/
    led_info[E_FAN_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ALARM].val = 0x2; /*light yellow led*/
    led_info[E_FAN_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ABSENT].val = 0x0; /*Turn off led*/

    led_info[E_PSU1_LED].exist = 1;
    led_info[E_PSU1_LED].led_ctl = E_LED_EPLD0;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;/*light green led*/
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ALARM].val = 0x2;/*light yellow led*/
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;/*Turn off led*/

    led_info[E_PSU2_LED].exist = 1;
    led_info[E_PSU2_LED].led_ctl = E_LED_EPLD0;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_PSU2_LED;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_PSU2_LED;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ALARM].val = 0x2;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_PSU2_LED;
    led_info[E_PSU2_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;	

    led_info[E_IND_LED].exist = 1;
    led_info[E_IND_LED].led_ctl = E_LED_EPLD0;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].op_reg = EPLD_IND_LED;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].val = 0x0;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].op_reg = EPLD_IND_LED;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].val = 0x1; /*Led lights alternately flashing yellow and green*/
	
    ret = led_init(led_info);
    if(ret < 0)
    {
        LCM_LOG_ERR("Init LEDs failed.");
        return LCM_E_HW_LED_GEN;
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init LED end.");
    LCM_LOG_BRINGUP("Lcm init LED end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_t65c_8t64x6q_ctc_chip_led(void)
{
    uint8 table_id = 0;
    uint8 table_num = 1;
    mac_led_info_t mac_led_info;
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init MAC LED begin.");
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
        mac_led_info.mac_led_api_para[table_id] = lcm_t65c_8t64x6q_mac_led_default_entry;
    }

    mac_led_info.table_num = table_num;
    mac_led_info.mac_num = T65C_8T64X6Q_LED_MAC_NUM;
    mac_led_info.slice0_mac_num = T65C_8T64X6Q_SLICE0_LED_MAC_NUM;
    mac_led_info.polarity = 1;
    mac_led_info_register(&mac_led_info);
    mac_led_init();

    XFREE(CTCLIB_MEM_LCM_MODULE, mac_led_info.mac_led_api_para);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init MAC LED end.");
    LCM_LOG_BRINGUP("Lcm init MAC LED end."); 
    return LCM_E_SUCCESS;
}

int32
lcm_t65c_8t64x6q_get_sum_ffe_cfg(lcm_chip_serdes_ffe_t* serdes_ffe)
{
    uint8 serdes_id = 0;
    uint8 index = 0;

    serdes_id = serdes_ffe->serdes_id;
    serdes_ffe->board_material = glb_card->board_material;
    serdes_ffe->mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;             /* typical */
    for(index = 0; index < T65C_8T64X6Q_PANEL_PORT_NUM_MAX; index++)
    {
        if(serdes_id == lcm_t65c_8t64x6q_serdes_ffe_max[index].serdes_id)
        {                
            serdes_ffe->trace_len = lcm_t65c_8t64x6q_serdes_ffe_max[index].trace_len;
            return 0;
        }  
    }        
    return -1;
}

static int32
_lcm_init_t65c_8t64x6q_serdes_mode(void)
{
    uint8 i = 0;
    uint8 pos = 0;
    uint8 panel_port = 0;
    int32 ret = 0;
    ctc_chip_serdes_info_t serdes_info;

    sal_memset(&serdes_info, 0x0, sizeof(serdes_info));

    /* disable all 40G/100G Mac for serdes mode switch */
    for(panel_port = 1; panel_port <= glb_card->panel_port_num; panel_port++)
    {
        if (panel_port <= T65C_8T64X6Q_8T64X)          
        {
            continue;
        }
        else
        {
            pos = T65C_8T64X6Q_8T64X + (panel_port - T65C_8T64X6Q_8T64X-1) *GLB_QSFP_CHANNEL_NUM + 3;
        }
        /* default set mac disable */
        ctc_port_set_mac_en(lcm_t65c_8t64x6q_port_max[pos].logic_port_idx, 0);
    }

    for(panel_port = 0; panel_port < glb_card->panel_port_num; panel_port++)
    {
        if (panel_port < T65C_8T64X6Q_8T64X) /* no serdes mode change for 10G port and 1G*/
        {
            continue;
        }
        else if(GLB_PPT_OP_NONE == glb_card->pp_ppt[panel_port]->op)
        {
            continue;
        }
        else if(GLB_PPT_OP_SPLIT == glb_card->pp_ppt[panel_port]->op)
        {
            if(glb_card->pp_ppt[panel_port]->ppt_numchg_type == GLB_PPT_NUMCHG_TYPE_10G)
            {
                serdes_info.serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
                for(i = 0; i < GLB_QSFP_CHANNEL_NUM; i++)           /* set all four serdes to XFI mode */
                {
                    serdes_info.serdes_id = lcm_t65c_8t64x6q_serdes_ffe_max[T65C_8T64X6Q_8T64X+ \
                        (panel_port-T65C_8T64X6Q_8T64X)*GLB_QSFP_CHANNEL_NUM+i].serdes_id;
                    ret += ctc_chip_set_serdes_mode(0, &serdes_info);
                }
            }
            else if(glb_card->pp_ppt[panel_port]->ppt_numchg_type == GLB_PPT_NUMCHG_TYPE_1G) 
            {
                serdes_info.serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
                for(i = 0; i < GLB_QSFP_CHANNEL_NUM; i++)          
                {
                    serdes_info.serdes_id = lcm_t65c_8t64x6q_serdes_ffe_max[T65C_8T64X6Q_8T64X+ \
                        (panel_port-T65C_8T64X6Q_8T64X)*GLB_QSFP_CHANNEL_NUM+i].serdes_id;
                    ret += ctc_chip_set_serdes_mode(0, &serdes_info);
                }
            }
        }
    }
    
    return ret;
}

int32
_lcm_init_t65c_8t64x6q_serdes_ffe(void)
{
    ctc_chip_serdes_ffe_t serdes_ffe;
    uint8 index;
    int32 ret;

    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes ffe begin.");
    LCM_LOG_BRINGUP("Lcm init serdes ffe begin.");
    
    sal_memset(&serdes_ffe, 0, sizeof(ctc_chip_serdes_ffe_t));
    
    serdes_ffe.board_material = glb_card->board_material;   /* FR4 */    

    /* init typical ffe */
    serdes_ffe.mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;             /* typical */    
    for(index = 0; index < T65C_8T64X6Q_PANEL_PORT_NUM_MAX; index++)
    {
        serdes_ffe.serdes_id = lcm_t65c_8t64x6q_serdes_ffe_max[index].serdes_id;
        serdes_ffe.trace_len = lcm_t65c_8t64x6q_serdes_ffe_max[index].trace_len;
        ret = ctc_chip_set_property(0, CTC_CHIP_PROP_SERDES_FFE, (void *)&serdes_ffe);
        if (ret < 0)
        {
            LCM_LOG_ERR("LCM set serdes ffe parameter failed.\n");
            return LCM_E_INIT_FAILED;
        }  
    }
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_serdes_mode());

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes ffe end.");
    LCM_LOG_BRINGUP("Lcm init serdes ffe end.");    
    return ret;
}

static int32
_lcm_init_t65c_8t64x6q_reset_device(void)
{
    /* make sure the states of every device before init is reset */
    /* reset GE phy */
    epld_item_write(0, EPLD_PHY_RST, 0x0);
    sal_task_sleep(100);
    return LCM_E_SUCCESS;
}


int32
lcm_init_t65c_8t64x6q_cb(void)
{
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init callback begin.");
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_gpio());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_fiber());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_phy());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_ctc_chip_led());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_serdes_ffe());
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init callback end.");

    return LCM_E_SUCCESS;
}

int32
lcm_t65c_8t64x6q_config_clock()
{
    int32 ret = 0;
    uint32 val = 0;
    uint32 timeout = 20000;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm config clock begin.");
    LCM_LOG_BRINGUP("Lcm config clock begin.");
    
    /* step 1 */
    ret += clock_write(0, 0x0a22, 0x01);
    ret += clock_write(0, 0x0a42, 0x01);

    /* step 2 */
    ret += clock_write(0, 0x0005, 0x01);

    /* step 3 */
    ret += clock_write(0, 0x0, 0x80);
    ret += clock_write(0, 0x4, 0x00);
    ret += clock_write(0, 0x5, 0x00);
    ret += clock_write(0, 0xa, 0x12);
    ret += clock_write(0, 0xb, 0x0F);
    ret += clock_write(0, 0xc, 0x02);
    ret += clock_write(0, 0xd, 0x00);
    ret += clock_write(0, 0xe, 0x01);
    ret += clock_write(0, 0xf, 0x00);
#if 0 
    ret += clock_write(0, 0x80, 0x07);
    ret += clock_write(0, 0x81, 0x00);
    ret += clock_write(0, 0x82, 0x01);
    ret += clock_write(0, 0x83, 0x03);
    ret += clock_write(0, 0x84, 0x00);
    ret += clock_write(0, 0x85, 0x01);
    ret += clock_write(0, 0x86, 0x01);
    ret += clock_write(0, 0x87, 0x00);
    ret += clock_write(0, 0x88, 0x00);
#endif
    ret += clock_write(0, 0x100, 0x01);
    ret += clock_write(0, 0x101, 0x00);
    ret += clock_write(0, 0x102, 0x90);
    ret += clock_write(0, 0x103, 0x00);
    ret += clock_write(0, 0x104, 0x00);
    ret += clock_write(0, 0x105, 0x00);
    ret += clock_write(0, 0x106, 0x00);
    ret += clock_write(0, 0x107, 0x00);
    ret += clock_write(0, 0x108, 0x00);
    ret += clock_write(0, 0x109, 0x00);
    ret += clock_write(0, 0x10a, 0x00);
    ret += clock_write(0, 0x10b, 0x00);
    ret += clock_write(0, 0x10c, 0x00);
    ret += clock_write(0, 0x10d, 0x0F);
    ret += clock_write(0, 0x10e, 0x60);
    ret += clock_write(0, 0x10f, 0x00);
    ret += clock_write(0, 0x110, 0x0F);
    ret += clock_write(0, 0x111, 0x60);
    ret += clock_write(0, 0x112, 0x00);
    #if 0 //change by qianj,2018/05/07
    ret += clock_write(0, 0x200, 0x1E);
    ret += clock_write(0, 0x201, 0x01);
    #endif
    #if 1
    ret += clock_write(0, 0x200, 0x3C);
    ret += clock_write(0, 0x201, 0x00);
    #endif
    ret += clock_write(0, 0x202, 0x43);
    ret += clock_write(0, 0x203, 0xDE);
    ret += clock_write(0, 0x204, 0x13);
    ret += clock_write(0, 0x205, 0x32);
    ret += clock_write(0, 0x206, 0x00);
    ret += clock_write(0, 0x207, 0x00);
                      
    /* step 4 */
    ret += clock_write(0, 0x0005, 0x01);

    /* step 5 */
    ret += clock_read(0, 0x0d01, &val);
    while ((1 != ((val >> 1) & 0x1)) && (timeout--))
    {
        ret += clock_read(0, 0x0d01, &val);
    }
    if (!timeout)
    {
        LCM_LOG_DEBUG(LCM_CARD, "LCM config clock step 5 timeout!");
        LCM_LOG_BRINGUP("LCM config clock step 5 timeout!");
        return -1;
    }

    /* step 6 */
    /* For the outputs to toggle prior to DPLL phase or frequency lock */
    ret += clock_write(0, 0x0a20, 0x4 );
    ret += clock_write(0, 0x0a40, 0x4 );
    ret += clock_write(0, 0x0005, 0x01);
	
    /* step 7 */
    ret += clock_write(0, 0x300, 0x03);
    ret += clock_write(0, 0x301, 0x32);
    ret += clock_write(0, 0x302, 0x04);
    ret += clock_write(0, 0x303, 0x00);
    ret += clock_write(0, 0x304, 0xCA);
    ret += clock_write(0, 0x305, 0xF6);
    ret += clock_write(0, 0x306, 0x93);
    ret += clock_write(0, 0x307, 0x00);
    ret += clock_write(0, 0x308, 0x00);
    ret += clock_write(0, 0x309, 0x1A);
    ret += clock_write(0, 0x30a, 0x17);
    ret += clock_write(0, 0x30b, 0x02);
    ret += clock_write(0, 0x30c, 0x84);
    ret += clock_write(0, 0x30d, 0x70);
    ret += clock_write(0, 0x30e, 0x01);
    ret += clock_write(0, 0x30f, 0xE8);
    ret += clock_write(0, 0x310, 0x03);
    ret += clock_write(0, 0x311, 0xE1);
    ret += clock_write(0, 0x312, 0x25);
    ret += clock_write(0, 0x313, 0x00);
    ret += clock_write(0, 0x314, 0x0A);
    ret += clock_write(0, 0x315, 0x0A);
    ret += clock_write(0, 0x316, 0xE8);
    ret += clock_write(0, 0x317, 0x03);
    ret += clock_write(0, 0x318, 0x00);
    ret += clock_write(0, 0x319, 0x0A);
    ret += clock_write(0, 0x31a, 0x0A);
    ret += clock_write(0, 0x320, 0x03);
    ret += clock_write(0, 0x321, 0x32);
    ret += clock_write(0, 0x322, 0x04);
    ret += clock_write(0, 0x323, 0x00);
    ret += clock_write(0, 0x324, 0xCA);
    ret += clock_write(0, 0x325, 0xF6);
    ret += clock_write(0, 0x326, 0x93);
    ret += clock_write(0, 0x327, 0x00);
    ret += clock_write(0, 0x328, 0x00);
    ret += clock_write(0, 0x329, 0x1A);
    ret += clock_write(0, 0x32a, 0x17);
    ret += clock_write(0, 0x32b, 0x02);
    ret += clock_write(0, 0x32c, 0x84);
    ret += clock_write(0, 0x32d, 0x70);
    ret += clock_write(0, 0x32e, 0x01);
    ret += clock_write(0, 0x32f, 0xE8);
    ret += clock_write(0, 0x330, 0x03);
    ret += clock_write(0, 0x331, 0xE1);
    ret += clock_write(0, 0x332, 0x25);
    ret += clock_write(0, 0x333, 0x00);
    ret += clock_write(0, 0x334, 0x0A);
    ret += clock_write(0, 0x335, 0x0A);
    ret += clock_write(0, 0x336, 0xE8);
    ret += clock_write(0, 0x337, 0x03);
    ret += clock_write(0, 0x338, 0x00);
    ret += clock_write(0, 0x339, 0x0A);
    ret += clock_write(0, 0x33a, 0x0A);
    ret += clock_write(0, 0x340, 0x03);
    ret += clock_write(0, 0x341, 0x45);
    ret += clock_write(0, 0x342, 0x01);
    ret += clock_write(0, 0x343, 0x00);
    ret += clock_write(0, 0x344, 0x00);
    ret += clock_write(0, 0x345, 0x48);
    ret += clock_write(0, 0x346, 0xE8);
    ret += clock_write(0, 0x347, 0x01);
    ret += clock_write(0, 0x348, 0x00);
    ret += clock_write(0, 0x349, 0x1A);
    ret += clock_write(0, 0x34a, 0x17);
    ret += clock_write(0, 0x34b, 0x02);
    ret += clock_write(0, 0x34c, 0x84);
    ret += clock_write(0, 0x34d, 0x70);
    ret += clock_write(0, 0x34e, 0x01);
    ret += clock_write(0, 0x34f, 0xE8);
    ret += clock_write(0, 0x350, 0x03);
    ret += clock_write(0, 0x351, 0x00);
    ret += clock_write(0, 0x352, 0x7D);
    ret += clock_write(0, 0x353, 0x00);
    ret += clock_write(0, 0x354, 0x0A);
    ret += clock_write(0, 0x355, 0x0A);
    ret += clock_write(0, 0x356, 0xE8);
    ret += clock_write(0, 0x357, 0x03);
    ret += clock_write(0, 0x358, 0x00);
    ret += clock_write(0, 0x359, 0x0A);
    ret += clock_write(0, 0x35a, 0x0A);
    ret += clock_write(0, 0x360, 0x00);
    ret += clock_write(0, 0x361, 0x04);
    ret += clock_write(0, 0x362, 0x01);
    ret += clock_write(0, 0x363, 0x00);
    ret += clock_write(0, 0x364, 0x00);
    ret += clock_write(0, 0x365, 0x5A);
    ret += clock_write(0, 0x366, 0x62);
    ret += clock_write(0, 0x367, 0x02);
    ret += clock_write(0, 0x368, 0x00);
    ret += clock_write(0, 0x369, 0x1A);
    ret += clock_write(0, 0x36a, 0x17);
    ret += clock_write(0, 0x36b, 0x02);
    ret += clock_write(0, 0x36c, 0x84);
    ret += clock_write(0, 0x36d, 0x70);
    ret += clock_write(0, 0x36e, 0x01);
    ret += clock_write(0, 0x36f, 0xE8);
    ret += clock_write(0, 0x370, 0x03);
    ret += clock_write(0, 0x371, 0x40);
    ret += clock_write(0, 0x372, 0x9C);
    ret += clock_write(0, 0x373, 0x00);
    ret += clock_write(0, 0x374, 0x0A);
    ret += clock_write(0, 0x375, 0x0A);
    ret += clock_write(0, 0x376, 0xE8);
    ret += clock_write(0, 0x377, 0x03);
    ret += clock_write(0, 0x378, 0x00);
    ret += clock_write(0, 0x379, 0x0A);
    ret += clock_write(0, 0x37a, 0x0A);
    ret += clock_write(0, 0x400, 0x23);
    ret += clock_write(0, 0x401, 0x15);
    ret += clock_write(0, 0x402, 0xC6);
    ret += clock_write(0, 0x403, 0x16);
    ret += clock_write(0, 0x404, 0x08);
    ret += clock_write(0, 0x405, 0x51);
    ret += clock_write(0, 0x406, 0xB8);
    ret += clock_write(0, 0x407, 0x02);
    ret += clock_write(0, 0x408, 0x3E);
    ret += clock_write(0, 0x409, 0x0A);
    ret += clock_write(0, 0x40a, 0x0B);
    ret += clock_write(0, 0x40b, 0x0A);
    ret += clock_write(0, 0x40c, 0x00);
    ret += clock_write(0, 0x40d, 0x00);
    ret += clock_write(0, 0x40e, 0x00);
    ret += clock_write(0, 0x40f, 0x00);
    ret += clock_write(0, 0x410, 0x00);
    ret += clock_write(0, 0x411, 0x00);
    ret += clock_write(0, 0x412, 0x00);
    ret += clock_write(0, 0x413, 0x00);
    ret += clock_write(0, 0x414, 0x00);
    ret += clock_write(0, 0x415, 0x00);
    ret += clock_write(0, 0x420, 0x81);
    ret += clock_write(0, 0x421, 0x11);
    ret += clock_write(0, 0x422, 0x47);
    ret += clock_write(0, 0x423, 0x00);
    ret += clock_write(0, 0x424, 0x02);
    ret += clock_write(0, 0x425, 0x00);
    ret += clock_write(0, 0x426, 0x00);
    ret += clock_write(0, 0x427, 0x10);
    ret += clock_write(0, 0x428, 0x04);
    ret += clock_write(0, 0x429, 0x00);
    ret += clock_write(0, 0x42a, 0x00);
    ret += clock_write(0, 0x42b, 0x10);
    ret += clock_write(0, 0x42c, 0x04);
    ret += clock_write(0, 0x42d, 0x00);
    ret += clock_write(0, 0x42e, 0x00);
    ret += clock_write(0, 0x440, 0x01);
    ret += clock_write(0, 0x441, 0xF4);
    ret += clock_write(0, 0x442, 0x01);
    ret += clock_write(0, 0x443, 0x00);
    ret += clock_write(0, 0x444, 0x7B);
    ret += clock_write(0, 0x445, 0x07);
    ret += clock_write(0, 0x446, 0x00);
    ret += clock_write(0, 0x447, 0x7C);
    ret += clock_write(0, 0x448, 0x00);
    ret += clock_write(0, 0x449, 0x00);
    ret += clock_write(0, 0x44a, 0x31);
    ret += clock_write(0, 0x44b, 0x02);
    ret += clock_write(0, 0x44c, 0x00);
    ret += clock_write(0, 0x44d, 0x01);
    ret += clock_write(0, 0x44e, 0xF4);
    ret += clock_write(0, 0x44f, 0x01);
    ret += clock_write(0, 0x450, 0x00);
    ret += clock_write(0, 0x451, 0x7B);
    ret += clock_write(0, 0x452, 0x07);
    ret += clock_write(0, 0x453, 0x00);
    ret += clock_write(0, 0x454, 0x7C);
    ret += clock_write(0, 0x455, 0x00);
    ret += clock_write(0, 0x456, 0x00);
    ret += clock_write(0, 0x457, 0x31);
    ret += clock_write(0, 0x458, 0x02);
    ret += clock_write(0, 0x459, 0x00);
    ret += clock_write(0, 0x45a, 0x01);
    ret += clock_write(0, 0x45b, 0xF4);
    ret += clock_write(0, 0x45c, 0x01);
    ret += clock_write(0, 0x45d, 0x00);
    ret += clock_write(0, 0x45e, 0x7C);
    ret += clock_write(0, 0x45f, 0x07);
    ret += clock_write(0, 0x460, 0x00);
    ret += clock_write(0, 0x461, 0x0B);
    ret += clock_write(0, 0x462, 0x00);
    ret += clock_write(0, 0x463, 0x00);
    ret += clock_write(0, 0x464, 0x11);
    ret += clock_write(0, 0x465, 0x00);
    ret += clock_write(0, 0x466, 0x00);
    ret += clock_write(0, 0x467, 0x01);
    ret += clock_write(0, 0x468, 0xF4);
    ret += clock_write(0, 0x469, 0x01);
    ret += clock_write(0, 0x46a, 0x00);
    ret += clock_write(0, 0x46b, 0x7E);
    ret += clock_write(0, 0x46c, 0x07);
    ret += clock_write(0, 0x46d, 0x00);
    ret += clock_write(0, 0x46e, 0x02);
    ret += clock_write(0, 0x46f, 0x00);
    ret += clock_write(0, 0x470, 0x00);
    ret += clock_write(0, 0x471, 0x11);
    ret += clock_write(0, 0x472, 0x00);
    ret += clock_write(0, 0x473, 0x00);
    ret += clock_write(0, 0x500, 0x20);
    ret += clock_write(0, 0x501, 0x79);
    ret += clock_write(0, 0x502, 0xA6);
    ret += clock_write(0, 0x503, 0x15);
    ret += clock_write(0, 0x504, 0x08);
    ret += clock_write(0, 0x505, 0x51);
    ret += clock_write(0, 0x506, 0xB8);
    ret += clock_write(0, 0x507, 0x02);
    ret += clock_write(0, 0x508, 0x3E);
    ret += clock_write(0, 0x509, 0x0A);
    ret += clock_write(0, 0x50a, 0x0B);
    ret += clock_write(0, 0x50b, 0x0A);
    ret += clock_write(0, 0x50c, 0x00);
    ret += clock_write(0, 0x50d, 0x00);
    ret += clock_write(0, 0x50e, 0x00);
    ret += clock_write(0, 0x50f, 0x00);
    ret += clock_write(0, 0x510, 0x00);
    ret += clock_write(0, 0x511, 0x00);
    ret += clock_write(0, 0x512, 0x00);
    ret += clock_write(0, 0x513, 0x00);
    ret += clock_write(0, 0x514, 0x00);
    ret += clock_write(0, 0x515, 0x00);
    ret += clock_write(0, 0x520, 0x81);
    ret += clock_write(0, 0x521, 0x13);
    ret += clock_write(0, 0x522, 0x47);
    ret += clock_write(0, 0x523, 0x00);
    ret += clock_write(0, 0x524, 0x02);
    ret += clock_write(0, 0x525, 0x00);
    ret += clock_write(0, 0x526, 0x00);
    ret += clock_write(0, 0x527, 0x10);
    ret += clock_write(0, 0x528, 0x06);
    ret += clock_write(0, 0x529, 0x00);
    ret += clock_write(0, 0x52a, 0x00);
    ret += clock_write(0, 0x52b, 0xC6);
    ret += clock_write(0, 0x52c, 0x22);
    ret += clock_write(0, 0x52d, 0x00);
    ret += clock_write(0, 0x52e, 0x00);
    ret += clock_write(0, 0x540, 0x01);
    ret += clock_write(0, 0x541, 0xF4);
    ret += clock_write(0, 0x542, 0x01);
    ret += clock_write(0, 0x543, 0x00);
    ret += clock_write(0, 0x544, 0x80);
    ret += clock_write(0, 0x545, 0x07);
    ret += clock_write(0, 0x546, 0x00);
    ret += clock_write(0, 0x547, 0x0D);
    ret += clock_write(0, 0x548, 0x00);
    ret += clock_write(0, 0x549, 0x00);
    ret += clock_write(0, 0x54a, 0x13);
    ret += clock_write(0, 0x54b, 0x00);
    ret += clock_write(0, 0x54c, 0x00);
    ret += clock_write(0, 0x54d, 0x01);
    ret += clock_write(0, 0x54e, 0xF4);
    ret += clock_write(0, 0x54f, 0x01);
    ret += clock_write(0, 0x550, 0x00);
    ret += clock_write(0, 0x551, 0x82);
    ret += clock_write(0, 0x552, 0x07);
    ret += clock_write(0, 0x553, 0x00);
    ret += clock_write(0, 0x554, 0x03);
    ret += clock_write(0, 0x555, 0x00);
    ret += clock_write(0, 0x556, 0x00);
    ret += clock_write(0, 0x557, 0x13);
    ret += clock_write(0, 0x558, 0x00);
    ret += clock_write(0, 0x559, 0x00);
    ret += clock_write(0, 0x55a, 0x01);
    ret += clock_write(0, 0x55b, 0xF4);
    ret += clock_write(0, 0x55c, 0x01);
    ret += clock_write(0, 0x55d, 0x00);
    ret += clock_write(0, 0x55e, 0x7F);
    ret += clock_write(0, 0x55f, 0x07);
    ret += clock_write(0, 0x560, 0x00);
    ret += clock_write(0, 0x561, 0xA0);
    ret += clock_write(0, 0x562, 0x00);
    ret += clock_write(0, 0x563, 0x00);
    ret += clock_write(0, 0x564, 0x73);
    ret += clock_write(0, 0x565, 0x02);
    ret += clock_write(0, 0x566, 0x00);
    ret += clock_write(0, 0x567, 0x01);
    ret += clock_write(0, 0x568, 0xF4);
    ret += clock_write(0, 0x569, 0x01);
    ret += clock_write(0, 0x56a, 0x00);
    ret += clock_write(0, 0x56b, 0x7F);
    ret += clock_write(0, 0x56c, 0x07);
    ret += clock_write(0, 0x56d, 0x00);
    ret += clock_write(0, 0x56e, 0xA0);
    ret += clock_write(0, 0x56f, 0x00);
    ret += clock_write(0, 0x570, 0x00);
    ret += clock_write(0, 0x571, 0x73);
    ret += clock_write(0, 0x572, 0x02);
    ret += clock_write(0, 0x573, 0x00);
    ret += clock_write(0, 0x800, 0x24);
    ret += clock_write(0, 0x801, 0x8C);
    ret += clock_write(0, 0x802, 0x49);
    ret += clock_write(0, 0x803, 0x55);
    ret += clock_write(0, 0x804, 0xC9);
    ret += clock_write(0, 0x805, 0x7B);
    ret += clock_write(0, 0x806, 0x9C);
    ret += clock_write(0, 0x807, 0xFA);
    ret += clock_write(0, 0x808, 0x55);
    ret += clock_write(0, 0x809, 0xEA);
    ret += clock_write(0, 0x80a, 0xE2);
    ret += clock_write(0, 0x80b, 0x57);
    ret += clock_write(0, 0x80c, 0x8C);
    ret += clock_write(0, 0x80d, 0xAD);
    ret += clock_write(0, 0x80e, 0x4C);
    ret += clock_write(0, 0x80f, 0xF5);
    ret += clock_write(0, 0x810, 0xCB);
    ret += clock_write(0, 0x811, 0x73);
    ret += clock_write(0, 0x812, 0x24);
    ret += clock_write(0, 0x813, 0xD8);
    ret += clock_write(0, 0x814, 0x59);
    ret += clock_write(0, 0x815, 0xD2);
    ret += clock_write(0, 0x816, 0x8D);
    ret += clock_write(0, 0x817, 0x5A);
    ret += clock_write(0, 0xa00, 0x00);
    ret += clock_write(0, 0xa01, 0x00);
    ret += clock_write(0, 0xa02, 0x00);
    ret += clock_write(0, 0xa03, 0x00);
    ret += clock_write(0, 0xa04, 0x00);
    ret += clock_write(0, 0xa05, 0x00);
    ret += clock_write(0, 0xa06, 0x00);
    ret += clock_write(0, 0xa07, 0x00);
    ret += clock_write(0, 0xa08, 0x00);
    ret += clock_write(0, 0xa09, 0x00);
    ret += clock_write(0, 0xa0a, 0x00);
    ret += clock_write(0, 0xa0b, 0x00);
    ret += clock_write(0, 0xa0c, 0x00);
    ret += clock_write(0, 0xa0d, 0x00);
    ret += clock_write(0, 0xa0e, 0x00);
    ret += clock_write(0, 0xa20, 0x00);
    ret += clock_write(0, 0xa21, 0x00);
    ret += clock_write(0, 0xa22, 0x00);
    ret += clock_write(0, 0xa23, 0x00);
    ret += clock_write(0, 0xa24, 0x00);
    ret += clock_write(0, 0xa40, 0x00);
    ret += clock_write(0, 0xa41, 0x00);
    ret += clock_write(0, 0xa42, 0x00);
    ret += clock_write(0, 0xa43, 0x00);
    ret += clock_write(0, 0xa44, 0x00);
    ret += clock_write(0, 0xd00, 0x00);
    ret += clock_write(0, 0xd01, 0x00);
    ret += clock_write(0, 0xd02, 0x00);
    ret += clock_write(0, 0xd03, 0x00);
    ret += clock_write(0, 0xd04, 0x00);
    ret += clock_write(0, 0xd05, 0x00);
    ret += clock_write(0, 0xd08, 0x00);
    ret += clock_write(0, 0xd09, 0x00);
    ret += clock_write(0, 0xd0a, 0x00);
    ret += clock_write(0, 0xd0b, 0x00);
    ret += clock_write(0, 0xd0c, 0x00);
    ret += clock_write(0, 0xd0d, 0x00);
    ret += clock_write(0, 0xd0e, 0x00);
    ret += clock_write(0, 0xd0f, 0x00);
    ret += clock_write(0, 0xd10, 0x00);
    ret += clock_write(0, 0xd20, 0x00);
    ret += clock_write(0, 0xd21, 0x00);
    ret += clock_write(0, 0xd22, 0x00);
    ret += clock_write(0, 0xd23, 0x00);
    ret += clock_write(0, 0xd24, 0x00);
    ret += clock_write(0, 0xd25, 0x00);
    ret += clock_write(0, 0xd26, 0x00);
    ret += clock_write(0, 0xd27, 0x00);
    ret += clock_write(0, 0xd28, 0x00);
    ret += clock_write(0, 0xd29, 0x00);
    ret += clock_write(0, 0xd2a, 0x00);
    ret += clock_write(0, 0xd40, 0x00);
    ret += clock_write(0, 0xd41, 0x00);
    ret += clock_write(0, 0xd42, 0x00);
    ret += clock_write(0, 0xd43, 0x00);
    ret += clock_write(0, 0xd44, 0x00);
    ret += clock_write(0, 0xd45, 0x00);
    ret += clock_write(0, 0xd46, 0x00);
    ret += clock_write(0, 0xd47, 0x00);
    ret += clock_write(0, 0xd48, 0x00);
    ret += clock_write(0, 0xd49, 0x00);
    ret += clock_write(0, 0xd4a, 0x00);
    ret += clock_write(0, 0xe00, 0x00);
    ret += clock_write(0, 0xe01, 0x00);
    ret += clock_write(0, 0xe02, 0x00);
    ret += clock_write(0, 0xe03, 0x00);
    ret += clock_write(0, 0xe10, 0x98);
    ret += clock_write(0, 0xe11, 0x01);
    ret += clock_write(0, 0xe12, 0x00);
    ret += clock_write(0, 0xe13, 0x0E);
    ret += clock_write(0, 0xe14, 0x12);
    ret += clock_write(0, 0xe15, 0x01);
    ret += clock_write(0, 0xe16, 0x00);
    ret += clock_write(0, 0xe17, 0x07);
    ret += clock_write(0, 0xe18, 0x02);
    ret += clock_write(0, 0xe19, 0x00);
    ret += clock_write(0, 0xe1a, 0x80);
    ret += clock_write(0, 0xe1b, 0x1A);
    ret += clock_write(0, 0xe1c, 0x03);
    ret += clock_write(0, 0xe1d, 0x00);
    ret += clock_write(0, 0xe1e, 0x1A);
    ret += clock_write(0, 0xe1f, 0x03);
    ret += clock_write(0, 0xe20, 0x20);
    ret += clock_write(0, 0xe21, 0x1A);
    ret += clock_write(0, 0xe22, 0x03);
    ret += clock_write(0, 0xe23, 0x40);
    ret += clock_write(0, 0xe24, 0x1A);
    ret += clock_write(0, 0xe25, 0x03);
    ret += clock_write(0, 0xe26, 0x60);
    ret += clock_write(0, 0xe27, 0x15);
    ret += clock_write(0, 0xe28, 0x04);
    ret += clock_write(0, 0xe29, 0x00);
    ret += clock_write(0, 0xe2a, 0x0E);
    ret += clock_write(0, 0xe2b, 0x04);
    ret += clock_write(0, 0xe2c, 0x20);
    ret += clock_write(0, 0xe2d, 0x33);
    ret += clock_write(0, 0xe2e, 0x04);
    ret += clock_write(0, 0xe2f, 0x40);
    ret += clock_write(0, 0xe30, 0x15);
    ret += clock_write(0, 0xe31, 0x05);
    ret += clock_write(0, 0xe32, 0x00);
    ret += clock_write(0, 0xe33, 0x0E);
    ret += clock_write(0, 0xe34, 0x05);
    ret += clock_write(0, 0xe35, 0x20);
    ret += clock_write(0, 0xe36, 0x33);
    ret += clock_write(0, 0xe37, 0x05);
    ret += clock_write(0, 0xe38, 0x40);
    ret += clock_write(0, 0xe39, 0x17);
    ret += clock_write(0, 0xe3a, 0x08);
    ret += clock_write(0, 0xe3b, 0x00);
    ret += clock_write(0, 0xe3c, 0x0E);
    ret += clock_write(0, 0xe3d, 0x0A);
    ret += clock_write(0, 0xe3e, 0x00);
    ret += clock_write(0, 0xe3f, 0x04);
    ret += clock_write(0, 0xe40, 0x0A);
    ret += clock_write(0, 0xe41, 0x20);
    ret += clock_write(0, 0xe42, 0x04);
    ret += clock_write(0, 0xe43, 0x0A);
    ret += clock_write(0, 0xe44, 0x40);
    ret += clock_write(0, 0xe45, 0x80);
    ret += clock_write(0, 0xe46, 0xA0);
    ret += clock_write(0, 0xe47, 0xFF);
    ret += clock_write(0, 0xe48, 0x00);
    ret += clock_write(0, 0xe49, 0x00);
    ret += clock_write(0, 0xe4a, 0x00);
    ret += clock_write(0, 0xe4b, 0x00);
    ret += clock_write(0, 0xe4c, 0x00);
    ret += clock_write(0, 0xe4d, 0x00);
    ret += clock_write(0, 0xe4e, 0x00);
    ret += clock_write(0, 0xe4f, 0x00);
    /* step 8 */
    ret += clock_write(0, 0x0a20, 0x2);
    ret += clock_write(0, 0x0a40, 0x2);

    /* step 9 */
    ret += clock_write(0, 0x0005, 0x1);

    /* step 10 */
    ret += clock_write(0, 0x0a22, 0x0);
    ret += clock_write(0, 0x0a42, 0x0);

    /* step 11 */
    ret += clock_write(0, 0x0005, 0x1);

#if 0/* step 12: enable DPLL interrupt */
    ret += clock_write(0, 0x0102, 0x90);
    ret += clock_write(0, 0x010d, 0x04);
    ret += clock_write(0, 0x010e, 0x60);
    ret += clock_write(0, 0x0110, 0x04);
    ret += clock_write(0, 0x0111, 0x60);

    ret += clock_write(0, 0x040b, 0xd0);
    ret += clock_write(0, 0x040c, 0x07);

    ret += clock_write(0, 0x050b, 0xd0);
    ret += clock_write(0, 0x050c, 0x07);
#endif 	
    LCM_LOG_DEBUG(LCM_CARD, "Lcm config clock end, ret val: %d.", ret);
    LCM_LOG_BRINGUP("Lcm config clock end, ret val: %d.", ret);
    
    return ret;
}

static int32
_lcm_init_t65c_8t64x6q_clock(void)
{    
    spi_gen_t  spi_gen[T65C_8T64X6Q_AD9559_NUM];
    clock_info_t clock_info[T65C_8T64X6Q_AD9559_NUM];
    int32 ret = 0;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init clock begin.");
    LCM_LOG_BRINGUP("Lcm init clock begin.");    
    sal_memset(spi_gen, 0, sizeof(spi_gen));
    sal_memset(clock_info, 0, sizeof(clock_info));

    spi_gen[0].spi_type = E_SPI_CPU; /* for E_SPI_GPIO need to insmod /dev/ctc_gpio_spi fisrt */
    spi_gen[0].spi_info.spi_cpu_info.fd = 0;
    spi_gen[0].spi_info.spi_cpu_info.chip_sel = SPI_CPU_CHIP_SEL_0;
    clock_info[0].clock_chip_type = CLOCK_AD9559;
    clock_info[0].clock_io_type = E_CLOCK_SPI;
    clock_info[0].clock_gen = (void *)&spi_gen[0];
    
    /* Init CLOCK */
    ret = clock_init(clock_info, T65C_8T64X6Q_AD9559_NUM);
    if ( ret < 0 )
    {
    	LCM_LOG_ERR("Init clock failed.");
    	return LCM_E_HW_CLOCK;
    }
    
    LCM_IF_ERROR_RETURN(lcm_t65c_8t64x6q_config_clock());

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init clock end.");
    LCM_LOG_BRINGUP("Lcm init clock end.");    
    return LCM_E_SUCCESS;
}

int32 
lcm_init_t65c_8t64x6q_syncE_enable(uint32 port_idx, uint32 status)
{       
    LCM_LOG_BRINGUP("Lcm synce set clock begin.");    

    glb_card_t* p_card = NULL;
    ctc_sync_ether_cfg_t sync_ether_cfg;
    int32 ret = LCM_E_SUCCESS;
    uint16 reg_val = 0;
    uint16 logic_port_idx;   /* logic port of ASIC chip*/

    p_card= lcm_mgt_get_card();   
    sal_memset(&sync_ether_cfg, 0x0, sizeof(sync_ether_cfg));   
    /*In fact,in the board card logic_port_idx is gport.Because in SDK, 
     * the func of synce needs to use lport,so gport needs to be changed to lport.*/
    if(p_card->pp_port[port_idx]->logic_port_idx >= GG_SLICE1_BASE)//slice1
    {
        logic_port_idx = (p_card->pp_port[port_idx]->logic_port_idx - GG_SLICE1_BASE) + 256;
    }
    else//slice 0
    {
        logic_port_idx = p_card->pp_port[port_idx]->logic_port_idx;
    }
    /*enable CTC chip*/
    if (port_idx >= T65C_8T64X6Q_8T)
    {
        //10G mode and 40G
        if ((GLB_SPEED_10G == p_card->pp_port[port_idx]->port_status.speed)
            || (GLB_SPEED_40G == p_card->pp_port[port_idx]->port_status.speed))
        {    
            /*1.disable 1G synce
            * sync-ether 0 clock 1 recovered-port 0 divider 1 output disable link-status-detect disable 
            */
            sync_ether_cfg.clock_output_en = 0;
            sync_ether_cfg.divider = 1;
            sync_ether_cfg.link_status_detect_en = 0;
            sync_ether_cfg.recovered_clock_lport = logic_port_idx;    
            ret += ctc_sync_ether_set_cfg(0, 1, &sync_ether_cfg);  

            /*2. enable 10G synce
            * sync-ether 0 clock 0 recovered-port 0 divider 4 output enable link-status-detect enable
            */
            sync_ether_cfg.clock_output_en = status;
            sync_ether_cfg.divider = 4;
            sync_ether_cfg.link_status_detect_en = TRUE;
            sync_ether_cfg.recovered_clock_lport = logic_port_idx;    
            ret += ctc_sync_ether_set_cfg(0, 0, &sync_ether_cfg);  
            /*set the default value of ICS83054*/
            epld_item_write(0, EPLD_RCV_CLK_SEL_REG, 0x3);

        }
        else//1G mode
        {
            /*1.disable 10G synce
            * sync-ether 0 clock 0 recovered-port 0 divider 4 output disable link-status-detect disable 
            */
            sync_ether_cfg.clock_output_en = 0;
            sync_ether_cfg.divider = 4;
            sync_ether_cfg.link_status_detect_en = 0;
            sync_ether_cfg.recovered_clock_lport = logic_port_idx;    
            ret += ctc_sync_ether_set_cfg(0, 0, &sync_ether_cfg);  

            /*2. enable 1G synce
            * sync-ether 0 clock 1 recovered-port 0 divider 1 output enable link-status-detect enable
            */
            sync_ether_cfg.clock_output_en = status;
            sync_ether_cfg.divider = 1;
            sync_ether_cfg.link_status_detect_en = TRUE;
            sync_ether_cfg.recovered_clock_lport = logic_port_idx;    
            ret += ctc_sync_ether_set_cfg(0, 1, &sync_ether_cfg);

            /*4.PPU0_RECV_CLK2 and  PPU0_RECV_CLK3*/
            //slice 1
            if(p_card->pp_port[port_idx]->ctl_id==1)
            {
                epld_item_write(0, EPLD_RCV_CLK_SEL_REG, 0x2);
            }
            else
            {
                epld_item_write(0, EPLD_RCV_CLK_SEL_REG, 0x1);
            }
        }

    }
    /*8t,phy provide recovery clock*/
    else
    {
        if(status)
        {
            ret = phy_cfg_syncE(port_idx, 1); //PHY of XGSW card
            /*config to 31.25M*/
            gephy_page_reg_read(port_idx , PHY_PAGE_GPIO, 0x17, &reg_val);
            reg_val |= 2<<8;
            gephy_page_reg_write(port_idx, PHY_PAGE_GPIO, 0x17, reg_val);
            /*channel 0*/
            epld_item_write(0, EPLD_RCV_CLK_SEL_REG, 0x0);
        }
        else
        {
            ret = phy_cfg_syncE(port_idx, 0);
            /*set the default value of ICS83054*/
            epld_item_write(0, EPLD_RCV_CLK_SEL_REG, 0x3);
        }
    }

    LCM_LOG_BRINGUP("Lcm synce set clock end.");    

    return ret;
}


/*Add by qianj for T65C,2019-04-28*/
int32
lcm_init_t65c_8t64x6q_daughter_card_name(glb_card_t *p_card)
{
    LCM_LOG_BRINGUP("Lcm init daughter card name begin.");
    char custom_hw_level = 'C';
    char custom_sw_level = 'T';
    char daughter_card_type[4]= "78Y";
    char daughter_card_name[12]= {0};
    char custom_hw_ver[6] = "1.0.0";
    char custom_sw_ver[CUSTOM_SW_VERSION]={0};
    
    char buf[256];
    FILE *fp = NULL;
    uint32 val;
    
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
    if(fp == NULL)
    {
       LCM_LOG_ERR("open global board file failed\n");
       return -1;
    }
    while(sal_fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];
        
        if(!sal_strncmp(buf, GLB_CUSTOM_HW_LEVEL_STRING, 15))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            custom_hw_level = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_CUSTOM_SW_LEVEL_STRING, 15))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            custom_sw_level = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_CUSTOM_HW_VERSION_STRING, 13))
        {
            sal_sscanf(buf, "%s %s", tmp, custom_hw_ver);
        }
        
        else if(!sal_strncmp(buf, GLB_SW_VERSION_STRING, 6))
        {
            sal_sscanf(buf, "%s %s", tmp, custom_sw_ver);
        }
        
    }

    /*The default value of eeprom register is 0xff*/
    if (0 == sal_strcmp(custom_hw_ver, "f.f.f"))
    {
        sal_strcpy(custom_hw_ver, "1.0.0");
    }
    
    /*The default value of eeprom register is 0xff*/
    if(sal_isalpha(custom_hw_level) == 0)
    {
        custom_hw_level = 'C'; 
    }
    if(sal_isalpha(custom_sw_level) == 0)
    {
        custom_sw_level = 'T'; 
    }
    
    sal_sprintf(daughter_card_name, "%c.%s.%c", custom_hw_level,daughter_card_type,custom_sw_level);
    sal_fprintf(fp, "%s %s\n", GLB_CUSTOM_DAUG_CARD_NAME, daughter_card_name);
    if (NULL != fp)
    {
        sal_fclose(fp);
    }

    sal_strncpy(p_card->daughter_card_name,daughter_card_name,sal_strlen(daughter_card_name));
    sal_strncpy(p_card->custom_hw_ver,custom_hw_ver,sal_strlen(custom_hw_ver));
    sal_strncpy(p_card->custom_sw_ver,custom_sw_ver,sal_strlen(custom_sw_ver));

    LCM_LOG_BRINGUP("Lcm init daughter card name end.");

    return LCM_E_SUCCESS;
}


extern int32 lcm_port_init_syce_enable_callback(lcm_port_syce_enable_callback_t func);
extern int32 lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func);
int32
lcm_init_t65c_8t64x6q(glb_card_t* p_card)
{
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init board begin.");
    LCM_LOG_BRINGUP( "Lcm init board begin.");
    glb_card = p_card;
    p_card->asic_chip_num = 1;
    p_card->chip_sensor_num = 1;

    lcm_common_get_slot_no(&p_card->logic_slot_no,&p_card->phy_slot_no);
    p_card->logic_slot_no = 1;
    p_card->phy_slot_no = 0;

    p_card->panel_port_num = T65C_8T64X6Q_PANEL_PORT_NUM;
    
    p_card->l2switch_num = 0;
    p_card->sensor_num = T65C_8T64X6Q_SENSOR_NUM;
    if(p_card->chip_sensor_num+p_card->sensor_num > MAX_TMPR_SENSOR_NUM)
    {
        LCM_LOG_ERR("Sensor structure is too small to store sensor info on board.\n");        
        return LCM_E_NO_MEMORY;
    }
    p_card->fan_module_num = T65C_8T64X6Q_FAN_MODULE_MAX;
    p_card->psu_module_num = T65C_8T64X6Q_PSU_MODULE_MAX;
    p_card->psu_led_mode = GLB_PSU_LED_MODE_MULTI;
    /* Modified by liuht for bug38424 2016-05-11 */
    p_card->volt_curr_mon_num = 0;
    p_card->cpu_type = GLB_CPU_PPC_P1010;
        /*phy*/
    p_card->phy_chip_num = T65C_8T64X6Q_VSC8512_PHY_NUM; //VSC8512
    p_card->phy_int_bit_num = T65C_8T64X6Q_PHY_INT_BITS_SUM;
    p_card->phy_interrupt_mode = GLB_PHY_INTERRUPT_MODE_EPLD;   /*get phy interrupt info from EPLD*/
#ifndef _GLB_UML_SYSTEM_ 
    ctc_phy_fd_init();
#endif
    p_card->phy_intr_en = 1;
    p_card->p_port_range = lcm_t65c_8t64x6q_phy_chip_port_range;
    
    p_card->ptp_en = 1;
    p_card->epld_type = GLB_EPLD_TYPE_VME;
    p_card->bootrom_type = E_BOOTROM_TYPE_512K;
    p_card->board_material = BOARD_MATERIAL_FR4; //all use FR4

    sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                DATAPATH_PROFILE_PATH, T65C_8T64X6Q_DATAPATH_NAME);

    lcm_common_init_cpu_info();
    lcm_common_config_ptp_enable(1);
    lcm_common_config_syncE_enable(1);
    
    lcm_common_config_reboot_record_type(REBOOT_FILE_EPLD,0xf6);

    ctc_hw_fd_init();

    LCM_LOG_DEBUG(LCM_CARD,   "card product type %x, board type %x.", p_card->board_type.series, p_card->board_type.type);
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_epld(p_card, p_card->epld_ver));
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_eeprom_info(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_led());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_i2c());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_eeprom());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_port(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_sensor());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_fan());
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_psu());

    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_IND_LED, 0x0));
    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_reset_device());

#ifndef _GLB_UML_SYSTEM_ 
    ctc_phy_fd_init();
    p_card->dpll_num = 1;
    p_card->dpll_interrupt_mode = GLB_PHY_INTERRUPT_MODE_EPLD;
    ctc_dpll_fd_init(); /* For dpll interrupt */
#endif

    LCM_IF_ERROR_RETURN(_lcm_init_t65c_8t64x6q_clock()); 
    lcm_port_init_syce_enable_callback(lcm_init_t65c_8t64x6q_syncE_enable);
     /*Initially,the local clock enables the dpll to lock and provides interrupt,but 
     *  the interrupt in the dpll is masked,the interrupt is processed and remain low,
     * so need to write AD9559_IRQ_CLEARING register to clean up dpll interrupt and 
     * release the dpll mask register of epld
     */
    clock_write( 0, AD9559_IRQ_CLEARING, 0x01);
    clock_write( 0, AD9559_SOFT_IO_UPDATE, 0x01);
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_DPLL_INT_MASK, 0x0));

    lcm_card_init_callback(lcm_init_t65c_8t64x6q_cb);
    lcm_port_init_ffe_cfg_callback(lcm_t65c_8t64x6q_get_sum_ffe_cfg);

    lcm_init_t65c_8t64x6q_daughter_card_name(p_card);
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init board end.");
    LCM_LOG_BRINGUP("Lcm init board end.");

    return LCM_E_SUCCESS;
}

