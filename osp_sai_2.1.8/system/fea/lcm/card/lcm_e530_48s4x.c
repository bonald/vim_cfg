/****************************************************************************
* $Id$
*  E530-48S4X board init functions
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : qianj
* Date          : 2019-7-31
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
#include "sysenv_api.h"
#include "sysenv_spiflash.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
 
/*
 * CPU I2C slave device
 */

/* Sysenv */
#define E530_48S4X_SYSENV_NUM                      0x01

/* Power supply */
#define E530_48S4X_PSU_MODULE_MAX                  0x2

/* Fan */
#define E530_48S4X_FAN_MODULE_MAX                  0x1

/* I2C to GPIO: PCA9505 */
#define E530_48S4X_GPIO_CHIP_NUM                   0x02
#define E530_48S4X_ADDR0_GPIO                      0x21
#define E530_48S4X_ADDR1_GPIO                      0x22

#define E530_48S4X_EPLD_I2C_ADDR                 0x36

#define E530_48S4X_PANEL_PORT_NUM_MAX     (48+4)
#define E530_48S4X_PANEL_PORT_NUM         (48+4)
#define E530_48S4X_FIBER_PORT_NUM         (48+4)

#define E530_48S4X_SLICE0_LED_MAC_NUM       20 
#define E530_48S4X_LED_MAC_NUM              20

#define E530_48S4X_PHY_INT_BITS_SUM         1

#define GLB_I2C_IDX GLB_I2C_IDX_0

/****************************************************************************
*
* Global and Declarations
*
****************************************************************************/
static const glb_card_t         *glb_card;
static lcm_card_port_t          *glb_lcm_e530_48s4x_port             = NULL;
static fiber_port_info_t        *glb_lcm_e530_48s4x_fiber            = NULL;

extern int32 lcm_card_init_callback(card_init_callback_t func);
extern int32
lcm_common_get_defaultfec_from_speedability(uint16 speed_ability, uint32*  fec_ability);

lcm_card_port_t lcm_e530_48s4x_port_max[E530_48S4X_PANEL_PORT_NUM_MAX] =
{       
    {0, 0, 0, 0},    /* Port 1 */
    {0, 1, 1, 1},    /* Port 2 */
    {0, 2, 2, 2},    /* Port 3 */
    {0, 3, 3, 3},    /* Port 4 */
    {0, 4, 4, 4},    /* Port 5 */
    {0, 5, 5, 5},    /* Port 6 */
    {0, 6, 6, 6},    /* Port 7 */
    {0, 7, 7, 7},    /* Port 8 */

    {0, 16, 16, 0},    /* Port 9 */
    {0, 17, 17, 1},    /* Port 10 */
    {0, 18, 18, 2},    /* Port 11 */
    {0, 19, 19, 3},    /* Port 12 */
    {0, 20, 20, 4},    /* Port 13 */
    {0, 21, 21, 5},    /* Port 14 */
    {0, 22, 22, 6},    /* Port 15 */
    {0, 23, 23, 7},    /* Port 16 */

    {0, 8, 8, 0},    /* Port 17 */
    {0, 9, 9, 1},    /* Port 18 */
    {0, 10, 10, 2},    /* Port 19 */
    {0, 11, 11, 3},    /* Port 20 */
    {0, 32, 32, 4},    /* Port 21 */
    {0, 33, 33, 5},    /* Port 22 */
    {0, 34, 34, 6},    /* Port 23 */
    {0, 35, 35, 7},    /* Port 24 */

    {0, 36, 36, 0},    /* Port 25 */
    {0, 37, 37, 1},    /* Port 26 */
    {0, 38, 38, 2},    /* Port 27 */
    {0, 39, 39, 3},    /* Port 28 */
    {0, 40, 40, 4},    /* Port 29 */
    {0, 41, 41, 5},    /* Port 30 */
    {0, 42, 42, 6},    /* Port 31 */
    {0, 43, 43, 7},    /* Port 32 */

    {0, 25, 25},    /* Port 33 */
    {0, 24, 24},    /* Port 34 */
    {0, 27, 27},    /* Port 35 */
    {0, 26, 26},    /* Port 36 */
    {0, 13, 13},    /* Port 37 */
    {0, 12, 12},    /* Port 38 */
    {0, 15, 15},    /* Port 39 */
    {0, 14, 14},    /* Port 40 */

    {0, 29, 29},    /* Port 41 */
    {0, 28, 28},    /* Port 42 */
    {0, 31, 31},    /* Port 43 */
    {0, 30, 30},    /* Port 44 */
    {0, 61, 61},    /* Port 45 */
    {0, 60, 60},    /* Port 46 */
    {0, 63, 63},    /* Port 47 */
    {0, 62, 62},    /* Port 48 */

    {0, 44, 44},    /* Port 49 */
    {0, 45, 45},    /* Port 50 */
    {0, 47, 47},    /* Port 51 */
    {0, 46, 46},    /* Port 52 */

};

glb_port_range_t lcm_e530_48s4x_phy_chip_port_range[E530_48S4X_PHY_INT_BITS_SUM] =
{
    { 1, 32},     
};

fiber_port_info_t lcm_e530_48s4x_fiber_max[E530_48S4X_FIBER_PORT_NUM] =
{      
    /* panel port 1~8 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,3}},{.en_epld={0,0xe,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,2}},{.en_epld={0,0xe,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,1}},{.en_epld={0,0xe,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,0}},{.en_epld={0,0xe,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,4}},{.en_epld={0,0xe,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,5}},{.en_epld={0,0xe,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,6}},{.en_epld={0,0xe,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,7}},{.en_epld={0,0xe,7}}},

    /* panel port 9~16 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,15}},{.en_epld={0,0xf,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,14}},{.en_epld={0,0xf,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,13}},{.en_epld={0,0xf,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,12}},{.en_epld={0,0xf,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,8}},{.en_epld={0,0xf,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,9}},{.en_epld={0,0xf,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,10}},{.en_epld={0,0xf,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,11}},{.en_epld={0,0xf,7}}},

    /* panel port 17~24 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,23}},{.en_epld={0,0x10,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,22}},{.en_epld={0,0x10,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,21}},{.en_epld={0,0x10,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,20}},{.en_epld={0,0x10,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,16}},{.en_epld={0,0x10,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,17}},{.en_epld={0,0x10,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,18}},{.en_epld={0,0x10,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,0,{.asic={0,19}},{.en_epld={0,0x10,7}}},

     /* panel port 25~32 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,2}},{.en_epld={0,0x11,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,3}},{.en_epld={0,0x11,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,7}},{.en_epld={0,0x11,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,6}},{.en_epld={0,0x11,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,5}},{.en_epld={0,0x11,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,4}},{.en_epld={0,0x11,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,0}},{.en_epld={0,0x11,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,1}},{.en_epld={0,0x11,7}}},

    /* panel port 33~40 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,8}},{.en_epld={0,0x12,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,9}},{.en_epld={0,0x12,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,10}},{.en_epld={0,0x12,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,11}},{.en_epld={0,0x12,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,12}},{.en_epld={0,0x12,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,13}},{.en_epld={0,0x12,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,14}},{.en_epld={0,0x12,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,15}},{.en_epld={0,0x12,7}}},

    /* panel port 41~48 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,16}},{.en_epld={0,0x13,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,17}},{.en_epld={0,0x13,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,18}},{.en_epld={0,0x13,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,19}},{.en_epld={0,0x13,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,20}},{.en_epld={0,0x13,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,21}},{.en_epld={0,0x13,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,22}},{.en_epld={0,0x13,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,23}},{.en_epld={0,0x13,7}}},
    
    /* panel port 49-52 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,24}},{.en_epld={0,0x14,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,25}},{.en_epld={0,0x14,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,26}},{.en_epld={0,0x14,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_EPLD,1,{.asic={0,27}},{.en_epld={0,0x14,3}}},   
};

/* TM mac led is reverse to port *////TODO:
mac_led_api_para_t lcm_e530_48s4x_mac_led_default_entry[E530_48S4X_LED_MAC_NUM]=
{
    /* panel port 33-36 */
    {25, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {24, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {27, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {26, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    /* panel port 37-40 */
    {13, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {12, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {15, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {14, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    /* panel port 41-44 */
    {29, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {28, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {31, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {30, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    /* panel port 45-48 */
    {61, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {60, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {63, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {62, 0, 0, LED_MODE_2_FORCE_OFF, 0},        

    /* panel port 49-52 */
    {44, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {45, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {47, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {46, 0, 0, LED_MODE_2_FORCE_OFF, 0},
};

/* typical ffe */
lcm_card_serdes_ffe_t 
lcm_e530_48s4x_serdes_ffe_max[E530_48S4X_PANEL_PORT_NUM_MAX]=
{
    {0, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 1 */
    {0, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 2 */
    {0, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 3 */
    {0, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 4 */
    {1, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 5 */
    {1, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 6 */
    {1, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 7 */
    {1, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 8 */

    {2, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 9 */
    {2, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 10 */
    {2, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 11 */
    {2, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 12 */
    {3, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 13 */
    {3, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 14 */
    {3, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 15 */
    {3, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 16 */

    {4, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 17 */
    {4, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 18 */
    {4, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 19 */
    {4, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 20 */
    {5, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 21 */
    {5, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 22 */
    {5, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 23 */
    {5, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 24 */

    {6, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 25 */
    {6, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 26 */
    {6, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 27 */
    {6, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 28 */
    {7, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 29 */
    {7, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 30 */
    {7, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 31 */
    {7, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 32 */

    {9, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 33 */
    {8, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 34 */
    {11, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 35 */
    {10, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 36 */
    {13, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 37 */
    {12, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 38 */
    {15, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 39 */
    {14, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 40 */

    {17, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 41 */
    {16, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 42 */
    {19, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 43 */
    {18, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 44 */
    {21, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 45 */
    {20, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 46 */
    {23, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 47 */
    {22, 0xff, 0xff,COEFF1(2,70,7,0)},    /* Port 48 */
    /*4~7inch*/
    {26, 0xff, 0xff,COEFF1(0,15,63,0)},    /* Port 49 */
    {25, 0xff, 0xff,COEFF1(0,15,63,0)},    /* Port 50 */
    {27, 0xff, 0xff,COEFF1(0,15,63,0)},    /* Port 51 */
    {24, 0xff, 0xff,COEFF1(0,15,63,0)},    /* Port 52 */

};

/****************************************************************************
 *
* Function
*
****************************************************************************/
static int32
lcm_init_e530_48s4x_init_port_table(glb_card_t* p_card)
{
    uint8 port_id, panel_port_no;
    uint8 pos = 0;

    for(port_id=0; port_id<p_card->port_num; port_id++)
    {
        panel_port_no = p_card->pp_port[port_id]->panel_port_no;

        pos = panel_port_no - 1;

        sal_memcpy(&glb_lcm_e530_48s4x_port[port_id], &lcm_e530_48s4x_port_max[pos], sizeof(lcm_card_port_t));
        glb_lcm_e530_48s4x_port[port_id].serdes_id = lcm_e530_48s4x_serdes_ffe_max[pos].serdes_id;
    }

    return LCM_E_SUCCESS;
}

static int32
lcm_init_e530_48s4x_init_fiber_table(glb_card_t* p_card)
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
        sal_memcpy(&glb_lcm_e530_48s4x_fiber[port_id], &lcm_e530_48s4x_fiber_max[panel_port_no-1], sizeof(fiber_port_info_t));
        glb_lcm_e530_48s4x_fiber[port_id].fiber_channel = fiber_channel;
        glb_lcm_e530_48s4x_fiber[port_id].fiber_id = port_id;
    }
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_panel_port(glb_card_t* p_card)
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
        if (ppt_id < 48)
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_1G|GLB_SUPPORT_SPEED_100M;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_1G;
        } 
        else  /* 4X */
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_10G;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_10G;
        }
    }
    LCM_LOG_BRINGUP("Lcm init panel port end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_port(glb_card_t* p_card)
{
    int32 port_id = 0;
    uint32 ppt_idx;
    uint16 logic_port_idx;
    lcm_card_port_panel_mapping_t port_panel_mapping[256];

    LCM_LOG_BRINGUP("Lcm init port begin.");
     /* 1, init panel port */
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_panel_port(p_card));
     
    /* 2, read '/mnt/flash/startup-config.conf' file, get port split/merge info */
    LCM_IF_ERROR_RETURN(lcm_common_parse_port_numchg_info(p_card));

    p_card->split_flag = 0;
    
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
        p_card->pp_port[port_id]->port_numchg_type = port_panel_mapping[port_id].port_numchg_type;
     }
 
    /* 5, allocate memory for port/fiber/serdes data structure */
    glb_lcm_e530_48s4x_port = (lcm_card_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(lcm_card_port_t)* p_card->port_num);
    if (!glb_lcm_e530_48s4x_port)
    {
        LCM_LOG_ERR("Allocate pointer to lcm_card_port_t fail.");
        return LCM_E_NO_MEMORY;
    }
 
    glb_lcm_e530_48s4x_fiber = (fiber_port_info_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(fiber_port_info_t)*p_card->port_num);
    if (!glb_lcm_e530_48s4x_fiber)
    {
        LCM_LOG_ERR("Allocate pointer to fiber_port_info_t fail.");
        return LCM_E_NO_MEMORY;
    }

    /* 6, init current running port/fiber info */
    LCM_IF_ERROR_RETURN(lcm_init_e530_48s4x_init_port_table(p_card));
    LCM_IF_ERROR_RETURN(lcm_init_e530_48s4x_init_fiber_table(p_card));
 
    /* 7, init port properties */
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        logic_port_idx = glb_lcm_e530_48s4x_port[port_id].logic_port_idx;
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
        p_card->pp_port[port_id]->lp_support = 0;  /* added by tongzb TM do not support loopback */
        p_card->pp_port[port_id]->eee_support = 0;
        p_card->pp_port[port_id]->logic_port_idx = logic_port_idx;
        p_card->pp_port[port_id]->local_chip_idx = glb_lcm_e530_48s4x_port[port_id].chip_idx;
        p_card->pp_port[port_id]->mac_idx = glb_lcm_e530_48s4x_port[port_id].mac_idx;
        /* for bug 48433, do phy mapping */
        p_card->pp_port[port_id]->phy_addr = glb_lcm_e530_48s4x_port[port_id].phy_addr;
        p_card->pp_port[port_id]->chip_serdes_id = glb_lcm_e530_48s4x_port[port_id].serdes_id;
        p_card->pp_port[port_id]->g_port_index = (p_card->pp_port[port_id]->glb_chip_idx << 8) | logic_port_idx;

        /* for bug 48433 */
        p_card->pp_port[port_id]->mdio_bus_addr = 0xff;/* default */
        if (port_id < 8)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS1;
        }
        else if(port_id < 16)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS2;
        }
        else if(port_id < 24)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS3;
        }
        else if(port_id < 32)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS0;
        }        

        if (ppt_idx <= 32)
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_PHY_TWO_LED;
            p_card->pp_port[port_id]->unidir_support = 0;
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
        }
        else if(ppt_idx <= 48)
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED_LOWSPEED;
            p_card->pp_port[port_id]->unidir_support = 1;
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
        }  
        else
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED;
            p_card->pp_port[port_id]->unidir_support = 1;
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
        }        

        if(ppt_idx > 32 && ppt_idx <= 48)
        {
            p_card->pp_port[port_id]->port_led_reverse = 0;
        }
        else
        {
            p_card->pp_port[port_id]->port_led_reverse = 1;
        }
        
        p_card->pp_port[port_id]->ctl_id = 0;/* DUET2 only have one ctl */
        p_card->pp_port[port_id]->port_led_mac = glb_lcm_e530_48s4x_port[port_id].mac_idx;
        /*useless code*/
        p_card->pp_port[port_id]->mac_ctl_id = 0;/* Tsingma only have one mac led slice */
     }

    /* 8, create file '/tmp/ctcos_port_info' */
    LCM_IF_ERROR_RETURN(lcm_common_save_port_info_file(p_card));
     
    LCM_LOG_BRINGUP("Lcm init port end.");    

    return LCM_E_SUCCESS;
}

static int32_t
_lcm_reg_e530_48s4x_epld(uint8 hw_ver, epld_info_t * p_epld_info)
{
    p_epld_info->epld_bus_type = EPLD_I2C_TYPE;
    p_epld_info->i2c_gen.bridge_flag = 0;
    p_epld_info->i2c_gen.addr = E530_48S4X_EPLD_I2C_ADDR;
    p_epld_info->i2c_gen.i2c_type = E_I2C_CPM;
    p_epld_info->i2c_gen.i2c_bus_idx = GLB_I2C_IDX;
    p_epld_info->i2c_gen.alen = EPLD_DATA_LENTH;

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_EPLD_VERSION]),          0x1, 0, 7, 8);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SYS_LED_SYS]),           0x2, 4, 7, 4);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_IND_LED]),               0x3, 0, 0, 1);
        
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_PRESENT_STATUS]),    0x1e, 2, 3, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_WORK_STATUS]),       0x1e, 4, 5, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_ALERT_STATUS]),      0x1e, 0, 1, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_TYPE]),              0x1e, 6, 7, 2);
    
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_ASIC_I2C_BRIDGE_RST]),   0x05, 0, 6, 7);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PHY_RST]),               0x07, 0, 7, 8);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PHY_INT_MASK1]),         0x0b, 0, 7, 8);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE1]),          0xe, 0, 7, 8);//info in fiber table
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE2]),          0xf, 0, 7, 8);//info in fiber table
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE3]),          0x10, 0, 7, 8);//info in fiber table
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE4]),          0x11, 0, 7, 8);//info in fiber table
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE5]),          0x12, 0, 7, 8);//info in fiber table
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE6]),          0x13, 0, 7, 8);//info in fiber table
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SFP_DISABLE7]),          0x14, 0, 7, 8);//info in fiber table

    //epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PPU_INT_MASK]),          0x0f, 0, 3, 4);
    /* default location */
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_REBOOT_POWER_RST]),      0xf6, 0, 7, 8);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_POWER_DOWN]),            0x23, 0, 1, 2);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_TEST]),                  0x7f, 0, 7, 8);

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_epld(glb_card_t* p_card, uint8 epld_version)
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

    _lcm_reg_e530_48s4x_epld(p_card->hw_ver, p_epld_info);
    p_epld_info->diag_level = p_card->bootup_diag_level;
    p_epld_info->req_diag_level = E_DIAG_LEVEL_MINIMAL;
    ret = epld_init(p_epld_info);
    if (0 != ret)
    {
        LCM_LOG_ERR("EPLD Init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init epld end.");
    LCM_LOG_BRINGUP("Lcm init epld end.");    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_sysenv_info(glb_card_t* p_card)
{
    LCM_LOG_BRINGUP("Lcm init sysenv info.");

    p_card->p_sysenv_alloc = (sysenv_info_t *)XCALLOC(MTYPE_BUFFER_DATA, sizeof(sysenv_info_t)*SYSENV_MAX);
    if(NULL == p_card->p_sysenv_alloc)
    {
        LCM_LOG_ERR("alloc p_sysenv_alloc array fail.");
        return LCM_E_INVALID_PTR;
    }

    p_card->p_sysenv_alloc[SYSENV_TCAM_PROFILE].sysenv_idx = 0;
    p_card->p_sysenv_alloc[SYSENV_TCAM_PROFILE].base_addr = 0x1260;

#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
    p_card->p_sysenv_alloc[SYSENV_SMARTCFG].sysenv_idx = 0;
    p_card->p_sysenv_alloc[SYSENV_SMARTCFG].base_addr = 0x1265;
#endif /* !HAVE_SMARTCFG */

    p_card->p_sysenv_alloc[SYSENV_OEM_INFO].sysenv_idx = 0;
    p_card->p_sysenv_alloc[SYSENV_OEM_INFO].base_addr = 0x1300;

    p_card->p_sysenv_alloc[SYSENV_PORT_INFO].sysenv_idx = 0;
    p_card->p_sysenv_alloc[SYSENV_PORT_INFO].base_addr = 0x1400;

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_sysenv(glb_card_t* p_card)
{
    int32 ret = 0;
    sysenv_chip_info_t sysenv_chip_info[E530_48S4X_SYSENV_NUM];
    sysenv_spiflash_info_t spiflash_info[E530_48S4X_SYSENV_NUM];

    LCM_LOG_BRINGUP("Lcm init sysenv begin.");
    sal_memset(sysenv_chip_info, 0, sizeof(sysenv_chip_info));
    sal_memset(spiflash_info, 0, sizeof(spiflash_info));
    /* use sysenv api, sysenv_info should init */
    p_card->use_sysenv_api = 1;

    sysenv_chip_info[0].sysenv_chip_type = SYSENV_CHIP_SPIFLASH;
    spiflash_info[0].base_offset = 0x000000;
    spiflash_info[0].block_size = 0x01000;
    spiflash_info[0].block_num = 2;
    spiflash_info[0].dev_fd = -1;
    sal_strncpy(spiflash_info[0].dev_name, "/dev/mtd2", 64);

    sysenv_chip_info[0].sysenv_gen = (void *)(&spiflash_info[0]);

    ret = sysenv_init(sysenv_chip_info, E530_48S4X_SYSENV_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm sysenv init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_BRINGUP("Lcm init sysenv end.");
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_e530_48s4x_fiber(void)
{
    int32 ret = 0;
    
    LCM_LOG_BRINGUP( "Lcm init fiber begin.");
    ret = fiber_init(glb_card->port_num,  /* fiber num */
                     glb_card->port_num, /* port num */
                     glb_lcm_e530_48s4x_fiber);

    LCM_LOG_BRINGUP("Lcm init fiber end.");

    return ret;
}

static int32
lcm_init_e530_48s4x_gpio(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[E530_48S4X_GPIO_CHIP_NUM];
    gpio_chip_t gpio_chip[E530_48S4X_GPIO_CHIP_NUM];
    void *p_data[E530_48S4X_GPIO_CHIP_NUM];

    LCM_LOG_BRINGUP("Lcm init i2c to gpio module begin.")
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(gpio_chip, 0, sizeof(gpio_chip));

    i2c_gen[0].addr = E530_48S4X_ADDR0_GPIO;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 0;   
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[0].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[0].io_type = GPIO_I2C;    
    gpio_chip[0].chip_type = GPIO_PCA9505;
    gpio_chip[0].scan_group_bitmap = 0x00; /*group 1*/

    i2c_gen[1].addr = E530_48S4X_ADDR1_GPIO;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].bridge_flag = 0;   
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[1].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[1].io_type = GPIO_I2C;    
    gpio_chip[1].chip_type = GPIO_PCA9505;
    gpio_chip[1].scan_group_bitmap = 0x00; /*group 1*/

    for(i=0; i<E530_48S4X_GPIO_CHIP_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = gpio_init(p_data, gpio_chip, E530_48S4X_GPIO_CHIP_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Gpio device init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_BRINGUP("Lcm init i2c to gpio module end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_fan(glb_card_t* p_card)
{
    int32 ret = 0;
    uint8 fan_module_idx;
    fan_chip_t fan_chip[E530_48S4X_FAN_MODULE_MAX];
    void *p_data[E530_48S4X_FAN_MODULE_MAX];

    LCM_LOG_BRINGUP("Lcm init fan module begin.");    
    sal_memset(fan_chip, 0, sizeof(fan_chip));

    fan_chip[0].chip_type = FAN_CTC5236;
    fan_chip[0].io_type = E_FAN_CTC5236;
    fan_chip[0].support_hot_swap = 0;
    p_data[0] = NULL;
    fan_chip[0].fan_num = 3;
    fan_chip[0].speed_adjust = 1;
    fan_chip[0].rpm = 9000; //need change to 10000rap
    fan_chip[0].pwm_map_table = NULL;
    fan_chip[0].pwm.bootom_rate = DEFAULT_FAN_RATE_BOTTOM; 
    fan_chip[0].pwm.low_rate = DEFAULT_FAN_RATE_LOW;
    fan_chip[0].pwm.high_rate = DEFAULT_FAN_RATE_HIGH;
    fan_chip[0].pwm.full_rate = DEFAULT_FAN_RATE_FULL;

    /*if don't use default channel order*/    
    fan_chip[0].pwm_channel_flag = CUSTOM_FAN_PWM_CHANNAL;    
    fan_chip[0].pwm_channel_idx[0] = 1;     
    fan_chip[0].pwm_channel_idx[1] = 0;     
    fan_chip[0].pwm_channel_idx[2] = 2;     
    fan_chip[0].pwm_channel_idx[3] = 3;
            
    ret = fan_init(p_data, fan_chip, E530_48S4X_FAN_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Fan driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(fan_module_idx=0; fan_module_idx < E530_48S4X_FAN_MODULE_MAX; fan_module_idx++)
    {
        fan_dev_init(fan_module_idx);
    }
    if (p_card->bootup_diag_level >= E_DIAG_LEVEL_COMPLETE)
    {
        ret = fan_diagnostic_test_and_report(p_card->fan_module_num);
        if (ret != 0)
        {
            LCM_LOG_ERR("Fan driver diag fail.");
            //return LCM_E_INIT_FAILED;
        }
    }
    LCM_LOG_BRINGUP("Lcm init fan module end.");    

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_psu(glb_card_t* p_card)
{
    int32 ret;
    uint8 epld_idx[E530_48S4X_PSU_MODULE_MAX];
    psu_chip_t psu_chip[E530_48S4X_PSU_MODULE_MAX];
    psu_private_t p_data[E530_48S4X_PSU_MODULE_MAX];

    LCM_LOG_BRINGUP("Lcm init psu module begin.");    

    sal_memset(p_data, 0, sizeof(p_data));
    sal_memset(psu_chip, 0, sizeof(psu_chip));

    epld_idx[0] = 0;
    psu_chip[0].chip_type = PSU_EPLD;
    psu_chip[0].io_type = PSU_IO_EPLD;
    psu_chip[0].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[0].support_pmbus = 0;
    psu_chip[0].fixed_status = 1;
    p_data[0].p_data_epld = &epld_idx[0];

    epld_idx[1] = 0;
    psu_chip[1].chip_type = PSU_EPLD;
    psu_chip[1].io_type = PSU_IO_EPLD;
    psu_chip[1].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[1].support_pmbus = 0;
    psu_chip[1].fixed_status = 1;
    p_data[1].p_data_epld = &epld_idx[1];

    ret = psu_init(p_data, psu_chip, E530_48S4X_PSU_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Psu driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    if (p_card->bootup_diag_level >= E_DIAG_LEVEL_COMPLETE)
    {
        ret = psu_diagnostic_test_and_report(p_card->psu_module_num);
        if (ret != 0)
        {
            LCM_LOG_ERR("Psu driver diag fail.");
            //return LCM_E_INIT_FAILED;
        }
    }
    LCM_LOG_BRINGUP("Lcm int psu end.");    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_phy(void)
{
    uint16 port_id;
    phy_info_t phyinfo;
    phy_handle_t** pphdl = NULL;
    glb_port_t* p_port;
    int32 ret;

    LCM_LOG_BRINGUP("Lcm init phy begin.");    

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
        
        if(port_id < 32) /* 32 with PHYs */
        {
            phyinfo.phy_device_type = PORT_PHY_MV88E1548;
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_1000BX;
            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_QSGMII;
            phyinfo.vct_support = 1;
            phyinfo.phy_addr = p_port->phy_addr;
            phyinfo.mdio_bus = ASIC_TM_PHY_1G_TYPE;
            phyinfo.base_addr = p_port->mdio_bus_addr;            
            lcm_common_get_defaultspeed_from_speedability(p_port->port_speed_ability, 
                    &(phyinfo.phy_manage_info.speed), &(phyinfo.phy_manage_info.duplex));

            phyinfo.phy_manage_info.phy_init_seq_flag = GLB_NO_INIT_FLAG;
            phyinfo.led_type = GLB_PHY_LED_8;
            phyinfo.led_freq = GLB_PHY_LED_FREQ_DEFAULT;
            phyinfo.port_led_ctrl = 0x03;
        }
        else  /* 4X without PHYs */
        {
            phyinfo.phy_device_type = PORT_PHY_NULL;

            phyinfo.phy_manage_info.media_type = GLB_MEDIA_SFP_PLUS;
            phyinfo.phy_manage_info.fec_en = GLB_FEC_DEFAULT|GLB_FEC_OFF;

            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_SERDES;
            lcm_common_get_defaultspeed_from_speedability(p_port->port_speed_ability, 
                    &(phyinfo.phy_manage_info.speed), &(phyinfo.phy_manage_info.duplex));
                    
            phyinfo.port_info.serdes_num = 1;  /* bug44010, support serdes loopback, qicx, 2017-05-15 */
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
        phyinfo.port_info.lchip = p_port->local_chip_idx;
        phyinfo.port_info.serdes_id = p_port->chip_serdes_id;
        
        /* Modified by liuht for access port id in phy handle for bug 25808 */
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
        lcm_common_get_defaultfec_from_speedability(p_port->port_speed_ability,&(phyinfo.phy_manage_info.fec_ability));

        phyinfo.phy_stat_flag.duplex = phyinfo.phy_manage_info.duplex;
        phyinfo.phy_stat_flag.speed = phyinfo.phy_manage_info.speed;
        phyinfo.phy_stat_flag.link_up = GLB_LINK_DOWN;
        phyinfo.phy_stat_flag.flowctrl.send = GLB_FLOWCTRL_DISABLE;
        phyinfo.phy_stat_flag.flowctrl.recv = GLB_FLOWCTRL_DISABLE;
        phyinfo.phy_stat_flag.fec_status = GLB_FEC_OFF;

        pphdl[port_id] = phy_dev_register(&phyinfo);
        if(NULL ==  pphdl[port_id])
        {
            LCM_LOG_ERR("Register phy handle failed\n");
            return LCM_E_INIT_FAILED;
        }
    }

    LCM_LOG_BRINGUP("Lcm ready to init phy.");
    ret = phy_init(pphdl, (glb_card->port_num));
    if(ret)
    {
        return LCM_E_INIT_FAILED;
    }
    if (glb_card->bootup_diag_level >= E_DIAG_LEVEL_MINIMAL)
    {
        phy_diagnostic_test_and_report();
        ((glb_card_t *)glb_card)->do_phy_bootup_diag = 1;
    }

    LCM_LOG_BRINGUP("Lcm init phy end.");    
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_e530_48s4x_i2c(void)
{
    int32 ret;

    sal_task_sleep(100);
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
_lcm_init_e530_48s4x_led(void)
{
    int32 ret = 0;
    led_info_t led_info[E_MAX_LED];

    LCM_LOG_BRINGUP("Lcm init LED begin.");
    sal_memset(led_info, 0, sizeof(led_info_t)*E_MAX_LED);

    /* init sys led */
    led_info[E_SYS_LED].exist = 1;
    led_info[E_SYS_LED].led_ctl = E_LED_EPLD0;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].op_reg = EPLD_SYS_LED_SYS;//for bug 53242, by tongzb, fix system led, 2019-08-22
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].val = 0x5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].val = 0xb;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].val = 0x5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].op_reg = EPLD_SYS_LED_SYS;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].val = 0xb;
    
    /* End of Merge */
    led_info[E_IND_LED].exist = 1;
    led_info[E_IND_LED].led_ctl = E_LED_EPLD0;
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
    LCM_LOG_BRINGUP( "Lcm init LED end.");    

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48s4x_ctc_chip_led(void)
{
    uint8 table_id = 0;
    uint8 table_num = 1;
    mac_led_info_t mac_led_info;

    LCM_LOG_INFO("Lcm init MAC LED begin.");
 
    mac_led_info.mac_led_api_para = (mac_led_api_para_t **)XCALLOC(CTCLIB_MEM_LCM_MODULE, 
        sizeof(mac_led_api_para_t*)*table_num);
    if(NULL == mac_led_info.mac_led_api_para)
    {
        LCM_LOG_ERR("Alloc mac_led_api_para_t array fail.");
        return LCM_E_NO_MEMORY;
    }
    
    for(table_id = 0; table_id < table_num; table_id++)
    {
        mac_led_info.mac_led_api_para[table_id] = lcm_e530_48s4x_mac_led_default_entry;
    }

    mac_led_info.table_num = table_num;
    mac_led_info.mac_num = E530_48S4X_LED_MAC_NUM;
    mac_led_info.slice0_mac_num = E530_48S4X_SLICE0_LED_MAC_NUM;
    mac_led_info.polarity = 1; /* 1:for low driver led, 0:for high driver led */
    mac_led_info_register(&mac_led_info);
    mac_led_init();


    XFREE(CTCLIB_MEM_LCM_MODULE, mac_led_info.mac_led_api_para);

    LCM_LOG_BRINGUP("Lcm init MAC LED end.");        
    return LCM_E_SUCCESS;
}

int32
lcm_e530_48s4x_get_sum_ffe_cfg(lcm_chip_serdes_ffe_t* serdes_ffe)
{
    uint8 serdes_id;
    uint8 index;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;
    uint8 default_trace_len;

    serdes_id = serdes_ffe->serdes_id;
    serdes_ffe->board_material = glb_card->board_material;

    for(index = 0; index < E530_48S4X_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_48s4x_serdes_ffe_max[index];
        if(serdes_id == lcm_serdes_ffe->serdes_id)
        {
            default_trace_len = lcm_serdes_ffe->trace_len;

            if (0xff == default_trace_len) 
            {
                serdes_ffe->mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;  /*user-defined*/
                serdes_ffe->c0 = lcm_serdes_ffe->coeff1.c0;
                serdes_ffe->c1 = lcm_serdes_ffe->coeff1.c1;
                serdes_ffe->c2 = lcm_serdes_ffe->coeff1.c2;
                serdes_ffe->c3 = lcm_serdes_ffe->coeff1.c3;
            }
            else
            {
                serdes_ffe->mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;  /*typical*/
                serdes_ffe->trace_len = default_trace_len;
            } 
            return 0;
        }
    }    
    LCM_LOG_ERR("Lcm get sum ffe cfg error, doesn't find ffe param.");

    return -1;
}

int32
_lcm_init_e530_48s4x_serdes_ffe(void)
{
    ctc_chip_serdes_ffe_t serdes_ffe;
    uint8 index;
    int32 ret = 0;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;        
    uint8 default_trace_len;
    
    LCM_LOG_BRINGUP("Lcm init serdes ffe begin.");    

    sal_memset(&serdes_ffe, 0, sizeof(ctc_chip_serdes_ffe_t));
    for(index = 0; index < E530_48S4X_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_48s4x_serdes_ffe_max[index];

        default_trace_len = lcm_serdes_ffe->trace_len;

        if (0xff == default_trace_len) 
        {
            serdes_ffe.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
            serdes_ffe.serdes_id = lcm_serdes_ffe->serdes_id;
            serdes_ffe.coefficient[0] = lcm_serdes_ffe->coeff1.c0;
            serdes_ffe.coefficient[1] = lcm_serdes_ffe->coeff1.c1;
            serdes_ffe.coefficient[2] = lcm_serdes_ffe->coeff1.c2;
            serdes_ffe.coefficient[3] = lcm_serdes_ffe->coeff1.c3;
        }
        else
        {
            serdes_ffe.mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;
            serdes_ffe.board_material = glb_card->board_material;
            serdes_ffe.serdes_id = lcm_serdes_ffe->serdes_id;
            serdes_ffe.trace_len = default_trace_len;
        }
        ret = ctc_chip_set_property(0, CTC_CHIP_PROP_SERDES_FFE, (void *)&serdes_ffe);
        if (ret < 0)
        {
            LCM_LOG_ERR("LCM set serdes ffe parameter failed.\n");
            return LCM_E_INIT_FAILED;
        }         
    }

    LCM_LOG_BRINGUP("Lcm init serdes ffe end. ret %d",ret);    
    
    return ret;
}

int32
lcm_init_e530_48s4x_cb(void)
{
    LCM_LOG_BRINGUP("Lcm init callback begin.");
    
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_fiber());    
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_phy());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_ctc_chip_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_serdes_ffe());

    LCM_LOG_BRINGUP( "Lcm init callback end.");    

    return LCM_E_SUCCESS;
}

extern int32 lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func);
int32
lcm_init_e530_48s4x(glb_card_t* p_card)
{
    LCM_LOG_BRINGUP( "Lcm init board begin.");    
    glb_card = p_card;
    p_card->asic_chip_num = 1;
    p_card->chip_sensor_num = 1;

    lcm_common_get_slot_no(&p_card->logic_slot_no,&p_card->phy_slot_no);

    p_card->panel_port_num = E530_48S4X_PANEL_PORT_NUM;
    p_card->l2switch_num = 0;
    p_card->sensor_num = 0;
    if(p_card->chip_sensor_num+p_card->sensor_num > MAX_TMPR_SENSOR_NUM)
    {
        LCM_LOG_ERR("Sensor structure is too small to store sensor info on board.\n");        
        return LCM_E_NO_MEMORY;
    }
    p_card->tmpr_limit.board_crit = DEFAULT_CRIT_TMPR;
    p_card->tmpr_limit.board_high = DEFAULT_HIGH_TMPR;
    p_card->tmpr_limit.board_low = DEFAULT_LOW_TMPR;
    
    p_card->fan_module_num = E530_48S4X_FAN_MODULE_MAX;
    p_card->psu_module_num = E530_48S4X_PSU_MODULE_MAX;
    p_card->cpu_type = GLB_CPU_MAX;
    p_card->phy_chip_num = 8; 
    p_card->phy_int_bit_num = E530_48S4X_PHY_INT_BITS_SUM;
    p_card->phy_interrupt_mode = GLB_PHY_INTERRUPT_MODE_CPU;
    
#ifndef _GLB_UML_SYSTEM_ 
    ctc_phy_fd_init();
#endif
    p_card->phy_intr_en = 1;
    p_card->p_port_range = lcm_e530_48s4x_phy_chip_port_range;
    /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    p_card->ptp_en = 0;
    p_card->epld_type = GLB_EPLD_TYPE_MAX; 
    p_card->bootrom_type = E_BOOTROM_TYPE_4M; /*useless code*/

    sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                    DATAPATH_PROFILE_PATH, E530_48S4X_DATAPATH_NAME);
    /* for bug 53243 by tongzb , support reboot info, 2019-08-22 */
    p_card->support_reboot_info = 1;

    lcm_common_init_cpu_info();
    lcm_common_config_ptp_enable(0);//config ptp
    lcm_common_config_syncE_enable(0);//config syncE
    lcm_common_config_reboot_record_type(REBOOT_FILE_EPLD,0xf6);
    LCM_LOG_BRINGUP("card product type %x, board type %x.", p_card->board_type.series, p_card->board_type.type);
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_i2c());
    LCM_IF_ERROR_RETURN(lcm_init_e530_48s4x_gpio());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_epld(p_card, p_card->epld_ver));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_sysenv_info(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_sysenv(p_card));
    /* 0 for FR4 board, 1 for M4 board, 2 for M6 board */
    p_card->board_material = BOARD_MATERIAL_FR4;
    
    LCM_LOG_BRINGUP( "Lcm init board material is %d.",p_card->board_material);    
    /* epld write should be after the i2c init */
    epld_item_write(0, EPLD_ASIC_I2C_BRIDGE_RST, 0xff);
    epld_item_write(0, EPLD_PHY_RST, 0xff);
    epld_item_write(0, EPLD_PHY_INT_MASK1, 0x00);
    sal_delay(1);

    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_port(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_fan(p_card));

    LCM_IF_ERROR_RETURN(_lcm_init_e530_48s4x_psu(p_card));

    /* enable fiber TX default for keep status with fiber->run_info.enable */
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE1, 0x00));
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE2, 0x00));
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE3, 0x00));
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE4, 0x00));
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE5, 0x00));
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE6, 0x00));
    LCM_IF_ERROR_RETURN(epld_item_write(0, EPLD_SFP_DISABLE7, 0x00));

    lcm_card_init_callback(lcm_init_e530_48s4x_cb);
    lcm_port_init_ffe_cfg_callback(lcm_e530_48s4x_get_sum_ffe_cfg);
    LCM_LOG_BRINGUP("Lcm init board end.");

    return LCM_E_SUCCESS;
}

