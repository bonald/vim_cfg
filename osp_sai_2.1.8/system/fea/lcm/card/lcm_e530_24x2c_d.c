/****************************************************************************
* $Id$
*  E530-24X16C board init functions
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : tongzb
* Date          : 2018-12-17
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

/* EEPROM */
//#define E530_24X2C_D_EEPROM_NUM                      0x01
//#define E530_24X2C_D_ADDR_EEPROM                     0x57

#define E530_24X2C_D_SYSENV_NUM                   0x01

/* vcm (voltage and current monitor): ltc2991/bmr464 */
#define E530_24X2C_D_VCM_NUM                4
#define E530_24X2C_D_ADDR0_VCM                    0x53  /*max20743*/
#define E530_24X2C_D_ADDR1_VCM                    0x4c  /*ltc2991*/
#define E530_24X2C_D_ADDR2_VCM                    0x4d  /*ltc2991*/
#define E530_24X2C_D_ADDR3_VCM                    0x4e  /*ltc2991*/

/* I2C bridge */
#define E530_24X2C_D_I2C_BRIDGE_ADDR                 0x74

/*
 * CPU I2C slave device, after i2c bridge
 */ 
/* Power supply */
#define E530_24X2C_D_PSU_MODULE_MAX                  0x2
#define E530_24X2C_D_ADDR0_PSU                       0x50
#define E530_24X2C_D_ADDR1_PSU                       0x51

/* clock */
#define E530_24X2C_D_CLOCK_NUM                        0x1
#define E530_24X2C_D_ADDR0_CLOCK                      0x6c

/* Fan */
#define E530_24X2C_D_FAN_MODULE_MAX                  0x1
/* Fan controller0, controll fan 0/1/2/3 */
#define E530_24X2C_D_ADDR0_FAN                       0x2f

/* Thermal sensor */
#define E530_24X2C_D_SENSOR_NUM                      0x02
/* Sensor 0/1/2 on motherboard */
#define E530_24X2C_D_ADDR0_SENSOR                    0x48
#define E530_24X2C_D_ADDR1_SENSOR                    0x49

/* I2C to GPIO: PCA9505 */
#define E530_24X2C_D_GPIO_CHIP_NUM                   0x02
#define E530_24X2C_D_ADDR0_GPIO                      0x21
#define E530_24X2C_D_ADDR1_GPIO                      0x22

#define E530_24X2C_D_EPLD_I2C_ADDR                   0x36

#define E530_24X2C_D_PANEL_PORT_NUM_MAX              (24+2*4)
#define E530_24X2C_D_PANEL_PORT_NUM                  (24+2)
#define E530_24X2C_D_FIBER_PORT_NUM                  (24+2)

#define E530_24X2C_D_SLICE0_LED_MAC_NUM              (24+2+4)
#define E530_24X2C_D_LED_MAC_NUM                     (24+2+4)
#define E530_24X2C_D_LED_TBL_NUM                     4

#if (defined _CTC_ARM_CTC5236_)
#define GLB_I2C_IDX GLB_I2C_IDX_0
#elif (defined _CTC_ARM_LS1023A_)
#define GLB_I2C_IDX GLB_I2C_IDX_1
#else
#define GLB_I2C_IDX GLB_I2C_IDX_0
#endif

/****************************************************************************
*
* Global and Declarations
*
****************************************************************************/
static const glb_card_t         *glb_card;
static lcm_card_port_t          *glb_lcm_e530_24x2c_d_port             = NULL;
static fiber_port_info_t        *glb_lcm_e530_24x2c_d_fiber            = NULL;

extern int32 lcm_card_init_callback(card_init_callback_t func);
extern int32
lcm_common_get_defaultfec_from_speedability(uint16 speed_ability, uint32*  fec_ability);

lcm_card_port_t lcm_e530_24x2c_d_port_max[E530_24X2C_D_PANEL_PORT_NUM_MAX] =
{
    { 0,   0,   0,},    /* Port 1 */
    { 0,   1,   1,},    /* Port 2 */
    { 0,   2,   2,},    /* Port 3 */ 
    { 0,   3,   3,},    /* Port 4 */
    { 0,   8,   8,},    /* Port 5 */
    { 0,   9,   9,},    /* Port 6 */
    { 0,  10,  10,},    /* Port 7 */
    { 0,  11,  11,},    /* Port 8 */
                 
    { 0,  20,  20,},    /* Port 9 */
    { 0,  21,  21,},    /* Port 10 */
    { 0,  22,  22,},    /* Port 11 */ 
    { 0,  23,  23,},    /* Port 12 */
    { 0,  12,  12,},    /* Port 13 */
    { 0,  13,  13,},    /* Port 14 */
    { 0,  14,  14,},    /* Port 15 */
    { 0,  15,  15,},    /* Port 16 */
                 
    { 0,  24,  24,},    /* Port 17 */
    { 0,  25,  25,},    /* Port 18 */
    { 0,  26,  26,},    /* Port 19 */
    { 0,  27,  27,},    /* Port 20 */
    { 0,  28,  28,},    /* Port 21 */
    { 0,  29,  29,},    /* Port 22 */
    { 0,  30,  30,},    /* Port 23 */
    { 0,  31,  31,},    /* Port 24 */ 

    { 0,  61,  61,},    /* Port 25 */
    { 0,  60,  60,},    /* Port 26 */
    { 0,  63,  63,},    /* Port 27 */
    { 0,  62,  62,},    /* Port 28 */    
    { 0,  44,  44,},    /* Port 29 */
    { 0,  46,  46,},    /* Port 30 */
    { 0,  47,  47,},    /* Port 31 */
    { 0,  45,  45,},    /* Port 32 */

};

fiber_port_info_t lcm_e530_24x2c_d_fiber_max[E530_24X2C_D_FIBER_PORT_NUM] =
{
    /* panel port 1~8 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,0}},{.en_gpio={0,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,1}},{.en_gpio={0,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,7}},{.en_gpio={0,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,6}},{.en_gpio={0,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,5}},{.en_gpio={0,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,2}},{.en_gpio={0,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,4}},{.en_gpio={0,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,3}},{.en_gpio={0,7}}},

    /* panel port 9~16 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,8}},{.en_gpio={0,24}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,9}},{.en_gpio={0,25}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,15}},{.en_gpio={0,26}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,14}},{.en_gpio={0,27}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,13}},{.en_gpio={0,28}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,10}},{.en_gpio={0,29}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,12}},{.en_gpio={0,30}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,11}},{.en_gpio={0,31}}},

    /* panel port 17~24 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,20}},{.en_gpio={1,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,19}},{.en_gpio={1,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,21}},{.en_gpio={1,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,18}},{.en_gpio={1,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,22}},{.en_gpio={1,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,17}},{.en_gpio={1,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,23}},{.en_gpio={1,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,16}},{.en_gpio={1,7}}},

    {E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_MAX,0,{.asic={0,24}}},
    {E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_MAX,0,{.asic={0,25}}},    
};

/* Modified by liuht for bug 34540, 2015-09-09 */
mac_led_api_para_t lcm_e530_24x2c_d_mac_led_default_entry[E530_24X2C_D_LED_MAC_NUM]=
{
    /* panel port 1~8 */
    {0, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {1, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {2, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {3, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {8, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {9, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {10, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {11, 0, 0, LED_MODE_2_FORCE_OFF, 0},

    /* panel port 9~16 */
    {20, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {21, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {22, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {23, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {12, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {13, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {14, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {15, 0, 0, LED_MODE_2_FORCE_OFF, 0},   

    /* panel port 17~24 */
    {24, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {25, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {26, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {27, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {28, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {29, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {30, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {31, 0, 0, LED_MODE_2_FORCE_OFF, 0},

    /* panel port 25~26 */
    {60, 0, 0, LED_MODE_2_FORCE_OFF, 0},    
    {44, 0, 0, LED_MODE_2_FORCE_OFF, 0},

    /* fanout led, anytime there is only 1 or 0 LED is on 
     * using a NA mac 0 to send LED signal
     */
    {64, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    {65, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    {66, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    {67, 0, 0, LED_MODE_1_FORCE_OFF, 0},        
};

/* typical ffe */
lcm_card_serdes_ffe_t 
lcm_e530_24x2c_d_serdes_ffe_max[E530_24X2C_D_PANEL_PORT_NUM_MAX]=
{
    /* Panel Port 1~8 */
    { 0, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 1, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 2, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 3, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 4, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 5, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 6, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    { 7, 0xff, 0xff,COEFF1(2,90,11,0)}, 
    /* Panel Port 9~16 */
    { 8, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 9, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 10, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 11, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 12, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 13, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 14, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    { 15, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    /* Panel Port 17~24 */
    {16, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {17, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {18, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {19, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {20, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {21, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {22, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    {23, 0xff, 0xff,COEFF1(2,70,7,0)}, 
    /* Panel Port 25~26 */
    {29, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {28, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {31, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {30, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {24, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {26, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {27, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {25, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 

};

PWM_RPM_MAP g_fan_530_24x2c_d_9000_PWM_map_table[]=  
    {{0x00, 0xff, 170000, FAN_SPEED_00, 900},   
     {0x30, 0xff, 170000, FAN_SPEED_10, 900},   
     {0x50, 0xff, 250000, FAN_SPEED_20, 1800},
     {0x60, 0xff, 320000, FAN_SPEED_30, 2700}, 
     {0x70, 0xf7, 390000, FAN_SPEED_40, 3600}, /* merge from old */
     {0x80, 0xff, 460000, FAN_SPEED_50, 4500}, 
     {0xa0, 0xfa, 540000, FAN_SPEED_60, 5400}, /* merge from old */
     {0xb0, 0xff, 610000, FAN_SPEED_70, 6300}, 
     {0xc0, 0xfc, 700000, FAN_SPEED_80, 7200}, /* merge from old */
     {0xe0, 0xff, 820000, FAN_SPEED_90, 8100}, 
     {0xff, 0xff, 1000000, FAN_SPEED_100,9000}, 
     };

/****************************************************************************
 *
* Function
*
****************************************************************************/
static int32
lcm_init_e530_24x2c_d_init_port_table(glb_card_t* p_card)
{
    uint8 port_id, panel_port_no;
    uint8 panel_subport_no,port_numchg_type;
    uint8 pos = 0;

    for(port_id=0; port_id<p_card->port_num; port_id++)
    {
        panel_port_no = p_card->pp_port[port_id]->panel_port_no;
        panel_subport_no = p_card->pp_port[port_id]->panel_sub_port_no;
        port_numchg_type = p_card->pp_port[port_id]->port_numchg_type;
        if (port_id < 24)  /* no port num change */
        {
            pos = panel_port_no - 1;
        }
        else
        {
            if(GLB_PPT_NUMCHG_TYPE_50G == port_numchg_type)
            {
                if(panel_port_no == 25) /* for this board only one CG port support split into 2*50G */
                {
                    if(panel_subport_no == 1)
                        pos = 24 + (panel_port_no-24-1) * 4 + 1;
                    else if(panel_subport_no == 2)
                        pos = 24 + (panel_port_no-24-1) * 4 + 3;                  
                }
            }
            else if((GLB_PPT_NUMCHG_TYPE_10G == port_numchg_type)||(GLB_PPT_NUMCHG_TYPE_25G == port_numchg_type))
            {
                pos = 24 + (panel_port_no-24-1) * 4 + panel_subport_no-1;
            }
            else if(GLB_PPT_NUMCHG_TYPE_NONE == port_numchg_type)
            {
                if(panel_port_no == 25)
                    pos = 24 + (panel_port_no-24-1)*4+1;
                else if(panel_port_no == 26)
                    pos = 24 + (panel_port_no-24-1)*4+0;
            }
        }
        sal_memcpy(&glb_lcm_e530_24x2c_d_port[port_id], &lcm_e530_24x2c_d_port_max[pos], sizeof(lcm_card_port_t));
        glb_lcm_e530_24x2c_d_port[port_id].serdes_id = lcm_e530_24x2c_d_serdes_ffe_max[pos].serdes_id;
    }

    return LCM_E_SUCCESS;
}

static int32
lcm_init_e530_24x2c_d_init_fiber_table(glb_card_t* p_card)
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
        sal_memcpy(&glb_lcm_e530_24x2c_d_fiber[port_id], &lcm_e530_24x2c_d_fiber_max[panel_port_no-1], sizeof(fiber_port_info_t));
        glb_lcm_e530_24x2c_d_fiber[port_id].fiber_channel = fiber_channel;
        glb_lcm_e530_24x2c_d_fiber[port_id].fiber_id = port_id;
    }
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_d_panel_port(glb_card_t* p_card)
{
    uint8 ppt_id;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init panel port begin.");

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
        if (ppt_id < 24)
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_NONE;
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_10G|GLB_SUPPORT_SPEED_1G;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_10G;
        }
        else
        {
            p_card->pp_ppt[ppt_id]->op = GLB_PPT_OP_SPLIT;
            p_card->pp_ppt[ppt_id]->op_ability = GLB_PPT_OP_SPLIT_10G | GLB_PPT_OP_SPLIT_25G;
            if (ppt_id == 24)
            {
                p_card->pp_ppt[ppt_id]->op_ability = GLB_PPT_OP_SPLIT_10G | GLB_PPT_OP_SPLIT_25G | GLB_PPT_OP_SPLIT_50G;
            }
            p_card->pp_ppt[ppt_id]->disable_speed_change = 1;
            p_card->pp_ppt[ppt_id]->ppt_numchg_num = 1;///TODO: need optimize
            /*Bug33327, support 100G, but current default is 40G. This affect phy init, the phy default speed is 100G or 40G. */
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_100G|GLB_SUPPORT_SPEED_40G;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_40G;
        }
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init panel port end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_d_port(glb_card_t* p_card)
{
    int32 port_id = 0;
    uint32 ppt_idx;
    uint16 logic_port_idx;
    lcm_card_port_panel_mapping_t port_panel_mapping[256];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init port begin.");
    LCM_LOG_BRINGUP("Lcm init port begin.");
     /* 1, init panel port */
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_panel_port(p_card));
     
    /* 2, read '/mnt/flash/startup-config.conf' file, get port split/merge info */
    LCM_IF_ERROR_RETURN(lcm_common_parse_port_numchg_info(p_card));

    if(p_card->port_num != E530_24X2C_D_PANEL_PORT_NUM)
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
        p_card->pp_port[port_id]->port_numchg_type = port_panel_mapping[port_id].port_numchg_type;
     }
 
    /* 5, allocate memory for port/fiber/serdes data structure */
    glb_lcm_e530_24x2c_d_port = (lcm_card_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(lcm_card_port_t)* p_card->port_num);
    if (!glb_lcm_e530_24x2c_d_port)
    {
        LCM_LOG_ERR("Allocate pointer to lcm_card_port_t fail.");
        return LCM_E_NO_MEMORY;
    }
 
    glb_lcm_e530_24x2c_d_fiber = (fiber_port_info_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(fiber_port_info_t)*p_card->port_num);
    if (!glb_lcm_e530_24x2c_d_fiber)
    {
        LCM_LOG_ERR("Allocate pointer to fiber_port_info_t fail.");
        return LCM_E_NO_MEMORY;
    }

    /* 6, init current running port/fiber info */
    LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_d_init_port_table(p_card));
    LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_d_init_fiber_table(p_card));
 
    /* 7, init port properties */
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        logic_port_idx = glb_lcm_e530_24x2c_d_port[port_id].logic_port_idx;
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
        p_card->pp_port[port_id]->local_chip_idx = glb_lcm_e530_24x2c_d_port[port_id].chip_idx;
        p_card->pp_port[port_id]->mac_idx = glb_lcm_e530_24x2c_d_port[port_id].mac_idx;
        /* for bug 48433, do phy mapping */
        p_card->pp_port[port_id]->phy_addr = glb_lcm_e530_24x2c_d_port[port_id].phy_addr;
        p_card->pp_port[port_id]->chip_serdes_id = glb_lcm_e530_24x2c_d_port[port_id].serdes_id;
        p_card->pp_port[port_id]->g_port_index = (p_card->pp_port[port_id]->glb_chip_idx << 8) | logic_port_idx;
        /* for bug 48433 */
        p_card->pp_port[port_id]->mdio_bus_addr = 0xff;/* default */
    
        /* for bug 47572,add by tongzb, support speed 1G/10G/25G */
        p_card->pp_port[port_id]->speed_mode_cfg_en = 0;
        p_card->pp_port[port_id]->speed_mode = E_SPEED_10_25G;
        p_card->pp_port[port_id]->default_speed_mode = E_SPEED_10_25G;      
        /* Modified by liuht for bug 34540, 2015-09-09 */
        p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
        p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
        p_card->pp_port[port_id]->unidir_support = 1;
        if (ppt_idx <= 24)
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED;
        }
        else
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_QSFP_TWO_LED;
        }

        p_card->pp_port[port_id]->port_led_reverse = 0;
        p_card->pp_port[port_id]->ctl_id = 0;/* DUET2 only have one ctl */
        p_card->pp_port[port_id]->port_led_mac = glb_lcm_e530_24x2c_d_port[port_id].mac_idx;
        p_card->pp_port[port_id]->mac_ctl_id = 0;/* Tsingma only have one mac led slice */
     }

    /* 8, create file '/tmp/ctcos_port_info' */
    LCM_IF_ERROR_RETURN(lcm_common_save_port_info_file(p_card));
     
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init port end.");
    LCM_LOG_BRINGUP("Lcm init port end.");    

    return LCM_E_SUCCESS;
}


static int32_t
_lcm_reg_e530_24x2c_d_epld(uint8 hw_ver, epld_info_t * p_epld_info)
{
    p_epld_info->epld_bus_type = EPLD_I2C_TYPE;
    p_epld_info->i2c_gen.bridge_flag = 0;
    p_epld_info->i2c_gen.addr = E530_24X2C_D_EPLD_I2C_ADDR;
    p_epld_info->i2c_gen.i2c_type = E_I2C_CPM;
    p_epld_info->i2c_gen.i2c_bus_idx = GLB_I2C_IDX;
    p_epld_info->i2c_gen.alen = EPLD_DATA_LENTH;

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_EPLD_VERSION]),          0x1, 0, 7, 8);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_SYS_LED_SYS]),           0x2, 4, 7, 4);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_IND_LED]),               0x3, 0, 0, 1);
        
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_PRESENT_STATUS]),    0x19, 4, 5, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_WORK_STATUS]),       0x19, 6, 7, 2);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PSU_ALERT_STATUS]),      0x19, 2, 3, 2);
    
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_ASIC_I2C_BRIDGE_RST]),   0x08, 4, 7, 4);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_I2C_BRIDGE_RST]),        0x08, 3, 3, 1);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_DPLL_RST]),              0x08, 2, 2, 1);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_GPIO_RST]),              0x08, 0, 1, 2);
    
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_PPU_INT_MASK]),          0x0f, 0, 3, 4);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_DPLL_INT_MASK]),         0x0b, 2, 2, 1);
    /* default location */
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_REBOOT_POWER_RST]),      0xf6, 0, 7, 8);
    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_POWER_DOWN]),            0x23, 0, 1, 2);

    epld_set_reg_desc(&(p_epld_info->reg_desc[EPLD_TEST]),                  0x7f, 0, 7, 8);

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_d_epld(glb_card_t* p_card, uint8 epld_version)
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

    _lcm_reg_e530_24x2c_d_epld(p_card->hw_ver, p_epld_info);
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
_lcm_init_e530_24x2c_d_sysenv_info(glb_card_t* p_card)
{
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sysenv info.");
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
_lcm_init_e530_24x2c_d_sysenv(glb_card_t* p_card)
{
    int32 ret = 0;
    sysenv_chip_info_t sysenv_chip_info[E530_24X2C_D_SYSENV_NUM];
    sysenv_spiflash_info_t spiflash_info[E530_24X2C_D_SYSENV_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sysenv begin.");
    LCM_LOG_BRINGUP("Lcm init sysenv begin.");
    sal_memset(sysenv_chip_info, 0, sizeof(sysenv_chip_info));
    sal_memset(spiflash_info, 0, sizeof(spiflash_info));
    /* use sysenv api, sysenv_info should init */
    p_card->use_sysenv_api = 1;

#ifdef _CTC_ARM_LS1023A_
    sysenv_chip_info[0].sysenv_chip_type = SYSENV_CHIP_SPIFLASH;
    spiflash_info[0].base_offset = 0x310000;
    spiflash_info[0].block_size = 0x10000;
    spiflash_info[0].block_num = 1;
    spiflash_info[0].dev_fd = -1;
    sal_strncpy(spiflash_info[0].dev_name, "/dev/mtd0", 64);
#endif
#ifdef _CTC_ARM_CTC5236_
    sysenv_chip_info[0].sysenv_chip_type = SYSENV_CHIP_SPIFLASH;
    spiflash_info[0].base_offset = 0x000000;
    spiflash_info[0].block_size = 0x01000;
    spiflash_info[0].block_num = 2;
    spiflash_info[0].dev_fd = -1;
    sal_strncpy(spiflash_info[0].dev_name, "/dev/mtd2", 64);
#endif
    sysenv_chip_info[0].sysenv_gen = (void *)(&spiflash_info[0]);

    ret = sysenv_init(sysenv_chip_info, E530_24X2C_D_SYSENV_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm sysenv init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sysenv end.");
    LCM_LOG_BRINGUP("Lcm init sysenv end.");
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_e530_24x2c_d_fiber(void)
{
    int32 ret = 0;
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fiber begin.");
    LCM_LOG_BRINGUP( "Lcm init fiber begin.");
    ret = fiber_init(glb_card->port_num,  /* fiber num */
                     glb_card->port_num, /* port num */
                     glb_lcm_e530_24x2c_d_fiber);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fiber end.");
    LCM_LOG_BRINGUP("Lcm init fiber end.");

    return ret;
}

static int32
_lcm_init_e530_24x2c_d_sensor(glb_card_t* p_card)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[E530_24X2C_D_SENSOR_NUM];
    sensor_chip_t sensor_chip[E530_24X2C_D_SENSOR_NUM];
    void *p_data[E530_24X2C_D_SENSOR_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sensor begin.");
    LCM_LOG_BRINGUP("Lcm init sensor begin.");
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(sensor_chip, 0, sizeof(sensor_chip));

    /* sensor0: on motherboard, before GG chip */
    i2c_gen[0].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[0].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[0].p_br.channel = 6;
    i2c_gen[0].addr = E530_24X2C_D_ADDR0_SENSOR;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 1;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;

    sensor_chip[0].io_type = SENSOR_I2C;
    i2c_gen[0].alen = LM77_OFFSET_WIDTH;
    sensor_chip[0].chip_type = SENSOR_LM77;
    sensor_chip[0].pos = SENSOR_BEFORE_CHIP;

    /* sensor1: on motherboard, behind GG chip */
    i2c_gen[1].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[1].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[1].p_br.channel = 6;
    i2c_gen[1].addr = E530_24X2C_D_ADDR1_SENSOR;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].bridge_flag = 1;
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX;

    sensor_chip[1].io_type = SENSOR_I2C;
    i2c_gen[1].alen = LM77_OFFSET_WIDTH;
    sensor_chip[1].chip_type = SENSOR_LM77;
    sensor_chip[1].pos = SENSOR_BEHIND_CHIP;

    for(i=0; i<E530_24X2C_D_SENSOR_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = sensor_init(p_data, sensor_chip, E530_24X2C_D_SENSOR_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Temperature sensor init fail.");
        return LCM_E_INIT_FAILED;
    }
    if (p_card->bootup_diag_level >= E_DIAG_LEVEL_COMPLETE)
    {
        ret = sensor_diagnostic_test_and_report(p_card->sensor_num);
        if(ret < 0)
        {
            LCM_LOG_ERR("Sensor diag fail.");
            //return LCM_E_INIT_FAILED;
        }
    }
    for(i=0; i<E530_24X2C_D_SENSOR_NUM; i++)
    {
        sensor_dev_init(i);
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sensor end.");
    LCM_LOG_BRINGUP("Lcm init sensor end.");
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_e530_24x2c_d_cfg_gpio(uint32 chip_id)
{
    uint8 reg, val;

    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module begin.");
    switch (chip_id)
    {
    case 0:    
        /* bank 1,2,4 : input
         * bank 0,3   : output
        */
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* mask all interrupt */
        reg = PCA9505_INT_MASK_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK2;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_INT_MASK_REG_BANK4;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* tx enable */
        reg = PCA9505_OUTPUT_PORT_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_OUTPUT_PORT_REG_BANK3;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        break;
        
    case 1:
        /* bank 1, 2.0-2.4, 2.7, 3,4 : input
         * bank 0,2.5,2.6       : output
        */
        reg = PCA9505_DIR_CTRL_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK2;
        val = 0x9f;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);
        reg = PCA9505_DIR_CTRL_REG_BANK3;
        val = 0xff;
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
        reg = PCA9505_OUTPUT_PORT_REG_BANK1;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);

        /* QSFP28 reset */
        reg = PCA9505_OUTPUT_PORT_REG_BANK2;
        val = 0x60;
        gpio_reg_write(chip_id, reg, &val, PCA9505_VALUE_LEN);        
        break;
    }
    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module end.");
    
    return RESULT_OK;
}

static int32
lcm_init_e530_24x2c_d_gpio(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[E530_24X2C_D_GPIO_CHIP_NUM];
    gpio_chip_t gpio_chip[E530_24X2C_D_GPIO_CHIP_NUM];
    void *p_data[E530_24X2C_D_GPIO_CHIP_NUM];

    LCM_LOG_BRINGUP("Lcm init i2c to gpio module begin.")
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(gpio_chip, 0, sizeof(gpio_chip));

    i2c_gen[0].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[0].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[0].p_br.channel = 4;
    i2c_gen[0].addr = E530_24X2C_D_ADDR0_GPIO;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 1;   
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[0].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[0].io_type = GPIO_I2C;    
    gpio_chip[0].chip_type = GPIO_PCA9505;
    gpio_chip[0].scan_group_bitmap = 0x09; /*group 0,3*/

    i2c_gen[1].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[1].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[1].p_br.channel = 4;
    i2c_gen[1].addr = E530_24X2C_D_ADDR1_GPIO;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].bridge_flag = 1; 
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[1].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[1].io_type = GPIO_I2C;    
    gpio_chip[1].chip_type = GPIO_PCA9505;
    gpio_chip[1].scan_group_bitmap = 0x05; /*group 0,2*/


    for(i=0; i<E530_24X2C_D_GPIO_CHIP_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = gpio_init(p_data, gpio_chip, E530_24X2C_D_GPIO_CHIP_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Gpio device init fail.");
        return LCM_E_INIT_FAILED;
    }

    for(i=0; i<E530_24X2C_D_GPIO_CHIP_NUM; i++)
    {
        _lcm_e530_24x2c_d_cfg_gpio(i);
    }
    LCM_LOG_BRINGUP("Lcm init i2c to gpio module end.");

    return LCM_E_SUCCESS;
}

#ifndef _CTC_ARM_CTC5236_
/* Added by liuht for bug26671, 2014-02-14 */
static int32
_lcm_init_e530_24x2c_d_fan(glb_card_t* p_card)
{
    int32 ret = 0;
    uint8 fan_module_idx;
    i2c_gen_t i2c_gen[E530_24X2C_D_FAN_MODULE_MAX];
    fan_chip_t fan_chip[E530_24X2C_D_FAN_MODULE_MAX];
    void *p_data[E530_24X2C_D_FAN_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module begin.");
    LCM_LOG_BRINGUP("Lcm init fan module begin.");    
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(fan_chip, 0, sizeof(fan_chip));

    i2c_gen[0].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[0].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[0].p_br.channel = 5;
    i2c_gen[0].addr = E530_24X2C_D_ADDR0_FAN;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = ADT7470_OFFSET_WIDTH;
    i2c_gen[0].bridge_flag = 1;   
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;

    fan_chip[0].chip_type = FAN_ADT7470;
    fan_chip[0].io_type = E_FAN_I2C;
    fan_chip[0].support_hot_swap = 0;  /* stable on GG mother board, do not support hot swap */
    p_data[0] = &i2c_gen[0];
    fan_chip[0].fan_num = 4;
    fan_chip[0].speed_adjust = 1;
    fan_chip[0].rpm = 9000;
    fan_chip[0].pwm_map_table = NULL;
    fan_chip[0].pwm.bootom_rate = DEFAULT_FAN_RATE_BOTTOM; 
    fan_chip[0].pwm.low_rate = DEFAULT_FAN_RATE_LOW;
    fan_chip[0].pwm.high_rate = DEFAULT_FAN_RATE_HIGH;
    fan_chip[0].pwm.full_rate = DEFAULT_FAN_RATE_FULL;

    ret = fan_init(p_data, fan_chip, E530_24X2C_D_FAN_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Fan driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(fan_module_idx=0; fan_module_idx<E530_24X2C_D_FAN_MODULE_MAX; fan_module_idx++)
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
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module end.");
    LCM_LOG_BRINGUP("Lcm init fan module end.");    

    return LCM_E_SUCCESS;
}
#else
static int32
_lcm_init_e530_24x2c_d_fan(glb_card_t* p_card)
{
    int32 ret = 0;
    uint8 fan_module_idx;
    fan_chip_t fan_chip[E530_24X2C_D_FAN_MODULE_MAX];
    void *p_data[E530_24X2C_D_FAN_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module begin.");
    LCM_LOG_BRINGUP("Lcm init fan module begin.");    
    sal_memset(fan_chip, 0, sizeof(fan_chip));

    fan_chip[0].chip_type = FAN_CTC5236;
    fan_chip[0].io_type = E_FAN_CTC5236;
    fan_chip[0].support_hot_swap = 0;
    p_data[0] = NULL;
    fan_chip[0].fan_num = 4;
    fan_chip[0].speed_adjust = 1;
    fan_chip[0].rpm = 9000;
    fan_chip[0].pwm_map_table = NULL;
    fan_chip[0].pwm.bootom_rate = DEFAULT_FAN_RATE_BOTTOM; 
    fan_chip[0].pwm.low_rate = DEFAULT_FAN_RATE_LOW;
    fan_chip[0].pwm.high_rate = DEFAULT_FAN_RATE_HIGH;
    fan_chip[0].pwm.full_rate = DEFAULT_FAN_RATE_FULL;
    fan_chip[0].pwm_channel_flag = DEFAULT_FAN_PWM_CHANNAL;    /*if use default channel order*/

    fan_chip[0].pwm_map_table = g_fan_530_24x2c_d_9000_PWM_map_table;
    fan_chip[0].pwm_map_table_num = FAN_SPEED_NUM;

    ret = fan_init(p_data, fan_chip, E530_24X2C_D_FAN_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Fan driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(fan_module_idx=0; fan_module_idx < E530_24X2C_D_FAN_MODULE_MAX; fan_module_idx++)
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
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module end.");
    LCM_LOG_BRINGUP("Lcm init fan module end.");    

    return LCM_E_SUCCESS;
}
#endif

static int32
_lcm_init_e530_24x2c_d_psu(glb_card_t* p_card)
{
    int32 ret;
    i2c_gen_t i2c_gen[E530_24X2C_D_PSU_MODULE_MAX];
    psu_chip_t psu_chip[E530_24X2C_D_PSU_MODULE_MAX];
    psu_private_t p_data[E530_24X2C_D_PSU_MODULE_MAX];
    uint8 epld_idx[E530_24X2C_D_PSU_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init psu module begin.");
    LCM_LOG_BRINGUP("Lcm init psu module begin.");    
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(p_data, 0, sizeof(p_data));
    sal_memset(psu_chip, 0, sizeof(psu_chip));

    epld_idx[0] = 0;
    i2c_gen[0].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[0].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[0].p_br.channel = 3;
    i2c_gen[0].addr = E530_24X2C_D_ADDR0_PSU;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].alen = PSU_I2C_OFFSET_WIDTH;
    i2c_gen[0].bridge_flag = 1;   
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;

    psu_chip[0].chip_type = PSU_I2C_EPLD;
    psu_chip[0].io_type = PSU_IO_I2C_EPLD;
    psu_chip[0].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[0].psu_mode_type = (1<<PSU_MODE_TYPE_REG_50_01_04)|(1<<PSU_MODE_TYPE_REG_50_20_08);
    psu_chip[0].support_pmbus = 1;
    p_data[0].p_data_i2c= &i2c_gen[0];
    p_data[0].p_data_epld= &epld_idx[0];

    epld_idx[1] = 0;
    i2c_gen[1].p_br.i2c_br_type = I2C_BR_CPM;
    i2c_gen[1].p_br.bridge_addr = E530_24X2C_D_I2C_BRIDGE_ADDR;
    i2c_gen[1].p_br.channel = 2;
    i2c_gen[1].addr = E530_24X2C_D_ADDR1_PSU;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].alen = PSU_I2C_OFFSET_WIDTH;
    i2c_gen[1].bridge_flag = 1;   
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX;

    psu_chip[1].chip_type = PSU_I2C_EPLD;
    psu_chip[1].io_type = PSU_IO_I2C_EPLD;
    psu_chip[1].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[1].psu_mode_type = (1<<PSU_MODE_TYPE_REG_51_01_04)|(1<<PSU_MODE_TYPE_REG_51_20_08);
    psu_chip[1].support_pmbus = 1;
    p_data[1].p_data_i2c= &i2c_gen[1];
    p_data[1].p_data_epld= &epld_idx[1];

    ret = psu_init(p_data, psu_chip, E530_24X2C_D_PSU_MODULE_MAX);
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
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init psu module end.");
    LCM_LOG_BRINGUP("Lcm int psu end.");    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_d_volt_mon(glb_board_type_t board_type)
{    
#if 0
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[e550_24t16y_VOLT_MON_NUM];
    ltc2991_chip_t ltc2991_chip[e550_24T16Y_VOLT_MON_NUM];
    void *p_data[e550_24T16Y_VOLT_MON_NUM];

    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
   
    i2c_gen[0].addr = e550_24T16Y_ADDR_VOLT_MON;
    for(i=0; i<e550_24T16Y_VOLT_MON_NUM; i++)
    {
        i2c_gen[i].i2c_type = E_I2C_CPM;
        i2c_gen[i].alen = LTC2991_OFFSET_WIDTH;
        i2c_gen[i].bridge_flag = 0;
        ltc2991_chip[i].chip_type = LTC2991_CMS;
        ltc2991_chip[i].io_type = LTC2991_I2C;
        p_data[i] = &i2c_gen[i];
    }
    ret = ltc2991_init(p_data, ltc2991_chip, e550_24T16Y_VOLT_MON_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Ltc2991 init fail.");
        return LCM_E_INIT_FAILED;
    }    
    for(i=0; i<e550_24T16Y_VOLT_MON_NUM; i++)
    {
        ret = ltc2991_dev_init(board_type, i);
        if(ret < 0)
        {
            LCM_LOG_ERR("Ltc2991 dev init fail.");
            return LCM_E_INIT_FAILED;
        }
    }
#endif
    return LCM_E_SUCCESS;
}

int32
_lcm_e530_24x2c_d_config_clock()
{
    int32 ret = 0;
    uint32 val;
    uint32 timeout = 20000;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm config clock begin.");
    LCM_LOG_BRINGUP("Lcm config clock begin.");
    
    /* step 1 */
    ret += clock_write(0, 0x0a22, 0x01);
    ret += clock_write(0, 0x0a42, 0x01);

    /* step 2 */
    ret += clock_write(0, 0x0005, 0x01);

    /* step 3 */
    ret += clock_write(0, 0x0,0x80);
    ret += clock_write(0, 0x4,0x00);
    ret += clock_write(0, 0x5,0x00);
    ret += clock_write(0, 0xa,0x12);
    ret += clock_write(0, 0xb,0x0F);
    ret += clock_write(0, 0xc,0x02);
    ret += clock_write(0, 0xd,0x00);
    ret += clock_write(0, 0xe,0x01);
    ret += clock_write(0, 0xf,0x00);
#if 0
    ret += clock_write(0, 0x80,0x07);
    ret += clock_write(0, 0x81,0x00);
    ret += clock_write(0, 0x82,0x01);
    ret += clock_write(0, 0x83,0x03);
    ret += clock_write(0, 0x84,0x00);
    ret += clock_write(0, 0x85,0x01);
    ret += clock_write(0, 0x86,0x01);
    ret += clock_write(0, 0x87,0x00);
    ret += clock_write(0, 0x88,0x00);
#endif
    ret += clock_write(0, 0x100,0x01);
    ret += clock_write(0, 0x101,0x00);
    ret += clock_write(0, 0x102,0x90);
    ret += clock_write(0, 0x103,0x00);
    ret += clock_write(0, 0x104,0x00);
    ret += clock_write(0, 0x105,0x00);
    ret += clock_write(0, 0x106,0x00);
    ret += clock_write(0, 0x107,0x00);
    ret += clock_write(0, 0x108,0x00);
    ret += clock_write(0, 0x109,0x00);
    ret += clock_write(0, 0x10a,0x00);
    ret += clock_write(0, 0x10b,0x00);
    ret += clock_write(0, 0x10c,0x00);
    ret += clock_write(0, 0x10d,0x0F);
    ret += clock_write(0, 0x10e,0x60);
    ret += clock_write(0, 0x10f,0x00);
    ret += clock_write(0, 0x110,0x0F);
    ret += clock_write(0, 0x111,0x60);
    ret += clock_write(0, 0x112,0x00);

    ret += clock_write(0, 0x200,0x10);
    ret += clock_write(0, 0x200,0x10);
    ret += clock_write(0, 0x200,0x10);
    ret += clock_write(0, 0x201,0x01);
    ret += clock_write(0, 0x202,0xd0);
    ret += clock_write(0, 0x203,0x12);
    ret += clock_write(0, 0x204,0x13);
    ret += clock_write(0, 0x205,0x32);
    ret += clock_write(0, 0x206,0x00);
    ret += clock_write(0, 0x207,0x00);
                      
    /* step 4 */
    ret += clock_write(0, 0x0005, 0x01);

    /* step 5 */
    ret += clock_read(0, 0x0d01, &val);
    while (1 != ((val >> 1) & 0x1))
    {
        timeout--;
        if(timeout)
        {
            //ret += clock_write(0, 0x0005, 0x01);
            ret += clock_read(0, 0x0d01, &val);
        }
        else
        {
            break;
        }
    }
    if (!timeout)
    {
        LCM_LOG_DEBUG(LCM_CARD, "LCM config clock step 5 timeout!");
        LCM_LOG_BRINGUP("LCM config clock step 5 timeout!");        
        return 0;/* for some e550-24x8y2c board do not use ad9595 */
    }

    /* step 6 */
    /* For the outputs to toggle prior to DPLL phase or frequency lock */
    ret += clock_write(0, 0x0a20, 0x4 );
    ret += clock_write(0, 0x0a40, 0x4 );
    ret += clock_write(0, 0x0005, 0x01);
	
    /* step 7 */
    ret += clock_write(0, 0x300,0x03);
    ret += clock_write(0, 0x301,0x13);
    ret += clock_write(0, 0x302,0x00);
    ret += clock_write(0, 0x303,0x00);
    ret += clock_write(0, 0x304,0x00);
    ret += clock_write(0, 0x305,0x80);
    ret += clock_write(0, 0x306,0x84);
    ret += clock_write(0, 0x307,0x1E);
    ret += clock_write(0, 0x308,0x00);
    ret += clock_write(0, 0x309,0x1A);
    ret += clock_write(0, 0x30a,0x17);
    ret += clock_write(0, 0x30b,0x02);
    ret += clock_write(0, 0x30c,0x84);
    ret += clock_write(0, 0x30d,0x70);
    ret += clock_write(0, 0x30e,0x01);
    ret += clock_write(0, 0x30f,0xE8);
    ret += clock_write(0, 0x310,0x03);
    ret += clock_write(0, 0x311,0x40);
    ret += clock_write(0, 0x312,0x9C);
    ret += clock_write(0, 0x313,0x00);
    ret += clock_write(0, 0x314,0x0A);
    ret += clock_write(0, 0x315,0x0A);
    ret += clock_write(0, 0x316,0xE8);
    ret += clock_write(0, 0x317,0x03);
    ret += clock_write(0, 0x318,0x00);
    ret += clock_write(0, 0x319,0x0A);
    ret += clock_write(0, 0x31a,0x0A);
    ret += clock_write(0, 0x320,0x03);
    ret += clock_write(0, 0x321,0x09);
    ret += clock_write(0, 0x322,0x00);
    ret += clock_write(0, 0x323,0x00);
    ret += clock_write(0, 0x324,0x00);
    ret += clock_write(0, 0x325,0x00);
    ret += clock_write(0, 0x326,0x09);
    ret += clock_write(0, 0x327,0x3D);
    ret += clock_write(0, 0x328,0x00);
    ret += clock_write(0, 0x329,0x1A);
    ret += clock_write(0, 0x32a,0x17);
    ret += clock_write(0, 0x32b,0x02);
    ret += clock_write(0, 0x32c,0x84);
    ret += clock_write(0, 0x32d,0x70);
    ret += clock_write(0, 0x32e,0x01);
    ret += clock_write(0, 0x32f,0xE8);
    ret += clock_write(0, 0x330,0x03);
    ret += clock_write(0, 0x331,0xE1);
    ret += clock_write(0, 0x332,0x25);
    ret += clock_write(0, 0x333,0x00);
    ret += clock_write(0, 0x334,0x0A);
    ret += clock_write(0, 0x335,0x0A);
    ret += clock_write(0, 0x336,0xE8);
    ret += clock_write(0, 0x337,0x03);
    ret += clock_write(0, 0x338,0x00);
    ret += clock_write(0, 0x339,0x0A);
    ret += clock_write(0, 0x33a,0x0A);
    ret += clock_write(0, 0x340,0x03);
    ret += clock_write(0, 0x341,0x04);
    ret += clock_write(0, 0x342,0x01);
    ret += clock_write(0, 0x343,0x00);
    ret += clock_write(0, 0x344,0x8D);
    ret += clock_write(0, 0x345,0x65);
    ret += clock_write(0, 0x346,0x1F);
    ret += clock_write(0, 0x347,0x00);
    ret += clock_write(0, 0x348,0x00);
    ret += clock_write(0, 0x349,0x1A);
    ret += clock_write(0, 0x34a,0x17);
    ret += clock_write(0, 0x34b,0x02);
    ret += clock_write(0, 0x34c,0x84);
    ret += clock_write(0, 0x34d,0x70);
    ret += clock_write(0, 0x34e,0x01);
    ret += clock_write(0, 0x34f,0xE8);
    ret += clock_write(0, 0x350,0x03);
    ret += clock_write(0, 0x351,0x40);
    ret += clock_write(0, 0x352,0x9C);
    ret += clock_write(0, 0x353,0x00);
    ret += clock_write(0, 0x354,0x0A);
    ret += clock_write(0, 0x355,0x0A);
    ret += clock_write(0, 0x356,0xE8);
    ret += clock_write(0, 0x357,0x03);
    ret += clock_write(0, 0x358,0x00);
    ret += clock_write(0, 0x359,0x0A);
    ret += clock_write(0, 0x35a,0x0A);
    ret += clock_write(0, 0x360,0x03);
    ret += clock_write(0, 0x361,0x04);
    ret += clock_write(0, 0x362,0x01);
    ret += clock_write(0, 0x363,0x00);
    ret += clock_write(0, 0x364,0x8D);
    ret += clock_write(0, 0x365,0x65);
    ret += clock_write(0, 0x366,0x1F);
    ret += clock_write(0, 0x367,0x00);
    ret += clock_write(0, 0x368,0x00);
    ret += clock_write(0, 0x369,0x1A);
    ret += clock_write(0, 0x36a,0x17);
    ret += clock_write(0, 0x36b,0x02);
    ret += clock_write(0, 0x36c,0x84);
    ret += clock_write(0, 0x36d,0x70);
    ret += clock_write(0, 0x36e,0x01);
    ret += clock_write(0, 0x36f,0xE8);
    ret += clock_write(0, 0x370,0x03);
    ret += clock_write(0, 0x371,0x40);
    ret += clock_write(0, 0x372,0x9C);
    ret += clock_write(0, 0x373,0x00);
    ret += clock_write(0, 0x374,0x0A);
    ret += clock_write(0, 0x375,0x0A);
    ret += clock_write(0, 0x376,0xBC);
    ret += clock_write(0, 0x377,0x02);
    ret += clock_write(0, 0x378,0x00);
    ret += clock_write(0, 0x379,0x0A);
    ret += clock_write(0, 0x37a,0x0A);
    ret += clock_write(0, 0x400,0x78);
    ret += clock_write(0, 0x401,0x18);
    ret += clock_write(0, 0x402,0xB4);
    ret += clock_write(0, 0x403,0x29);
    ret += clock_write(0, 0x404,0x18);
    ret += clock_write(0, 0x405,0x51);
    ret += clock_write(0, 0x406,0xB8);
    ret += clock_write(0, 0x407,0x02);
    ret += clock_write(0, 0x408,0xFF);
    ret += clock_write(0, 0x409,0xFF);
    ret += clock_write(0, 0x40a,0x0F);
    ret += clock_write(0, 0x40b,0x0A);
    ret += clock_write(0, 0x40c,0x00);
    ret += clock_write(0, 0x40d,0x00);
    ret += clock_write(0, 0x40e,0x00);
    ret += clock_write(0, 0x40f,0x00);
    ret += clock_write(0, 0x410,0x00);
    ret += clock_write(0, 0x411,0x00);
    ret += clock_write(0, 0x412,0x00);
    ret += clock_write(0, 0x413,0x00);
    ret += clock_write(0, 0x414,0x00);
    ret += clock_write(0, 0x415,0x00);
    ret += clock_write(0, 0x420,0x81);
    ret += clock_write(0, 0x421,0x10);
    ret += clock_write(0, 0x422,0x47);
    ret += clock_write(0, 0x423,0x00);
    ret += clock_write(0, 0x424,0x01);
    ret += clock_write(0, 0x425,0x00);
    ret += clock_write(0, 0x426,0x00);
    ret += clock_write(0, 0x427,0x10);
    ret += clock_write(0, 0x428,0xF8);
    ret += clock_write(0, 0x429,0x01);
    ret += clock_write(0, 0x42a,0x00);
    ret += clock_write(0, 0x42b,0xC6);
    ret += clock_write(0, 0x42c,0xF1);
    ret += clock_write(0, 0x42d,0x03);
    ret += clock_write(0, 0x42e,0x00);
    ret += clock_write(0, 0x440,0x01);
    ret += clock_write(0, 0x441,0xF4);
    ret += clock_write(0, 0x442,0x01);
    ret += clock_write(0, 0x443,0x00);
    ret += clock_write(0, 0x444,0x64);
    ret += clock_write(0, 0x445,0x07);
    ret += clock_write(0, 0x446,0x00);
    ret += clock_write(0, 0x447,0x03);
    ret += clock_write(0, 0x448,0x00);
    ret += clock_write(0, 0x449,0x00);
    ret += clock_write(0, 0x44a,0x04);
    ret += clock_write(0, 0x44b,0x00);
    ret += clock_write(0, 0x44c,0x00);
    ret += clock_write(0, 0x44d,0x01);
    ret += clock_write(0, 0x44e,0xF4);
    ret += clock_write(0, 0x44f,0x01);
    ret += clock_write(0, 0x450,0x00);
    ret += clock_write(0, 0x451,0x64);
    ret += clock_write(0, 0x452,0x07);
    ret += clock_write(0, 0x453,0x00);
    ret += clock_write(0, 0x454,0x03);
    ret += clock_write(0, 0x455,0x00);
    ret += clock_write(0, 0x456,0x00);
    ret += clock_write(0, 0x457,0x04);
    ret += clock_write(0, 0x458,0x00);
    ret += clock_write(0, 0x459,0x00);
    ret += clock_write(0, 0x45a,0x00);
    ret += clock_write(0, 0x45b,0xF4);
    ret += clock_write(0, 0x45c,0x01);
    ret += clock_write(0, 0x45d,0x00);
    ret += clock_write(0, 0x45e,0x7E);
    ret += clock_write(0, 0x45f,0x07);
    ret += clock_write(0, 0x460,0x00);
    ret += clock_write(0, 0x461,0x02);
    ret += clock_write(0, 0x462,0x00);
    ret += clock_write(0, 0x463,0x00);
    ret += clock_write(0, 0x464,0x11);
    ret += clock_write(0, 0x465,0x00);
    ret += clock_write(0, 0x466,0x00);
    ret += clock_write(0, 0x467,0x00);
    ret += clock_write(0, 0x468,0xF4);
    ret += clock_write(0, 0x469,0x01);
    ret += clock_write(0, 0x46a,0x00);
    ret += clock_write(0, 0x46b,0x7E);
    ret += clock_write(0, 0x46c,0x07);
    ret += clock_write(0, 0x46d,0x00);
    ret += clock_write(0, 0x46e,0x02);
    ret += clock_write(0, 0x46f,0x00);
    ret += clock_write(0, 0x470,0x00);
    ret += clock_write(0, 0x471,0x11);
    ret += clock_write(0, 0x472,0x00);
    ret += clock_write(0, 0x473,0x00);
    ret += clock_write(0, 0x500,0x3B);
    ret += clock_write(0, 0x501,0xB1);
    ret += clock_write(0, 0x502,0x13);
    ret += clock_write(0, 0x503,0x24);
    ret += clock_write(0, 0x504,0x08);
    ret += clock_write(0, 0x505,0x51);
    ret += clock_write(0, 0x506,0xB8);
    ret += clock_write(0, 0x507,0x02);
    ret += clock_write(0, 0x508,0x3E);
    ret += clock_write(0, 0x509,0x0A);
    ret += clock_write(0, 0x50a,0x0B);
    ret += clock_write(0, 0x50b,0x0A);
    ret += clock_write(0, 0x50c,0x00);
    ret += clock_write(0, 0x50d,0x00);
    ret += clock_write(0, 0x50e,0x00);
    ret += clock_write(0, 0x50f,0x00);
    ret += clock_write(0, 0x510,0x00);
    ret += clock_write(0, 0x511,0x00);
    ret += clock_write(0, 0x512,0x00);
    ret += clock_write(0, 0x513,0x00);
    ret += clock_write(0, 0x514,0x00);
    ret += clock_write(0, 0x515,0x00);
    ret += clock_write(0, 0x520,0x81);
    ret += clock_write(0, 0x521,0x13);
    ret += clock_write(0, 0x522,0x47);
    ret += clock_write(0, 0x523,0x00);
    ret += clock_write(0, 0x524,0x02);
    ret += clock_write(0, 0x525,0x00);
    ret += clock_write(0, 0x526,0x00);
    ret += clock_write(0, 0x527,0x10);
    ret += clock_write(0, 0x528,0x06);
    ret += clock_write(0, 0x529,0x00);
    ret += clock_write(0, 0x52a,0x00);
    ret += clock_write(0, 0x52b,0xC6);
    ret += clock_write(0, 0x52c,0x9F);
    ret += clock_write(0, 0x52d,0x01);
    ret += clock_write(0, 0x52e,0x00);
    ret += clock_write(0, 0x540,0x00);
    ret += clock_write(0, 0x541,0xF4);
    ret += clock_write(0, 0x542,0x01);
    ret += clock_write(0, 0x543,0x00);
    ret += clock_write(0, 0x544,0x82);
    ret += clock_write(0, 0x545,0x07);
    ret += clock_write(0, 0x546,0x00);
    ret += clock_write(0, 0x547,0x03);
    ret += clock_write(0, 0x548,0x00);
    ret += clock_write(0, 0x549,0x00);
    ret += clock_write(0, 0x54a,0x13);
    ret += clock_write(0, 0x54b,0x00);
    ret += clock_write(0, 0x54c,0x00);
    ret += clock_write(0, 0x54d,0x00);
    ret += clock_write(0, 0x54e,0xF4);
    ret += clock_write(0, 0x54f,0x01);
    ret += clock_write(0, 0x550,0x00);
    ret += clock_write(0, 0x551,0x82);
    ret += clock_write(0, 0x552,0x07);
    ret += clock_write(0, 0x553,0x00);
    ret += clock_write(0, 0x554,0x03);
    ret += clock_write(0, 0x555,0x00);
    ret += clock_write(0, 0x556,0x00);
    ret += clock_write(0, 0x557,0x13);
    ret += clock_write(0, 0x558,0x00);
    ret += clock_write(0, 0x559,0x00);
    ret += clock_write(0, 0x55a,0x00);
    ret += clock_write(0, 0x55b,0xF4);
    ret += clock_write(0, 0x55c,0x01);
    ret += clock_write(0, 0x55d,0x00);
    ret += clock_write(0, 0x55e,0x4F);
    ret += clock_write(0, 0x55f,0x07);
    ret += clock_write(0, 0x560,0x00);
    ret += clock_write(0, 0x561,0x50);
    ret += clock_write(0, 0x562,0x7C);
    ret += clock_write(0, 0x563,0x00);
    ret += clock_write(0, 0x564,0xEF);
    ret += clock_write(0, 0x565,0xE7);
    ret += clock_write(0, 0x566,0x00);
    ret += clock_write(0, 0x567,0x00);
    ret += clock_write(0, 0x568,0xF4);
    ret += clock_write(0, 0x569,0x01);
    ret += clock_write(0, 0x56a,0x00);
    ret += clock_write(0, 0x56b,0x4C);
    ret += clock_write(0, 0x56c,0x07);
    ret += clock_write(0, 0x56d,0x00);
    ret += clock_write(0, 0x56e,0x45);
    ret += clock_write(0, 0x56f,0xD9);
    ret += clock_write(0, 0x570,0x00);
    ret += clock_write(0, 0x571,0x27);
    ret += clock_write(0, 0x572,0x32);
    ret += clock_write(0, 0x573,0x01);
    ret += clock_write(0, 0x800,0x24);
    ret += clock_write(0, 0x801,0x8C);
    ret += clock_write(0, 0x802,0x49);
    ret += clock_write(0, 0x803,0x55);
    ret += clock_write(0, 0x804,0xC9);
    ret += clock_write(0, 0x805,0x7B);
    ret += clock_write(0, 0x806,0x9C);
    ret += clock_write(0, 0x807,0xFA);
    ret += clock_write(0, 0x808,0x55);
    ret += clock_write(0, 0x809,0xEA);
    ret += clock_write(0, 0x80a,0xE2);
    ret += clock_write(0, 0x80b,0x57);
    ret += clock_write(0, 0x80c,0x8C);
    ret += clock_write(0, 0x80d,0xAD);
    ret += clock_write(0, 0x80e,0x4C);
    ret += clock_write(0, 0x80f,0xF5);
    ret += clock_write(0, 0x810,0xCB);
    ret += clock_write(0, 0x811,0x73);
    ret += clock_write(0, 0x812,0x24);
    ret += clock_write(0, 0x813,0xD8);
    ret += clock_write(0, 0x814,0x59);
    ret += clock_write(0, 0x815,0xD2);
    ret += clock_write(0, 0x816,0x8D);
    ret += clock_write(0, 0x817,0x5A);
    ret += clock_write(0, 0xa00,0x00);
    ret += clock_write(0, 0xa01,0x0C);
    ret += clock_write(0, 0xa02,0x00);
    ret += clock_write(0, 0xa03,0x00);
    ret += clock_write(0, 0xa04,0x00);
    ret += clock_write(0, 0xa05,0x00);
    ret += clock_write(0, 0xa06,0x00);
    ret += clock_write(0, 0xa07,0x00);
    ret += clock_write(0, 0xa08,0x00);
    ret += clock_write(0, 0xa09,0x00);
    ret += clock_write(0, 0xa0a,0x00);
    ret += clock_write(0, 0xa0b,0x00);
    ret += clock_write(0, 0xa0c,0x00);
    ret += clock_write(0, 0xa0d,0x00);
    ret += clock_write(0, 0xa0e,0x00);
    ret += clock_write(0, 0xa20,0x00);
    ret += clock_write(0, 0xa21,0x00);
    ret += clock_write(0, 0xa22,0x00);
    ret += clock_write(0, 0xa23,0x00);
    ret += clock_write(0, 0xa24,0x00);
    ret += clock_write(0, 0xa40,0x01);
    ret += clock_write(0, 0xa41,0x0F);
    ret += clock_write(0, 0xa42,0x00);
    ret += clock_write(0, 0xa43,0x00);
    ret += clock_write(0, 0xa44,0x00);
    ret += clock_write(0, 0xd00,0x00);
    ret += clock_write(0, 0xd01,0x00);
    ret += clock_write(0, 0xd02,0x00);
    ret += clock_write(0, 0xd03,0x00);
    ret += clock_write(0, 0xd04,0x00);
    ret += clock_write(0, 0xd05,0x00);
    ret += clock_write(0, 0xd08,0x00);
    ret += clock_write(0, 0xd09,0x00);
    ret += clock_write(0, 0xd0a,0x00);
    ret += clock_write(0, 0xd0b,0x00);
    ret += clock_write(0, 0xd0c,0x00);
    ret += clock_write(0, 0xd0d,0x00);
    ret += clock_write(0, 0xd0e,0x00);
    ret += clock_write(0, 0xd0f,0x00);
    ret += clock_write(0, 0xd10,0x00);
    ret += clock_write(0, 0xd20,0x00);
    ret += clock_write(0, 0xd21,0x00);
    ret += clock_write(0, 0xd22,0x00);
    ret += clock_write(0, 0xd23,0x00);
    ret += clock_write(0, 0xd24,0x00);
    ret += clock_write(0, 0xd25,0x00);
    ret += clock_write(0, 0xd26,0x00);
    ret += clock_write(0, 0xd27,0x00);
    ret += clock_write(0, 0xd28,0x00);
    ret += clock_write(0, 0xd29,0x00);
    ret += clock_write(0, 0xd2a,0x00);
    ret += clock_write(0, 0xd40,0x00);
    ret += clock_write(0, 0xd41,0x00);
    ret += clock_write(0, 0xd42,0x00);
    ret += clock_write(0, 0xd43,0x00);
    ret += clock_write(0, 0xd44,0x00);
    ret += clock_write(0, 0xd45,0x00);
    ret += clock_write(0, 0xd46,0x00);
    ret += clock_write(0, 0xd47,0x00);
    ret += clock_write(0, 0xd48,0x00);
    ret += clock_write(0, 0xd49,0x00);
    ret += clock_write(0, 0xd4a,0x00);
    ret += clock_write(0, 0xe00,0x00);
    ret += clock_write(0, 0xe01,0x00);
    ret += clock_write(0, 0xe02,0x00);
    ret += clock_write(0, 0xe03,0x00);
    ret += clock_write(0, 0xe10,0x98);
    ret += clock_write(0, 0xe11,0x01);
    ret += clock_write(0, 0xe12,0x00);
    ret += clock_write(0, 0xe13,0x0E);
    ret += clock_write(0, 0xe14,0x12);
    ret += clock_write(0, 0xe15,0x01);
    ret += clock_write(0, 0xe16,0x00);
    ret += clock_write(0, 0xe17,0x07);
    ret += clock_write(0, 0xe18,0x02);
    ret += clock_write(0, 0xe19,0x00);
    ret += clock_write(0, 0xe1a,0x80);
    ret += clock_write(0, 0xe1b,0x1A);
    ret += clock_write(0, 0xe1c,0x03);
    ret += clock_write(0, 0xe1d,0x00);
    ret += clock_write(0, 0xe1e,0x1A);
    ret += clock_write(0, 0xe1f,0x03);
    ret += clock_write(0, 0xe20,0x20);
    ret += clock_write(0, 0xe21,0x1A);
    ret += clock_write(0, 0xe22,0x03);
    ret += clock_write(0, 0xe23,0x40);
    ret += clock_write(0, 0xe24,0x1A);
    ret += clock_write(0, 0xe25,0x03);
    ret += clock_write(0, 0xe26,0x60);
    ret += clock_write(0, 0xe27,0x15);
    ret += clock_write(0, 0xe28,0x04);
    ret += clock_write(0, 0xe29,0x00);
    ret += clock_write(0, 0xe2a,0x0E);
    ret += clock_write(0, 0xe2b,0x04);
    ret += clock_write(0, 0xe2c,0x20);
    ret += clock_write(0, 0xe2d,0x33);
    ret += clock_write(0, 0xe2e,0x04);
    ret += clock_write(0, 0xe2f,0x40);
    ret += clock_write(0, 0xe30,0x15);
    ret += clock_write(0, 0xe31,0x05);
    ret += clock_write(0, 0xe32,0x00);
    ret += clock_write(0, 0xe33,0x0E);
    ret += clock_write(0, 0xe34,0x05);
    ret += clock_write(0, 0xe35,0x20);
    ret += clock_write(0, 0xe36,0x33);
    ret += clock_write(0, 0xe37,0x05);
    ret += clock_write(0, 0xe38,0x40);
    ret += clock_write(0, 0xe39,0x17);
    ret += clock_write(0, 0xe3a,0x08);
    ret += clock_write(0, 0xe3b,0x00);
    ret += clock_write(0, 0xe3c,0x0E);
    ret += clock_write(0, 0xe3d,0x0A);
    ret += clock_write(0, 0xe3e,0x00);
    ret += clock_write(0, 0xe3f,0x04);
    ret += clock_write(0, 0xe40,0x0A);
    ret += clock_write(0, 0xe41,0x20);
    ret += clock_write(0, 0xe42,0x04);
    ret += clock_write(0, 0xe43,0x0A);
    ret += clock_write(0, 0xe44,0x40);
    ret += clock_write(0, 0xe45,0x80);
    ret += clock_write(0, 0xe46,0xA0);
    ret += clock_write(0, 0xe47,0xFF);
    ret += clock_write(0, 0xe48,0x00);
    ret += clock_write(0, 0xe49,0x00);
    ret += clock_write(0, 0xe4a,0x00);
    ret += clock_write(0, 0xe4b,0x00);
    ret += clock_write(0, 0xe4c,0x00);
    ret += clock_write(0, 0xe4d,0x00);
    ret += clock_write(0, 0xe4e,0x00);
    ret += clock_write(0, 0xe4f,0x00);    
    
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

    /* step 12: enable DPLL interrupt */   ///TODO:
#if 1
    ret += clock_write(0, 0x0102, 0x90);
    ret += clock_write(0, 0x010d, 0x0f);
    ret += clock_write(0, 0x010e, 0x60);
    ret += clock_write(0, 0x0110, 0x0f);
    ret += clock_write(0, 0x0111, 0x60);

    ret += clock_write(0, 0x040b, 0xd0);
    ret += clock_write(0, 0x040c, 0x07);

    ret += clock_write(0, 0x050b, 0xd0);
    ret += clock_write(0, 0x050c, 0x07);
#endif
    /* step 12 clear DPLL interrupt */
    ret += clock_write( 0, 0x0a05, 0x1);
    ret += clock_write( 0, 0x0005, 0x1);
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm config clock end, ret val: %d.", ret);
    LCM_LOG_BRINGUP("Lcm config clock end, ret val: %d.", ret);

    return ret;
}

#ifndef _CTC_ARM_CTC5236_
static int32
_lcm_init_e530_24x2c_d_clock(void)
{
    spi_gen_t  spi_gen[E530_24X2C_D_CLOCK_NUM];
    clock_info_t clock_info[E530_24X2C_D_CLOCK_NUM];
    int32 ret = 0;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init clock begin.");
    LCM_LOG_BRINGUP("Lcm init clock begin.");    
    sal_memset(spi_gen, 0, sizeof(spi_gen));
    sal_memset(clock_info, 0, sizeof(clock_info));

    spi_gen[0].spi_type = E_SPI_GPIO; /* for E_SPI_GPIO need to insmod /dev/ctc_gpio_spi fisrt */
    clock_info[0].clock_chip_type = CLOCK_AD9559;
    clock_info[0].clock_io_type = E_CLOCK_SPI;
    clock_info[0].clock_gen = (void *)&spi_gen[0];
    
    /* Init CLOCK */
    ret = clock_init(clock_info, E530_24X2C_D_CLOCK_NUM);
    if ( ret < 0 )
    {
    	LCM_LOG_ERR("Init clock failed.");
    	return LCM_E_HW_CLOCK;
    }

    LCM_IF_ERROR_RETURN(_lcm_e530_24x2c_d_config_clock());
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init clock end.");
    LCM_LOG_BRINGUP("Lcm init clock end.");    
    return LCM_E_SUCCESS;
}

#else
static int32
_lcm_init_e530_24x2c_d_clock(void)
{
    spi_gen_t  spi_gen[E530_24X2C_D_CLOCK_NUM];
    clock_info_t clock_info[E530_24X2C_D_CLOCK_NUM];
    int32 ret = 0;
    uint32 val = 0;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init clock begin.");
    LCM_LOG_BRINGUP("Lcm init clock begin.");    
    sal_memset(spi_gen, 0, sizeof(spi_gen));
    sal_memset(clock_info, 0, sizeof(clock_info));

    spi_gen[0].spi_type = E_SPI_CPU;
    spi_gen[0].spi_info.spi_cpu_info.fd = 0;
    spi_gen[0].spi_info.spi_cpu_info.chip_sel = SPI_CPU_CHIP_SEL_1;
    clock_info[0].clock_chip_type = CLOCK_AD9559;
    clock_info[0].clock_io_type = E_CLOCK_SPI;
    clock_info[0].clock_gen = (void *)&spi_gen[0];
    
    /* Init CLOCK */
    ret = clock_init(clock_info, E530_24X2C_D_CLOCK_NUM);
    if ( ret < 0 )
    {
    	LCM_LOG_ERR("Init clock failed.");
    	return LCM_E_HW_CLOCK;
    }

    LCM_IF_ERROR_RETURN(_lcm_e530_24x2c_d_config_clock());

    clock_read(0, 0x0d01, &val);
    if (val == 0xff)
    {
        epld_item_write(0, EPLD_DPLL_INT_MASK, 0x1); 
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init clock end.");
    LCM_LOG_BRINGUP("Lcm init clock end.");    
    return LCM_E_SUCCESS;
}
#endif

static int32
_lcm_init_e530_24x2c_d_phy(void)
{
    uint16 port_id;
    phy_info_t phyinfo;
    phy_handle_t** pphdl = NULL;
    glb_port_t* p_port;
    int32 ret;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init phy begin.");
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
        phyinfo.phy_device_type = PORT_PHY_NULL;
        if (port_id < 24)
        {
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_SFP_PLUS;
        }
        else
        {
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_QSFP_PLUS;          
        } 
        if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)
            ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
            ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G))
        {
            phyinfo.phy_manage_info.fec_en = GLB_FEC_DEFAULT|GLB_FEC_RS;
        }
        else
        {
            phyinfo.phy_manage_info.fec_en = GLB_FEC_DEFAULT|GLB_FEC_OFF;
        }
        lcm_common_get_defaultspeed_from_speedability(p_port->port_speed_ability, 
            &(phyinfo.phy_manage_info.speed), &(phyinfo.phy_manage_info.duplex));
        if(p_port->port_speed_ability & 
            (GLB_SUPPORT_SPEED_100G | GLB_SUPPORT_SPEED_40G |GLB_SUPPORT_SPEED_25G |GLB_SUPPORT_SPEED_50G))
        {
            phyinfo.port_info.serdes_num = 4; /* added by tongzb for bug 44368, support loopback on CNOS */
            if ((phyinfo.phy_manage_info.speed==GLB_SPEED_AUTO)||(phyinfo.phy_manage_info.duplex==GLB_DUPLEX_AUTO))
            {
                phyinfo.phy_manage_info.cl73_enable = 1;
            }
            else
            {
                phyinfo.phy_manage_info.cl73_enable = 0;
            }
            if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
                phyinfo.port_info.serdes_num = 2;
            if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)
                phyinfo.port_info.serdes_num = 1;
        }
        else
        {
            phyinfo.port_info.serdes_num = 1;/* added by tongzb for bug 44368, support loopback on CNOS */
            phyinfo.phy_manage_info.cl73_enable = 0;
        }
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

    LCM_LOG_DEBUG(LCM_CARD, "Lcm ready to init phy.");
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

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init phy end.");
    LCM_LOG_BRINGUP("Lcm init phy end.");    
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_e530_24x2c_d_i2c(void)
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
_lcm_init_e530_24x2c_d_led(void)
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
    led_info[E_FAN_LED].exist = 0;
    led_info[E_FAN_LED].led_ctl = E_LED_EPLD0;
    led_info[E_FAN_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ALARM].val = 0x2;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_FAN_LED;
    led_info[E_FAN_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;
    led_info[E_PSU1_LED].exist = 0;
    led_info[E_PSU1_LED].led_ctl = E_LED_EPLD0;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ALARM].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ALARM].val = 0x2;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ABSENT].op_reg = EPLD_PSU1_LED;
    led_info[E_PSU1_LED].led_para[E_LED_SYS_ABSENT].val = 0x0;
    led_info[E_PSU2_LED].exist = 0;
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
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].val = 0x1;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].op_reg = EPLD_IND_LED;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].val = 0x0;
	
    ret = led_init(led_info);
    if(ret < 0)
    {
        LCM_LOG_ERR("Init LEDs failed.");
        return LCM_E_HW_LED_GEN;
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init LED end.");
    LCM_LOG_BRINGUP( "Lcm init LED end.");    

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_d_ctc_chip_led(void)
{
    uint8 split_flag=0;
    uint8 table_id;
    uint8 table_num;
    uint8 src_pos, des_pos;
    uint8 port_id;
    mac_led_info_t mac_led_info;
    mac_led_api_para_t glb_lcm_mac_led[E530_24X2C_D_LED_TBL_NUM][E530_24X2C_D_LED_MAC_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init MAC LED begin.");
    LCM_LOG_BRINGUP("Lcm init MAC LED begin.");

    split_flag = glb_card->split_flag;
    if(split_flag)
    {
        table_num = E530_24X2C_D_LED_TBL_NUM;
    }
    else
    {
        table_num = 1;
    }
    
    /* if any port split:
     * table0 : for fan out led : led0 force on and led1,2,3 force off
     *          for 40G port led: if not split,led status depend on 40G mac of this port
     *                            if split, led status depend on subport 1 mac of this port
     *          for 10G port led: if work on sfp+ media, then normal
     *                            if work on qsfp+ media, then led status depend on 
     *                               subport 1 mac of this port and port 1 40G led force off
     * table1 : for fan out led : led1 force on and led0,2,3 force off
     *          for 40G port led: if not split,led status depend on 40G mac of this port
     *                            if split, led status depend on subport 2 mac of this port
     *          for 10G port led: if work on sfp+ media, then normal
     *                            if work on qsfp+ media, then led status depend on 
     *                               subport 2 mac of this port and port 1 40G led force off
     * table2 : for fan out led : led2 force on and led0,1,3 force off
     *          for 40G port led: if not split,led status depend on 40G mac of this port
     *                            if split, led status depend on subport 3 mac of this port
     *          for 10G port led: if work on sfp+ media, then normal
     *                            if work on qsfp+ media, then led status depend on 
     *                               subport 3 mac of this port and port 1 40G led force off
     * table3 : for fan out led : led3 force on and led0,1,2 force off
     *          for 40G port led: if not split,led status depend on 40G mac of this port
     *                            if split, led status depend on subport 4 mac of this port
     *          for 10G port led: if work on sfp+ media, then normal
     *                            if work on qsfp+ media, then led status depend on 
     *                               subport 4 mac of this port and port 1 40G led force off
     * if no port split:
     * table0   : for fan out led  : all force off
     *          : for 40G port led : led status depend on 40G mac of this port
     *          : for 10G port led : all force off
     */
    for(table_id = 0; table_id < table_num; table_id++)
    {
        sal_memcpy(glb_lcm_mac_led[table_id], lcm_e530_24x2c_d_mac_led_default_entry, 
            sizeof(mac_led_api_para_t)*E530_24X2C_D_LED_MAC_NUM);

        for(port_id=0; port_id<E530_24X2C_D_PANEL_PORT_NUM; port_id++)
        {
            if (GLB_PPT_NUMCHG_TYPE_10G == glb_card->pp_ppt[port_id]->ppt_numchg_type
                || GLB_PPT_NUMCHG_TYPE_25G == glb_card->pp_ppt[port_id]->ppt_numchg_type)
            {
                des_pos = port_id;
                src_pos = 24 + (port_id -24) * 4 + table_id;
                glb_lcm_mac_led[table_id][des_pos].port_id = lcm_e530_24x2c_d_port_max[src_pos].logic_port_idx;
            }
            else if(GLB_PPT_NUMCHG_TYPE_50G == glb_card->pp_ppt[port_id]->ppt_numchg_type)
            {
                des_pos = port_id;
                /* only panel port 25 support split 50G */
                if((table_id == 0)||(table_id == 1))
                {
                    src_pos = 24 + (port_id-24)*4 + 1;
                }
                else
                {
                    src_pos = 24 + (port_id-24)*4 + 3;
                }
                glb_lcm_mac_led[table_id][des_pos].port_id = lcm_e530_24x2c_d_port_max[src_pos].logic_port_idx;
            }
        }

        /* When any port split, light on fan out LED periodically */
        if(split_flag)
        {
            /* channel 1: force on fan out led
             * channel 0: force off fan out led
             */
            src_pos = 24 + 2 + table_id;
            glb_lcm_mac_led[table_id][src_pos].mode = LED_MODE_1_FORCE_ON;
        }        
    }

    mac_led_info.mac_led_api_para = (mac_led_api_para_t **)XCALLOC(CTCLIB_MEM_LCM_MODULE, 
        sizeof(mac_led_api_para_t*)*table_num);
    if(NULL == mac_led_info.mac_led_api_para)
    {
        LCM_LOG_ERR("alloc mac_led_api_para_t array fail.");
        return LCM_E_NO_MEMORY;
    }

    for(table_id = 0; table_id < table_num; table_id++)
    {
        mac_led_info.mac_led_api_para[table_id] = glb_lcm_mac_led[table_id];
    }
    mac_led_info.table_num = table_num;
    mac_led_info.mac_num = E530_24X2C_D_LED_MAC_NUM;
    mac_led_info.slice0_mac_num = E530_24X2C_D_SLICE0_LED_MAC_NUM;
    mac_led_info.polarity = 1; /* 1:for low driver led, 0:for high driver led */
    mac_led_info_register(&mac_led_info);
    mac_led_init();


    XFREE(CTCLIB_MEM_LCM_MODULE, mac_led_info.mac_led_api_para);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init MAC LED end.");
    LCM_LOG_BRINGUP("Lcm init MAC LED end.");        
    return LCM_E_SUCCESS;
}

int32
lcm_e530_24x2c_d_get_sum_ffe_cfg(lcm_chip_serdes_ffe_t* serdes_ffe)
{
    uint8 serdes_id;
    uint8 index;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;
    uint8 default_trace_len;

    serdes_id = serdes_ffe->serdes_id;
    serdes_ffe->board_material = glb_card->board_material;

    for(index = 0; index < E530_24X2C_D_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_24x2c_d_serdes_ffe_max[index];
        if(serdes_id == lcm_serdes_ffe->serdes_id)
        {
            if(serdes_ffe->board_material == BOARD_MATERIAL_M4) /* default board material */
            {
                default_trace_len = lcm_serdes_ffe->trace_len;
            }
            else
            {
                default_trace_len = lcm_serdes_ffe->trace_len2;
            }
            if (0xff == default_trace_len) 
            {
                serdes_ffe->mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;  /*user-defined*/
                if(index < 24)
                {
                    serdes_ffe->c0 = lcm_serdes_ffe->coeff1.c0;
                    serdes_ffe->c1 = lcm_serdes_ffe->coeff1.c1;
                    serdes_ffe->c2 = lcm_serdes_ffe->coeff1.c2;
                    serdes_ffe->c3 = lcm_serdes_ffe->coeff1.c3;
                }
                else
                {
                    if(serdes_ffe->is_DAC)
                    {
                        if((serdes_ffe->speed == GLB_SPEED_100G)||(serdes_ffe->speed == GLB_SPEED_25G))
                        {
                            serdes_ffe->c0 = lcm_serdes_ffe->coeff3.c0;
                            serdes_ffe->c1 = lcm_serdes_ffe->coeff3.c1;
                            serdes_ffe->c2 = lcm_serdes_ffe->coeff3.c2;
                            serdes_ffe->c3 = lcm_serdes_ffe->coeff3.c3;
                        }
                        else if((serdes_ffe->speed == GLB_SPEED_40G)||(serdes_ffe->speed == GLB_SPEED_10G))
                        {
                            serdes_ffe->c0 = lcm_serdes_ffe->coeff4.c0;
                            serdes_ffe->c1 = lcm_serdes_ffe->coeff4.c1;
                            serdes_ffe->c2 = lcm_serdes_ffe->coeff4.c2;
                            serdes_ffe->c3 = lcm_serdes_ffe->coeff4.c3;
                        }
                        else
                        {
                            serdes_ffe->c0 = lcm_serdes_ffe->coeff3.c0;
                            serdes_ffe->c1 = lcm_serdes_ffe->coeff3.c1;
                            serdes_ffe->c2 = lcm_serdes_ffe->coeff3.c2;
                            serdes_ffe->c3 = lcm_serdes_ffe->coeff3.c3;
                        }                        
                    }
                    else
                    {
                        if((serdes_ffe->speed == GLB_SPEED_100G)||(serdes_ffe->speed == GLB_SPEED_25G))
                        {
                            serdes_ffe->c0 = lcm_serdes_ffe->coeff1.c0;
                            serdes_ffe->c1 = lcm_serdes_ffe->coeff1.c1;
                            serdes_ffe->c2 = lcm_serdes_ffe->coeff1.c2;
                            serdes_ffe->c3 = lcm_serdes_ffe->coeff1.c3;
                        }
                        else if((serdes_ffe->speed == GLB_SPEED_40G)||(serdes_ffe->speed == GLB_SPEED_10G))
                        {
                            serdes_ffe->c0 = lcm_serdes_ffe->coeff2.c0;
                            serdes_ffe->c1 = lcm_serdes_ffe->coeff2.c1;
                            serdes_ffe->c2 = lcm_serdes_ffe->coeff2.c2;
                            serdes_ffe->c3 = lcm_serdes_ffe->coeff2.c3;
                        }
                        else
                        {
                            serdes_ffe->c0 = lcm_serdes_ffe->coeff1.c0;
                            serdes_ffe->c1 = lcm_serdes_ffe->coeff1.c1;
                            serdes_ffe->c2 = lcm_serdes_ffe->coeff1.c2;
                            serdes_ffe->c3 = lcm_serdes_ffe->coeff1.c3;
                        }
                    }
                }
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

static int32
_lcm_init_e530_24x2c_d_serdes_mode(void)
{
    uint8 pos = 0;
    uint8 panel_port = 0;
    int32 ret = 0;
#if (defined DUET2)||(defined TSINGMA)
    uint16 gport;
    ctc_port_if_mode_t if_mode;
    ctc_port_speed_t     speed;
    ctc_port_if_type_t   if_type;    
#endif
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes mode begin.");
    LCM_LOG_BRINGUP("Lcm init serdes mode begin.");

    for(panel_port = 0; panel_port < glb_card->panel_port_num; panel_port++)
    {
        if (panel_port < 24)
        {
            continue;
        }
        else
        {
            /* Modified by liuht for bug 32804, 2015-04-15 */
            /* disable all 40G/100G Mac for serdes mode switch */
            if(panel_port == 24)
                pos = 24 + (panel_port-24)*4+1;
            else if(panel_port == 25)
                pos = 24 + (panel_port-24)*4+0;

            ctc_port_set_mac_en(lcm_e530_24x2c_d_port_max[pos].logic_port_idx, 0);
            if(GLB_PPT_OP_NONE == glb_card->pp_ppt[panel_port]->op)
            {
                continue;
            }
            else if(GLB_PPT_OP_SPLIT == glb_card->pp_ppt[panel_port]->op)
            {  
#if (defined DUET2)||(defined TSINGMA)
                if(glb_card->pp_ppt[panel_port]->ppt_numchg_type == GLB_PPT_NUMCHG_TYPE_10G)
                { 
                    speed = CTC_PORT_SPEED_10G;
                    if_type = CTC_PORT_IF_XFI; 
                }
                else if(glb_card->pp_ppt[panel_port]->ppt_numchg_type == GLB_PPT_NUMCHG_TYPE_1G)
                { 
                    speed = CTC_PORT_SPEED_1G;
                    if_type = CTC_PORT_IF_SGMII; 
                }
                else if(glb_card->pp_ppt[panel_port]->ppt_numchg_type == GLB_PPT_NUMCHG_TYPE_25G)
                { 
                    speed = CTC_PORT_SPEED_25G;
                    if_type = CTC_PORT_IF_CR; 
                }
                else if(glb_card->pp_ppt[panel_port]->ppt_numchg_type == GLB_PPT_NUMCHG_TYPE_50G)
                { 
                    speed = CTC_PORT_SPEED_50G;
                    if_type = CTC_PORT_IF_CR2; 
                }                
                else
                {
                    continue;
                }   
                sal_memset(&if_mode, 0, sizeof(ctc_port_if_mode_t));
                gport = lcm_e530_24x2c_d_port_max[pos ].logic_port_idx;
                if_mode.speed = speed;
                if_mode.interface_type = if_type;
                ret += ctc_port_set_interface_mode(gport, &if_mode);   
#endif                
            }
        }
    }    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes mode end.");
    LCM_LOG_BRINGUP("Lcm init serdes mode end. ret %d",ret);    
    return ret;
}

int32
_lcm_init_e530_24x2c_d_serdes_ffe(void)
{
    ctc_chip_serdes_ffe_t serdes_ffe;
    uint8 index;
    int32 ret = 0;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;        
    uint8 default_trace_len;
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes ffe begin.");
    LCM_LOG_BRINGUP("Lcm init serdes ffe begin.");    

    sal_memset(&serdes_ffe, 0, sizeof(ctc_chip_serdes_ffe_t));
    for(index = 0; index < E530_24X2C_D_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_24x2c_d_serdes_ffe_max[index];
        if(glb_card->board_material == BOARD_MATERIAL_M4)
        {
            default_trace_len = lcm_serdes_ffe->trace_len;
        }
        else
        {
            default_trace_len = lcm_serdes_ffe->trace_len2;
        }
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

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes ffe end.");
    LCM_LOG_BRINGUP("Lcm init serdes ffe end. ret %d",ret);    
    
    return ret;
}

int32
lcm_init_e530_24x2c_d_cb(void)
{
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init callback begin.");
    LCM_LOG_BRINGUP("Lcm init callback begin.");
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_fiber());    
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_serdes_mode());/* for set    default auto-neg */
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_phy());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_ctc_chip_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_serdes_ffe());
    clock_write(0,0xa,0xc0);/* disable Loss-of-Signal Interrupt */
    clock_write(0,0xad,0x00);/* disable Loss-of-Signal Interrupt */
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init callback end.");
    LCM_LOG_BRINGUP( "Lcm init callback end.");    

    return LCM_E_SUCCESS;
}

int32 
lcm_init_e530_24x2c_d_syncE_enable(uint32 port_idx, uint32 status)
{
    glb_card_t* p_card = NULL;
    ctc_sync_ether_cfg_t sync_ether_cfg;
    int32 ret = LCM_E_SUCCESS;
    static int prev_port_idx = -1;
    static int prev_port_speed = -1;
    uint8 clock_id = 0;

    p_card= lcm_mgt_get_card();   
    sal_memset(&sync_ether_cfg, 0x0, sizeof(ctc_sync_ether_cfg_t));

    if(status == TRUE)
    {
        LCM_LOG_DEBUG(LCM_CARD, "Lcm set synce enable, prev_port_idx %d, prev_port_speed %d." ,prev_port_idx,prev_port_speed);
        if (prev_port_idx == -1)/* this is the first syncE port */
        {
            /* update pre synce port info */
            prev_port_idx = port_idx;
            prev_port_speed = p_card->pp_port[port_idx]->port_status.speed;
        }
        else if (prev_port_idx != -1) /* this is not the first syncE port, need disable clock */
        {
            //clock_write(0,0xc7,0x00);/* disable Loss-of-Signal Interrupt */
            //sync-ether 0 clock 0 recovered-port 0 divider 4 output disable link-status-detect disable 
            if ((GLB_SPEED_25G == prev_port_speed)
                ||(GLB_SPEED_50G == prev_port_speed)
                ||(GLB_SPEED_100G == prev_port_speed))
            {
                clock_id = 1;
            }
            sync_ether_cfg.divider = 0xa4;/* 1.95Mhz */
            sync_ether_cfg.clock_output_en = FALSE;
            sync_ether_cfg.link_status_detect_en = TRUE;
            sync_ether_cfg.recovered_clock_lport = p_card->pp_port[prev_port_idx]->logic_port_idx;
            ret = ctc_sync_ether_set_cfg(0, clock_id, &sync_ether_cfg);
            LCM_LOG_DEBUG(LCM_CARD, "Lcm set synce disable, port_idx %d, port_speed %d.", prev_port_idx,prev_port_speed);
            /* update pre synce port info */
            prev_port_idx = port_idx;
            prev_port_speed = p_card->pp_port[port_idx]->port_status.speed;
        }
        sal_udelay(5000);
        sal_memset(&sync_ether_cfg, 0x0, sizeof(ctc_sync_ether_cfg_t));
        
        //clock_intr_fixup(0);/* clear interrupt status */
        //sync-ether 0 clock 0 recovered-port 0 divider 4 output enable link-status-detect disable 
        if (GLB_SPEED_1G == prev_port_speed)  //1G mode
        {
            sync_ether_cfg.divider = 0x3f; /* 1.953125M */
            clock_id = 0;
        }
        else if ((GLB_SPEED_10G == prev_port_speed)  //10G mode
               ||(GLB_SPEED_40G == prev_port_speed))  //40G mode
        {
            sync_ether_cfg.divider = 0xa4; /* 1.953125M */
            clock_id = 0;
        }
        else if ((GLB_SPEED_25G == prev_port_speed)  //25G mode
               ||(GLB_SPEED_50G == prev_port_speed)
               ||(GLB_SPEED_100G == prev_port_speed)) //100G mode
        {
            sync_ether_cfg.divider = 0x338; /* 1.953125M */
            clock_id = 1;
        }
        sync_ether_cfg.clock_output_en = status;
        sync_ether_cfg.link_status_detect_en = TRUE;
        sync_ether_cfg.recovered_clock_lport = p_card->pp_port[port_idx]->logic_port_idx;
        ret = ctc_sync_ether_set_cfg(0, clock_id, &sync_ether_cfg);
        LCM_LOG_DEBUG(LCM_CARD, "Lcm set synce enable, port_idx %d, port_speed %d." ,prev_port_idx,prev_port_speed);
        sal_udelay(5000);        
    }
    else
    {
        if (prev_port_idx == port_idx)/* if disable synce port, need clear pre port info */
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm set synce enable, prev_port_idx %d, prev_port_speed %d." ,prev_port_idx,prev_port_speed);
            sal_memset(&sync_ether_cfg, 0x0, sizeof(ctc_sync_ether_cfg_t));
            //clock_intr_fixup(0);/* clear interrupt status */
            //sync-ether 0 clock 0 recovered-port 0 divider 4 output enable link-status-detect disable 
            if (GLB_SPEED_1G == prev_port_speed)  //1G mode
            {
                sync_ether_cfg.divider = 0x3f; /* 1.953125M */
                clock_id = 0;
            }
            else if ((GLB_SPEED_10G == prev_port_speed)  //10G mode
                   ||(GLB_SPEED_40G == prev_port_speed))  //40G mode
            {
                sync_ether_cfg.divider = 0xa4; /* 1.953125M */
                clock_id = 0;
            }
            else if ((GLB_SPEED_25G == prev_port_speed)  //25G mode
                   ||(GLB_SPEED_100G == prev_port_speed)) //100G mode
            {
                sync_ether_cfg.divider = 0x338; /* 1.953125M */
                clock_id = 1;
            }
            sync_ether_cfg.clock_output_en = status;
            sync_ether_cfg.link_status_detect_en = TRUE;
            sync_ether_cfg.recovered_clock_lport = p_card->pp_port[port_idx]->logic_port_idx;
            ret = ctc_sync_ether_set_cfg(0, clock_id, &sync_ether_cfg);
            LCM_LOG_DEBUG(LCM_CARD, "Lcm set synce disable, port_idx %d, port_speed %d." ,prev_port_idx,prev_port_speed);
            sal_udelay(5000);            
            prev_port_idx = -1;
            prev_port_speed = -1;
        }
        else
        {
            return 0;
        }
    }
    sal_udelay(5000);
    sal_memset(&sync_ether_cfg, 0x0, sizeof(ctc_sync_ether_cfg_t));
    
    //clock_intr_fixup(0);/* clear interrupt status */
    //sync-ether 0 clock 0 recovered-port 0 divider 4 output enable link-status-detect disable 
    if (GLB_SPEED_1G == prev_port_speed)  //1G mode
    {
        sync_ether_cfg.divider = 0x3f; /* 1.953125M */
        clock_id = 0;
    }
    else if ((GLB_SPEED_10G == prev_port_speed)  //10G mode
           ||(GLB_SPEED_40G == prev_port_speed))  //40G mode
    {
        sync_ether_cfg.divider = 0xa4; /* 1.953125M */
        clock_id = 0;
    }
    else if ((GLB_SPEED_25G == prev_port_speed)  //25G mode
           ||(GLB_SPEED_100G == prev_port_speed)) //100G mode
    {
        sync_ether_cfg.divider = 0x338; /* 1.953125M */
        clock_id = 1;
    }
    sync_ether_cfg.clock_output_en = status;
    sync_ether_cfg.link_status_detect_en = TRUE;
    sync_ether_cfg.recovered_clock_lport = p_card->pp_port[port_idx]->logic_port_idx;
    ret = ctc_sync_ether_set_cfg(0, clock_id, &sync_ether_cfg);
    LCM_LOG_DEBUG(LCM_CARD, "Lcm set synce enable, port_idx %d, port_speed %d." ,prev_port_idx,prev_port_speed);
    sal_udelay(5000);

    if(status == TRUE)
    {
        if ((GLB_SPEED_25G == prev_port_speed)
                ||(GLB_SPEED_100G == prev_port_speed))
        {
            //clock_write(0,0xc7,0x22);/* enable Loss-of-Signal Interrupt and hold-over interrupt*/
        }
        else
        {
            //clock_write(0,0xc7,0x11);/* enable Loss-of-Signal Interrupt and hold-over interrupt*/
        }
    }
    else
    {
        //clock_write(0,0xc7,0x00);/* disable Loss-of-Signal Interrupt */
    }
    return ret;
}

extern int32 lcm_port_init_syce_enable_callback(lcm_port_syce_enable_callback_t func);
extern int32 lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func);
int32
lcm_init_e530_24x2c_d(glb_card_t* p_card)
{
    uint8 bm;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init board begin.");
    LCM_LOG_BRINGUP( "Lcm init board begin.");    
    glb_card = p_card;
    p_card->asic_chip_num = 1;
    p_card->chip_sensor_num = 1;

    lcm_common_get_slot_no(&p_card->logic_slot_no,&p_card->phy_slot_no);

    p_card->panel_port_num = E530_24X2C_D_PANEL_PORT_NUM;
    p_card->l2switch_num = 0;
    p_card->sensor_num = E530_24X2C_D_SENSOR_NUM;
    if(p_card->chip_sensor_num+p_card->sensor_num > MAX_TMPR_SENSOR_NUM)
    {
        LCM_LOG_ERR("Sensor structure is too small to store sensor info on board.\n");        
        return LCM_E_NO_MEMORY;
    }
    p_card->tmpr_limit.board_crit = DEFAULT_CRIT_TMPR;
    p_card->tmpr_limit.board_high = DEFAULT_HIGH_TMPR;
    p_card->tmpr_limit.board_low = DEFAULT_LOW_TMPR;
    
    p_card->fan_module_num = E530_24X2C_D_FAN_MODULE_MAX;
    p_card->psu_module_num = E530_24X2C_D_PSU_MODULE_MAX;
    /* Modified by liuht for bug38424 2016-05-11 */
    //p_card->volt_curr_mon_num = 3;
    p_card->cpu_type = GLB_CPU_LS1023A;
    p_card->phy_chip_num = 0;

#ifndef _GLB_UML_SYSTEM_ 
    ctc_phy_fd_init();
    p_card->dpll_num = 1;
    p_card->dpll_interrupt_mode = GLB_PHY_INTERRUPT_MODE_CPU;
    ctc_dpll_fd_init(); /* For dpll interrupt */
#endif
    p_card->phy_intr_en = 0;

    /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    p_card->ptp_en = 0;
    p_card->epld_type = GLB_EPLD_TYPE_VME;
    p_card->bootrom_type = E_BOOTROM_TYPE_512K;

    sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                    DATAPATH_PROFILE_PATH, E530_24X2C_D_DATAPATH_NAME);
    /* Fix bug29772, cr9895, qicx, 2014-08-26 */
    p_card->support_reboot_info = 1;

    lcm_common_init_cpu_info();
    lcm_common_config_ptp_enable(1);//config ptp
    lcm_common_config_syncE_enable(1);//config syncE
    lcm_common_config_reboot_record_type(REBOOT_FILE_EPLD,0xf6);
    LCM_LOG_DEBUG(LCM_CARD,   "card product type %x, board type %x.", p_card->board_type.series, p_card->board_type.type);
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_i2c());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_epld(p_card, p_card->epld_ver));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_sysenv_info(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_led());
    /* epld write should be after the i2c init */
    epld_item_write(0, EPLD_I2C_BRIDGE_RST, 0x01);
    epld_item_write(0, EPLD_ASIC_I2C_BRIDGE_RST, 0x0f);
    epld_item_write(0, EPLD_GPIO_RST, 0xf);
    epld_item_write(0, EPLD_DPLL_RST, 0x1);
    epld_item_write(0, EPLD_PPU_INT_MASK, 0x00);
    epld_item_write(0, EPLD_DPLL_INT_MASK, 0x0); 
    sal_delay(1);

    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_sysenv(p_card));

    /* 0 for FR4 board, 1 for M4 board, 2 for M6 board */
    if(lcm_common_get_board_material(&bm)) /*default value*/
    {
        p_card->board_material = BOARD_MATERIAL_FR4;
    }
    else
    {
        p_card->board_material = bm;
    }
    LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_d_gpio());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_port(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_sensor(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_volt_mon(p_card->board_type));//TODO
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_fan(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_psu(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_d_clock());

    lcm_card_init_callback(lcm_init_e530_24x2c_d_cb);
    lcm_port_init_ffe_cfg_callback(lcm_e530_24x2c_d_get_sum_ffe_cfg);
    lcm_port_init_syce_enable_callback(lcm_init_e530_24x2c_d_syncE_enable);
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init board end.");
    LCM_LOG_BRINGUP("Lcm init board end.");

    return LCM_E_SUCCESS;
}

