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
 /* system env */
#define E530_24X2C_SYSENV_NUM                   0x01
 
/* Power supply */
#define E530_24X2C_PSU_MODULE_MAX                  0x2

/* clock */
#define E530_24X2C_CLOCK_NUM                        0x0     ///TODO: 
#define E530_24X2C_ADDR0_CLOCK                      0x68

/* Fan */
/* Use SOC PWM*/
#define E530_24X2C_FAN_MODULE_MAX                  0x1

/* I2C to GPIO: PCA9505 */
#define E530_24X2C_GPIO_CHIP_NUM                   0x03
#define E530_24X2C_ADDR0_GPIO                      0x21
#define E530_24X2C_ADDR1_GPIO                      0x22
#define E530_24X2C_ADDR2_GPIO                      0x23

#define E530_24X2C_PANEL_PORT_NUM_MAX              (24+2*4)
#define E530_24X2C_PANEL_PORT_NUM                  (24+2)
#define E530_24X2C_FIBER_PORT_NUM                  (24+2)

#define E530_24X2C_SLICE0_LED_MAC_NUM              (24+2+4)
#define E530_24X2C_LED_MAC_NUM                     (24+2+4)
#define E530_24X2C_LED_TBL_NUM                     4

#define GLB_I2C_IDX GLB_I2C_IDX_0

/****************************************************************************
*
* Global and Declarations
*
****************************************************************************/
static const glb_card_t         *glb_card;
static lcm_card_port_t          *glb_lcm_e530_24x2c_port             = NULL;
static fiber_port_info_t        *glb_lcm_e530_24x2c_fiber            = NULL;

extern int32 lcm_card_init_callback(card_init_callback_t func);
extern int32
lcm_common_get_defaultfec_from_speedability(uint16 speed_ability, uint32*  fec_ability);

lcm_card_port_t lcm_e530_24x2c_port_max[E530_24X2C_PANEL_PORT_NUM_MAX] =
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

    /* for lane swap port, mac need swap */
    { 0,  61,  61,},    /* Port 29 */
    { 0,  60,  60,},    /* Port 30 */
    { 0,  63,  63,},    /* Port 31 */
    { 0,  62,  62,},    /* Port 32 */
    { 0,  45,  45,},    /* Port 25 */
    { 0,  44,  44,},    /* Port 26 */
    { 0,  47,  47,},    /* Port 27 */
    { 0,  46,  46,},    /* Port 28 */    
};

fiber_port_info_t lcm_e530_24x2c_fiber_max[E530_24X2C_FIBER_PORT_NUM] =
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
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,8}},{.en_gpio={0,8}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,9}},{.en_gpio={0,9}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,15}},{.en_gpio={0,10}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,14}},{.en_gpio={0,11}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,13}},{.en_gpio={0,12}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,10}},{.en_gpio={0,13}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,12}},{.en_gpio={0,14}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,11}},{.en_gpio={0,15}}},

    /* panel port 17~24 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,20}},{.en_gpio={2,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,19}},{.en_gpio={2,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,21}},{.en_gpio={2,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,18}},{.en_gpio={2,3}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,22}},{.en_gpio={2,4}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,17}},{.en_gpio={2,5}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,23}},{.en_gpio={2,6}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,16}},{.en_gpio={2,7}}},

    {E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_MAX,0,{.asic={0,24}}},
    {E_FIBER_QSFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_MAX,0,{.asic={0,25}}},    
};

/* Modified by liuht for bug 34540, 2015-09-09 */
mac_led_api_para_t lcm_e530_24x2c_mac_led_default_entry[E530_24X2C_LED_MAC_NUM]=
{
    /* fanout led, anytime there is only 1 or 0 LED is on 
     * using a NA mac 0 to send LED signal
     */  
    
    {67, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    {66, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    {65, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    {64, 0, 0, LED_MODE_1_FORCE_OFF, 0},
    
    {44, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {60, 0, 0, LED_MODE_2_FORCE_OFF, 0},

    {31, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {30, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {29, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {28, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {27, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {26, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {25, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {24, 0, 0, LED_MODE_2_FORCE_OFF, 0},

    {15, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {14, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {13, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {12, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {23, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {22, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {21, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {20, 0, 0, LED_MODE_2_FORCE_OFF, 0},   

    {11, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {10, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {9, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {8, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {3, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {2, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {1, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {0, 0, 0, LED_MODE_2_FORCE_OFF, 0},
};

/* typical ffe */
lcm_card_serdes_ffe_t 
lcm_e530_24x2c_serdes_ffe_max[E530_24X2C_PANEL_PORT_NUM_MAX]=
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
    {25, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {24, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {27, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 
    {26, 0xff, 0xff,COEFF1(63,20,60,0),COEFF2(0,15,63,0),COEFF3(58,19,0,0),COEFF4(61,22,0,0)}, 

};

/****************************************************************************
 *
* Function
*
****************************************************************************/
static int32
lcm_init_e530_24x2c_init_port_table(glb_card_t* p_card)
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
                if(panel_subport_no == 1)
                    pos = 24 + (panel_port_no-24-1) * 4 + 1;
                else if(panel_subport_no == 2)
                    pos = 24 + (panel_port_no-24-1) * 4 + 3;
            }
            else if((GLB_PPT_NUMCHG_TYPE_10G == port_numchg_type)||(GLB_PPT_NUMCHG_TYPE_25G == port_numchg_type))
            {
                pos = 24 + (panel_port_no-24-1) * 4 + panel_subport_no-1;
            }
            else if(GLB_PPT_NUMCHG_TYPE_NONE == port_numchg_type)
            {
                pos = 24 + (panel_port_no-24-1)*4+1;
            }
        }
        sal_memcpy(&glb_lcm_e530_24x2c_port[port_id], &lcm_e530_24x2c_port_max[pos], sizeof(lcm_card_port_t));
        glb_lcm_e530_24x2c_port[port_id].serdes_id = lcm_e530_24x2c_serdes_ffe_max[pos].serdes_id;
    }

    return LCM_E_SUCCESS;
}

static int32
lcm_init_e530_24x2c_init_fiber_table(glb_card_t* p_card)
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
        sal_memcpy(&glb_lcm_e530_24x2c_fiber[port_id], &lcm_e530_24x2c_fiber_max[panel_port_no-1], sizeof(fiber_port_info_t));
        glb_lcm_e530_24x2c_fiber[port_id].fiber_channel = fiber_channel;
        glb_lcm_e530_24x2c_fiber[port_id].fiber_id = port_id;
    }
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_panel_port(glb_card_t* p_card)
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
            if(GLB_BOARD_E530_24X2C == p_card->board_type.type)
            {
                p_card->pp_ppt[ppt_id]->op_ability = GLB_PPT_OP_SPLIT_10G | GLB_PPT_OP_SPLIT_25G| GLB_PPT_OP_SPLIT_50G;
                /* for bug 53396, by tongzb, can not change speed when port split, 2019-09-04 */
                p_card->pp_ppt[ppt_id]->disable_speed_change = 1;
                p_card->pp_ppt[ppt_id]->ppt_numchg_num = 1;///TODO: need optimize
                /*Bug33327, support 100G, but current default is 40G. This affect phy init, the phy default speed is 100G or 40G. */
                p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_100G|GLB_SUPPORT_SPEED_40G;
            }
            else
            {
                p_card->pp_ppt[ppt_id]->op_ability = GLB_PPT_OP_SPLIT_10G ;
                /* for bug 53280, by tongzb, can not change speed when port split, 2019-08-22 */
                p_card->pp_ppt[ppt_id]->disable_speed_change = 1;
                p_card->pp_ppt[ppt_id]->ppt_numchg_num = 1;///TODO: need optimize
                /*Bug33327, support 100G, but current default is 40G. This affect phy init, the phy default speed is 100G or 40G. */
                p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_40G;                
            }
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_40G;
        }
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init panel port end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_port(glb_card_t* p_card)
{
    int32 port_id = 0;
    uint32 ppt_idx;
    uint16 logic_port_idx;
    lcm_card_port_panel_mapping_t port_panel_mapping[256];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init port begin.");
    LCM_LOG_BRINGUP("Lcm init port begin.");
     /* 1, init panel port */
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_panel_port(p_card));
     
    /* 2, read '/mnt/flash/startup-config.conf' file, get port split/merge info */
    LCM_IF_ERROR_RETURN(lcm_common_parse_port_numchg_info(p_card));

    if(p_card->port_num != E530_24X2C_PANEL_PORT_NUM)
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
    glb_lcm_e530_24x2c_port = (lcm_card_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(lcm_card_port_t)* p_card->port_num);
    if (!glb_lcm_e530_24x2c_port)
    {
        LCM_LOG_ERR("Allocate pointer to lcm_card_port_t fail.");
        return LCM_E_NO_MEMORY;
    }
 
    glb_lcm_e530_24x2c_fiber = (fiber_port_info_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(fiber_port_info_t)*p_card->port_num);
    if (!glb_lcm_e530_24x2c_fiber)
    {
        LCM_LOG_ERR("Allocate pointer to fiber_port_info_t fail.");
        return LCM_E_NO_MEMORY;
    }

    /* 6, init current running port/fiber info */
    LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_init_port_table(p_card));
    LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_init_fiber_table(p_card));
 
    /* 7, init port properties */
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        logic_port_idx = glb_lcm_e530_24x2c_port[port_id].logic_port_idx;
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
        p_card->pp_port[port_id]->local_chip_idx = glb_lcm_e530_24x2c_port[port_id].chip_idx;
        p_card->pp_port[port_id]->mac_idx = glb_lcm_e530_24x2c_port[port_id].mac_idx;
        /* for bug 48433, do phy mapping */
        p_card->pp_port[port_id]->phy_addr = glb_lcm_e530_24x2c_port[port_id].phy_addr;
        p_card->pp_port[port_id]->chip_serdes_id = glb_lcm_e530_24x2c_port[port_id].serdes_id;
        p_card->pp_port[port_id]->g_port_index = (p_card->pp_port[port_id]->glb_chip_idx << 8) | logic_port_idx;
        /* for bug 48433 */
        p_card->pp_port[port_id]->mdio_bus_addr = 0xff;/* default */
    
        /* for bug 47572,add by tongzb, support speed 1G/10G/25G */
        p_card->pp_port[port_id]->speed_mode_cfg_en = 0;     
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
        /* for bug 53283, by tongzb, fix port led , 2019-08-22 */
        p_card->pp_port[port_id]->port_led_reverse = 1;
        p_card->pp_port[port_id]->ctl_id = 0;/* DUET2 only have one ctl */
        p_card->pp_port[port_id]->port_led_mac = glb_lcm_e530_24x2c_port[port_id].mac_idx;
        p_card->pp_port[port_id]->mac_ctl_id = 0;/* Tsingma only have one mac led slice */
     }

    /* 8, create file '/tmp/ctcos_port_info' */
    LCM_IF_ERROR_RETURN(lcm_common_save_port_info_file(p_card));
     
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init port end.");
    LCM_LOG_BRINGUP("Lcm init port end.");    

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_sysenv_info(glb_card_t* p_card)
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
_lcm_init_e530_24x2c_sysenv(glb_card_t* p_card)
{
    int32 ret = 0;
    sysenv_chip_info_t sysenv_chip_info[E530_24X2C_SYSENV_NUM];
    sysenv_spiflash_info_t spiflash_info[E530_24X2C_SYSENV_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init sysenv begin.");
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

    ret = sysenv_init(sysenv_chip_info, E530_24X2C_SYSENV_NUM);
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
_lcm_init_e530_24x2c_fiber(void)
{
    int32 ret = 0;
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fiber begin.");
    LCM_LOG_BRINGUP( "Lcm init fiber begin.");
    ret = fiber_init(glb_card->port_num,  /* fiber num */
                     glb_card->port_num, /* port num */
                     glb_lcm_e530_24x2c_fiber);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fiber end.");
    LCM_LOG_BRINGUP("Lcm init fiber end.");

    return ret;
}

static int32
_lcm_e530_24x2c_cfg_gpio(uint32 chip_id)
{
    uint8 reg, val;

    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module begin.");
    switch (chip_id)
    {
    case 0:    
        /* tx enable */
        reg = PCA9535_OUTPUT_PORT_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);

        reg = PCA9535_OUTPUT_PORT_REG_BANK1;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
                
        /* bank 0,1 : output */
        reg = PCA9535_DIR_CTRL_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9535_DIR_CTRL_REG_BANK1;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        break;        
    case 1:
        /* system led and psu status */
        reg = PCA9535_OUTPUT_PORT_REG_BANK0;
        val = 0xbf;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);

        reg = PCA9535_OUTPUT_PORT_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
                
        /* bank 1 : input
         * bank 0 : output
        */
        reg = PCA9535_DIR_CTRL_REG_BANK0;
        val = 0x0c;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9535_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        break;
    case 2:
        /* tx enable */
        reg = PCA9535_OUTPUT_PORT_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);

        reg = PCA9535_OUTPUT_PORT_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
                
        /* bank 1 : input
         * bank 0 : output
        */
        reg = PCA9535_DIR_CTRL_REG_BANK0;
        val = 0x00;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        reg = PCA9535_DIR_CTRL_REG_BANK1;
        val = 0xff;
        gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);
        break;        
    }
    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module end.");
    
    return RESULT_OK;
}

static int32
lcm_init_e530_24x2c_gpio(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[E530_24X2C_GPIO_CHIP_NUM];
    gpio_chip_t gpio_chip[E530_24X2C_GPIO_CHIP_NUM];
    void *p_data[E530_24X2C_GPIO_CHIP_NUM];

    LCM_LOG_BRINGUP("Lcm init i2c to gpio module begin.")
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(gpio_chip, 0, sizeof(gpio_chip));

    i2c_gen[0].addr = E530_24X2C_ADDR0_GPIO;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 0;   
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[0].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[0].io_type = GPIO_I2C;    
    gpio_chip[0].chip_type = GPIO_PCA9535;
    gpio_chip[0].scan_group_bitmap = 0x00;

    i2c_gen[1].addr = E530_24X2C_ADDR1_GPIO;
    i2c_gen[1].i2c_type = E_I2C_CPM;
    i2c_gen[1].bridge_flag = 0; 
    i2c_gen[1].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[1].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[1].io_type = GPIO_I2C;    
    gpio_chip[1].chip_type = GPIO_PCA9535;
    gpio_chip[1].scan_group_bitmap = 0x02; /*group 1*/

    i2c_gen[2].addr = E530_24X2C_ADDR2_GPIO;
    i2c_gen[2].i2c_type = E_I2C_CPM;
    i2c_gen[2].bridge_flag = 0; 
    i2c_gen[2].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[2].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[2].io_type = GPIO_I2C;    
    gpio_chip[2].chip_type = GPIO_PCA9535;
    gpio_chip[2].scan_group_bitmap = 0x00;

    for(i=0; i<E530_24X2C_GPIO_CHIP_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = gpio_init(p_data, gpio_chip, E530_24X2C_GPIO_CHIP_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Gpio device init fail.");
        return LCM_E_INIT_FAILED;
    }

    for(i=0; i<E530_24X2C_GPIO_CHIP_NUM; i++)
    {
        _lcm_e530_24x2c_cfg_gpio(i);
    }
    LCM_LOG_BRINGUP("Lcm init i2c to gpio module end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_24x2c_fan(glb_card_t* p_card)
{
    int32 ret = 0;
    uint8 fan_module_idx;
    fan_chip_t fan_chip[E530_24X2C_FAN_MODULE_MAX];
    void *p_data[E530_24X2C_FAN_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init fan module begin.");
    LCM_LOG_BRINGUP("Lcm init fan module begin.");    
    sal_memset(fan_chip, 0, sizeof(fan_chip));

    fan_chip[0].chip_type = FAN_CTC5236;
    fan_chip[0].io_type = E_FAN_CTC5236;
    fan_chip[0].support_hot_swap = 0;
    p_data[0] = NULL;
    fan_chip[0].fan_num = 3;
    fan_chip[0].speed_adjust = 1;
    fan_chip[0].rpm = 9000;
    fan_chip[0].pwm_map_table = NULL;
    fan_chip[0].pwm.bootom_rate = DEFAULT_FAN_RATE_BOTTOM; 
    fan_chip[0].pwm.low_rate = DEFAULT_FAN_RATE_LOW;
    fan_chip[0].pwm.high_rate = DEFAULT_FAN_RATE_HIGH;
    fan_chip[0].pwm.full_rate = DEFAULT_FAN_RATE_FULL;
    fan_chip[0].pwm_channel_flag = DEFAULT_FAN_PWM_CHANNAL;    /*if use default channel order*/
        
    ret = fan_init(p_data, fan_chip, E530_24X2C_FAN_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Fan driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(fan_module_idx=0; fan_module_idx < E530_24X2C_FAN_MODULE_MAX; fan_module_idx++)
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

static int32
_lcm_init_e530_24x2c_psu(glb_card_t* p_card)
{
    int32 ret;
    psu_data_gpio psu_gpio[E530_24X2C_PSU_MODULE_MAX];
    psu_chip_t psu_chip[E530_24X2C_PSU_MODULE_MAX];
    psu_private_t p_data[E530_24X2C_PSU_MODULE_MAX];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init psu module begin.");
    LCM_LOG_BRINGUP("Lcm init psu module begin.");    
    sal_memset(p_data, 0, sizeof(p_data));
    sal_memset(psu_chip, 0, sizeof(psu_chip));
    sal_memset(psu_gpio, 0, sizeof(psu_gpio));

    psu_chip[0].chip_type = PSU_GPIO;
    psu_chip[0].io_type = PSU_IO_GPIO;
    psu_chip[0].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[0].psu_mode_type = (1<<PSU_MODE_TYPE_REG_50_01_04)|(1<<PSU_MODE_TYPE_REG_50_20_08);
    psu_chip[0].support_pmbus = 0;
    psu_chip[0].fixed_status = 1;  /* if pus don't support alert signal */
    
    psu_gpio[0].present_chip = 1;
    psu_gpio[0].present_no = 9;
    psu_gpio[0].present_reverse = 1;
    psu_gpio[0].workstate_chip = 1;
    psu_gpio[0].workstate_no = 11;
    psu_gpio[0].workstate_reverse = 0;
    psu_gpio[0].alert_chip = 1;
    psu_gpio[0].alert_no = 8;
    psu_gpio[0].alert_reverse = 0;  
    psu_gpio[0].power_mode_chip = 1;
    psu_gpio[0].power_mode_no = 10;
    psu_gpio[0].power_mode_reverse = 0;     
    p_data[0].p_data_gpio = &psu_gpio[0];


    psu_chip[1].chip_type = PSU_GPIO;
    psu_chip[1].io_type = PSU_IO_GPIO;
    psu_chip[1].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[1].psu_mode_type = (1<<PSU_MODE_TYPE_REG_51_01_04)|(1<<PSU_MODE_TYPE_REG_51_20_08);
    psu_chip[1].support_pmbus = 0;
    psu_chip[1].fixed_status = 1;  /* if pus don't support alert signal */
    
    psu_gpio[1].present_chip = 1;
    psu_gpio[1].present_no = 13;
    psu_gpio[1].present_reverse = 1;
    psu_gpio[1].workstate_chip = 1;
    psu_gpio[1].workstate_no = 15;
    psu_gpio[1].workstate_reverse = 0;
    psu_gpio[1].alert_chip = 1;
    psu_gpio[1].alert_no = 12;
    psu_gpio[1].alert_reverse = 0;
    psu_gpio[1].power_mode_chip = 1;
    psu_gpio[1].power_mode_no = 14;
    psu_gpio[1].power_mode_reverse = 0;      
    p_data[1].p_data_gpio = &psu_gpio[1];

    ret = psu_init(p_data, psu_chip, E530_24X2C_PSU_MODULE_MAX);
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
_lcm_init_e530_24x2c_phy(void)
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
_lcm_init_e530_24x2c_i2c(void)
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
_lcm_init_e530_24x2c_led(void)
{
    int32 ret = 0;
    led_info_t led_info[E_MAX_LED];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init LED begin.");
    LCM_LOG_BRINGUP("Lcm init LED begin.");
    sal_memset(led_info, 0, sizeof(led_info_t)*E_MAX_LED);

    /* init sys led */
    led_info[E_SYS_LED].exist = 1;
    led_info[E_SYS_LED].led_ctl = E_LED_I2C_GPIO1;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].op_reg = 5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].val = 0x0;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].op_reg = 5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].val = 0x1;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = 6;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].val = 0x1;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].op_reg = 6;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].val = 0x0;
    /* End of Merge */
    led_info[E_IND_LED].exist = 1;
    led_info[E_IND_LED].led_ctl = E_LED_I2C_GPIO1;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].op_reg = 4;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].val = 0x1;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].op_reg = 4;
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
_lcm_init_e530_24x2c_ctc_chip_led(void)
{
    uint8 split_flag=0;
    uint8 table_id;
    uint8 table_num;
    uint8 src_pos, des_pos;
    uint8 port_id;
    mac_led_info_t mac_led_info;
    mac_led_api_para_t glb_lcm_mac_led[E530_24X2C_LED_TBL_NUM][E530_24X2C_LED_MAC_NUM];

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init MAC LED begin.");
    LCM_LOG_BRINGUP("Lcm init MAC LED begin.");

    split_flag = glb_card->split_flag;
    if(split_flag)
    {
        table_num = E530_24X2C_LED_TBL_NUM;
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
        sal_memcpy(glb_lcm_mac_led[table_id], lcm_e530_24x2c_mac_led_default_entry, 
            sizeof(mac_led_api_para_t)*E530_24X2C_LED_MAC_NUM);

        for(port_id=0; port_id<E530_24X2C_PANEL_PORT_NUM; port_id++)
        {
            if (GLB_PPT_NUMCHG_TYPE_10G == glb_card->pp_ppt[port_id]->ppt_numchg_type
                || GLB_PPT_NUMCHG_TYPE_25G == glb_card->pp_ppt[port_id]->ppt_numchg_type)
            {
                des_pos = E530_24X2C_LED_MAC_NUM - port_id - 1;
                src_pos = 24 + (port_id -24) * 4 + table_id;
                glb_lcm_mac_led[table_id][des_pos].port_id = lcm_e530_24x2c_port_max[src_pos].logic_port_idx;
            }
            else if(GLB_PPT_NUMCHG_TYPE_50G == glb_card->pp_ppt[port_id]->ppt_numchg_type)
            {
                des_pos = E530_24X2C_LED_MAC_NUM - port_id - 1;;
                /* only panel port 25 support split 50G */
                if((table_id == 0)||(table_id == 1))
                {
                    src_pos = 24 + (port_id-24)*4 + 1;
                }
                else
                {
                    src_pos = 24 + (port_id-24)*4 + 3;
                }
                glb_lcm_mac_led[table_id][des_pos].port_id = lcm_e530_24x2c_port_max[src_pos].logic_port_idx;
            }
        }

        /* When any port split, light on fan out LED periodically */
        if(split_flag)
        {
            /* channel 1: force on fan out led
             * channel 0: force off fan out led
             */
            src_pos = 3 - table_id;
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
    mac_led_info.mac_num = E530_24X2C_LED_MAC_NUM;
    mac_led_info.slice0_mac_num = E530_24X2C_SLICE0_LED_MAC_NUM;
    mac_led_info.polarity = 1; /* 1:for low driver led, 0:for high driver led */
    mac_led_info_register(&mac_led_info);
    mac_led_init();


    XFREE(CTCLIB_MEM_LCM_MODULE, mac_led_info.mac_led_api_para);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init MAC LED end.");
    LCM_LOG_BRINGUP("Lcm init MAC LED end.");        
    return LCM_E_SUCCESS;
}

int32
lcm_e530_24x2c_get_sum_ffe_cfg(lcm_chip_serdes_ffe_t* serdes_ffe)
{
    uint8 serdes_id;
    uint8 index;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;
    uint8 default_trace_len;

    serdes_id = serdes_ffe->serdes_id;
    serdes_ffe->board_material = glb_card->board_material;

    for(index = 0; index < E530_24X2C_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_24x2c_serdes_ffe_max[index];
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
_lcm_init_e530_24x2c_serdes_mode(void)
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
            if((panel_port == 24)||(panel_port == 25))
                pos = 24 + (panel_port-24)*4+1;

            ctc_port_set_mac_en(lcm_e530_24x2c_port_max[pos].logic_port_idx, 0);
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
                gport = lcm_e530_24x2c_port_max[pos ].logic_port_idx;
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
_lcm_init_e530_24x2c_serdes_ffe(void)
{
    ctc_chip_serdes_ffe_t serdes_ffe;
    uint8 index;
    int32 ret = 0;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;        
    uint8 default_trace_len;
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init serdes ffe begin.");
    LCM_LOG_BRINGUP("Lcm init serdes ffe begin.");    

    sal_memset(&serdes_ffe, 0, sizeof(ctc_chip_serdes_ffe_t));
    for(index = 0; index < E530_24X2C_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_24x2c_serdes_ffe_max[index];
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
lcm_init_e530_24x2c_cb(void)
{
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init callback begin.");
    LCM_LOG_BRINGUP("Lcm init callback begin.");
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_fiber());    
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_serdes_mode());/* for set    default auto-neg */
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_phy());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_ctc_chip_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_serdes_ffe());
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init callback end.");
    LCM_LOG_BRINGUP( "Lcm init callback end.");    

    return LCM_E_SUCCESS;
}

extern int32 lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func);
int32
lcm_init_e530_24x2c(glb_card_t* p_card)
{
    uint8 bm;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init board begin.");
    LCM_LOG_BRINGUP( "Lcm init board begin.");    
    glb_card = p_card;
    p_card->asic_chip_num = 1;
    p_card->chip_sensor_num = 1;

    lcm_common_get_slot_no(&p_card->logic_slot_no,&p_card->phy_slot_no);

    p_card->panel_port_num = E530_24X2C_PANEL_PORT_NUM;
    p_card->l2switch_num = 0;
    p_card->sensor_num = 0;
    /* for bug 53288, added by tongzb, 2019-10-14, for board have no epld */
    p_card->epld_status = GLB_EPLD_STATUS_ABSENT;
    if(p_card->chip_sensor_num+p_card->sensor_num > MAX_TMPR_SENSOR_NUM)
    {
        LCM_LOG_ERR("Sensor structure is too small to store sensor info on board.\n");        
        return LCM_E_NO_MEMORY;
    }
    p_card->tmpr_limit.board_crit = DEFAULT_CRIT_TMPR;
    p_card->tmpr_limit.board_high = DEFAULT_HIGH_TMPR;
    p_card->tmpr_limit.board_low = DEFAULT_LOW_TMPR;
    
    p_card->fan_module_num = E530_24X2C_FAN_MODULE_MAX;
    p_card->psu_module_num = E530_24X2C_PSU_MODULE_MAX;
    
    /* Modified by liuht for bug38424 2016-05-11 */
    //p_card->volt_curr_mon_num = 3;
    p_card->cpu_type = GLB_CPU_MAX;
    p_card->phy_chip_num = 0;
    p_card->phy_intr_en = 0;

    /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    p_card->ptp_en = 0;
    p_card->epld_type = GLB_EPLD_TYPE_MAX;
    p_card->bootrom_type = E_BOOTROM_TYPE_512K;

    if(GLB_BOARD_E530_24X2Q == p_card->board_type.type)
    {
        sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                        DATAPATH_PROFILE_PATH, E530_24X2Q_DATAPATH_NAME);
    }
    else
    {        
        sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                        DATAPATH_PROFILE_PATH, E530_24X2C_DATAPATH_NAME);
    }
    /* Fix bug29772, cr9895, qicx, 2014-08-26 */
    p_card->support_reboot_info = 1;

    lcm_common_init_cpu_info();
    lcm_common_config_ptp_enable(0);//config ptp
    lcm_common_config_syncE_enable(0);//config syncE
    lcm_common_config_reboot_record_type(REBOOT_FILE_EPLD,0xf6);
    LCM_LOG_DEBUG(LCM_CARD,   "card product type %x, board type %x.", p_card->board_type.series, p_card->board_type.type);
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_i2c());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_sysenv_info(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_sysenv(p_card));

    /* 0 for FR4 board, 1 for M4 board, 2 for M6 board */
    if(lcm_common_get_board_material(&bm)) /*default value*/
    {
        p_card->board_material = BOARD_MATERIAL_FR4;
    }
    else
    {
        p_card->board_material = bm;
    }
    LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_gpio());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_port(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_fan(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_24x2c_psu(p_card));

    lcm_card_init_callback(lcm_init_e530_24x2c_cb);
    lcm_port_init_ffe_cfg_callback(lcm_e530_24x2c_get_sum_ffe_cfg);
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init board end.");
    LCM_LOG_BRINGUP("Lcm init board end.");

    return LCM_E_SUCCESS;
}

