/****************************************************************************
* $Id$
*  E530-48T4X board init functions
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
#include "poe_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
 
/*
 * CPU I2C slave device
 */

/* Sysenv */
#define E530_48T4X_SYSENV_NUM                      0x01

/* Power supply */
#define E530_48T4X_PSU_MODULE_MAX                  0x2
#define E530_48T4X_ADDR0_PSU                       0x50
#define E530_48T4X_ADDR1_PSU                       0x51

/* Fan */
#define E530_48T4X_FAN_MODULE_MAX                  0x1

/* I2C to GPIO: PCA9505 */
#define E530_48T4X_GPIO_CHIP_NUM                   0x01
#define E530_48T4X_ADDR0_GPIO                      0x22

#define E530_48T4X_PANEL_PORT_NUM_MAX     (48+4)
#define E530_48T4X_PANEL_PORT_NUM         (48+4)
#define E530_48T4X_FIBER_PORT_NUM           4

#define E530_48T4X_SLICE0_LED_MAC_NUM       4
#define E530_48T4X_LED_MAC_NUM              4

#define E530_48T4X_PHY_INT_BITS_SUM         1

#define E530_48P4X_POE_NUM                            1
#define E530_48P4X_POE_CHANNEL_NUM                   48
#define E530_48P4X_POE0_ADDR                       0x10

#define GLB_I2C_IDX GLB_I2C_IDX_0

/****************************************************************************
*
* Global and Declarations
*
****************************************************************************/
static const glb_card_t         *glb_card;
static lcm_card_port_t          *glb_lcm_e530_48t4x_port             = NULL;
static fiber_port_info_t        *glb_lcm_e530_48t4x_fiber            = NULL;

extern int32 lcm_card_init_callback(card_init_callback_t func);
extern int32
lcm_common_get_defaultfec_from_speedability(uint16 speed_ability, uint32*  fec_ability);

lcm_card_port_t lcm_e530_48t4x_port_max[E530_48T4X_PANEL_PORT_NUM_MAX] =
{
    {0, 1, 1, 1},    /* Port 1 */
    {0, 0, 0, 0},    /* Port 2 */
    {0, 3, 3, 3},    /* Port 3 */
    {0, 2, 2, 2},    /* Port 4 */
    {0, 5, 5, 5},    /* Port 5 */
    {0, 4, 4, 4},    /* Port 6 */
    {0, 7, 7, 7},    /* Port 7 */
    {0, 6, 6, 6},    /* Port 8 */

    {0, 17, 17, 9},    /* Port 9 */
    {0, 16, 16, 8},    /* Port 10 */
    {0, 19, 19, 11},    /* Port 11 */
    {0, 18, 18, 10},    /* Port 12 */
    {0, 21, 21, 13},    /* Port 13 */
    {0, 20, 20, 12},    /* Port 14 */
    {0, 23, 23, 15},    /* Port 15 */
    {0, 22, 22, 14},    /* Port 16 */

    {0, 9, 9, 1},    /* Port 17 */
    {0, 8, 8, 0},    /* Port 18 */
    {0, 11, 11, 3},    /* Port 19 */
    {0, 10, 10, 2},    /* Port 20 */
    {0, 33, 33, 5},    /* Port 21 */
    {0, 32, 32, 4},    /* Port 22 */
    {0, 35, 35, 7},    /* Port 23 */
    {0, 34, 34, 6},    /* Port 24 */

    {0, 37, 37, 9},    /* Port 25 */
    {0, 36, 36, 8},    /* Port 26 */
    {0, 39, 39, 11},    /* Port 27 */
    {0, 38, 38, 10},    /* Port 28 */
    {0, 41, 41, 13},    /* Port 29 */
    {0, 40, 40, 12},    /* Port 30 */
    {0, 43, 43, 15},    /* Port 31 */
    {0, 42, 42, 14},    /* Port 32 */

    {0, 25, 25, 1},    /* Port 33 */
    {0, 24, 24, 0},    /* Port 34 */
    {0, 27, 27, 3},    /* Port 35 */
    {0, 26, 26, 2},    /* Port 36 */
    {0, 49, 49, 5},    /* Port 37 */
    {0, 48, 48, 4},    /* Port 38 */
    {0, 51, 51, 7},    /* Port 39 */
    {0, 50, 50, 6},    /* Port 40 */

    {0, 53, 53, 1},    /* Port 41 */
    {0, 52, 52, 0},    /* Port 42 */
    {0, 55, 55, 3},    /* Port 43 */
    {0, 54, 54, 2},    /* Port 44 */
    {0, 57, 57, 5},    /* Port 45 */
    {0, 56, 56, 4},    /* Port 46 */
    {0, 59, 59, 7},    /* Port 47 */
    {0, 58, 58, 6},    /* Port 48 */

    {0, 13, 13},    /* Port 49 */
    {0, 12, 12},    /* Port 50 */
    {0, 15, 15},    /* Port 51 */
    {0, 14, 14},    /* Port 52 */

};

glb_port_range_t lcm_e530_48t4x_phy_chip_port_range[E530_48T4X_PHY_INT_BITS_SUM] =
{
    { 1, 48},     
};

fiber_port_info_t lcm_e530_48t4x_fiber_max[48+E530_48T4X_FIBER_PORT_NUM] =
{
    /* 48T */
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    
    /* panel port 49-52 */
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,0}},{.en_gpio={0,0}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,1}},{.en_gpio={0,1}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,2}},{.en_gpio={0,2}}},
    {E_FIBER_SFP_P, E_FIBER_ASIC_ASIC,E_FIBER_EN_GPIO,0,{.asic={0,3}},{.en_gpio={0,3}}},   
};

/* TM mac led is reverse to port *////TODO:
mac_led_api_para_t lcm_e530_48t4x_mac_led_default_entry[E530_48T4X_LED_MAC_NUM]=
{
    /* panel port 49-52 */
    {14, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {15, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {12, 0, 0, LED_MODE_2_FORCE_OFF, 0},
    {13, 0, 0, LED_MODE_2_FORCE_OFF, 0},
};

/* typical ffe */
lcm_card_serdes_ffe_t 
lcm_e530_48t4x_serdes_ffe_max[E530_48T4X_PANEL_PORT_NUM_MAX]=
{
    {0, 0xff, 0xff},    /* Port 1 */
    {0, 0xff, 0xff},    /* Port 2 */
    {0, 0xff, 0xff},    /* Port 3 */
    {0, 0xff, 0xff},    /* Port 4 */
    {1, 0xff, 0xff},    /* Port 5 */
    {1, 0xff, 0xff},    /* Port 6 */
    {1, 0xff, 0xff},    /* Port 7 */
    {1, 0xff, 0xff},    /* Port 8 */

    {2, 0xff, 0xff},    /* Port 9 */
    {2, 0xff, 0xff},    /* Port 10 */
    {2, 0xff, 0xff},    /* Port 11 */
    {2, 0xff, 0xff},    /* Port 12 */
    {3, 0xff, 0xff},    /* Port 13 */
    {3, 0xff, 0xff},    /* Port 14 */
    {3, 0xff, 0xff},    /* Port 15 */
    {3, 0xff, 0xff},    /* Port 16 */

    {4, 0xff, 0xff},    /* Port 17 */
    {4, 0xff, 0xff},    /* Port 18 */
    {4, 0xff, 0xff},    /* Port 19 */
    {4, 0xff, 0xff},    /* Port 20 */
    {5, 0xff, 0xff},    /* Port 21 */
    {5, 0xff, 0xff},    /* Port 22 */
    {5, 0xff, 0xff},    /* Port 23 */
    {5, 0xff, 0xff},    /* Port 24 */

    {6, 0xff, 0xff},    /* Port 25 */
    {6, 0xff, 0xff},    /* Port 26 */
    {6, 0xff, 0xff},    /* Port 27 */
    {6, 0xff, 0xff},    /* Port 28 */
    {7, 0xff, 0xff},    /* Port 29 */
    {7, 0xff, 0xff},    /* Port 30 */
    {7, 0xff, 0xff},    /* Port 31 */
    {7, 0xff, 0xff},    /* Port 32 */

    {8, 0xff, 0xff},    /* Port 33 */
    {8, 0xff, 0xff},    /* Port 34 */
    {8, 0xff, 0xff},    /* Port 35 */
    {8, 0xff, 0xff},    /* Port 36 */
    {9, 0xff, 0xff},    /* Port 37 */
    {9, 0xff, 0xff},    /* Port 38 */
    {9, 0xff, 0xff},    /* Port 39 */
    {9, 0xff, 0xff},    /* Port 40 */

    {10, 0xff, 0xff},    /* Port 41 */
    {10, 0xff, 0xff},    /* Port 42 */
    {10, 0xff, 0xff},    /* Port 43 */
    {10, 0xff, 0xff},    /* Port 44 */
    {11, 0xff, 0xff},    /* Port 45 */
    {11, 0xff, 0xff},    /* Port 46 */
    {11, 0xff, 0xff},    /* Port 47 */
    {11, 0xff, 0xff},    /* Port 48 */
    /*4~7inch*/
    {13, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 49 */
    {12, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 50 */
    {15, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 51 */
    {14, 0xff, 0xff,COEFF1(2,90,11,0)},    /* Port 52 */

};

glb_poe_port_info_t lcm_48p4x_poe_entry[E530_48P4X_POE_CHANNEL_NUM]=
{
    /* 1 - 12 */
    {0, 0, 0, 0},
    {0, 0, 1, 1},
    {0, 0, 2, 2},
    {0, 0, 3, 3}, 
    {0, 0, 4, 4},
    {0, 0, 5, 5},
    {0, 0, 6, 6},
    {0, 0, 7, 7},
    {0, 1, 8, 8},
    {0, 1, 9, 9},
    {0, 1, 10,10},
    {0, 1, 11,11},
    /* 13 - 24 */
    {0, 1, 12,12},
    {0, 1, 13,13},
    {0, 1, 14,14},
    {0, 1, 15,15},
    {0, 2, 16,16},
    {0, 2, 17,17},
    {0, 2, 18,18},
    {0, 2, 19,19},
    {0, 2, 20,20},
    {0, 2, 21,21},
    {0, 2, 22,22},
    {0, 2, 23,23}, 
    /* 24 - 36 */
    {0, 3, 24,24},
    {0, 3, 25,25},
    {0, 3, 26,26},
    {0, 3, 27,27},
    {0, 3, 28,28},
    {0, 3, 29,29},
    {0, 3, 30,30},
    {0, 3, 31,31},
    {0, 4, 31,31},
    {0, 4, 33,33},
    {0, 4, 34,34},
    {0, 4, 35,35}, 

     /* 37 - 48 */
    {0, 4, 36,36},
    {0, 4, 37,37},
    {0, 4, 38,38},
    {0, 4, 39,39},
    {0, 5, 30,30},
    {0, 5, 40,40},
    {0, 5, 42,42},
    {0, 5, 43,43},
    {0, 5, 44,44},
    {0, 5, 45,45},
    {0, 5, 46,46},
    {0, 5, 47,47}, 
};

/****************************************************************************
 *
* Function
*
****************************************************************************/
static int32
lcm_init_e530_48t4x_init_port_table(glb_card_t* p_card)
{
    uint8 port_id, panel_port_no;
    //uint8 panel_subport_no,port_numchg_type;
    uint8 pos = 0;

    for(port_id=0; port_id<p_card->port_num; port_id++)
    {
        panel_port_no = p_card->pp_port[port_id]->panel_port_no;
        //panel_subport_no = p_card->pp_port[port_id]->panel_sub_port_no;
        //port_numchg_type = p_card->pp_port[port_id]->port_numchg_type;

        pos = panel_port_no - 1;

        sal_memcpy(&glb_lcm_e530_48t4x_port[port_id], &lcm_e530_48t4x_port_max[pos], sizeof(lcm_card_port_t));
        glb_lcm_e530_48t4x_port[port_id].serdes_id = lcm_e530_48t4x_serdes_ffe_max[pos].serdes_id;
    }

    return LCM_E_SUCCESS;
}

static int32
lcm_init_e530_48t4x_init_fiber_table(glb_card_t* p_card)
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
        sal_memcpy(&glb_lcm_e530_48t4x_fiber[port_id], &lcm_e530_48t4x_fiber_max[panel_port_no-1], sizeof(fiber_port_info_t));
        glb_lcm_e530_48t4x_fiber[port_id].fiber_channel = fiber_channel;
        glb_lcm_e530_48t4x_fiber[port_id].fiber_id = port_id;
    }
    
    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48t4x_panel_port(glb_card_t* p_card)
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
            p_card->pp_ppt[ppt_id]->ppt_speed_ability = GLB_SUPPORT_SPEED_1G | GLB_SUPPORT_SPEED_100M;
            p_card->pp_ppt[ppt_id]->ppt_speed_real = GLB_SUPPORT_SPEED_1G;
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
_lcm_init_e530_48t4x_port(glb_card_t* p_card)
{
    int32 port_id = 0;
    uint32 ppt_idx;
    uint16 logic_port_idx;
    lcm_card_port_panel_mapping_t port_panel_mapping[256];

    LCM_LOG_BRINGUP("Lcm init port begin.");
     /* 1, init panel port */
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_panel_port(p_card));
     
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
    glb_lcm_e530_48t4x_port = (lcm_card_port_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(lcm_card_port_t)* p_card->port_num);
    if (!glb_lcm_e530_48t4x_port)
    {
        LCM_LOG_ERR("Allocate pointer to lcm_card_port_t fail.");
        return LCM_E_NO_MEMORY;
    }
 
    glb_lcm_e530_48t4x_fiber = (fiber_port_info_t* )XCALLOC(CTCLIB_MEM_LCM_PORT, sizeof(fiber_port_info_t)*p_card->port_num);
    if (!glb_lcm_e530_48t4x_fiber)
    {
        LCM_LOG_ERR("Allocate pointer to fiber_port_info_t fail.");
        return LCM_E_NO_MEMORY;
    }

    /* 6, init current running port/fiber info */
    LCM_IF_ERROR_RETURN(lcm_init_e530_48t4x_init_port_table(p_card));
    LCM_IF_ERROR_RETURN(lcm_init_e530_48t4x_init_fiber_table(p_card));
 
    /* 7, init port properties */
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        logic_port_idx = glb_lcm_e530_48t4x_port[port_id].logic_port_idx;
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
        p_card->pp_port[port_id]->local_chip_idx = glb_lcm_e530_48t4x_port[port_id].chip_idx;
        p_card->pp_port[port_id]->mac_idx = glb_lcm_e530_48t4x_port[port_id].mac_idx;
        /* for bug 48433, do phy mapping */
        p_card->pp_port[port_id]->phy_addr = glb_lcm_e530_48t4x_port[port_id].phy_addr;
        p_card->pp_port[port_id]->chip_serdes_id = glb_lcm_e530_48t4x_port[port_id].serdes_id;
        p_card->pp_port[port_id]->g_port_index = (p_card->pp_port[port_id]->glb_chip_idx << 8) | logic_port_idx;

        /* for bug 48433 */
        p_card->pp_port[port_id]->mdio_bus_addr = 0xff;/* default */
        if (port_id < 16)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS0;
        }
        else if(port_id < 32)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS1;
        }
        else if(port_id < 40)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS2;
        }
        else if(port_id < 48)
        {
            p_card->pp_port[port_id]->mdio_bus_addr = GLB_MIDO_BUS3;
        }        

        if (ppt_idx <= 48)
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_UNSUPPORT;
            p_card->pp_port[port_id]->unidir_support = 0;
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_1000BASE_T;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_COPPER;
        }
        else
        {
            p_card->pp_port[port_id]->port_led_mode_change = GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED;
            p_card->pp_port[port_id]->unidir_support = 1;
            p_card->pp_port[port_id]->phy_type = GLB_PHY_TYPE_UNKNOWN;
            p_card->pp_port[port_id]->port_cfg.media = GLB_PORT_TYPE_FIBER;
        }        
 
        p_card->pp_port[port_id]->port_led_reverse = 1;
        p_card->pp_port[port_id]->ctl_id = 0;/* DUET2 only have one ctl */
        p_card->pp_port[port_id]->port_led_mac = glb_lcm_e530_48t4x_port[port_id].mac_idx;
        /*useless code*/
        p_card->pp_port[port_id]->mac_ctl_id = 0;/* Tsingma only have one mac led slice */
     }

    /* 8, create file '/tmp/ctcos_port_info' */
    LCM_IF_ERROR_RETURN(lcm_common_save_port_info_file(p_card));
     
    LCM_LOG_BRINGUP("Lcm init port end.");    

    return LCM_E_SUCCESS;
}


static int32
_lcm_init_e530_48t4x_sysenv_info(glb_card_t* p_card)
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

    p_card->p_sysenv_alloc[SYSENV_SW_VER].sysenv_idx = 0;
    p_card->p_sysenv_alloc[SYSENV_SW_VER].base_addr = 0x1502; 

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48t4x_sysenv(glb_card_t* p_card)
{
    int32 ret = 0;
    sysenv_chip_info_t sysenv_chip_info[E530_48T4X_SYSENV_NUM];
    sysenv_spiflash_info_t spiflash_info[E530_48T4X_SYSENV_NUM];

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

    ret = sysenv_init(sysenv_chip_info, E530_48T4X_SYSENV_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm sysenv init fail.");
        return LCM_E_INIT_FAILED;
    }

    LCM_LOG_BRINGUP("Lcm init sysenv end.");
    return LCM_E_SUCCESS;
}


static int32
_lcm_init_e530_48t4x_fiber(void)
{
    int32 ret = 0;
    
    LCM_LOG_BRINGUP( "Lcm init fiber begin.");
    ret = fiber_init(glb_card->port_num,  /* fiber num */
                     glb_card->port_num, /* port num */
                     glb_lcm_e530_48t4x_fiber);

    LCM_LOG_BRINGUP("Lcm init fiber end.");

    return ret;
}

static int32
_lcm_e530_48t4x_cfg_gpio(uint32 chip_id)
{
    uint8 reg, val;

    LCM_LOG_BRINGUP("Lcm cfg i2c to gpio module begin.");
    switch (chip_id)
    {
        case 0:  
            /* tx enable and release mac led and close indicate led*/
            reg = PCA9535_OUTPUT_PORT_REG_BANK0;
            val = 0xf0;
            gpio_reg_write(chip_id, reg, &val, PCA95X5_VALUE_LEN);

            /* bank 0 : output
             * bank 1 : input
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
lcm_init_e530_48t4x_gpio(void)
{
    int32 i, ret = 0;
    i2c_gen_t i2c_gen[E530_48T4X_GPIO_CHIP_NUM];
    gpio_chip_t gpio_chip[E530_48T4X_GPIO_CHIP_NUM];
    void *p_data[E530_48T4X_GPIO_CHIP_NUM];

    LCM_LOG_BRINGUP("Lcm init i2c to gpio module begin.")
    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(gpio_chip, 0, sizeof(gpio_chip));

    i2c_gen[0].addr = E530_48T4X_ADDR0_GPIO;
    i2c_gen[0].i2c_type = E_I2C_CPM;
    i2c_gen[0].bridge_flag = 0;   
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX;
    i2c_gen[0].alen = PCA9505_OFFSET_WIDTH;
    gpio_chip[0].io_type = GPIO_I2C;    
    gpio_chip[0].chip_type = GPIO_PCA9535;
    gpio_chip[0].scan_group_bitmap = 0x02; /*group 1*/

    for(i=0; i<E530_48T4X_GPIO_CHIP_NUM; i++)
    {
        p_data[i] = &i2c_gen[i];
    }
    ret = gpio_init(p_data, gpio_chip, E530_48T4X_GPIO_CHIP_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Gpio device init fail.");
        return LCM_E_INIT_FAILED;
    }

    for(i=0; i<E530_48T4X_GPIO_CHIP_NUM; i++)
    {
        _lcm_e530_48t4x_cfg_gpio(i);
    }
    LCM_LOG_BRINGUP("Lcm init i2c to gpio module end.");

    return LCM_E_SUCCESS;
}

static int32
_lcm_init_e530_48t4x_fan(glb_card_t* p_card)
{
    int32 ret = 0;
    uint8 fan_module_idx;
    fan_chip_t fan_chip[E530_48T4X_FAN_MODULE_MAX];
    void *p_data[E530_48T4X_FAN_MODULE_MAX];

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
    fan_chip[0].pwm_channel_flag = DEFAULT_FAN_PWM_CHANNAL;    /*if use default channel order*/

    ret = fan_init(p_data, fan_chip, E530_48T4X_FAN_MODULE_MAX);
    if(ret < 0)
    {
        LCM_LOG_ERR("Fan driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(fan_module_idx=0; fan_module_idx < E530_48T4X_FAN_MODULE_MAX; fan_module_idx++)
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
_lcm_init_e530_48t4x_psu(glb_card_t* p_card)
{
    int32 ret;
    psu_data_gpio psu_gpio[E530_48T4X_PSU_MODULE_MAX];
    psu_chip_t psu_chip[E530_48T4X_PSU_MODULE_MAX];
    psu_private_t p_data[E530_48T4X_PSU_MODULE_MAX];

    LCM_LOG_BRINGUP("Lcm init psu module begin.");    

    sal_memset(p_data, 0, sizeof(p_data));
    sal_memset(psu_chip, 0, sizeof(psu_chip));
    sal_memset(psu_gpio, 0, sizeof(psu_gpio));


    psu_chip[0].chip_type = PSU_GPIO;

    psu_chip[0].io_type = PSU_IO_GPIO;
    psu_chip[0].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[0].support_pmbus = 0;
    psu_chip[0].fixed_status = 1;   /* if pus don't support alert signal */

    psu_gpio[0].workstate_chip = 0;
    psu_gpio[0].workstate_no = 11;
    
    psu_gpio[0].present_chip = 0;
    psu_gpio[0].present_no = 9;
    psu_gpio[0].present_reverse = 1;

    psu_gpio[0].power_mode_chip = 0;
    psu_gpio[0].power_mode_no = 10;
    psu_gpio[0].power_mode_reverse = 0;
    
    /*psu fan's status*/
    psu_gpio[0].alert_chip = 0;
    psu_gpio[0].alert_no = 8;

    p_data[0].p_data_gpio = &psu_gpio[0];


    psu_chip[1].chip_type = PSU_GPIO;
    psu_chip[1].io_type = PSU_IO_GPIO;
    psu_chip[1].psu_type = PSU_SHOW_FULL_STATUS;
    psu_chip[1].support_pmbus = 0;
    psu_chip[1].fixed_status = 1;   /* if pus don't support alert signal */
    
    psu_gpio[1].workstate_chip = 0;
    psu_gpio[1].workstate_no = 15;
    
    psu_gpio[1].present_chip = 0;
    psu_gpio[1].present_no = 13;
    psu_gpio[1].present_reverse = 1;

    psu_gpio[1].power_mode_chip = 0;
    psu_gpio[1].power_mode_no = 14;
    psu_gpio[1].power_mode_reverse = 0;

    /*psu fan's status*/
    psu_gpio[1].alert_chip = 0;
    psu_gpio[1].alert_no = 12;

    p_data[1].p_data_gpio= &psu_gpio[1];

    ret = psu_init(p_data, psu_chip, E530_48T4X_PSU_MODULE_MAX);
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
_lcm_init_e530_48t4x_phy(void)
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
        
        if(port_id < 48) /* 48T with PHYs */
        {
            phyinfo.phy_device_type = PORT_PHY_MV88E1680;
            phyinfo.phy_manage_info.media_type = GLB_MEDIA_RJ45;
            phyinfo.phy_manage_info.mac_if = GLB_MAC_INTERFACE_QSGMII;
            phyinfo.vct_support = 1;
            phyinfo.phy_addr = p_port->phy_addr;
            phyinfo.mdio_bus = ASIC_TM_PHY_1G_TYPE;
            phyinfo.base_addr = p_port->mdio_bus_addr;            
            lcm_common_get_defaultspeed_from_speedability(p_port->port_speed_ability, 
                    &(phyinfo.phy_manage_info.speed), &(phyinfo.phy_manage_info.duplex));

            phyinfo.phy_manage_info.phy_init_seq_flag = GLB_NO_INIT_FLAG;
            phyinfo.led_type = GLB_PHY_LED_16;
            phyinfo.led_freq = GLB_PHY_LED_FREQ_DEFAULT;
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
_lcm_init_e530_48t4x_i2c(void)
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
_lcm_init_e530_48t4x_led(void)
{
    int32 ret = 0;
    led_info_t led_info[E_MAX_LED];

    LCM_LOG_BRINGUP("Lcm init LED begin.");
    sal_memset(led_info, 0, sizeof(led_info_t)*E_MAX_LED);

    /* init sys led */
    led_info[E_SYS_LED].exist = 1;
    led_info[E_SYS_LED].led_ctl = E_LED_I2C_GPIO0;
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].op_reg = 6;//for bug 53242, by tongzb, fix system led, 2019-08-22
    led_info[E_SYS_LED].led_para[E_LED_SYS_RUN].val = 0;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].op_reg = 6;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ALARM].val = 1;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].op_reg = 5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_NO_ALARM].val = 1;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].op_reg = 5;
    led_info[E_SYS_LED].led_para[E_LED_SYS_ABNORMAL].val = 0;
    led_info[E_IND_LED].exist = 1;
    led_info[E_IND_LED].led_ctl = E_LED_I2C_GPIO0;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].op_reg = 4;
    led_info[E_IND_LED].led_para[E_LED_SYS_OFF].val = 1;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].op_reg = 4;
    led_info[E_IND_LED].led_para[E_LED_SYS_ON].val = 0;
	
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
_lcm_init_e530_48t4x_ctc_chip_led(void)
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
        mac_led_info.mac_led_api_para[table_id] = lcm_e530_48t4x_mac_led_default_entry;
    }

    mac_led_info.table_num = table_num;
    mac_led_info.mac_num = E530_48T4X_LED_MAC_NUM;
    mac_led_info.slice0_mac_num = E530_48T4X_SLICE0_LED_MAC_NUM;
    mac_led_info.polarity = 1; /* 1:for low driver led, 0:for high driver led */
    mac_led_info_register(&mac_led_info);
    mac_led_init();


    XFREE(CTCLIB_MEM_LCM_MODULE, mac_led_info.mac_led_api_para);

    LCM_LOG_BRINGUP("Lcm init MAC LED end.");        
    return LCM_E_SUCCESS;
}

int32
lcm_e530_48t4x_get_sum_ffe_cfg(lcm_chip_serdes_ffe_t* serdes_ffe)
{
    uint8 serdes_id;
    uint8 index;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;
    uint8 default_trace_len;

    serdes_id = serdes_ffe->serdes_id;
    serdes_ffe->board_material = glb_card->board_material;

    for(index = 0; index < E530_48T4X_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_48t4x_serdes_ffe_max[index];
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
_lcm_init_e530_48t4x_serdes_ffe(void)
{
    ctc_chip_serdes_ffe_t serdes_ffe;
    uint8 index;
    int32 ret = 0;
    lcm_card_serdes_ffe_t *lcm_serdes_ffe;        
    uint8 default_trace_len;
    
    LCM_LOG_BRINGUP("Lcm init serdes ffe begin.");    

    sal_memset(&serdes_ffe, 0, sizeof(ctc_chip_serdes_ffe_t));
    for(index = 0; index < E530_48T4X_PANEL_PORT_NUM_MAX; index++)
    {
        lcm_serdes_ffe = &lcm_e530_48t4x_serdes_ffe_max[index];

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

static int32
_lcm_init_e530_48p4x_poe(glb_card_t* p_card)
{
    uint8 i,port_id,poeid = 0;
    glb_port_t* p_port = NULL;
    glb_poe_port_info_t *p_poe_info=NULL;
    int32 ret;
    uint8 epld_idx[E530_48P4X_POE_NUM];
    i2c_gen_t i2c_gen[E530_48P4X_POE_NUM];
    poe_info_t poe_info[E530_48P4X_POE_NUM];
    poe_module_info_t poe_module[E530_48P4X_POE_NUM];

    sal_memset(i2c_gen, 0, sizeof(i2c_gen));
    sal_memset(poe_info, 0, sizeof(poe_info));
    sal_memset(poe_module, 0, sizeof(poe_module));

    /* release poe device */
    epld_item_write(0, EPLD_POE_RST, 0x1);
    /* disable all poe port */
    epld_item_write(0, EPLD_POE_DISABLE_PORT, 0x0);

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init poe module begin.");
    LCM_LOG_BRINGUP( "Lcm init poe module begin.");
    p_card->poe_module_num = E530_48P4X_POE_NUM;
    /* global poe info init */
    for(poeid=0;poeid<E530_48P4X_POE_NUM;poeid++)
    {
        p_card->poe_sys_info[poeid].pse_id = poeid;
        p_card->poe_sys_info[poeid].sys_cfg.enable = DEFAULT_POE_ENALBE; /* default configure */
        p_card->poe_sys_info[poeid].sys_cfg.power_limit = DEFAULT_POE_POWER_MAX;/* default power limit */
        p_card->poe_sys_info[poeid].status = 0;/* get from pse control */
        p_card->poe_sys_info[poeid].cur_consump = 0;
        p_card->poe_sys_info[poeid].aver_consump = 0;
        p_card->poe_sys_info[poeid].peak_consump = 0;
        p_card->poe_sys_info[poeid].pse_channel_num = 48; /* one pse power for 48 channel */
        p_card->poe_sys_info[poeid].const_power_limit = 320;
        poe_module[poeid].pse_id = poeid;
    }
    for (i = 0; i < E530_48P4X_POE_CHANNEL_NUM; i++)
    {
        port_id = lcm_48p4x_poe_entry[i].port_idx;
        p_port = p_card->pp_port[port_id];
        p_port->poe_support = 1;
        p_poe_info = &p_port->poe_info;
        p_poe_info->pse_id = lcm_48p4x_poe_entry[i].pse_id;
        p_poe_info->pse_mgmt_id = lcm_48p4x_poe_entry[i].pse_mgmt_id;
        p_poe_info->pse_channel_id = lcm_48p4x_poe_entry[i].pse_channel_id;
        p_poe_info->port_idx = port_id;
        p_poe_info->channel_cfg.enable = DEFAULT_POE_ENALBE;
        p_poe_info->channel_cfg.force_power = 0;
        p_poe_info->channel_cfg.mode = POE_AT;/* defalut poe mode is AF*/
        p_poe_info->channel_cfg.priority = POE_LOW;/* defalut poe priority is low */
        p_poe_info->channel_cfg.power_limit = DEFAULT_POE_AT_POWER;
        p_poe_info->class = 0;
        p_poe_info->status = 0;
        p_poe_info->cur_consump = 0;
        p_poe_info->aver_consump = 0;
        p_poe_info->peak_consump = 0;
        p_poe_info->current = 0;
        p_poe_info->voltage = 0;
        p_poe_info->temper = 0;
        
        poe_module[p_poe_info->pse_id].ch_info[p_poe_info->pse_channel_id].channel_id= p_poe_info->pse_channel_id;
        poe_module[p_poe_info->pse_id].ch_info[p_poe_info->pse_channel_id].mgmt_id= p_poe_info->pse_mgmt_id;
    }

    epld_idx[0] = 0;
    i2c_gen[0].bridge_flag = 0;
    i2c_gen[0].addr = E530_48P4X_POE0_ADDR;
    i2c_gen[0].i2c_type = E_I2C_CPM_RAW;
    i2c_gen[0].alen = POE_PD69200_OFFSET_WIDTH;
    i2c_gen[0].i2c_bus_idx = GLB_I2C_IDX_0;
    
    poe_info[0].chip_type = POE_PD69200;
    poe_info[0].io_type = POE_IO_I2C;
    poe_info[0].p_data_i2c = &i2c_gen[0];
    poe_info[0].p_data_epld = &epld_idx[0];
    sal_memcpy(&poe_info[0].info, &poe_module[0], sizeof(poe_module_info_t));

    ret = poe_init(poe_info, E530_48P4X_POE_NUM);
    if(ret < 0)
    {
        LCM_LOG_ERR("Poe driver init fail.");
        return LCM_E_INIT_FAILED;
    }
    for(i=0; i<E530_48P4X_POE_NUM; i++)
    {
        ret = poe_dev_init(i);
        if(ret < 0)
        {
            LCM_LOG_ERR("poe  %d dev init fail.",i);
            return LCM_E_INIT_FAILED;
        }
    }

    LCM_LOG_DEBUG(LCM_CARD, "Lcm init poe module end.");
    LCM_LOG_BRINGUP( "Lcm init poe module end.");
    return LCM_E_SUCCESS;
}

int32
lcm_init_e530_48t4x_cb(void)
{
    LCM_LOG_BRINGUP("Lcm init callback begin.");
    
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_fiber());    
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_phy());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_ctc_chip_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_serdes_ffe());

    LCM_LOG_BRINGUP( "Lcm init callback end.");    

    return LCM_E_SUCCESS;
}

/*Add by qianj for T65C,2019-04-28*/
int32
lcm_init_e530_48t4x_daughter_card_name(glb_card_t *p_card)
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

extern int32 lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func);
int32
lcm_init_e530_48t4x(glb_card_t* p_card)
{
    LCM_LOG_BRINGUP( "Lcm init board begin.");    
    glb_card = p_card;
    p_card->asic_chip_num = 1;
    p_card->chip_sensor_num = 1;

    lcm_common_get_slot_no(&p_card->logic_slot_no,&p_card->phy_slot_no);

    p_card->panel_port_num = E530_48T4X_PANEL_PORT_NUM;
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
    
    p_card->fan_module_num = E530_48T4X_FAN_MODULE_MAX;
    p_card->psu_module_num = E530_48T4X_PSU_MODULE_MAX;
    p_card->cpu_type = GLB_CPU_MAX;
    p_card->phy_chip_num = 6;
    p_card->phy_int_bit_num = E530_48T4X_PHY_INT_BITS_SUM;
    p_card->phy_interrupt_mode = GLB_PHY_INTERRUPT_MODE_NULL;
    
#ifndef _GLB_UML_SYSTEM_ 
    ctc_phy_fd_init();
#endif
    p_card->phy_intr_en = 1;
    p_card->p_port_range = lcm_e530_48t4x_phy_chip_port_range;
    /* for some board like e350 8t12xg don't support ptp, added by jcao for bug 25500, 2013-10-25 */
    p_card->ptp_en = 0;
    p_card->epld_type = GLB_EPLD_TYPE_MAX; 
    p_card->bootrom_type = E_BOOTROM_TYPE_4M; /*useless code*/

    sal_snprintf(p_card->cfg_profile_name, DATAPATH_LEN_MAX, "%s%s",
                    DATAPATH_PROFILE_PATH, E530_48T4X_DATAPATH_NAME);
    /* for bug 53243 by tongzb , support reboot info, 2019-08-22 */
    p_card->support_reboot_info = 1;

    lcm_common_init_cpu_info();
    lcm_common_config_ptp_enable(0);//config ptp
    lcm_common_config_syncE_enable(0);//config syncE
    lcm_common_config_reboot_record_type(REBOOT_FILE_EPLD,0xf6);
    LCM_LOG_BRINGUP("card product type %x, board type %x.", p_card->board_type.series, p_card->board_type.type);
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_i2c());
    LCM_IF_ERROR_RETURN(lcm_init_e530_48t4x_gpio());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_led());
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_sysenv_info(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_sysenv(p_card));
    /* 0 for FR4 board, 1 for M4 board, 2 for M6 board */
    p_card->board_material = BOARD_MATERIAL_FR4;
    
    LCM_LOG_BRINGUP( "Lcm init board material is %d.",p_card->board_material);    

    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_port(p_card));
    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_fan(p_card));

    LCM_IF_ERROR_RETURN(_lcm_init_e530_48t4x_psu(p_card));
    
    if(GLB_BOARD_E530_48P4X == p_card->board_type.type)
    {
		LCM_IF_ERROR_RETURN(_lcm_init_e530_48p4x_poe(p_card));
    }

    lcm_card_init_callback(lcm_init_e530_48t4x_cb);
    lcm_port_init_ffe_cfg_callback(lcm_e530_48t4x_get_sum_ffe_cfg);

    lcm_init_e530_48t4x_daughter_card_name(p_card);
    LCM_LOG_BRINGUP("Lcm init board end.");

    return LCM_E_SUCCESS;
}

