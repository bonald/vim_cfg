/****************************************************************************
 * lcm_port.c   :    set line card port status
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       ychen
 * Date         :       2010-08-16
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "lcm_port.h"
#include "lcm_specific.h"
#include "glb_hw_define.h"
#include "glb_phy_define.h"
#include "glb_if_define.h"
#include "glb_macro.h"
#include "lcapi.h"
#include "lcm_debug.h"
#include "lcm_mgt.h"
#include "lcm_msg.h"
#include "lcm_error.h"
#include "lcm_hagttlk.h"
#include "lcm_client.h"
#include "lcm_card.h"
#include "lcm_log.h"
#include "ctc_pci.h"
#include "phy_api.h"
#include "fiber_api.h"
#include "epld_api.h"
#include "led_api.h"
#include "poe_api.h"
#include <linux/if.h>
#include "ctc_port.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctc_chip.h"
#ifdef CNOS_OS
#include "ctclib_memory.h"
#include "ctc_task.h"
#include "ctc_debug.h"
#include "ctc_interrupt.h"
#include "laiinc.h"
#endif
/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/

extern int32 if_build_port_shortname_by_slot_port (char ifname[IFNAMSIZ], uint16 slot_id, uint16 port_id, uint16 sub_port_id);
extern int32 if_build_port_fullname_by_slot_port (char ifname[GLB_PHY_PORT_FULL_NAME_SZ], uint16 slot_id, uint16 port_id, uint16 sub_port_id);

/* jqiu modified for bug38657 40G/100G auto-neg. update interval from 3000 to 2600, reduce fail times. 2017-01-13*/
#define LCM_PORT_DEFAULT_CL73_AUTO_INTERVAL 2600

#define FIBER_MODULE_SCAN_TIME 2000
#define FIBER_MODULE_SCAN_NUM_ONE_ROUND    4
#define FIBER_MODULE_UPDATE_NUM_ONE_ROUND 4
#define FIBER_MODULE_ACCESS_ERR_NOT_TRY_MAX 3
#define FIBER_MODULE_POWER_ON_STABLE_TIME 1000
#define LCM_SYNCE_DISABLE_SLOT  0xFFFFFFFF

/* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
#ifndef _GLB_UML_SYSTEM_
static int32 g_port_timer_cnt = 0;
#endif
/* end of qicx added */
lcm_port_get_sum_ffe_cfg_callback_t lcm_port_get_sum_ffe_cfg_cb;
lcm_port_syce_enable_callback_t lcm_syce_enable_cb;
/* fix bug44604, liangf, 2017-07-06 */
lcm_port_cfg_mux_callback_t lcm_port_cfg_mux_cb;

#ifndef _GLB_UML_SYSTEM_
void lcm_port_cl73_auto_timer (void* p_data);
void lcm_port_copper_sfp_init(void* p_data);
void lcm_port_port_status_timer (void* p_data);
void lcm_port_led_cfg(void* p_data);
static int32
_lcm_port_mgt_led_mode(glb_port_t* p_port);
#if (!defined DUET2)&&(!defined TSINGMA)
LCM_DEFINE_TASK_ENCAP(lcm_port_cl73_auto_timer)
#endif
LCM_DEFINE_TASK_ENCAP(lcm_port_copper_sfp_init)
LCM_DEFINE_TASK_ENCAP(lcm_port_port_status_timer)
LCM_DEFINE_TASK_ENCAP(lcm_port_led_cfg)
#endif

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
typedef struct lcm_port_get_phy_type_param_s
{
    uint8 is_combo;
    glb_port_interface_t cfg_media;/*just useful when port is combo*/
    glb_port_interface_t linkup_media; /*just useful when port is combo and cfg media is auto*/
    glb_phy_type_t fiber_type;
    glb_phy_type_t default_type;
    
}lcm_port_get_phy_type_param_t;


/****************************************************************************
*  
* Function
*
*****************************************************************************/
/*Bug33637. update FFE when DAC/Fiber change.*/
int32
lcm_port_init_ffe_cfg_callback(lcm_port_get_sum_ffe_cfg_callback_t func)
{
    lcm_port_get_sum_ffe_cfg_cb = func;
    return LCM_E_SUCCESS;
}

/* for bug 42763 */
int32
lcm_port_init_syce_enable_callback(lcm_port_syce_enable_callback_t func)
{
    lcm_syce_enable_cb = func;
    return LCM_E_SUCCESS;
}

int32 
lcm_port_cfg_mux_callback(lcm_port_cfg_mux_callback_t func)
{
    lcm_port_cfg_mux_cb = func;
    return LCM_E_SUCCESS;
}

#ifndef _GLB_UML_SYSTEM_
glb_port_speed_t lcm_get_port_config_speed(glb_port_speed_t cfg_speed,glb_port_speed_t adjust_speed)
{
    glb_port_speed_t config_speed = GLB_SPEED_AUTO;
    if(cfg_speed == GLB_SPEED_AUTO)
    {
        config_speed = adjust_speed;
    }
    else
    {
        config_speed = cfg_speed;
    }
    return config_speed;
}

int32
lcm_get_port_ap_ffe_cfg(uint8 serdes_id, lcm_chip_serdes_ffe_t* ap_ffe)
{
    glb_card_t* p_card = NULL;

    p_card= lcm_mgt_get_card(); 
    /*28G serdes AP ffe cfg*/
    if(((serdes_id >=40)&&(serdes_id <= 47))
        ||((serdes_id >=88)&&(serdes_id <= 95)))
    {
        if((p_card->board_material == 1)||(p_card->board_material == 0)) //M4 or FR4
        {
            ap_ffe->c0 = 0;
            ap_ffe->c1 = 0x7;
            ap_ffe->c2 = 0x37;
            ap_ffe->c3 = 0x0;
        }
        else //M6
        {
            ap_ffe->c0 = 0;
            ap_ffe->c1 = 0x3;
            ap_ffe->c2 = 0x2b;
            ap_ffe->c3 = 0x12;
        }
    }
    else /* 15G serdes AP ffe cfg*/
    {        
        if((p_card->board_material == 1) ||(p_card->board_material == 2)) //M4 or M6
        {
            ap_ffe->c0 = 2;
            ap_ffe->c1 = 0x22;
            ap_ffe->c2 = 0xe;
        }
        else //FR4
        {
            ap_ffe->c0 = 0x3;
            ap_ffe->c1 = 0x24;
            ap_ffe->c2 = 0x12;
        }        
    }
    return 0;
}

int32 ctc_lcm_set_ffe(lcapi_hagt_set_ffe_t ffe_cfg)
{
    int32 ret=0;
    ctc_chip_serdes_ffe_t ffe_param;
    sal_memset(&ffe_param, 0, sizeof(ctc_chip_serdes_ffe_t));
    ffe_param.serdes_id = ffe_cfg.serdes;
    ffe_param.mode = ffe_cfg.mode;
    ffe_param.trace_len = ffe_cfg.trace_len;
    ffe_param.board_material = ffe_cfg.board_material;
    ffe_param.coefficient[0] = ffe_cfg.c0;
    ffe_param.coefficient[1] = ffe_cfg.c1;
    ffe_param.coefficient[2] = ffe_cfg.c2;
    ffe_param.coefficient[3] = ffe_cfg.c3;
    ret = ctc_chip_set_property(ffe_cfg.chip, CTC_CHIP_PROP_SERDES_FFE, (void*)&ffe_param);
    return ret;
}
/************************************************
 * bug33637. update ffe                               *
 * For serdes, DAC, optical or DAC auto-neg use diff ffe param.  *
 * For optical, use sum mode ffe param, not change until remove*
 * For DAC, when auto-neg off, need re-config ffe param.         *
 * for 40G/100G port, need cfg 4 lane, other just 1 lane.           *
 ************************************************/ 
int32
lcm_port_update_ffe(glb_port_t* p_port)
{
    uint8 i;
    uint8 serdes_num, is_DAC=0;
    uint8 fiber_ability;
    uint32 cl73_speed_ability;
    lcm_chip_serdes_ffe_t serdes_ffe;
    lcapi_hagt_set_ffe_t sum_ffe_cfg; 
#ifdef GOLDENGATE
    lcapi_hagt_set_ffe_t ap_ffe_cfg;
#endif
    LCM_LOG_DEBUG(LCM_CARD, "Lcm port update ffe.");

    if((p_port->port_speed_ability & (GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_100G)) != 0)
    {        
        serdes_num = 4;
    }
    else if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G) != 0)
    {
        serdes_num = 2;
    }    
    else
    {
        serdes_num = 1;
    }

    if((p_port->p_fiber != NULL) && (fiber_is_direct_attach_cable(p_port->p_fiber)))
    {
        is_DAC = 1;
    }

    sal_memset(&serdes_ffe, 0, sizeof(serdes_ffe));
    
    if(is_DAC)
    {
        /*If DAC in 25G/40G/100G port and enable auto-neg, needn't config ffe*/
        if((p_port->port_cfg.speed == GLB_SPEED_AUTO)||(p_port->port_cfg.duplex == GLB_DUPLEX_AUTO))
        {
            //if((serdes_num== 2)||(serdes_num== 4))
            //    return 0;
            /* for use this update ffe function, we need do update clause73 ability first(in lcm_update_clause73_ability) */
            phy_get_clause73_ability(p_port->port_idx-1,&cl73_speed_ability);
            if(cl73_speed_ability != 0)/* port and fiber do support auto-neg, so need not config ffe */
                return 0;           
        }
#ifdef GOLDENGATE  /* for D2, do not deal with DAC */ 
        for(i=0; i<serdes_num; i++)
        {
            lcm_get_port_ap_ffe_cfg(p_port->chip_serdes_id+i, &serdes_ffe);
            ap_ffe_cfg.chip = p_port->local_chip_idx;            
            ap_ffe_cfg.serdes = p_port->chip_serdes_id+i;
            if(p_port->port_cfg.ffe.config[i] != 0)/* has config ffe by cli */
            {
                ap_ffe_cfg.c0 = p_port->port_cfg.ffe.c0[i];
                ap_ffe_cfg.c1 = p_port->port_cfg.ffe.c1[i];
                ap_ffe_cfg.c2 = p_port->port_cfg.ffe.c2[i];
                ap_ffe_cfg.c3 = p_port->port_cfg.ffe.c3[i];
                ap_ffe_cfg.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
                ctc_lcm_set_ffe(ap_ffe_cfg);
            }
            else
            {
                ap_ffe_cfg.c0 = serdes_ffe.c0;
                ap_ffe_cfg.c1 = serdes_ffe.c1;
                ap_ffe_cfg.c2 = serdes_ffe.c2;
                ap_ffe_cfg.c3 = serdes_ffe.c3;
                ap_ffe_cfg.mode = CTC_CHIP_SERDES_FFE_MODE_3AP;
                ctc_lcm_set_ffe(ap_ffe_cfg);
            }            
        }
#endif        
    }
#ifdef GOLDENGATE
    else /* for bug 46136, support update ffe when config speed/duplex or insert/remove fiber */
#endif  
    {               
        for(i=0; i<serdes_num; i++)
        {
            sal_memset(&serdes_ffe, 0, sizeof(serdes_ffe));
            sal_memset(&sum_ffe_cfg, 0, sizeof(sum_ffe_cfg));
            phy_get_fiber_ability(p_port->port_idx-1, &fiber_ability,&is_DAC);
            serdes_ffe.serdes_id = p_port->chip_serdes_id+i;
            serdes_ffe.speed = p_port->port_cfg.speed==GLB_SPEED_AUTO?fiber_ability:p_port->port_cfg.speed;
            serdes_ffe.is_DAC = is_DAC;
            lcm_port_get_sum_ffe_cfg_cb(&serdes_ffe);
            sum_ffe_cfg.chip = serdes_ffe.lchip;            
            sum_ffe_cfg.serdes = serdes_ffe.serdes_id;
            sum_ffe_cfg.board_material = serdes_ffe.board_material;
            sum_ffe_cfg.mode = serdes_ffe.mode;
            sum_ffe_cfg.trace_len = serdes_ffe.trace_len;
            if(p_port->port_cfg.ffe.config[i] == 0)/* has no config ffe by cli */
            {
                sum_ffe_cfg.c0 = serdes_ffe.c0;
                sum_ffe_cfg.c1 = serdes_ffe.c1;
                sum_ffe_cfg.c2 = serdes_ffe.c2;
                sum_ffe_cfg.c3 = serdes_ffe.c3;
            }
            else
            {
                sum_ffe_cfg.c0 = p_port->port_cfg.ffe.c0[i];
                sum_ffe_cfg.c1 = p_port->port_cfg.ffe.c1[i];
                sum_ffe_cfg.c2 = p_port->port_cfg.ffe.c2[i];
                sum_ffe_cfg.c3 = p_port->port_cfg.ffe.c3[i];
                sum_ffe_cfg.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;   
            }
            ctc_lcm_set_ffe(sum_ffe_cfg);
        }
    }
    return 0;
}

/************************************************
 * Bug39456. 
 * For 28G serdes, DAC and optical should use diff rx_peak        
 * for 40G/100G port, need cfg 4 lane, other just 1 lane.         
 * If not 28G serdes, sdk will return and do nothing.                
 * This function call when Fiber insert.               
 ************************************************/
#ifdef CNOS_OS
#if (!defined DUET2)&&(!defined TSINGMA)
int32
lcm_port_update_rx_peaking(glb_port_t* p_port)
{
    uint8 i, serdes_num, peak_enable=1;
    lcapi_hagt_set_rx_peaking_t rx_peaking_cfg;
    ctc_chip_serdes_peaking_t serdes_peaking;
    LCM_LOG_DEBUG(LCM_CARD, "Lcm port update rx_peaking.");
    if((p_port->port_speed_ability & (GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_100G)) != 0)
    {        
        serdes_num = 4;
    }
    else if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G) != 0)
    {
        serdes_num = 2;
    }      
    else
    {
        serdes_num = 1;
    } 
    if((p_port->p_fiber != NULL) && (fiber_is_direct_attach_cable(p_port->p_fiber)))
    {
        peak_enable = 0;
    }
    rx_peaking_cfg.enable = peak_enable;
    rx_peaking_cfg.chip = p_port->local_chip_idx;
    for(i=0; i<serdes_num; i++)
    {
        rx_peaking_cfg.serdes = p_port->chip_serdes_id+i;
        //LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_SET_RX_PEAKING, &rx_peaking_cfg);
    
        serdes_peaking.serdes_id = rx_peaking_cfg.serdes;
        serdes_peaking.enable = rx_peaking_cfg.enable;
        ctc_chip_set_property(rx_peaking_cfg.chip, CTC_CHIP_PROP_SERDES_RX_PEAKING, (void*)&serdes_peaking);
        
    }
    return 0;
}
#endif
#endif

int32
lcm_update_fec_ability(glb_port_t* p_port, glb_port_speed_t cfg_speed)
{
    uint32 port_id;
    uint32 fec_ability=0;

    port_id = p_port->port_idx-1;
    if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
      ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)
#ifdef TSINGMA
      ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)
#endif
    )
    {
#if (defined DUET2)||(defined TSINGMA)
        /* open all FEC ability default */
        fec_ability |= CTC_PORT_CL73_FEC_ABILITY;
        fec_ability |= CTC_PORT_CL73_FEC_REQUESTED;
        fec_ability |= CTC_PORT_CL73_25G_RS_FEC_REQUESTED;
        fec_ability |= CTC_PORT_CL73_25G_BASER_FEC_REQUESTED;
        fec_ability |= CTC_PORT_CSTM_RS_FEC_ABILITY;
        fec_ability |= CTC_PORT_CSTM_RS_FEC_REQUESTED;
        fec_ability |= CTC_PORT_CSTM_BASER_FEC_ABILITY;
        fec_ability |= CTC_PORT_CSTM_BASER_FEC_REQUESTED;
        /* for FEC support port when auto or set to unsupport-fec speed, need disable all fec ability */
        if((cfg_speed != GLB_SPEED_100G)&&(cfg_speed != GLB_SPEED_50G)
            &&(cfg_speed != GLB_SPEED_25G)&&(cfg_speed != GLB_SPEED_AUTO)
#ifdef TSINGMA
            &&(cfg_speed != GLB_SPEED_40G)&&(cfg_speed != GLB_SPEED_10G)
#endif
        )
        {
            fec_ability &= (~CTC_PORT_CL73_25G_BASER_FEC_REQUESTED);
            fec_ability &= (~CTC_PORT_CSTM_BASER_FEC_ABILITY);
            fec_ability &= (~CTC_PORT_CSTM_BASER_FEC_REQUESTED);
            fec_ability &= (~CTC_PORT_CL73_25G_RS_FEC_REQUESTED);
            fec_ability &= (~CTC_PORT_CSTM_RS_FEC_ABILITY);
            fec_ability &= (~CTC_PORT_CSTM_RS_FEC_REQUESTED);
            fec_ability &= (~CTC_PORT_CL73_FEC_ABILITY);
            fec_ability &= (~CTC_PORT_CL73_FEC_REQUESTED);
        }
#endif
    }
    phy_update_fec_ability(port_id, fec_ability);

    return 0;
}

/************************************************
 * Bug39456. support CL73 auto-neg.
 * 1. Called periodly.
 * 2. get CL73 ability depend on port type and Fiber info;
 * 3. update ability form port to phy;
 * 4. optical fiber's ability is 0;
 ************************************************/
int32
lcm_update_clause73_ability(glb_port_t* p_port, glb_port_speed_t cfg_speed)
{
    uint32 port_id;
    uint32 ability=0;

    port_id = p_port->port_idx-1;
    /*If it's backplane, ability depend on phy type.*/
    if(p_port->phy_type == GLB_PHY_TYPE_100GBASE_KR4)
    {
        ability = CTC_PORT_CL73_100GBASE_KR4 | CTC_PORT_CL73_40GBASE_KR4;
    }
    else if(p_port->phy_type == GLB_PHY_TYPE_40GBASE_KR4)
    {
        ability = CTC_PORT_CL73_40GBASE_KR4;
    }
    else if(p_port->phy_type == GLB_PHY_TYPE_10GBASE_KR)
    {
        ability = CTC_PORT_CL73_10GBASE_KR;
    } 
    else /* If hot-plug, ability depend on Fiber type and speed config. */
    {
        /*For DAC, ability depend on port speed config/ability and DAC ability.*/
        if(fiber_is_direct_attach_cable(p_port->p_fiber))
        {
            if(cfg_speed == GLB_SPEED_100G)
            {
                ability = CTC_PORT_CL73_100GBASE_CR4;
            }
            else if(cfg_speed == GLB_SPEED_40G)
            {
                ability = CTC_PORT_CL73_40GBASE_CR4;
            }
#if (defined DUET2)||(defined TSINGMA)
            else if(cfg_speed == GLB_SPEED_50G)
            {
                ability = CTC_PORT_CSTM_50GBASE_CR2;
            }
            else if(cfg_speed == GLB_SPEED_25G)
            {
                ability = CTC_PORT_CL73_25GBASE_CR | CTC_PORT_CL73_25GBASE_CRS | CTC_PORT_CSTM_25GBASE_CR1;
            }
#endif
            /* for bug 47963, support 10G cl73 */
            /* 10G do not support cl73 now */
            //else if(cfg_speed == GLB_SPEED_10G)
            //{
            //    ability |= CTC_PORT_CL73_10GBASE_KR;
            //}

            else if(cfg_speed == GLB_SPEED_AUTO)
            {
                if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)
                {
                    ability |= CTC_PORT_CL73_40GBASE_CR4;
                }
#if (defined DUET2)||(defined TSINGMA)
                if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
                {
                    ability |= CTC_PORT_CSTM_50GBASE_CR2;
                }
                if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)
                    &&((((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_25GBASE_CR_L)
                        ||(((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_25GBASE_CR_S)
                        ||(((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_25GBASE_CR_N)
                        ||(((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE)                
                    ||(((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_100GBASE_CR4)))
                {
                    /* for bug 47967, add 10G speed for 25G port */
                    ability |= CTC_PORT_CL73_25GBASE_CR | CTC_PORT_CL73_25GBASE_CRS
                            | CTC_PORT_CSTM_25GBASE_CR1 | CTC_PORT_CL73_10GBASE_KR;
                }
#endif
                /*Both port has 100G speed ability and DAC has 100G ability, then enable 100G ability.*/
                if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)
                    &&(((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_100GBASE_CR4))
                {
                    ability |= CTC_PORT_CL73_100GBASE_CR4;
                }
                /* for bug 47549, when insert 10G cable, need add CTC_PORT_CL73_10GBASE_KR*/
                /* 10G do not support cl73 now */
                //if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)
                //    &&((((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER)
                //    ||(((fiber_info_t*)(p_port->p_fiber))->fiber_type == GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER)))
                //{
                //    ability |= CTC_PORT_CL73_10GBASE_KR;
                //}                
            }        
        }
    }
    phy_update_clause73_ability(port_id, ability);
    lcm_update_fec_ability(p_port,cfg_speed);
    return 0;
}

/*Update fiber ability depend on port speed ability and fiber type, trigger by fiber insert/remove*/
int lcm_get_fiber_speed_ability(glb_port_t* p_port)
{
    uint8 fiber_type;
    fiber_info_t *p_fiber = NULL;
    int32 fiber_speed_ability = 0;

    p_fiber = (fiber_info_t *)(p_port->p_fiber);
    if(!p_fiber)
    {
        return fiber_speed_ability;
    }

    fiber_type = p_fiber->fiber_type;
    
    if((fiber_type >= GLB_PHY_TYPE_100G_AOC)
        && (fiber_type <= GLB_PHY_TYPE_100GBASE_CR4))
    {
        /* for port split*/
        fiber_speed_ability = GLB_SUPPORT_SPEED_100G|GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_25G
        |GLB_SUPPORT_SPEED_10G|GLB_SUPPORT_SPEED_50G;
    }
    else if((fiber_type == GLB_PHY_TYPE_40GBASE_SR4)
          ||(fiber_type == GLB_PHY_TYPE_40GBASE_LR4)
          ||(fiber_type == GLB_PHY_TYPE_40GBASE_XLPPI)
          ||(fiber_type == GLB_PHY_TYPE_40GBASE_ER4)
          ||(fiber_type == GLB_PHY_TYPE_4X10GBASE_SR)
          ||(fiber_type == GLB_PHY_TYPE_40G_PSM4_SMF)
          ||(fiber_type == GLB_PHY_TYPE_40GBASE_CR4) /* for bug 46083 */
          ||(fiber_type == GLB_PHY_TYPE_40GBASE_KR4))          
    {
        fiber_speed_ability = GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_10G|GLB_SUPPORT_SPEED_1G;/* for port split*/
    }
    else if((fiber_type >= GLB_PHY_TYPE_10GBASE_ZR)
            && (fiber_type <= GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER))
    {
        fiber_speed_ability = GLB_SUPPORT_SPEED_10G|GLB_SUPPORT_SPEED_1G;
    }
    /* for bug 52401, support BASE-BX10 and BASE-PX fiber */
    else if(((fiber_type >= GLB_PHY_TYPE_1000BASE_T_SFP) && (fiber_type <= GLB_PHY_TYPE_1000BASE_ZX))
        ||(fiber_type == GLB_PHY_TYPE_BASE_BX10)||(fiber_type == GLB_PHY_TYPE_BASE_PX))
    {
        fiber_speed_ability = GLB_SUPPORT_SPEED_1G;
    }
    else if((fiber_type == GLB_PHY_TYPE_100BASE_FX)||(fiber_type == GLB_PHY_TYPE_100BASE_LX))
    {
        fiber_speed_ability = GLB_SUPPORT_SPEED_100M;
    }
    else if((fiber_type >= GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L)
            && (fiber_type <= GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE))
    {
        /* AVAGO 25GBASE_SR fiber do not support 10G */
        fiber_speed_ability = GLB_SUPPORT_SPEED_25G | GLB_SUPPORT_SPEED_10G;
    }

    return fiber_speed_ability;
}

void
lcm_update_fiber_ability(glb_port_t* p_port)
{
    uint32 port_id;
    uint8 is_DAC=0;
    fiber_info_t * p_fiber=NULL;

    int32 fiber_speed_ability = 0;
    int32 port_speed_ability = 0;
    int32 fiber_auto_adapt_speed = 0;
    int32 fiber_port_speed_adility = 0;

    fiber_speed_ability = lcm_get_fiber_speed_ability(p_port);
    port_speed_ability = p_port->port_speed_ability;

    fiber_port_speed_adility = fiber_speed_ability & port_speed_ability;
    if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_100G) == GLB_SUPPORT_SPEED_100G)
    {
        fiber_auto_adapt_speed = GLB_SPEED_100G;
    }
    else if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_50G) == GLB_SUPPORT_SPEED_50G)
    {
        fiber_auto_adapt_speed = GLB_SPEED_50G;
    }
    else if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_40G) == GLB_SUPPORT_SPEED_40G)
    {
        fiber_auto_adapt_speed = GLB_SPEED_40G;
    }
    else if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_25G) == GLB_SUPPORT_SPEED_25G)
    {
        fiber_auto_adapt_speed = GLB_SPEED_25G;
    }
    else if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_10G) == GLB_SUPPORT_SPEED_10G)
    {
        fiber_auto_adapt_speed = GLB_SPEED_10G;
    }
    else if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_1G) == GLB_SUPPORT_SPEED_1G)
    {
        fiber_auto_adapt_speed = GLB_SPEED_1G;
    }
    else if((fiber_port_speed_adility & GLB_SUPPORT_SPEED_100M) == GLB_SUPPORT_SPEED_100M)
    {
        fiber_auto_adapt_speed = GLB_SPEED_100M;
    }

    p_fiber = (fiber_info_t *)(p_port->p_fiber);
    if(p_fiber)
    {
        if(fiber_is_direct_attach_cable(p_fiber))
        {
            is_DAC = 1;
        }
    }
    port_id = p_port->port_idx - 1;
    phy_update_fiber_ability(port_id, fiber_auto_adapt_speed, is_DAC);
}

/* fix bug44604, liangf, 2017-07-06 */
void
lcm_port_update_mux_info(glb_port_t* p_port)
{  
    fiber_info_t *p_fiber = NULL;
    
    p_fiber = (fiber_info_t *)(p_port->p_fiber);
    //AOC && MELLANOX && MUX, liangf, 2017-07-06
    if(p_port->mux_enable  && lcm_port_cfg_mux_cb)
    {
        if (fiber_is_mellanox_cable(p_fiber))
        {                
            lcm_port_cfg_mux_cb(p_port->port_idx - 1, 1);
        }
        else
        {
            lcm_port_cfg_mux_cb(p_port->port_idx - 1, 0);
        }
    }
}

#if (defined DUET2)||(defined TSINGMA)
static uint8 fec_valid[GLB_CARD_PORT_NUM_MAX]={1};
#endif
int32
lcm_update_fec_config(glb_port_t* p_port,glb_port_speed_t cfg_speed,uint8 old_fec_config,uint8 *new_fec_config)
{
#if (defined DUET2)||(defined TSINGMA)
    uint8 fiber_ability,is_DAC;
    glb_port_speed_t port_config_speed;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    uint8 old_fec_valid = fec_valid[p_port->port_idx-1];
    fec_valid[p_port->port_idx-1] = 1;

    phy_get_fiber_ability(p_port->port_idx-1,&fiber_ability,&is_DAC);
    /* when insert fiber, need reset fec */
    /* for bug 45590 added by tongzb, reset fec_en for keep it same with manage_info when cooper to fiber */
    /* when fiber to cooper need also reset fec_en */
    port_config_speed = lcm_get_port_config_speed(cfg_speed,fiber_ability);

    *new_fec_config = old_fec_config;//keep fec config same as before
    
    /* default fec config is GLB_FEC_DEFAULT|GLB_FEC_ALL for all */
    if((old_fec_config&GLB_FEC_DEFAULT_MASK) == GLB_FEC_DEFAULT)
    {
        if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
            ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)
#ifdef TSINGMA
            ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)
#endif
        )
        {
            if(GLB_SPEED_100G == port_config_speed)
            {
                *new_fec_config = GLB_FEC_DEFAULT|GLB_FEC_RS;
            }
#ifdef TSINGMA            
            else if((GLB_SPEED_40G == port_config_speed)||(GLB_SPEED_10G == port_config_speed))
            {
                *new_fec_config  = GLB_FEC_DEFAULT|GLB_FEC_OFF;
            }
#endif            
            else if((GLB_SPEED_50G == port_config_speed)||(GLB_SPEED_25G == port_config_speed))
            {
                *new_fec_config  = GLB_FEC_DEFAULT|GLB_FEC_ALL;
            }
            else if(GLB_SPEED_AUTO == port_config_speed) /* for bug 50048 */
            {
                if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)
                {
                    *new_fec_config = GLB_FEC_DEFAULT|GLB_FEC_RS;
                }
                else if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
                {
                    *new_fec_config = GLB_FEC_DEFAULT|GLB_FEC_ALL;
                }
                else if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)
                {
                    *new_fec_config = GLB_FEC_DEFAULT|GLB_FEC_ALL;
                }
#ifdef TSINGMA                 
                else if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)
                {
                    *new_fec_config = GLB_FEC_DEFAULT|GLB_FEC_OFF;
                }
                else if(p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)
                {
                    *new_fec_config = GLB_FEC_DEFAULT|GLB_FEC_OFF;
                }
#endif                
                else
                {
                    *new_fec_config  = GLB_FEC_DEFAULT|GLB_FEC_ALL;
                }
            }
            //else 
            //{
            //    *new_fec_config  = GLB_FEC_DEFAULT|GLB_FEC_OFF;
            //}
        }
    }
    else/* if has fec config, here for check availability */
    {
        if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)
            ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G) 
#ifdef TSINGMA             
            ||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)||(p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)
#endif
        ) 
        {
            if_build_port_fullname_by_slot_port(port_name,p_port->panel_slot_no,p_port->panel_port_no,p_port->panel_sub_port_no);
            if(cfg_speed == GLB_SPEED_AUTO)
            {
                if((port_config_speed != GLB_SPEED_AUTO) 
                    &&(port_config_speed != GLB_SPEED_100G)
                    &&(port_config_speed != GLB_SPEED_50G)
                    &&(port_config_speed != GLB_SPEED_25G)
#ifdef TSINGMA
                    &&(port_config_speed != GLB_SPEED_40G)
                    &&(port_config_speed != GLB_SPEED_10G)            
#endif
                    )
                {
                    fec_valid[p_port->port_idx-1] = 0;
                }
            }
            else
            {
                if((port_config_speed != GLB_SPEED_25G)
                    &&(port_config_speed != GLB_SPEED_50G)
                    &&(port_config_speed != GLB_SPEED_100G)
#ifdef TSINGMA
                    &&(port_config_speed != GLB_SPEED_40G)
                    &&(port_config_speed != GLB_SPEED_10G)
#endif
                ) 
                {
                    fec_valid[p_port->port_idx-1] = 0;
                }
            }
              
            if((old_fec_config&GLB_FEC_MASK) == GLB_FEC_BASE_R)
            { /* 100G can not support fec baser */
                if(port_config_speed == GLB_SPEED_100G)
                {
                    fec_valid[p_port->port_idx-1] = 0;
                }
            }
#ifdef TSINGMA               
            else if((old_fec_config&GLB_FEC_MASK) == GLB_FEC_RS)
            {
                if((port_config_speed == GLB_SPEED_40G)||(port_config_speed == GLB_SPEED_10G))
                {
                    fec_valid[p_port->port_idx-1] = 0;
                }
            }
#endif             
        }
    }
    if((old_fec_valid != fec_valid[p_port->port_idx-1])&&(fec_valid[p_port->port_idx-1] == 0))/* there is fec conflict */
    {
        LCM_LOG_USER(E_ERROR,LCM_3_PORT_FEC_CONFLICT,port_name);/* print to diag log */
        LCM_LOG_ERR("%% Port %s fec configure is invalid.\n", port_name);/* print to log */
        LCM_LOG_PTS("%% Port %s fec configure is invalid.\n", port_name);/* print to tty when no terminal monitor */
        LCM_LOG_CONSOLE("%% Port %s fec configure is invalid.\n", port_name);/* print to console when no terminal monitor */
        return LCM_E_INVALID_PARAM;
    }
#endif     
    return 0;
}

int32
lcm_fiber_poll_event(glb_port_t* p_port)
{
    uint8 new_fec_config,old_fec_config;
    if(p_port->fiber_triger)
    {
        p_port->fiber_triger = 0;
    }
    else
    {
        return 0;
    }
    lcm_port_update_mux_info(p_port);
    lcm_update_fiber_ability(p_port);
    lcm_update_clause73_ability(p_port, p_port->port_cfg.speed);

    /* for bug 50023, 50222, config fec after speed set */
    phy_get_fec_en(p_port->port_idx-1,&old_fec_config);//get soft configure of fec
    lcm_update_fec_config(p_port,p_port->port_cfg.speed,old_fec_config,&new_fec_config);//adjust fec configure
    phy_update_fec_en(p_port->port_idx-1,new_fec_config);//configure soft configure of fec

    phy_cfg_speed(p_port->port_idx - 1, p_port->port_cfg.speed);
    phy_update_fec_en(p_port->port_idx-1,old_fec_config);//roll back soft configure of fec

#if defined (GOLDENGATE) || defined (DUET2) ||(defined TSINGMA)
    /*Bug42367. after config speed, update ffe and rx_peak. Because ffe depend on speed.*/
    lcm_port_update_ffe(p_port);
#ifdef CNOS_OS
#if (!defined DUET2)&&(!defined TSINGMA)
    lcm_port_update_rx_peaking(p_port);    
#endif
#endif
    
#endif
    return 0;
}

#endif

/***************************************************************************************************
 * Name         : lcm_get_port_by_panel_info 
 * Purpose      : get panel slot and port map to port_id           
 * Input        : panel slot, panel port
 * Output       : port id(start from 1.)             
 * Return       : LCM_E_SUCCESS
 * Note         : 
***************************************************************************************************/
/*Fix bug 14686. jqiu 2011-06-15*/
int32
lcm_get_port_by_panel_info(uint8 panel_slot, uint8 panel_port, uint8 panel_subport, uint8* port_id)
{
    glb_card_t* p_card = NULL;
    uint8 i;
    
    /*get local line card*/
    p_card= lcm_mgt_get_card(); 
    for(i=0; i<p_card->port_num; i++)
    {
        if((p_card->pp_port[i]->panel_slot_no == panel_slot)&&(p_card->pp_port[i]->panel_port_no == panel_port)
            &&(p_card->pp_port[i]->panel_sub_port_no == panel_subport))
        {
            *port_id = i+1;
            return LCM_E_SUCCESS;
        }
    }
    return LCM_E_NOT_FOUND;
}

static int32 _lcm_port_get_status_of_port(lcapi_lcm_get_port_status_t *p_port_status,uint8 port_idx)
{
    int32 ret = 0;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;

    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }

    p_port = p_card->pp_port[port_idx - 1];
    p_port_status->phy_type = GLB_PORT_PHY_TYPE(p_port)|p_port->phy_type; 
  
    p_port_status->enable = p_port->port_status.link_up;
    p_port_status->speed = p_port->port_status.speed;
    p_port_status->duplex = p_port->port_status.duplex;
    p_port_status->flowctrl.send = p_port->port_status.flowctrl.send;
    p_port_status->flowctrl.recv = p_port->port_status.flowctrl.recv;
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    p_port_status->eee_status.status= p_port->port_status.eee_status.status;
    p_port_status->eee_status.wake_error_count = p_port->port_status.eee_status.wake_error_count;
    /* End of liuht modified */
    /* Modified by liuht for bug 29005, 2014-06-17 */
    p_port_status->port_status_change = p_port->port_status.port_status_change;
    /* End of liuht modified */
    p_port_status->fec_status = p_port->port_status.fec_status;
    p_port_status->cl73_ability = p_port->port_status.cl73_ability; 

    LCM_LOG_DEBUG(LCM_LCMGT, "port %d status: speed %d, duplex %d, flowctrl send %d, recv %d,media type 0x%x, enable %d, \
eee status 0x%x, wake_error_count %d, port_status_change 0x%x,p_port->port_status.link_up = %d", 
                  port_idx-1, p_port_status->speed, p_port_status->duplex, p_port_status->flowctrl.send, 
                  p_port_status->flowctrl.recv, p_port_status->phy_type, p_port_status->enable, 
                  p_port_status->eee_status.status, p_port_status->eee_status.wake_error_count,
                  p_port_status->port_status_change,p_port->port_status.link_up);

    return ret;
}

/***************************************************************************************************
 * Name         : lcm_port_get_port_status 
 * Purpose      : get port status for speed, duplex and media type          
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
#ifdef CNOS_OS
int32
lcm_port_notify_port_status (void* pv_arg)
{
    lcapi_lcm_get_port_status_t* p_rcv_port_status = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    //int32 ifindex = 0;
    //lcm_if_phy_state_notify_t notify;
    lcapi_lcm_get_port_status_t port_status;
    lcapi_lcm_get_port_status_t* p_port_status;

    lai_port_info_t lai_port;
    lai_object_id_t oid = 0;

    int32 ret = 0;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
  
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    sal_memset(&port_status, 0, sizeof(port_status));
    p_rcv_port_status = (lcapi_lcm_get_port_status_t*)pv_arg;    
    /*get port index*/
    panel_slot = p_rcv_port_status->slot_no;
    panel_port = p_rcv_port_status->port_no;
    panel_subport = p_rcv_port_status->sub_port_no;

    p_port_status = &port_status;
    
    /*Fix bug 14686. jqiu 2011-06-15*/
    if_build_ethname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__,port_name);
        LCM_LOG_BRINGUP("%s Can't find port idx for %s",__FUNCTION__,port_name);
        return LCM_E_NOT_FOUND;
    }
    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }

    p_port = p_card->pp_port[port_idx - 1];  
    
    LCM_LOG_DEBUG(LCM_LCMGT, "Get interface %s status",port_name);
    ret = _lcm_port_get_status_of_port(p_port_status,port_idx);

#if 0 /* do not use on CNOS */
    /*send port status to switchd*/
    sal_memset(&notify, 0, sizeof(notify));
    ifindex = if_get_ifindex_by_name(port_name);
    if (ifindex < 0)
    {
        LCM_LOG_ERR(" _hsrv_msg_if_set_phy_state rx non-exist interface %s", notify.ifname);
        LCM_LOG_BRINGUP(" _hsrv_msg_if_set_phy_state rx non-exist interface %s", notify.ifname);
    }
    notify.ifindex = ifindex;
    notify.speed = p_port_status->speed;
    notify.duplex = p_port_status->duplex;
    notify.link_up = p_port_status->enable;
    notify.flowctrl_send = p_port_status->flowctrl.send;
    notify.flowctrl_recv = p_port_status->flowctrl.recv;
    notify.media = p_port_status->phy_type;
    sal_memcpy(notify.ifname, port_name, 33);
#endif
    /* send to switch */
        /*add port lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_port_event)
    {
        oid = (p_card->phy_slot_no<<16) + p_port->port_idx -1 ;
        
        LCM_IF_ERROR_RETURN(ctc_lai_get_port_info(oid, &lai_port));
        if(p_port_status->enable == GLB_LINK_UP)
        {
            p_lai_card_noti->on_port_event(LAI_PORT_EVENT_UP, &lai_port);
        }
        else
        {
            p_lai_card_noti->on_port_event(LAI_PORT_EVENT_DOWN, &lai_port);
        }

    }

    return ret;
}
#endif

/***************************************************************************************************
 * Name         : hagt_lcm_set_mac_speed 
 * Purpose      : callback function by lcm, set mac speed
 * Input        : 
 * Output       : N/A              
 * Return       : SUCCESS HAGT_E_NONE
 * Note         : 
***************************************************************************************************/
int32
lcm_set_mac_speed(void* p_arg)
{
    int ret=0;
    lcapi_hagt_set_mac_speed_t* p_set_mac_speed;
    ctc_port_speed_t ctc_port_speed;
        
    if(NULL == p_arg)
    {
        return LCM_E_INVALID_PTR;
    }

    p_set_mac_speed = (lcapi_hagt_set_mac_speed_t*)p_arg;
    switch(p_set_mac_speed->speed_mode)
    {
        case GLB_SPEED_10M:
            ctc_port_speed = CTC_PORT_SPEED_10M;
            break;
        case GLB_SPEED_100M:
            ctc_port_speed = CTC_PORT_SPEED_100M;
            break;
        case GLB_SPEED_1G:
            ctc_port_speed = CTC_PORT_SPEED_1G;
            break;
        case GLB_SPEED_AUTO:
        /*added by jcao for 10G, don't need to config chip*/    
        case GLB_SPEED_2_5G:
        case GLB_SPEED_5G:
        case GLB_SPEED_10G:
        case GLB_SPEED_40G:
        case GLB_SPEED_100G:
        case GLB_SPEED_50G:
        case GLB_SPEED_25G:
            return 0;
        default:
            return LCM_E_NOT_FOUND;
    }
    /* for bug 52771, do not care about the return of mac speed set */
    ret = ctc_port_set_speed(p_set_mac_speed->gport, ctc_port_speed);
    LCM_LOG_DEBUG(LCM_CARD,"Lcm set port speed %d, ret %d",ctc_port_speed,ret);
#if 0    
    if(CTC_E_INVALID_PORT_MAC_TYPE == ret)
        return LCM_E_SUCCESS;
    else
        return ret;
#endif    
    return LCM_E_SUCCESS;
}

/***************************************************************************************************
 * Name         : hagt_lcm_set_mac_flowcntrl
 * Purpose      : callback function by lcm, set mac flowcntrl
 * Input        : 
 * Output       : N/A              
 * Return       : SUCCESS HAGT_E_NONE
 * Note         : 
***************************************************************************************************/
int32
lcm_set_mac_flowcntrl(void* p_arg)
{
    lcapi_hagt_set_mac_flowctrl_t* p_set_mac_en;
    uint16 gportid;
    int32 ret = 0;
    ctc_port_fc_prop_t fc;

    if(NULL == p_arg)
    {
        return LCM_E_INVALID_PTR;
    }

    p_set_mac_en = (lcapi_hagt_set_mac_flowctrl_t*)p_arg;
    gportid = p_set_mac_en->gport;

    /* modified by chenxw for bug 24578, 2013-08-22. */ 
    sal_memset(&fc, 0, sizeof(fc));

    LCM_LOG_DEBUG(LCM_CARD,"Lcm set flowctrl send %d recv %d",p_set_mac_en->flowctrl.send,p_set_mac_en->flowctrl.recv);

    /*unsupport 10 G port*/
    /* flow control receive */
    // TODO:  merge sdk 2.1.13
    fc.gport = gportid;
    fc.priority_class = 0;
    fc.enable = p_set_mac_en->flowctrl.recv;
    fc.dir = CTC_INGRESS;
    fc.is_pfc = 0;
    ret += ctc_port_set_flow_ctl_en(&fc);
    
    /* flow control send */
    fc.dir = CTC_EGRESS;
    /* modified by chenxw for bug 24578, 2013-08-22. */ 
    fc.enable = p_set_mac_en->flowctrl.send;
    
    ret += ctc_port_set_flow_ctl_en(&fc);

    return ret;
    
}

/***************************************************************************************************
 * Name         : lcm_speed_adjust 
 * Purpose      : callback function by lcm, set mac serdes mode
 * Input        : 
 * Output       : N/A              
 * Return       : SUCCESS HAGT_E_NONE
 * Note         : 
***************************************************************************************************/
int32
lcm_set_serdes_mode(glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    int ret=0;
    uint16 gport;
    ctc_chip_serdes_mode_t serdes_mode;
    ctc_port_speed_t ctc_port_speed;
    ctc_chip_serdes_info_t serdes_info;
    lcapi_hagt_set_mac_flowctrl_t p_set_flowctrl;

    sal_memset(&serdes_info, 0, sizeof(ctc_chip_serdes_info_t));

    ctc_port_speed = CTC_PORT_SPEED_MAX;
    gport = p_port->g_port_index; 
    switch(phy_val->speed)
    {
        case GLB_SPEED_5G:
        case GLB_SPEED_10G:
            serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
            break;
#ifdef GREATBELT
        case GLB_SPEED_1G:
            serdes_mode = CTC_CHIP_SERDES_XSGMII_MODE;
            break;
#else
        case GLB_SPEED_2_5G:
            serdes_mode = CTC_CHIP_SERDES_2DOT5G_MODE;
            break;
        case GLB_SPEED_1G:
            serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
            ctc_port_speed = CTC_PORT_SPEED_1G;
            break;
        case GLB_SPEED_100M:
            serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
            ctc_port_speed = CTC_PORT_SPEED_100M;
            break;
        case GLB_SPEED_10M:
            serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
            ctc_port_speed = CTC_PORT_SPEED_10M;
            break;
#endif
        default:
            return 0;
    }

    //TODO for switch version < GB1.2 should not disable mac
    if(serdes_mode != p_port->serdes_mode)
    {
        LCM_LOG_DEBUG(LCM_CARD,"Lcm switch serdes mode to %d",serdes_mode);
        ctc_port_set_mac_en(gport, 0);
        serdes_info.serdes_id = p_port->chip_serdes_id;
        serdes_info.serdes_mode = serdes_mode;
        ret = ctc_chip_set_serdes_mode(p_port->glb_chip_idx, &serdes_info);
        if(!ret)
        {
            p_port->serdes_mode = serdes_mode;
        }
        /* update serdes mode only when port linkup, so just enable mac */
        ctc_port_set_mac_en(gport, 1);
    }

    /* for GG 1G/100M/10M need set speed */
    if(CTC_PORT_SPEED_MAX != ctc_port_speed)
    {
        LCM_LOG_DEBUG(LCM_CARD,"Lcm set port speed %d",ctc_port_speed);
        ret += ctc_port_set_speed(gport, ctc_port_speed);
    }

    /* for 5G port, need enable flowctrl */
    if(GLB_SPEED_5G == phy_val->speed)
    {
        p_set_flowctrl.gport = gport;
        p_set_flowctrl.flowctrl.send = 1;
        p_set_flowctrl.flowctrl.recv = 1;
    }
    else
    {
        p_set_flowctrl.gport = gport;
        p_set_flowctrl.flowctrl.send = phy_val->flowctrl.send;
        p_set_flowctrl.flowctrl.recv = phy_val->flowctrl.recv;
    }
    ret += lcm_set_mac_flowcntrl(&p_set_flowctrl);
    
    /* for 1G/100M/10M mode, need enable auto, 2.5G mode, need disable auto neg */
    if((GLB_SPEED_1G == phy_val->speed)
        ||(GLB_SPEED_100M == phy_val->speed)
        ||(GLB_SPEED_10M == phy_val->speed))
    {
        LCM_LOG_DEBUG(LCM_CARD,"Lcm set port auto neg enable");
        ctc_port_set_auto_neg(gport, 1);
    }
    else if(GLB_SPEED_2_5G == phy_val->speed)
    {
        LCM_LOG_DEBUG(LCM_CARD,"Lcm set port auto neg disable");
        ctc_port_set_auto_neg(gport, 0);
    }
    
    return ret;
}

#if 0
int32
lcm_if_set_phy_state(glb_phy_state_t* pst_notify)
{
    glb_phy_state_t phy_state;

    sal_memset(&phy_state, 0, sizeof(phy_state));
    phy_state.linkup_media = 0;
    phy_state.speed = pst_notify->speed;
    phy_state.duplex = pst_notify->duplex;
    phy_state.link_up = pst_notify->link_up;
    phy_state.flowctrl.send = pst_notify->flowctrl_send;
    phy_state.flowctrl.recv = pst_notify->flowctrl_recv;

    /* send to switch */
    HSRV2SWITCH_MSG_SEND_NOTIFY(SWITCH_FEI_PHY_NOTIFY, pst_notify, sizeof(fei_if_phy_state_notify_t));
    return 0;
}
#endif 

/*Fix bug 14685. jqiu 2011-06-01. when combo port phy type change, update real information to nsm.*/
/*lcm update fiber information to other module.
  notify_flag: LCM_FIBER_INSERT, LCM_FIBER_REMOVE*/
int32
lcm_notify_fiber_info(glb_port_t* p_port, lcm_fiber_notify_event_e notify_flag)
{
    lcapi_lcm_port_info_t panel_port_info;
    int32 ret;

    if(!p_port || !p_port->create_done)
    {
        LCM_LOG_DEBUG(LCM_CARD, "Lcm port not create.");
        return LCM_E_SUCCESS;
    }
    if(notify_flag == LCM_FIBER_INSERT)
        lcm_msg_tx_lcFiberAttach(p_port, 0);
    else
        lcm_msg_tx_lcFiberDeAttach(p_port, 0);

    /*modified by zhuj for bug 13406, 2010-11-19*/
    panel_port_info.port_no = p_port->panel_port_no;
    panel_port_info.sub_port_no= p_port->panel_sub_port_no;
    panel_port_info.slot_no= p_port->panel_slot_no;
#ifdef CNOS_OS
    ret = lcm_port_notify_port_status(&panel_port_info);
#else
    ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_LINK_STATUS, &panel_port_info);
#endif
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm update port idx %d status fail.", p_port->port_idx);
        return LCM_E_INIT_FAILED;
    }
    return LCM_E_SUCCESS;
}

glb_phy_type_t lcm_port_get_phy_type(lcm_port_get_phy_type_param_t* param)
{
    glb_phy_type_t phy_type=GLB_PHY_TYPE_UNKNOWN;

    /*For combo port, need to check linkup media*/
    if(param->is_combo)
    {
        if(param->cfg_media == GLB_PORT_TYPE_AUTO)
        {
            if(param->linkup_media == GLB_PORT_TYPE_COPPER)
            {
                phy_type = GLB_PHY_TYPE_1000BASE_T;
            }
            else if(param->linkup_media == GLB_PORT_TYPE_AUTO)
            {
                phy_type = GLB_PHY_TYPE_UNKNOWN;
            }
            else
            {
                phy_type = param->fiber_type;
            }
        }
        else if((param->cfg_media == GLB_PORT_TYPE_COPPER)
                ||(param->cfg_media == GLB_PORT_TYPE_SGMII))
        {
            phy_type = GLB_PHY_TYPE_1000BASE_T;/* modified by tongzb for bug 41009 2016-10-10, dual bug 44709 */
        }
        else
        {
            phy_type = param->fiber_type;
        }
    }
    else /*for not combo port, copper port is always 1000baseT, fiber port's phy type is same with fiber type*/
    {
        if((param->cfg_media == GLB_PORT_TYPE_COPPER)
            ||(param->cfg_media == GLB_PORT_TYPE_SGMII))
        {
            phy_type = param->default_type;
        }
        else
        {
            phy_type = param->fiber_type;
        }
    }
    return phy_type;
}
#ifndef _GLB_UML_SYSTEM_
/* bug23865. support copper SFP
  port medium depend on 3 param: media, speed, phy/no phy, and fiber type.
  media : copper, fiber, auto
  speed : 10M, 100M, 1G, 10G
  phy/no phy: GB direct to SFP is no phy, GB to phy to SFP is phy.
  fiber : 100Base-X, 1000base-X, serdes copper SFP, sgmii copper SFP
   */
int32 
lcm_port_update_medium(uint32 port_id, glb_port_interface_t media, glb_port_speed_t speed)
{
#ifdef _GGEMU_
    return 0; // TODO: removed by xgu for gg emulation board, 2014-6-3
#else
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    fiber_info_t* p_fiber;
    
    p_card= lcm_mgt_get_card();
    p_port = p_card->pp_port[port_id]; 
    LCM_LOG_DEBUG(LCM_LCMGT, "Port %d update medium media %d speed %d fiber %d", 
        port_id+1, media, speed, (p_port->p_fiber==NULL)?0xff:(((fiber_info_t*)p_port->p_fiber)->fiber_type));
    /* has SFP inserted*/
    if(p_port->p_fiber != NULL)
    {
        p_fiber = (fiber_info_t*)p_port->p_fiber;
        //media auto, mean may be copper or fiber, need process AMS
        if(media == GLB_PORT_TYPE_AUTO)
        {
            //speed must be auto when media is auto
            if(speed != GLB_SPEED_AUTO)
            {
                return LCM_E_INVALID_SPEED;
            }
            // has no phy, GB<->SFP
            if(phy_check_no_phy(port_id))
            {
                // GB port not support media auto when has no phy.
                return LCM_E_INVALID_MEDIA;
            }
            else //has phy, GB<->PHY<->SFP
            {
                //100Base-X SFP
                if((p_fiber->fiber_type == GLB_PHY_TYPE_100BASE_FX)||(p_fiber->fiber_type == GLB_PHY_TYPE_100BASE_LX))
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_AMS_FI_100FX);
                }
                // copper SFP
                else if(p_fiber->fiber_type == GLB_PHY_TYPE_1000BASE_T_SFP)
                {
                    //SGMII Copper SFP
                    if(phy_check_attach_phy(port_id))
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_AMS_FI_PASSTHRU);
                    }
                    //Serdes Copper SFP
                    else
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_AMS_FI_1000BX);
                    }
                }
                //1000BASE-X SFP
                else
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_AMS_FI_1000BX);
                }
            }
        }
        else if(media == GLB_PORT_TYPE_FIBER)
        {
            //GB Fiber not support 10M, 100M. 
            //1000BASE-X and 10GBASE-R dependon port speed ability
            if((speed == GLB_SPEED_10M)
              ||((speed == GLB_SPEED_100M)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100M)==0))  
              ||((speed == GLB_SPEED_1G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_1G)==0))
              ||((speed == GLB_SPEED_10G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)==0))
              ||((speed == GLB_SPEED_40G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)==0))
              ||((speed == GLB_SPEED_100G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)==0))
              ||((speed == GLB_SPEED_25G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)==0))
              ||((speed == GLB_SPEED_50G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)==0)))
            {
                return LCM_E_INVALID_SPEED;
            }
            // has no phy, GB<->SFP
            if(phy_check_no_phy(port_id))
            {                
                return LCM_E_SUCCESS;
            }
            //has phy, GB<->PHY<->SFP
            else
            {
                if(speed == GLB_SPEED_1G)
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_1000BX);
                }
                else if(speed == GLB_SPEED_100M)
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_FI_100FX);
                }
                else if(speed == GLB_SPEED_AUTO)
                {
                    //100Base-X SFP
                    if((p_fiber->fiber_type == GLB_PHY_TYPE_100BASE_FX)||(p_fiber->fiber_type == GLB_PHY_TYPE_100BASE_LX))
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_FI_100FX);
                    }
                    // copper SFP
                    else if(p_fiber->fiber_type == GLB_PHY_TYPE_1000BASE_T_SFP)
                    {
                        //SGMII Copper SFP
                        if(phy_check_attach_phy(port_id))
                        {
                            phy_cfg_medium(port_id, GLB_MEDIA_PASS_THROUGH);
                        }
                        //Serdes Copper SFP
                        else
                        {
                            phy_cfg_medium(port_id, GLB_MEDIA_1000BX);
                        }
                    }
                    //1000BASE-X SFP
                    else
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_1000BX);
                    }
                }
                else if(speed == GLB_SPEED_10G)
                {
                    if(check_fiber_is_XFP(port_id))
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_XFP);
                    }
                    else
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_SFP_PLUS);
                    }
                }
                else
                {
                    return LCM_E_INVALID_SPEED;
                }
            }
        }
        //media is copper, must has phy
        else if(media == GLB_PORT_TYPE_COPPER)
        {
            phy_cfg_medium(port_id, GLB_MEDIA_RJ45);
            return LCM_E_SUCCESS;
        }
        else if(media == GLB_PORT_TYPE_SGMII)
        {
            phy_cfg_medium(port_id, GLB_MEDIA_PASS_THROUGH);
            return LCM_E_SUCCESS;
        }
        else
        {
            return LCM_E_INVALID_MEDIA;
        }
    }
    else /* No SFP inserted*/ 
    {
        if(media == GLB_PORT_TYPE_AUTO)
        {
            if(speed != GLB_SPEED_AUTO)
            {
                return LCM_E_INVALID_SPEED;
            }
            // has no phy, GB<->SFP
            if(phy_check_no_phy(port_id))
            {
                // GB port not support media auto when has no phy.
                return LCM_E_INVALID_MEDIA;
            }
            else //has phy, GB<->PHY<->SFP, Not SFP, default as 1000BASE-X
            {
                phy_cfg_medium(port_id, GLB_MEDIA_AMS_FI_1000BX);
            }
        }
        else if(media == GLB_PORT_TYPE_FIBER)
        {
            //GB Fiber not support 10M, 100M. 
            //1000BASE-X and 10GBASE-R dependon port speed ability
            if((speed == GLB_SPEED_10M)
              ||((speed == GLB_SPEED_100M)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100M)==0))  
              ||((speed == GLB_SPEED_1G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_1G)==0))
              ||((speed == GLB_SPEED_10G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)==0))
              ||((speed == GLB_SPEED_40G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_40G)==0))
              ||((speed == GLB_SPEED_100G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_100G)==0))
              ||((speed == GLB_SPEED_25G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_25G)==0))
              ||((speed == GLB_SPEED_50G)&&((p_port->port_speed_ability & GLB_SUPPORT_SPEED_50G)==0)))
            {
                return LCM_E_INVALID_SPEED;
            }
            // has no phy, GB<->SFP
            if(phy_check_no_phy(port_id))
            {
                return LCM_E_SUCCESS;
            }
            //has phy, GB<->PHY<->SFP
            else
            {
                if(speed == GLB_SPEED_1G)
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_1000BX);
                }
                else if(speed == GLB_SPEED_100M)
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_FI_100FX);
                }
                else if(speed == GLB_SPEED_AUTO)
                {
                    phy_cfg_medium(port_id, GLB_MEDIA_1000BX);
                }
                else if(speed == GLB_SPEED_10G)
                {
                    if(check_fiber_is_XFP(port_id))
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_XFP);
                    }
                    else
                    {
                        phy_cfg_medium(port_id, GLB_MEDIA_SFP_PLUS);
                    }
                }
                else
                {
                    return LCM_E_INVALID_SPEED;
                }
            }
        }
        //media is copper
        else if(media == GLB_PORT_TYPE_COPPER)
        {
            phy_cfg_medium(port_id, GLB_MEDIA_RJ45);
            return LCM_E_SUCCESS;
        }
        else if(media == GLB_PORT_TYPE_SGMII)
        {
            phy_cfg_medium(port_id, GLB_MEDIA_PASS_THROUGH);
            return LCM_E_SUCCESS;
        }
        else
        {
            return LCM_E_INVALID_MEDIA;
        }
    }
    return LCM_E_SUCCESS;
#endif
}
#endif

/***************************************************************************************************
 * Name         : lcm_port_set_speed 
 * Purpose      : set port speed           
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_speed(void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    int32 ret=LCM_E_SUCCESS;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
#ifndef _GLB_UML_SYSTEM_    
    uint8 new_fec_config,old_fec_config;
#endif
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;    
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if (lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  speed %d", port_idx, p_port_status->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }

    p_port = p_card->pp_port[port_idx - 1];
#ifndef _GGEMU_    // TODO:modified by xgu for gg emuation board, 2014-6-2
#ifdef _GLB_UML_SYSTEM_
    if (GLB_SPEED_AUTO == p_port_status->status)
    {
        p_port_status->status = GLB_SPEED_1G;
    }
    p_port->port_status.speed = p_port_status->status;
#else
    ret = lcm_port_update_medium(port_idx - 1, p_port->port_cfg.media, p_port_status->status);
    if_build_port_shortname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
    if(ret != LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("%s port %d update medium error %d when set speed %d media %d.", 
                    port_name, port_idx-1, ret, p_port_status->status, p_port->port_cfg.media);
        return ret;
    }
    lcm_update_clause73_ability(p_port, p_port_status->status);

    phy_get_fec_en(p_port->port_idx-1,&old_fec_config);//get soft configure of fec
    lcm_update_fec_config(p_port,p_port_status->status,old_fec_config,&new_fec_config);//adjust fec configure
    phy_update_fec_en(p_port->port_idx-1,new_fec_config);//configure soft configure of fec
    
    phy_cfg_speed(port_idx - 1, p_port_status->status);
    phy_update_fec_en(p_port->port_idx-1,old_fec_config);//roll back soft configure of fec
    
#endif /*_GLB_UML_SYSTEM_*/
#else
    if (GLB_SPEED_AUTO == p_port_status->status)
    {
        p_port_status->status = GLB_SPEED_1G;
    }
    p_port->port_status.speed = p_port_status->status;

#endif

#ifndef _GLB_UML_SYSTEM_
/* modified by liul for gb osp, 2016-05-20 */
    /*store port status*/
    p_port->port_cfg.speed = p_port_status->status;

    /*when 40G/100G DAC port change from auto to force, need re-config 3AP ffe.*/
    if(p_port->port_speed_ability
        & (GLB_SUPPORT_SPEED_25G|GLB_SUPPORT_SPEED_50G|GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_100G))
    {
#ifdef GOLDENGATE 
        if(!fiber_is_direct_attach_cable(p_port->p_fiber))
        {
            lcm_port_update_ffe(p_port);
        }
        else
        {
            if((p_port->port_cfg.speed == GLB_SPEED_AUTO)
                &&(p_port->port_cfg.duplex != GLB_DUPLEX_AUTO)
                &&(p_port_status->status != GLB_SPEED_AUTO))
            {
                lcm_port_update_ffe(p_port);
            }
        }
#elif (defined DUET2) || (defined TSINGMA)
        lcm_port_update_ffe(p_port);
#endif
    }
#endif

    return ret;
}

int32
lcm_port_set_fec_en(void* pv_arg)
{
    lcapi_lcm_set_port_fec_enable_t * p_fec_en;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_fec_en = (lcapi_lcm_set_port_fec_enable_t*)pv_arg;
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_fec_en->slot_idx;
    panel_port = p_fec_en->port_idx;
    panel_subport = p_fec_en->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  fec %d", port_idx, p_fec_en->enable);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
#ifndef _GLB_UML_SYSTEM_
    uint8 fec_en = 0;
    glb_port_t* p_port = NULL;
    p_port = p_card->pp_port[port_idx - 1];
    /* get new_fec_config by cfg_speed and old_fec_config */
    lcm_update_fec_config(p_port, p_port->port_cfg.speed, p_fec_en->enable, &fec_en);
    /* then, set new_config to chip */
    if (phy_cfg_fec_enable(port_idx - 1, fec_en))
    {
        LCM_LOG_ERR("Can't set fec %s for port index %d", p_fec_en->enable ? "enable" : "disable", port_idx);
        return LCM_E_INVALID_PARAM;
    }
    /* fixup config in phy driver database, store config from cli */
    phy_update_fec_en(p_port->port_idx-1, p_fec_en->enable);
#endif
    return LCM_E_SUCCESS;
}

int32
lcm_port_set_poe(void* pv_arg)
{
    lcapi_lcm_set_port_poe_t * p_poe;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_poe = (lcapi_lcm_set_port_poe_t*)pv_arg;
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_poe->slot_idx;
    panel_port = p_poe->port_idx;
    panel_subport = p_poe->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    p_port = p_card->pp_port[port_idx - 1];
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d poe psd-id %d,channel_id%d,enble=%d,force_power=%d,mode=%d,priority=%d,power_limit=%d.",
        port_idx, p_port->poe_info.pse_id, p_port->poe_info.pse_channel_id,p_poe->enable,
        p_poe->force_power, p_poe->mode, p_poe->priority, p_poe->power_limit);
    
#ifndef _GLB_UML_SYSTEM_
    uint32 power_val = 0;
    uint8 sys_enable = 0;
    sys_enable = p_card->poe_sys_info[p_port->poe_info.pse_id].sys_cfg.enable;

    if(p_port->poe_info.channel_cfg.force_power != p_poe->force_power)
    {
        p_port->poe_info.channel_cfg.force_power = p_poe->force_power; 
        if(sys_enable)
        {
            if(p_poe->force_power)
            {
                if(p_port->poe_info.channel_cfg.enable == 0)
                    poe_set_enable(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,1);
            }
            poe_set_force_power(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,p_poe->force_power);
        } 
    }
    
    if(p_port->poe_info.channel_cfg.enable != p_poe->enable)
    {
        p_port->poe_info.channel_cfg.enable = p_poe->enable;
        if(sys_enable)
        {
            if(p_poe->enable == 0)
            {
                if(p_port->poe_info.channel_cfg.force_power)
                    p_poe->enable = 1;
            }
            poe_set_enable(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,p_poe->enable);
        }
    }
    if(p_port->poe_info.channel_cfg.mode != p_poe->mode)
    {
        poe_set_mode(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,p_poe->mode);
        p_port->poe_info.channel_cfg.mode = p_poe->mode;
        if(p_poe->power_limit==0)/* default power_limit according to poe mode */
        {
            if(p_poe->mode == POE_AF)
            {
                if(p_port->poe_info.channel_cfg.power_limit > DEFAULT_POE_AF_POWER)
                    p_poe->power_limit = DEFAULT_POE_AF_POWER;
            }
            else if(p_poe->mode == POE_AT)
            {
                if(p_port->poe_info.channel_cfg.power_limit > DEFAULT_POE_AT_POWER)
                    p_poe->power_limit = DEFAULT_POE_AT_POWER;
            }
            else if(p_poe->mode == POE_POH)
            {
                if(p_port->poe_info.channel_cfg.power_limit > DEFAULT_POE_POH_POWER)
                    p_poe->power_limit = DEFAULT_POE_POH_POWER;
            }
        }
    }
    if(p_port->poe_info.channel_cfg.priority != p_poe->priority)
    {
        poe_set_priority(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,p_poe->priority);
        p_port->poe_info.channel_cfg.priority = p_poe->priority;
    }
    if(p_port->poe_info.channel_cfg.power_limit != p_poe->power_limit)
    {
        power_val = p_poe->power_limit;
        if(p_poe->power_limit==0)
        {
            if(p_poe->mode == POE_AF)
                power_val = DEFAULT_POE_AF_POWER;
            else if(p_poe->mode == POE_AT)
                power_val = DEFAULT_POE_AT_POWER;
            else if(p_poe->mode == POE_POH)
                power_val = DEFAULT_POE_POH_POWER;
        }
        poe_set_power_limit(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,power_val);
        p_port->poe_info.channel_cfg.power_limit = power_val;
    }

#endif
    return LCM_E_SUCCESS;
}

int32
lcm_port_set_ffe(void* pv_arg)
{
    lcapi_lcm_set_port_ffe_t * p_ffe = NULL;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    uint8 i = 0;
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_ffe = (lcapi_lcm_set_port_ffe_t*)pv_arg;
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_ffe->slot_idx;
    panel_port = p_ffe->port_idx;
    panel_subport = p_ffe->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,   "Set port %d  ffe config %d channel %d c0 %d c1 %d c2 %d c3 %d", 
              port_idx, p_ffe->config,p_ffe->channel,p_ffe->c0,p_ffe->c1,p_ffe->c2,p_ffe->c3);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    p_port = p_card->pp_port[port_idx - 1];
    if(p_ffe->channel == 0)
    {
        for(i=0;i<4;i++)
        {
            p_port->port_cfg.ffe.config[i] = p_ffe->config;
            p_port->port_cfg.ffe.c0[i] = p_ffe->c0;
            p_port->port_cfg.ffe.c1[i] = p_ffe->c1;
            p_port->port_cfg.ffe.c2[i] = p_ffe->c2;
            p_port->port_cfg.ffe.c3[i] = p_ffe->c3;
            p_port->port_cfg.ffe.channel = p_ffe->channel;
        }
    }
    else
    {
        p_port->port_cfg.ffe.config[p_ffe->channel-1] = p_ffe->config;
        p_port->port_cfg.ffe.c0[p_ffe->channel-1] = p_ffe->c0;
        p_port->port_cfg.ffe.c1[p_ffe->channel-1] = p_ffe->c1;
        p_port->port_cfg.ffe.c2[p_ffe->channel-1] = p_ffe->c2;
        p_port->port_cfg.ffe.c3[p_ffe->channel-1] = p_ffe->c3;
        p_port->port_cfg.ffe.channel = p_ffe->channel;
    }
#ifndef _GLB_UML_SYSTEM_    
    lcm_port_update_ffe(p_port);
#endif
    return LCM_E_SUCCESS;
}

/***************************************************************************************************
 * Name         : lcm_port_set_duplex 
 * Purpose      : set port duplex          
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_duplex (void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  duplex %d", port_idx, p_port_status->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }

    p_port = p_card->pp_port[port_idx - 1];
 // TODO: modified by xgu for gg emulation board, 2014-6-2
#ifndef _GGEMU_
#ifdef _GLB_UML_SYSTEM_
    if (GLB_DUPLEX_AUTO == p_port_status->status)
    {
        p_port_status->status = GLB_DUPLEX_FULL;
    }
    p_port->port_status.duplex = p_port_status->status;
#else
    /*If port is copper, just config phy*/
    if(p_port->port_cfg.media == GLB_PORT_TYPE_COPPER)
    {
        phy_cfg_duplex(port_idx - 1, p_port_status->status);
    }
    else /*If port is fiber, duplex half is not supported.*/
    {
        if(p_port_status->status == GLB_DUPLEX_HALF)
        {
            return LCM_E_INVALID_DUPLEX;
        }
        else
        {
            phy_cfg_duplex(port_idx - 1, p_port_status->status);
        }
    }    
#endif /*_GLB_UML_SYSTEM_*/
#else 
    if (GLB_DUPLEX_AUTO == p_port_status->status)
    {
        p_port_status->status = GLB_DUPLEX_FULL;
    }
    p_port->port_status.duplex = p_port_status->status;
#endif

#ifndef _GLB_UML_SYSTEM_
    /* for bug 54054 , modified by tongzb, need update duplex before update ffe */
    /*store port status*/
    p_port->port_cfg.duplex = p_port_status->status;

/* modified by liul for gb osp, 2016-05-20 */
    /*when 40G/100G DAC port change from auto to force, need re-config 3AP ffe.*/
    if(p_port->port_speed_ability 
        & (GLB_SUPPORT_SPEED_25G|GLB_SUPPORT_SPEED_50G|GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_100G))
    {
#ifdef GOLDENGATE         
        if(!fiber_is_direct_attach_cable(p_port->p_fiber))
        {
            lcm_port_update_ffe(p_port);
        }
        else
        {
            if((p_port->port_cfg.duplex == GLB_DUPLEX_AUTO)
                &&(p_port->port_cfg.speed != GLB_SPEED_AUTO)
                &&(p_port_status->status != GLB_DUPLEX_AUTO))
            {
                lcm_port_update_ffe(p_port);
            }
        }
#elif (defined DUET2)||(defined TSINGMA)
        lcm_port_update_ffe(p_port);
#endif        
    }
#endif

    return LCM_E_SUCCESS;
}

/***************************************************************************************************
 * Name         : lcm_port_set_media 
 * Purpose      : set port media          
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/

int32
lcm_port_set_media (void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    glb_port_t* p_port = NULL;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
#ifndef _GLB_UML_SYSTEM_         
#ifndef _GGEMU_ // TODO: removed by xgu for gg emualtion board, 2014-6-2
    lcm_port_get_phy_type_param_t param;
    glb_phy_type_t fiber_type;
    fiber_info_t* p_fiber;
    int32 ret = 0;
#endif
#endif        

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;
          
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
     
    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT, "Set port %d  media %d", port_idx, p_port_status->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    p_port = p_card->pp_port[port_idx - 1];  
#ifdef _GLB_UML_SYSTEM_                                                                         
   
#else    
#ifndef _GGEMU_ // TODO: removed by xgu for gg emulation board, 2014-6-2
    /*Only combo port support set medium*/
    if(p_port->is_combo)
    {
        ret = lcm_port_update_medium(port_idx-1, p_port_status->status, GLB_SPEED_AUTO);
        if(ret != LCM_E_SUCCESS)
        {
            LCM_LOG_ERR("%s port %d update medium error %d when set media %d speed %d.", 
                port_name, port_idx-1, ret, p_port_status->status, GLB_SPEED_AUTO);
        }

        p_fiber = p_port->p_fiber;
        if(p_fiber != NULL)
        {
            fiber_type = p_fiber->fiber_type;
        }
        else
        {
            fiber_type = GLB_PHY_TYPE_UNKNOWN;
        }
        param.is_combo = p_port->is_combo;
        param.cfg_media = p_port_status->status;
        param.fiber_type = fiber_type;
        param.default_type = p_port->phy_type; 
        if(p_port->port_status.link_up)
        {
            param.linkup_media = p_port->port_status.linkup_media;
        }
        else
        {
            param.linkup_media = GLB_PORT_TYPE_AUTO;
        }
        p_port->phy_type = lcm_port_get_phy_type(&param); 
#ifdef CNOS_OS        
        lcapi_lcm_port_info_t panel_port_info;
        panel_port_info.port_no = p_port->panel_port_no;
        panel_port_info.sub_port_no = p_port->panel_sub_port_no;
        panel_port_info.slot_no= p_port->panel_slot_no;
        ret = lcm_port_notify_port_status(&panel_port_info);
#else
        ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_LINK_STATUS, &port_idx);
#endif
        if(ret < 0)
        {
            LCM_LOG_ERR("Lcm update port idx %d phy type fail.", port_idx);
            return LCM_E_INIT_FAILED;
        }
    }
    else
    {
        return LCM_E_SUCCESS;
    }
#endif
#endif 
               
    /*store port status*/
    p_port->port_cfg.media = p_port_status->status;

    return LCM_E_SUCCESS;
}


/***************************************************************************************************
 * Name         : lcm_port_set_flowctrl
 * Purpose      : set port flow control
 * Input        : pv_arg: port physical information
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_flowctrl(void* pv_arg)
{
    lcapi_lcm_set_flowctrl_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 panel_port, panel_subport, panel_slot, port_idx = 0;  
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_flowctrl_t *)pv_arg;

    /* get local line card */
    p_card = lcm_mgt_get_card();
    
    /* get port index */
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  flowctrl direction %d enable %d",port_idx, p_port_status->direction, p_port_status->enable);
    if(port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }

#if 1 // TODO: removed by xgu for gg emulation board, 2014-6-2
#ifdef _GLB_UML_SYSTEM_
    if(p_port_status->direction == GLB_FLOWCTRL_SEND)
    {
        p_card->pp_port[port_idx - 1]->port_status.flowctrl.send = p_port_status->enable;
    }
    else
    {
        p_card->pp_port[port_idx - 1]->port_status.flowctrl.recv = p_port_status->enable;
    }
#else
    phy_cfg_flowctrl(port_idx - 1, p_port_status->direction, p_port_status->enable);
#endif/*_GLB_UML_SYSTEM_*/   

#endif
    if(p_port_status->direction)
    {
        p_card->pp_port[port_idx - 1]->port_cfg.flowctrl.send = p_port_status->enable;
    }
    else
    {
        p_card->pp_port[port_idx - 1]->port_cfg.flowctrl.recv = p_port_status->enable;
    }
    return LCM_E_SUCCESS;
}

/* support eee function, modified by liuht for bug 28298, 2014-04-21 */
/***************************************************************************************************
 * Name         : lcm_port_set_eee_capability
 * Purpose      : set port eee capability
 * Input        : pv_arg: port physical information
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_eee_capability(void* pv_arg)
{
    lcapi_lcm_set_eee_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 panel_port, panel_subport, panel_slot, port_idx = 0;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    int32 ret = 0;

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_eee_status_t *)pv_arg;

    /* get local line card */
    p_card = lcm_mgt_get_card();
    
    /* get port index */
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  eee priority %d", port_idx, p_port_status->status);
    if(port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }

#ifdef _GLB_UML_SYSTEM_
    return ret;
#else
    ret += phy_cfg_eee_capability(port_idx - 1, p_port_status->status);
    if(ret)
    {
        LCM_LOG_ERR("Port index %d eee priority cfg driver failed", port_idx);
        return LCM_E_HW_GEN;
    }
    else
    {

    }
#endif/*_GLB_UML_SYSTEM_*/

    return LCM_E_SUCCESS;
}
/* end of liuht modified */

/***************************************************************************************************
 * Name         : lcm_port_set_enable 
 * Purpose      : set port mac enable           
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_enable(void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    glb_port_t* p_port = NULL;
#ifdef _GLB_UML_SYSTEM_
    lcapi_lcm_port_info_t panel_port_info;
#endif
//fdef _GLB_UML_SYSTEM_    
    lcapi_hagt_update_stack_port_t port_info;
//ndif
    fiber_info_t* p_fiber;
    uint16 value;
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();   

    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
   
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  enable %d ", port_idx, p_port_status->status);

    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }    
    p_port = p_card->pp_port[port_idx - 1];
#ifdef _GGEMU_  // TODO: remvoed by xgu for gg emulation board, 2014-6-2
    if (p_port->port_status.link_up == p_port_status->status)
    {
        LCM_LOG_DEBUG(LCM_LCMGT, "Port %d has been configured enable %d",port_idx, p_port_status->status);
        return LCM_E_SUCCESS;
    }    
    p_port->port_status.link_up = p_port_status->status;

    if (lcm_stack_is_enable())  /* Modified by Percy Wang for bug 25588, 2013-11-12 */
    {
        port_info.gport = GLB_TRANS_PORTID_TO_GPORT(p_port->glb_chip_idx, p_port->logic_port_idx);
        port_info.linkup = p_port->port_status.link_up;
        LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_STACK_PORT, &port_info);
    }

#else
#ifdef _GLB_UML_SYSTEM_
    (void)p_fiber;
    (void)value;
    if (p_port->port_status.link_up == p_port_status->status)
    {
        LCM_LOG_DEBUG(LCM_LCMGT,"Port %d has been configured enable %d",port_idx, p_port_status->status);
        return LCM_E_SUCCESS;
    }    
    p_port->port_status.link_up = p_port_status->status;

    panel_port_info.port_no = p_port->panel_port_no;
    panel_port_info.sub_port_no = p_port->panel_sub_port_no;
    panel_port_info.slot_no= p_port->panel_slot_no;

#ifdef CNOS_OS
    lcm_port_notify_port_status(&panel_port_info);
#else
    LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_LINK_STATUS, &panel_port_info);
#endif

    if (lcm_stack_is_enable())  /* Modified by Percy Wang for bug 25588, 2013-11-12 */
    {
        port_info.gport = GLB_TRANS_PORTID_TO_GPORT(p_port->glb_chip_idx, p_port->logic_port_idx);
        port_info.linkup = p_port->port_status.link_up;
        LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_STACK_PORT, &port_info);
    }
#else
    (void)port_info;
    phy_cfg_enable(port_idx - 1, p_port_status->status);

    /* Fix bug30160, qicx, 2014-10-10
     * Software reset is needed when power up port of the following condition.
     * (Only when (1 && 2 && 3) is true)
     * 1, From power down to power up.
     * 2, Configured media is AUTO.
     * 3, Fiber type is 100BASE-X SFP/fiber.
     */
    if (1 == p_port_status->status)
    {
        /* Auto-Media Sense with Cat5 media or 100BASE-FX fiber/SFP media */
        if ((p_port->p_fiber != NULL) && (GLB_PORT_TYPE_AUTO == p_port->port_cfg.media))
        {
            p_fiber = (fiber_info_t*)p_port->p_fiber;
            if(!phy_check_no_phy(port_idx - 1))
            {
                //has phy, GB<->PHY<->SFP
                //100Base-X SFP
                if((p_fiber->fiber_type == GLB_PHY_TYPE_100BASE_FX)||(p_fiber->fiber_type == GLB_PHY_TYPE_100BASE_LX))
                {
                    gephy_reg_read(port_idx - 1, PHY_CTL_REG, &value);
                    value |= 0x8000;
                    gephy_reg_write(port_idx - 1, PHY_CTL_REG, value);
                    sal_udelay(40 * 1000);/* pause after reset */
                }
            }
        }
    }
    /*bug32802, here remove fiber_set_enable() to avoid multi-thread on fiber.*/
#endif /*_GLB_UML_SYSTEM_*/
#endif
    /*store port status*/
    p_port->port_cfg.enable = p_port_status->status;

    return LCM_E_SUCCESS;
}

/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
/***************************************************************************************************
 * Name         : lcm_port_set_master_slave_mode 
 * Purpose      : set port master-slave mode           
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_master_slave_mode(void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    glb_port_t* p_port = NULL;
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();   

    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
   
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  master-slave mode %d ", port_idx, p_port_status->status);

    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }    
    p_port = p_card->pp_port[port_idx - 1];
#ifdef _GLB_UML_SYSTEM_    
    if (p_port->port_status.master_slave == p_port_status->status)
    {
        LCM_LOG_DEBUG(LCM_LCMGT,"Port %d has been configured master-slave mode %d",port_idx, p_port_status->status);
        return LCM_E_SUCCESS;
    }    
    p_port->port_status.master_slave = p_port_status->status;
#else
    phy_cfg_master_slave_mode(port_idx - 1, p_port_status->status);

#endif /*_GLB_UML_SYSTEM_*/

    /*store port status*/
    p_port->port_cfg.master_slave = p_port_status->status;

    return LCM_E_SUCCESS;
}
/* End of liuht modified */

/***************************************************************************************************
 * Name         : lcm_port_set_create_done 
 * Purpose      : set port created done in hsrv/hagt, 
                  if the port is destroyed, parm flag should be 0;     
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
/*modified by xgu for bug 13493, 2011-01-11*/
int32
lcm_port_set_create_done(void* pv_arg)
{
    lcapi_hagt_set_mac_en_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    uint16 gport = 0, portid;
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_hagt_set_mac_en_t*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();   

    /*get port index*/
    gport = p_port_status->gport;  

    for(portid = 0; portid < p_card->port_num; portid++)
    {
        p_port = p_card->pp_port[portid];
        if (p_port && (((p_port->glb_chip_idx << 8) | p_port->logic_port_idx) == gport))
        {
            if (p_port_status->flag)
                p_card->pp_port[portid]->create_done = 1;
            else
                p_card->pp_port[portid]->create_done = 0;
            LCM_LOG_DEBUG(LCM_LCMGT,"Set gport %d  created done %d, portid %d", 
                  gport, p_port_status->flag, portid);
            return LCM_E_SUCCESS;
        }
    }

    LCM_LOG_ERR("Set gport %d  created done %d fail",gport, p_port_status->flag);

    return LCM_E_INVALID_PORT;
}
/***************************************************************************************************
 * Name         : lcm_port_get_port_num 
 * Purpose      : get port num in this local linecard           
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_get_port_num (void* pv_arg)
{
    lcapi_lcm_get_port_num_t* p_ports_num = NULL;
    glb_card_t* p_card = NULL;    

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);    
    p_ports_num = (lcapi_lcm_get_port_num_t*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    p_ports_num->port_num = p_card->port_num;

    return LCM_E_SUCCESS;
}
/***************************************************************************************************
 * Name         : lcm_port_get_port_status 
 * Purpose      : get port status for speed, duplex and media type          
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_get_port_status (void* pv_arg)
{
    int32 ret = LCM_E_SUCCESS;
    lcapi_lcm_get_port_status_t* p_port_status = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_get_port_status_t*)pv_arg;    
    /*get port index*/
    panel_slot = p_port_status->slot_no;
    panel_port = p_port_status->port_no;
    panel_subport = p_port_status->sub_port_no;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__,port_name);
        return LCM_E_NOT_FOUND;
    }

    ret = _lcm_port_get_status_of_port(p_port_status,port_idx);
    
    return ret;
}

int32
lcm_port_get_port_link_up_status (void* pv_arg)
{
    int32 ret = LCM_E_SUCCESS;
    lcapi_lcm_get_port_status_t* p_port_status = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;

    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_get_port_status_t*)pv_arg;    
    /*get port index*/
    panel_slot = p_port_status->slot_no;
    panel_port = p_port_status->port_no;
    panel_subport = p_port_status->sub_port_no;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__,port_name);
        return LCM_E_NOT_FOUND;
    }

    p_port = p_card->pp_port[port_idx - 1];  
    p_port_status->enable = p_port->port_status.link_up; 
    
    return ret;
}

/***************************************************************************************************
 * Name         : lcm_port_set_lpbk 
 * Purpose      : set port loopback mode         
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_lpbk(void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    ctc_port_lbk_param_t port_lbk;
    int32 ret = LCM_E_SUCCESS;
    sal_memset(&port_lbk, 0, sizeof(port_lbk));
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();   

    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d  lp-type %d ", port_idx, p_port_status->status);

    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }    

    p_port = p_card->pp_port[port_idx - 1];
#if _GGEMU_ // TODO: modified by xgu for gg emulation board, 2014-6-2
    if (p_port->lp_type == p_port_status->status)
    {
        LCM_LOG_DEBUG(LCM_LCMGT, "Port %d has been configured lp-type %d", port_idx, p_port_status->status);
        return LCM_E_SUCCESS;
    }
    
    p_port->lp_type = p_port_status->status;

#else
#ifdef _GLB_UML_SYSTEM_
    if (p_port->lp_type == p_port_status->status)
    {
        LCM_LOG_DEBUG(LCM_LCMGT, "Port %d has been configured lp-type %d", port_idx, p_port_status->status);
        return LCM_E_SUCCESS;
    }
    
    p_port->lp_type = p_port_status->status;
#else
#ifdef CNOS_OS
    /*disable first*/
    if (GLB_LB_NONE != p_port->port_cfg.loopback)
    {
        port_lbk.src_gport  = p_port_status->lp_srcport;
        port_lbk.dst_gport  = p_port_status->lp_srcport;
        port_lbk.lbk_type   = CTC_PORT_LBK_TYPE_BYPASS;
        port_lbk.lbk_enable  = FALSE;
        switch(p_port->port_cfg.loopback)
        {
        case GLB_LB_PHY_NEAR:
            phy_cfg_loopback(port_idx - 1, GLB_LB_NONE);
            ret = ctc_port_set_loopback(&port_lbk);
            break;
        case GLB_LB_PHY_FAR:
            phy_cfg_loopback(port_idx - 1, GLB_LB_NONE);
            ret = ctc_port_set_srcdiscard_en(port_lbk.src_gport, 0);
            break;
        case GLB_LB_PHY_LOOPBACK:
            phy_cfg_loopback(port_idx - 1, GLB_LB_NONE);
            break;
        case GLB_LB_PORT:
            ret = ctc_port_set_loopback(&port_lbk);
            break;
        case GLB_LB_PORT_SWAP:
            ctc_port_set_vlan_filter_en(port_lbk.dst_gport, GLB_EGRESS, 1);
            ret = ctc_port_set_loopback(&port_lbk);
            break;
        default:
            break;
        }
        if(ret < 0)
        {
            return ret;
        }
    }

    sal_memset(&port_lbk, 0, sizeof(port_lbk));
    switch(p_port_status->status)
    {
    case GLB_LB_PHY_NEAR:
        port_lbk.src_gport  = p_port_status->lp_srcport;
        port_lbk.dst_gport  = p_port_status->lp_dstport;
        port_lbk.lbk_type   = CTC_PORT_LBK_TYPE_BYPASS;
        port_lbk.lbk_enable  = TRUE;
        phy_cfg_loopback(port_idx - 1, p_port_status->status);
        ret = ctc_port_set_loopback(&port_lbk);
        break;
    case GLB_LB_PHY_FAR:
        phy_cfg_loopback(port_idx - 1, p_port_status->status);
        ret = ctc_port_set_srcdiscard_en(p_port_status->lp_srcport, 1);
        break;
    case GLB_LB_PHY_LOOPBACK:
        phy_cfg_loopback(port_idx - 1, GLB_LB_PHY_NEAR);
        break;
    case GLB_LB_PORT:
        port_lbk.src_gport  = p_port_status->lp_srcport;
        port_lbk.dst_gport  = p_port_status->lp_srcport;
        port_lbk.lbk_type   = CTC_PORT_LBK_TYPE_BYPASS;
        port_lbk.lbk_enable  = TRUE;
        ret = ctc_port_set_loopback(&port_lbk);
        break;
    case GLB_LB_PORT_SWAP:
        port_lbk.src_gport  = p_port_status->lp_srcport;
        port_lbk.dst_gport  = p_port_status->lp_srcport;
        port_lbk.lbk_type   = CTC_PORT_LBK_TYPE_SWAP_MAC;
        port_lbk.lbk_enable  = TRUE;
        ctc_port_set_vlan_filter_en(port_lbk.dst_gport, GLB_EGRESS, 0);
        ret = ctc_port_set_loopback(&port_lbk);
        break;
    default:
        break;
    }
#else /* for IPI_OS, sdk loopback interface call in hagt_phy_loopback.c */
    phy_cfg_loopback(port_idx - 1, p_port_status->status);
#endif
#endif /*_GLB_UML_SYSTEM_*/
#endif
    /*store port status*/
    p_port->port_cfg.loopback = p_port_status->status;

    return ret;
}

/***************************************************************************************************
 * Name         : lcm_port_set_syncE_enable 
 * Purpose      : set port phy syncE enable           
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_syncE_enable(void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[IFNAMSIZ] = {0};
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();   

    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_shortname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d syncE %d ", port_idx, p_port_status->status);

    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
#ifndef _GLB_UML_SYSTEM_
    /* optimize code, liangf, 2017-06-02 */
    if(lcm_syce_enable_cb)
    {
        lcm_syce_enable_cb(port_idx-1, p_port_status->status);
    }
#endif
    p_card->pp_port[port_idx - 1]->port_cfg.syncE_enable = p_port_status->status;
    return LCM_E_SUCCESS;
}

/***************************************************************************************************
 * Name         : lcm_port_set_unidir 
 * Purpose      : set unidirectional channel          
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_unidir(void* pv_arg)
{
#ifndef _GLB_UML_SYSTEM_
    int32 ret=LCM_E_SUCCESS;
    glb_card_t* p_card = NULL;
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;    
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if (lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT, "Set port %d  unidirectional %d", port_idx, p_port_status->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    
    phy_cfg_unidir(port_idx - 1, p_port_status->status);
    
    p_card->pp_port[port_idx - 1]->port_cfg.unidir = p_port_status->status;
    /* config port led mode for bug 37466, 2016-03-11 */
    _lcm_port_mgt_led_mode(p_card->pp_port[port_idx - 1]);
    
    return ret;
#endif
    return LCM_E_SUCCESS;
}
/* added by tongzb for bug 43937 support tunable fiber */
/***************************************************************************************************
 * Name         : lcm_port_set_wavelength 
 * Purpose      : set the wavelength of fiber insert the panel port         
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_set_wavelength(void* pv_arg)
{
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    int32 ret=LCM_E_SUCCESS;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
#ifndef _GLB_UML_SYSTEM_
    fiber_info_t *p_fiber = NULL;
#endif

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;    
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if (lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,    "Set port %d wavelength %d", port_idx, p_port_status->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    ///TODO: modified by xgu for gg emulation board, 2014-6-2
#ifndef _GGEMU_
#ifdef _GLB_UML_SYSTEM_

#else
    glb_port_t* p_port = NULL;

    p_port = p_card->pp_port[port_idx - 1];
    if(p_port->fiber_freq == p_port_status->status)
        return ret;
    
    p_port->fiber_freq = p_port_status->status;
    if(p_port->p_fiber)
    {
        p_fiber = p_port->p_fiber;
        ret = fiber_config_wavelength(port_idx - 1, p_port_status->status,FREQ_MODE,p_fiber);
        if(ret == 0)
        {
            lcm_msg_tx_lcFiberUpdate(p_port, 0);
        }
    }

#endif /*_GLB_UML_SYSTEM_*/
#endif

    return ret;
}

int32
lcm_port_set_link_tolerable_time(void* pv_arg)
{
    lcapi_lcm_set_port_status_t * p_tolerable;
    glb_card_t* p_card = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    
    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_tolerable = (lcapi_lcm_set_port_status_t*)pv_arg;
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_tolerable->slot_idx;
    panel_port = p_tolerable->port_idx;
    panel_subport = p_tolerable->subport_idx;

    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT,"Set port %d   toleerable time to %d", port_idx, p_tolerable->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
#ifndef _GLB_UML_SYSTEM_
    if (phy_config_link_tolerable_time(port_idx - 1, p_tolerable->status))
    {
        LCM_LOG_ERR("Can't set toleerable time %s for port index %d", p_tolerable->status, port_idx);
        return LCM_E_INVALID_PARAM;
    }
#endif
    return LCM_E_SUCCESS;
}

int32
lcm_port_set_bypass(void* pv_arg)
{
#ifndef _GLB_UML_SYSTEM_
    int32 ret=LCM_E_SUCCESS;
    glb_card_t* p_card = NULL;
    lcapi_lcm_set_port_status_t* p_port_status = NULL;
    uint8 port_idx = 0, panel_port, panel_subport, panel_slot;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_port_status = (lcapi_lcm_set_port_status_t*)pv_arg;    
    /*get local line card*/
    p_card= lcm_mgt_get_card();   
    /*get port index*/
    panel_slot = p_port_status->slot_idx;
    panel_port = p_port_status->port_idx;
    panel_subport = p_port_status->subport_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    if (lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        if_build_port_fullname_by_slot_port(port_name, panel_slot, panel_port, panel_subport);
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, port_name);
        return LCM_E_NOT_FOUND;
    }
    LCM_LOG_DEBUG(LCM_LCMGT, "Set port %d  bypass %d", port_idx, p_port_status->status);
    if (port_idx > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d", port_idx, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
   
    p_card->pp_port[port_idx - 1]->bypass = (uint8)((p_port_status->status>>4)&0xf);
    p_card->pp_port[port_idx - 1]->bypass_slot = (uint8)(p_port_status->status&0xf);
    phy_cfg_bypass(port_idx - 1, p_card->pp_port[port_idx - 1]->bypass);
    return ret;
#endif
    return LCM_E_SUCCESS;
}

/* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
/***************************************************************************************************
 * Name         : lcm_get_logic_port_array 
 * Purpose      : get logic port array
 * Input        : pv_arg: port physical information         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_get_logic_port_array(void* pv_arg)
{
    uint8 port_id;
    lcapi_lcm_get_logic_port_array_t* p_logic_port_array = NULL;
    glb_card_t* p_card = NULL; 

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    p_logic_port_array = (lcapi_lcm_get_logic_port_array_t*)pv_arg;
    
    /*get local line card*/
    p_card = lcm_mgt_get_card();

    for (port_id = 0; port_id < p_card->port_num; port_id++)
    {
        if (GLB_PORT_TYPE_FIBER != p_card->pp_port[port_id]->port_cfg.media)
        {
            p_logic_port_array->logic_port[port_id] = 0xff;
        }
        else
        {
            p_logic_port_array->logic_port[port_id] = p_card->pp_port[port_id]->logic_port_idx;
        }
    }
    if (port_id < MAX_GB_PORT)
    {
        for (; port_id < MAX_GB_PORT; port_id++)
        {
            /* fill the blanks */
            p_logic_port_array->logic_port[port_id] = 0xff;
        }
    }        
    
    return LCM_E_SUCCESS;
}
/* end of qicx added */

/*bug30363 jqiu add 2014-10-23 for customer request*/
int32
lcm_port_config_phy_test_signal_mode(uint8 panel_slot, uint8 panel_port, uint8 panel_subport, uint8 mode)
{
#ifndef _GLB_UML_SYSTEM_     
    uint8 port_idx, serdes_id;
    glb_card_t* p_card = NULL; 
    char ifname [GLB_PHY_PORT_FULL_NAME_SZ] = {0};

    if_build_port_fullname_by_slot_port(ifname, panel_slot, panel_port, panel_subport);
    if(lcm_get_port_by_panel_info(panel_slot, panel_port, panel_subport, &port_idx)!=LCM_E_SUCCESS)
    {
        /*Modify by weij for support phy port name self-define. bug 31712*/
        LCM_LOG_ERR("%s Can't find port idx for %s",__FUNCTION__, ifname);
        return LCM_E_NOT_FOUND;
    }
    p_card= lcm_mgt_get_card();
    serdes_id = p_card->pp_port[port_idx-1]->chip_serdes_id;
    phy_config_phy_sig_test_mode(port_idx-1, serdes_id, mode);
#endif
    return LCM_E_SUCCESS;
}

#ifndef _GLB_UML_SYSTEM_
/*bug23865. support copper SFP*/
void lcm_port_copper_sfp_init(void* p_data)
{
#ifdef _GGEMU_
    return;// TODO: added by xgu for gg emulation board, 2014-6-2
#endif
    int32 ret = 0;
    uint32 port_id;
    glb_port_t *p_port;
    char port_name[IFNAMSIZ] = {0};
        
    p_port = (glb_port_t *)p_data;
    if (NULL == p_port)
        return;

    port_id = p_port->port_idx - 1;

    phy_attach_copper_sfp_phy(port_id);
    /*Bug29809. Combo port don't support SGMII copper SFP. 
      Because for combo port, system need check link status both rj45 and sfp media.
      For rj45+fiber SFP or rj45+Serdes Copper SFP, system just need check original phy;
      For rj45+SGMII Copper SFP, system need check original phy for rj45 and check marvell 
      phy in copper sfp for SGMII Copper SFP. This is confuse the polling logic. In order to 
      simplify the logic, we don't support this usage. So even SGMII Copper SFP insert, treat
      it as Serdes Copper SFP, and it won't work.*/
    if((p_port->is_combo)&&(phy_check_attach_phy(port_id)))
    {
        phy_deattach_copper_sfp_phy(port_id);
        if_build_port_shortname_by_slot_port(port_name, p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        log_sys(M_MOD_LCM, E_WARNING, "Interface %s is combo port, doesn't support SGMII interface copper SFP.", port_name);
    }
    /*When attach copper sfp phy:
      1. attach phy need cfg enable as p_port->port_cfg.enable;
      2. original phy need to cfg enable always; */
    if(phy_check_attach_phy(port_id))
    {
        /*Do item 1 and 2*/
        phy_cfg_enable(port_id, p_port->port_cfg.enable);
        /*Fix bug 30260, 30299, Before sfp insert, mac status depend on los, after copper sfp insert, mac depend on sfp status. 
          Needn't default status, because phy_api.c will copy phy status to next handle status. jqiu 2014-11-19*/  
    }
    else
    {
        /*If no attach phy, roll back fiber enable to cfg value.*/
        /* Modified by liuht for bug42776, 2017-02-17 */
        /* if interface shutdown or unidirectional rx-only is set, 
        * then disable fiber module tx */
        if ((p_port->port_cfg.enable == 0) 
            || (p_port->port_cfg.unidir == GLB_PORT_UNIDIR_RX_ONLY))
        {
            fiber_set_enable(port_id, 0);
        }
        else
        {
            fiber_set_enable(port_id, 1);
        }
    }
    ret = lcm_port_update_medium(port_id, p_port->port_cfg.media, p_port->port_cfg.speed);
    if(ret != LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("port %d update medium error %d when insert sfp speed %d media %d.", 
            port_id, ret, p_port->port_cfg.speed, p_port->port_cfg.media);
    }
    
    return;
}

int32 lcm_port_copper_sfp_remove(glb_port_t *p_port)
{
    int32 ret=0;
    uint32 port_id, attach_flag=0;
    lcapi_hagt_set_mac_en_t lcapi_hagt_set_mac_en;
    
    GLB_PTR_VALID_CHECK(p_port, LCM_E_INVALID_PTR);
    port_id = p_port->port_idx - 1;
    attach_flag = phy_check_attach_phy(port_id);    
    ret = phy_deattach_copper_sfp_phy(port_id);
    /*When remove attach copper sfp phy, need roll back:
      1. fiber tx disable need roll back to original value;
      2. original phy need roll back to original value;
      3. Mac need to disable and will update when original phy status change. default status of original phy is clear to down*/
    if(attach_flag != 0)
    {
        /*Do item 1*/
        /* Modified by liuht for bug42776, 2017-02-17 */
        /* if interface shutdown or unidirectional rx-only is set, 
        * then disable fiber module tx */
        if ((p_port->port_cfg.enable == 0) 
            || (p_port->port_cfg.unidir == GLB_PORT_UNIDIR_RX_ONLY))
        {
            fiber_set_enable(port_id, 0);
        }
        else
        {
            fiber_set_enable(port_id, 1);
        }
        /*Do item 2*/
        phy_cfg_enable(port_id, p_port->port_cfg.enable);
        /*Do item 3*/
        /* for bug 45848, modify by tongzb, 2018-05-11
        ** for PORT_PHY_NULL port when insert 1000BASE_T_SFP fiber, mac will disable when port down
        ** we should mac rollback when remove 1000BASE_T_SFP fiber
        */
        /* for bug 48430, even the port with no phy we also rollback mac status */
        lcapi_hagt_set_mac_en.flag = p_port->port_cfg.enable;
        lcapi_hagt_set_mac_en.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;
        //ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_SET_MAC_EN, &lcapi_hagt_set_mac_en);
        ret = ctc_port_set_mac_en(lcapi_hagt_set_mac_en.gport, lcapi_hagt_set_mac_en.flag);    

    }
    
    return ret;
}

int32 lcm_port_create_fiber(glb_port_t* p_port)
{
    fiber_info_t* p_fiber;
    glb_phy_type_t fiber_type;
    int32 ret;
    uint32 port_id;
    uint8 pannel_subport;
    lcm_port_get_phy_type_param_t param;
    uint32 freq=0;
    glb_card_t* p_card = NULL; 

    p_card= lcm_mgt_get_card();
     
    if(NULL == p_port || NULL == p_card)
    {
        return LCM_E_INVALID_PARAM;
    }
    
    pannel_subport = p_port->panel_sub_port_no;
    port_id = p_port->port_idx - 1;
    
    p_fiber = LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(fiber_info_t));
    if(NULL == p_fiber)
    {
        LCM_LOG_ALERT("Fiber port: Out of memory");
        return LCM_E_NO_MEMORY;
    }
    /*Modify by jqiu for bug 15855. 2011-08-25*/
    ret = fiber_get_chip_info(port_id, p_fiber);
    /*Bug18147. If access fail, return, wait next time scan and access.*/
    if(ret < 0)
    {        
        p_port->fiber_access_err_cnt++;
        if(p_port->fiber_access_err_cnt <= FIBER_MODULE_ACCESS_ERR_NOT_TRY_MAX)
        {
            LCM_LOG_WARN("Fiber port %d: access fail.",port_id);
            LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_fiber);        
            return LCM_E_SUCCESS;
        }
        else /*If continue error to max times, not try.*/
        {            
            LCM_LOG_WARN("Fiber port %d: access fail, creat unknown type.",port_id);
        }
    }
    if(p_fiber->channel_num == 1)
    {
        p_fiber->channel_idx = FIBER_CURRENT_VALUE1;
    }
    else if(p_fiber->channel_num > 1)
    {
        if(pannel_subport)
        {
            p_fiber->channel_idx = pannel_subport-1;
        }
        else
        {
            p_fiber->channel_idx = FIBER_VAL_TYPE_MAX2;
        }
    }

    p_fiber->updated = TRUE;///TODO:
    p_port->fiber_access_err_cnt = 0;
    fiber_type = p_fiber->fiber_type;
    param.is_combo = p_port->is_combo;
    param.cfg_media = p_port->port_cfg.media;
    param.fiber_type = fiber_type;
    param.linkup_media = p_port->port_status.linkup_media;
    param.default_type = p_port->phy_type;
    p_port->phy_type = lcm_port_get_phy_type(&param);    
    p_port->p_fiber = p_fiber;
    
    LCM_LOG_DEBUG(LCM_LCMGT, "Port %d insert fiber type %d", port_id, fiber_type);
    if(fiber_type == GLB_PHY_TYPE_1000BASE_T_SFP)
    {
        /*When copper SFP insert, must set fiber enable, then can access internal phy. If internal need managed, will 
          attach new phy, and keep fiber enable until remove, if internal phy needn't be managed, will roll back fiber 
          enable in lcm_port_copper_sfp_init.*/
        ret = fiber_set_enable(port_id, 1);
        /*If current port is disable, after set fiber enable, need wait a while for SFP power on to access Copper SFP phy*/
        if(p_port->port_cfg.enable == 0)
        {
            /* sleep 500ms for sfp ready */
            usleep(500000);
        }
        lcm_port_copper_sfp_init((void *)p_port);
        if(p_port->port_cfg.speed == GLB_SPEED_100M)
        {
            LCM_LOG_ERR("Warning: Port with speed 100M not support 1000BASE_T_SFP.\n");/* print to log */
            LCM_LOG_PTS("Warning: Port with speed 100M not support 1000BASE_T_SFP.\n");/* print to tty when no terminal monitor */
            LCM_LOG_CONSOLE("Warning: Port with speed 100M not support 1000BASE_T_SFP.\n");/* print to console when no terminal monitor */
        }
    }
    else
    {
        /*bug32802, here remove this because lcm_fiber_port_status_timer can do it.*/
        /*if(p_port->port_cfg.enable == 0)
        {
            fiber_set_enable(port_id, 0);
        }*/
        ret = lcm_port_update_medium(port_id, p_port->port_cfg.media, p_port->port_cfg.speed);
    }
    /* added by tongzb for bug 43937 support tunable fiber */
    if(check_fiber_support_tunable(p_port->p_fiber))
    {
        fiber_get_wavelength_by_freq(port_id,&freq,p_port->p_fiber);
        if(freq != p_port->fiber_freq)
        {
            if(p_port->fiber_freq == GLB_FIBER_FREQ_NO_0CONFIG) /* this only happen in init */
                p_port->fiber_freq = GLB_FIBER_DEFAULT_FREQ;/* the same as NSM_FIBER_DEFAULT_FREQ */
            ret = fiber_config_wavelength(port_id, p_port->fiber_freq,FREQ_MODE,p_port->p_fiber);
        }
    }

    p_port->fiber_triger = 1;
    lcm_notify_fiber_info(p_port, LCM_FIBER_INSERT);

#ifdef CNOS_OS
    lai_fiber_info_t lai_fiber;
    lai_object_id_t oid = 0;
    /*add fiber lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_fiber_event)
    {
        sal_memset(&lai_fiber, 0, sizeof(lai_fiber));
        oid = (p_port->port_idx - 1) + (p_card->phy_slot_no << 16);
        ctc_lai_get_fiber_info(oid, &lai_fiber);
        
        p_lai_card_noti->on_fiber_event(LAI_FIBER_EVENT_ADD, &lai_fiber);
    }
#endif        
    return LCM_E_SUCCESS;
}

int32 lcm_port_destroy_fiber(glb_port_t* p_port)
{
    int32 ret;
    uint32 port_id;
    glb_phy_type_t fiber_type;
    lcm_port_get_phy_type_param_t param;

    glb_card_t* p_card = NULL; 

    p_card= lcm_mgt_get_card();
    
    if(NULL == p_port || NULL == p_card)
    {
        return LCM_E_INVALID_PARAM;
    }
    
    port_id = p_port->port_idx - 1;
    fiber_type = ((fiber_info_t*)(p_port->p_fiber))->fiber_type;
    param.is_combo = p_port->is_combo;
    param.cfg_media = p_port->port_cfg.media;
    param.fiber_type = GLB_PHY_TYPE_UNKNOWN; /*after destroy fiber, it is unknown*/
    param.linkup_media = p_port->port_status.linkup_media;
    param.default_type = p_port->phy_type;
    p_port->phy_type = lcm_port_get_phy_type(&param);
    LCM_LOG_DEBUG(LCM_LCMGT, "Port %d remove fiber type %d", port_id, fiber_type);
    
    if(p_port->p_fiber)
    {
        LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_port->p_fiber);
    }
    p_port->p_fiber = NULL;    
    lcm_port_copper_sfp_remove(p_port);
    ret = lcm_port_update_medium(port_id, p_port->port_cfg.media, p_port->port_cfg.speed);
    if(ret != LCM_E_SUCCESS)
    {
        LCM_LOG_ERR("port %d update medium error %d when remove sfp speed %d media %d.", 
            port_id, ret, p_port->port_cfg.speed, p_port->port_cfg.media);
    }
    p_port->fiber_triger = 1;
    lcm_notify_fiber_info(p_port, LCM_FIBER_REMOVE);
#ifdef CNOS_OS
    lai_fiber_info_t lai_fiber;
    lai_object_id_t oid = 0;
    /*add fiber lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_fiber_event)
    {
        sal_memset(&lai_fiber, 0, sizeof(lai_fiber));
        oid = (p_port->port_idx - 1) + (p_card->phy_slot_no << 16);
        ctc_lai_get_fiber_info(oid, &lai_fiber);
        
        p_lai_card_noti->on_fiber_event(LAI_FIBER_EVENT_DELETE, &lai_fiber);
    }
#endif
    return LCM_E_SUCCESS;
}
/************************************************************************
 Fiber scan task:
 *** For fiber insert event: 
 ******Fiber must be in present for FIBER_MODULE_SCAN_TIME rounds, then believe it is present;
 ******Every round only scan FIBER_MODULE_SCAN_NUM_ONE_ROUND fiber insert event;
 *** For fiber remove event:
 ******Every round scan all of fiber remove event;
 *** For fiber DDM info update:
 ******Every round only update FIBER_MODULE_UPDATE_NUM_ONE_ROUND fiber DDM info;
 *************************************************************************/
//int32 lcm_fiber_port_status_timer(thread_t* p_arg)
int32 lcm_fiber_port_status_timer()
{

    //CTC_TASK_GET_MASTER
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    int32 port_id, ret = 0;
    uint8 present_status;
    uint8 fiber_scan_number=0;
    uint8 fiber_update_number=0, fiber_update_port_id_old;
    static uint8 fiber_update_port_id;

    //CTC_TASK_ADD_TIME_MSEC(lcm_fiber_port_status_timer, NULL, FIBER_MODULE_SCAN_TIME);

    if(lcm_mgt_get_bypass_timers())
        return LCM_E_SUCCESS;

    /* add timer control in lcsh */
    if(!lcm_check_timers_mask(FIBER_TIMER))
        return LCM_E_SUCCESS;
    
    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    fiber_update_port_id_old = fiber_update_port_id;
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        p_port = p_card->pp_port[port_id];
        /*modified by xgu for bug 13493, 2011-01-11*/        
        if(!p_port || !p_port->create_done)
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d not create.", port_id+1);
            continue;
        }
        /*bug32802, move all fiber operation in this thread.*/
        /*1. process present event*/
        ret = fiber_get_present(port_id, &present_status);
        /*Fix bug 14685. jqiu 2011-05-23. if return error, this port doesn't support fiber.*/
        if(ret == RESULT_ERROR)
        {
            continue;
        }
        if((present_status==FIBER_PRESENT) &&(NULL == p_port->p_fiber))
        {
            /*Bug17183. In one round, scan MAX fiber module*/
            if(fiber_scan_number < FIBER_MODULE_SCAN_NUM_ONE_ROUND)
            {
                LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d create sfp.", port_id+1);
                lcm_port_create_fiber(p_port);
                fiber_scan_number++;
            }
        }
        else if((present_status != FIBER_PRESENT) && (NULL != p_port->p_fiber))
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d destroy sfp.", port_id+1);
            lcm_port_destroy_fiber(p_port);
        }
        
        /*3. process tx_disable event*/
        /*if SFP is Copper SFP, and attach new phy, not set tx_disable, keep it tx enable always.*/
        if(phy_check_attach_phy(port_id))
        {
            //Do nothing
        }
        else
        {
            /* Modified by liuht for bug42776, 2017-02-17 */
            /* if interface shutdown or unidirectional rx-only is set, 
            * then disable fiber module tx */
            if ((p_port->port_cfg.enable == 0) 
                || (p_port->port_cfg.unidir == GLB_PORT_UNIDIR_RX_ONLY))
            {
                fiber_set_enable(port_id, 0);
            }
            else
            {
                fiber_set_enable(port_id, 1);
            }
        }
        

        /*4. process DDM event*/
        /*Bug17183. In one round, update MAX fiber module*/
        if(fiber_update_number >= FIBER_MODULE_UPDATE_NUM_ONE_ROUND)
            continue;
        /*Bug17183. In one round, store last update port id.*/
        if(port_id < fiber_update_port_id_old)
            continue;
        fiber_update_port_id = port_id+1;
        if(p_port->p_fiber)
        {
            /*If fiber doesn't support DDM, continue*/
            if(check_fiber_support_ddm(p_port->p_fiber) == 0)
                continue;
            fiber_update_number++;
            ret = fiber_update_state(port_id, p_port->p_fiber);
            if(ret == 0)
            {
                lcm_msg_tx_lcFiberUpdate(p_port, 0);
            }
            else
            {
                LCM_LOG_ERR("UPDATE DDM   fail.");
            }
        }  
    }
    if(fiber_update_port_id >= p_card->port_num)
    {
        fiber_update_port_id = 0;
    }
    return LCM_E_SUCCESS;
}

static int32
_lcm_port_get_port_range(glb_card_t* p_card, int32 phy_intr_stat, int32* start, int32* end)
{
    int32 phy_int_bit_no;
    
    *start = 1;
    *end = p_card->port_num;
    
    if(NULL == p_card->p_port_range)
    {
        return LCM_E_SUCCESS;   
    }

    for(phy_int_bit_no = 0; phy_int_bit_no < p_card->phy_int_bit_num; phy_int_bit_no++)
    {
        if(p_card->p_port_range[phy_int_bit_no].port_end < p_card->p_port_range[phy_int_bit_no].port_start)
        {
            continue;
        }
        if(phy_intr_stat & (0x1<<phy_int_bit_no))
        {
            continue;
        }
        else
        {
            *start = p_card->p_port_range[phy_int_bit_no].port_start;
            *end = p_card->p_port_range[phy_int_bit_no].port_end;
            /*modified by jqiu for optimize, when found 1 port range, just return to process.*/
            return LCM_E_SUCCESS;
        }
    }
    
    return LCM_E_SUCCESS;
}

/*modify for bug 53838, to support low speed lighting mode*/
static void
_lcm_port_sfp_two_led_cfg(mac_led_api_para_t *port_led, glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {            
        if(p_port->port_cfg.media == GLB_PORT_TYPE_COPPER)
        {
            if(p_port->port_status.speed == GLB_SPEED_1G)
            {
                port_led->mode = p_port->port_led_reverse ? LED_MODE_2_RXLNKBIACT_OFF : LED_MODE_2_OFF_RXLNKBIACT;
            }
            else
            {
                port_led->mode = p_port->port_led_reverse ? LED_MODE_2_OFF_RXLNKBIACT : LED_MODE_2_RXLNKBIACT_OFF;
            }
        }
        else
        {
            if(GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED_LOWSPEED == p_port->port_led_mode_change)
            {
               if(p_port->port_status.speed == GLB_SPEED_1G)
					goto HighSpeed;
			   else 
				    goto OtherSpeed;
            }
            else
            {
				if(p_port->port_status.speed == GLB_SPEED_10G)
					goto HighSpeed;
			    else 
				    goto OtherSpeed;
			}
      HighSpeed:
        	if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
        		port_led->mode = p_port->port_led_reverse ? LED_MODE_2_BIACT_OFF : LED_MODE_2_OFF_BIACT;
        	else
        		port_led->mode = p_port->port_led_reverse ? LED_MODE_2_RXLNKBIACT_OFF : LED_MODE_2_OFF_RXLNKBIACT;
        	return ;
				
      OtherSpeed:             
        	if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
        		port_led->mode = p_port->port_led_reverse ? LED_MODE_2_OFF_BIACT : LED_MODE_2_BIACT_OFF;
        	else
        		port_led->mode = p_port->port_led_reverse ? LED_MODE_2_OFF_RXLNKBIACT : LED_MODE_2_RXLNKBIACT_OFF;
            return ;
        }
    }
    else
    {
        port_led->mode = LED_MODE_2_FORCE_OFF;
    }
}

static void
_lcm_port_sfp_one_led_cfg(mac_led_api_para_t *port_led, glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {
        port_led->mode = LED_MODE_1_RXLNK_BIACT;
    }
    else
    {
        port_led->mode = LED_MODE_1_FORCE_OFF;
    }
}

static void
_lcm_port_qsfp_two_led_cfg(mac_led_api_para_t *port_led, glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {
        if((p_port->port_status.speed == GLB_SPEED_40G) || ((p_port->port_status.speed == GLB_SPEED_100G)))
        {
            if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
                port_led->mode = p_port->port_led_reverse ?LED_MODE_2_BIACT_OFF:LED_MODE_2_OFF_BIACT;
            else
                port_led->mode = p_port->port_led_reverse ?LED_MODE_2_RXLNKBIACT_OFF:LED_MODE_2_OFF_RXLNKBIACT;
        }
        else
        {
            if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
                port_led->mode = p_port->port_led_reverse ?LED_MODE_2_OFF_BIACT:LED_MODE_2_BIACT_OFF;
            else
                port_led->mode = p_port->port_led_reverse ?LED_MODE_2_OFF_RXLNKBIACT:LED_MODE_2_RXLNKBIACT_OFF;
        }
    }
    else
    {
        port_led->mode = LED_MODE_2_FORCE_OFF;
    }
}

static void
_lcm_port_qsfp_one_led_cfg(mac_led_api_para_t *port_led, glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {
        if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
            port_led->mode = LED_MODE_1_BIACT;
        else
            port_led->mode = LED_MODE_1_RXLNK_BIACT;
    }
    else
    {
        port_led->mode = LED_MODE_1_FORCE_OFF;
    }
}

static void
_lcm_port_qsfp28_no_split_cfg(mac_led_api_para_t *port_led, glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {
        if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
            port_led->mode = LED_MODE_2_TXLNK_BIACT;
        else
            port_led->mode = LED_MODE_2_TXLNK_RXLNKBIACT;
    }
    else
    {
        port_led->mode = LED_MODE_2_TXLNK_OFF;
    }
}

static void
_lcm_port_qsfp28_split_cfg(mac_led_api_para_t *port_led, glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {
        if(GLB_PORT_UNIDIR_DISABLE != p_port->port_cfg.unidir)
            port_led->mode = LED_MODE_2_BIACT_TXLNK;
        else
            port_led->mode = LED_MODE_2_RXLNKBIACT_TXLNK;
    }
    else
    {
        port_led->mode = LED_MODE_2_OFF_TXLNK;
    }
}

/*add by chenjr for bug 53838*/
static void
_lcm_port_phy_two_led_cfg(glb_port_t* p_port)
{
    if(p_port->port_status.link_up)
    {
        if(p_port->port_status.speed == GLB_SPEED_1G)
        {
            phy_cfg_led_mode(p_port->port_idx - 1, PHY_LED_ON, PHY_LED_OFF);
        }
        else
        {
            phy_cfg_led_mode(p_port->port_idx - 1, PHY_LED_OFF, PHY_LED_ON);
        }
    }
    else
    {
            phy_cfg_led_mode(p_port->port_idx - 1, PHY_LED_OFF, PHY_LED_OFF);
    }
}


/* Added by liuht for bug 34540, 2015-09-09 */
static int32
_lcm_port_mgt_led_mode(glb_port_t* p_port)
{
    mac_led_api_para_t port_led;

    /*Bug35259. jqiu modify 2016-09-13*/
    if(p_port->port_status.link_up)
    {
        led_linkup_process(p_port->local_chip_idx, p_port->g_port_index);
    }

    if(GLB_PORT_LED_MODE_CHANGE_UNSUPPORT == p_port->port_led_mode_change)
    {
        return 0;
    }
    
    if(GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED == p_port->port_led_mode_change
      || GLB_PORT_LED_MODE_CHANGE_SFP_TWO_LED_LOWSPEED== p_port->port_led_mode_change)
    {
        _lcm_port_sfp_two_led_cfg(&port_led, p_port);
    }
    else if(GLB_PORT_LED_MODE_CHANGE_SFP_ONE_LED == p_port->port_led_mode_change)
    {
        _lcm_port_sfp_one_led_cfg(&port_led, p_port);
    }
    else if(GLB_PORT_LED_MODE_CHANGE_QSFP_TWO_LED == p_port->port_led_mode_change)
    {
        _lcm_port_qsfp_two_led_cfg(&port_led, p_port);
    }
    else if(GLB_PORT_LED_MODE_CHANGE_QSFP_ONE_LED== p_port->port_led_mode_change)
    {
        _lcm_port_qsfp_one_led_cfg(&port_led, p_port);
    }
    else if(GLB_PORT_LED_MODE_CHANGE_QSFP28_NO_SPLIT == p_port->port_led_mode_change)
    {
        _lcm_port_qsfp28_no_split_cfg(&port_led, p_port);
    }
    else if(GLB_PORT_LED_MODE_CHANGE_QSFP28_SPLIT == p_port->port_led_mode_change)
    {
        _lcm_port_qsfp28_split_cfg(&port_led, p_port);
    }
    else if(GLB_PORT_LED_MODE_CHANGE_PHY_TWO_LED== p_port->port_led_mode_change)
    {
        _lcm_port_phy_two_led_cfg(p_port);
    }
    //port_led.port_id = (p_port->glb_chip_idx<<8) | p_port->port_led_mac;
    port_led.port_id = p_port->port_led_mac;
    port_led.lchip   = p_port->local_chip_idx;
#ifdef DUET2
    port_led.ctl_id  = p_port->mac_ctl_id;
#else
    port_led.ctl_id  = p_port->ctl_id;
#endif
    if(led_cfg_port_mode(&port_led))
    {
        LCM_LOG_ERR("Lcm set mac %d led mode %d fail.", port_led.port_id, port_led.mode);
    }
    else
    {
        LCM_LOG_DEBUG(LCM_CARD, "Lcm set mac %d led mode %d.", port_led.port_id, port_led.mode);
    }
    
    return 0;
}

static void
_lcm_port_status_update(glb_port_t* p_port, glb_phy_state_t* phy_val, 
                        phy_state_change_t* phy_change)
{
    lcm_port_get_phy_type_param_t param;
    
    if(p_port->p_fiber)
    {
        param.fiber_type = ((fiber_info_t *)(p_port->p_fiber))->fiber_type;
    }
    else
    {
        param.fiber_type = GLB_PHY_TYPE_UNKNOWN;
    }
    param.is_combo = p_port->is_combo;
    param.cfg_media = p_port->port_cfg.media;
    param.linkup_media = phy_val->linkup_media;
    param.default_type = p_port->phy_type;
    p_port->phy_type = lcm_port_get_phy_type(&param);
    p_port->port_status.link_up = phy_val->link_up;
    p_port->master_slave = phy_val->master_slave;

    p_port->port_status.speed = phy_val->speed;
    p_port->port_status.duplex = phy_val->duplex;
    p_port->port_status.flowctrl.send = phy_val->flowctrl.send;
    p_port->port_status.flowctrl.recv = phy_val->flowctrl.recv;
    p_port->port_status.linkup_media = phy_val->linkup_media;  
    p_port->port_status.eee_status.status= phy_val->eee_status.status;
    p_port->port_status.eee_status.wake_error_count= phy_val->eee_status.wake_error_count;
    /* Modified by liuht for bug 29005, 2014-06-17 */
    if(-1 != phy_change->action)
    {
        phy_change->link_change = 1;
    }
    else
    {
        phy_change->link_change = 0;
    }
    p_port->port_status.port_status_change = GLB_PORT_STATUS_CHANGE(phy_change); 
    p_port->port_status.fec_status = phy_val->fec_status;
    p_port->port_status.cl73_ability = phy_val->cl73_ability;
}


static int32
_lcm_port_update_flowctrl_cfg(glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    int ret = 0;
    lcapi_hagt_set_mac_flowctrl_t lcapi_hagt_set_mac_flowctrl;

    sal_memcpy(&lcapi_hagt_set_mac_flowctrl.flowctrl, &phy_val->flowctrl, sizeof(glb_port_flowctrl_t));
    lcapi_hagt_set_mac_flowctrl.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;
    /* added by tongzb for bug 44544 */
    if(GLB_SPEED_5G == phy_val->speed)
    {
        lcapi_hagt_set_mac_flowctrl.flowctrl.recv = GLB_FLOWCTRL_ENABLE;
    }

    ret = lcm_set_mac_flowcntrl(&lcapi_hagt_set_mac_flowctrl);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm set gport %d mac flow control send %d receive %d fail.", 
                    lcapi_hagt_set_mac_flowctrl.gport, phy_val->flowctrl.send,
                    phy_val->flowctrl.recv);
        /* just return ok */
        ret = 0;
    }
    else
    {
        LCM_LOG_DEBUG(LCM_CARD, "Lcm set gport %d mac flow control send %d receive %d.",
                lcapi_hagt_set_mac_flowctrl.gport, phy_val->flowctrl.send,phy_val->flowctrl.recv);
    }
    return ret;
}

#if 0
static int32
_lcm_port_update_remote_fault_status(glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    int ret = 0;
    bool is_error = FALSE;
    lcapi_lcm_get_remote_fault_status_t remote_fault_status;
    
    /* update remote fault status */
    remote_fault_status.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;
    remote_fault_status.is_error = &is_error;
    // ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_GET_REMOTE_FAULT_STATUS, &remote_fault_status);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm get 10G port %d remote fault fail.", remote_fault_status.gport);
    }
    p_port->remote_fault_status = (uint8)is_error;

    return 0;
}

static int32
_lcm_port_update_force_tx_en(glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    int ret=0;
    lcapi_hagt_set_force_tx_en_t lcapi_hagt_set_force_tx_en;
    
    /* need to set force tx when in remote fault state or down state, so MAC can flush pkts. Otherwise, unset force_tx */
    if (!p_port->port_status.link_up || p_port->remote_fault_status)
    {
        /* mac will be disabled and enabled in force_tx set API */
        lcapi_hagt_set_force_tx_en.flag = TRUE;
    }
    else
    {
        lcapi_hagt_set_force_tx_en.flag = FALSE;
    }

    /* avoid unnesessary mac disable/enable in force_tx setting */
    if (lcapi_hagt_set_force_tx_en.flag != p_port->force_tx_status)
    {
        lcapi_hagt_set_force_tx_en.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;
       // ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_SET_FORCE_TX_EN, &lcapi_hagt_set_force_tx_en);
        if(ret < 0)
        {
            LCM_LOG_ERR("Lcm set gport %d force tx flag %d fail.", 
                        lcapi_hagt_set_force_tx_en.gport, lcapi_hagt_set_force_tx_en.flag);
        }
        else
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm set gport %d force tx flag %d.", 
                        lcapi_hagt_set_force_tx_en.gport, lcapi_hagt_set_force_tx_en.flag);
        }
        p_port->force_tx_status = lcapi_hagt_set_force_tx_en.flag;
    }
    return 0;
}
#endif

static int32
_lcm_port_update_eee_cfg(glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    int ret = 0;
    lcapi_hagt_set_mac_eee_t lcapi_hagt_set_mac_eee;

    lcapi_hagt_set_mac_eee.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;
    lcapi_hagt_set_mac_eee.eee_capability = phy_val->eee_status.status& 0xff;
    if((phy_val->eee_status.status >> 16) & lcapi_hagt_set_mac_eee.eee_capability)
    {
        lcapi_hagt_set_mac_eee.eee_capability = 1;
    }
    else
    {
        lcapi_hagt_set_mac_eee.eee_capability = 0;
    }
    
    //ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_SET_EEE_CAPABILITY, &lcapi_hagt_set_mac_eee);
    if(ret < 0)
    {
        //LCM_LOG_ERR("Lcm set gport %d mac eee capability %d fail.", lcapi_hagt_set_mac_eee.gport, lcapi_hagt_set_mac_eee.eee_capability);
    }
    else
    {
        //LCM_LOG_ERR("Lcm set gport %d mac eee capability %d.", lcapi_hagt_set_mac_eee.gport, lcapi_hagt_set_mac_eee.eee_capability);
    }
    return 0;
}

static int32
_lcm_port_update_mac_en_cfg(glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    int ret=0;
    lcapi_hagt_set_mac_en_t lcapi_hagt_set_mac_en;

    sal_memset(&lcapi_hagt_set_mac_en, 0, sizeof(lcapi_hagt_set_mac_en));
    lcapi_hagt_set_mac_en.flag = phy_val->link_up;
    lcapi_hagt_set_mac_en.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;
    ret = ctc_port_set_mac_en(lcapi_hagt_set_mac_en.gport, lcapi_hagt_set_mac_en.flag);    
    
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm set gport %d mac en flag %d fail.", 
                    lcapi_hagt_set_mac_en.gport, phy_val->link_up);
    }
    else
    {
        LCM_LOG_DEBUG(LCM_CARD,"Lcm set gport %d mac en flag %d.", 
                    lcapi_hagt_set_mac_en.gport, phy_val->link_up); 
    }
    return 0;
}

static int32
_lcm_port_update_speed_cfg(glb_port_t* p_port, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    int ret=0;
    lcapi_hagt_set_mac_speed_t lcapi_hagt_set_mac_speed;

    lcapi_hagt_set_mac_speed.speed_mode = phy_val->speed;
    lcapi_hagt_set_mac_speed.gport = (p_port->glb_chip_idx<<8) | p_port->logic_port_idx;

    if((p_port->port_speed_ability & GLB_SUPPORT_SPEED_10G)
        || (p_port->port_speed_ability & GLB_SUPPORT_SPEED_5G)
        || (p_port->port_speed_ability & GLB_SUPPORT_SPEED_2_5G))
    {
        if(phy_check_attach_phy(p_port->port_idx -1))
        {
            /* for legacy 1G phy, just update speed */
            ret = lcm_set_mac_speed(&lcapi_hagt_set_mac_speed);
        }
        else
        {
            /* for 10G/5G/2.5G phy, need to update serdes mode */
            ret = lcm_set_serdes_mode(p_port, phy_val);
        }
    }
    else
    {
        /* for legacy 1G phy, just update speed */
        ret = lcm_set_mac_speed(&lcapi_hagt_set_mac_speed);
    }
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm set gport %d mac speed %d fail.", 
                    lcapi_hagt_set_mac_speed.gport, phy_val->speed);
    }
    else
    {
        LCM_LOG_DEBUG(LCM_CARD,"Lcm set gport %d mac speed %d.", 
                    lcapi_hagt_set_mac_speed.gport, phy_val->speed); 
    }
    return 0;
}



static void 
_lcm_port_process_with_no_phy(glb_port_t* p_port, glb_phy_state_t* phy_val, 
                                phy_state_change_t* phy_change)
{
#if 0
    /* modified by yaom for 10G remote fault 20140410 */
    if (GLB_SUPPORT_SPEED_10G != p_port->port_speed_ability)
    {
        /*Only do flow control update when port is up.*/
        if((p_port->port_status.link_up)&&(1 == phy_change->flowctrl_change))
        {
            _lcm_port_update_flowctrl_cfg(p_port, phy_val);
        }
    }
    else
    {
        _lcm_port_update_remote_fault_status(p_port, phy_val);
        _lcm_port_update_force_tx_en(p_port, phy_val);
        
        /* set flow ctrl when link up && no remote fault */
        /*Only do flow control update when port is up.*/
        if(p_port->port_status.link_up && !p_port->remote_fault_status 
            &&(1 == phy_change->flowctrl_change))
        {
            _lcm_port_update_flowctrl_cfg(p_port, phy_val);
        }
    }
#endif  
    /*Only do flow control update when port is up.*/
    if((p_port->port_status.link_up)&&(1 == phy_change->flowctrl_change))
    {
        _lcm_port_update_flowctrl_cfg(p_port, phy_val);
    }

    return ;
}

static void 
_lcm_port_process_with_phy(glb_port_t* p_port, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{    
    if(1 == phy_change->eee_status_change)
    {
        _lcm_port_update_eee_cfg(p_port, phy_val);
    }
    if(p_port->port_status.link_up)
    {
        if (-1 != phy_change->action)
        {
            _lcm_port_update_mac_en_cfg(p_port, phy_val);       
        }
        /* Modified by liuht for bug 29005, 2014-06-17 */
        if((-1 != phy_change->action)||(1 == phy_change->speed_change)
            ||(1==phy_change->duplex_change)||(1==phy_change->media_change)
            ||(1 == phy_change->flowctrl_change))
        {
            _lcm_port_update_speed_cfg(p_port, phy_val, phy_change);
        }
        /* End of liuht modified */
    
        if(1 == phy_change->flowctrl_change)
        {
            _lcm_port_update_flowctrl_cfg(p_port, phy_val);
        }
    }
    else
    {
        _lcm_port_update_mac_en_cfg(p_port, phy_val);
    }

#if 0    
    if (GLB_SUPPORT_SPEED_10G != p_port->port_speed_ability)
    {
        if(p_port->port_status.link_up)
        {
            if (-1 != phy_change->action)
            {
                _lcm_port_update_mac_en_cfg(p_port, phy_val);       
            }
            /* Modified by liuht for bug 29005, 2014-06-17 */
            if((-1 != phy_change->action)||(1 == phy_change->speed_change)
                ||(1==phy_change->duplex_change)||(1==phy_change->media_change)
                ||(1 == phy_change->flowctrl_change))
            {
                _lcm_port_update_speed_cfg(p_port, phy_val, phy_change);
            }
            /* End of liuht modified */

            if(1 == phy_change->flowctrl_change)
            {
                _lcm_port_update_flowctrl_cfg(p_port, phy_val);
            }
        }
        else
        {
            _lcm_port_update_mac_en_cfg(p_port, phy_val);
        }
    }
    else
    {
        _lcm_port_update_remote_fault_status(p_port, phy_val);
        _lcm_port_update_force_tx_en(p_port, phy_val);
        
        /* set flow ctrl when link up && no remote fault */
        /*Only do flow control update when port is up.*/
        if(p_port->port_status.link_up && !p_port->remote_fault_status)
        {
            if((-1 != phy_change->action)||(1 == phy_change->speed_change)
                ||(1==phy_change->duplex_change)||(1==phy_change->media_change)
                ||(1 == phy_change->flowctrl_change))
            {
                _lcm_port_update_speed_cfg(p_port, phy_val, phy_change);
            }

            if(1 == phy_change->flowctrl_change)
            {
                _lcm_port_update_flowctrl_cfg(p_port, phy_val);
            }
        }
    }
#endif    
    return;
}

static void
_lcm_board_specific_link_change_process(uint8 port_idx,glb_port_t* p_port, glb_phy_state_t* phy_val)
{
    glb_card_t* p_card = NULL; 
    p_card= lcm_mgt_get_card();
    phy_handle_t *phy_hdl = NULL;
    
    /* added by tongzb for support HF */
    if(p_card->board_type.series == GLB_SERIES_E350 && p_card->board_type.type == GLB_BOARD_E350_HF_12T4S8X)
    {
        int32 val =0;
        int32 mask = 0;
        
        phy_hdl = get_phy_hdl(port_idx-1);
        /* up */
        if(phy_val->link_up)
        {
            if(phy_hdl->phy_info.phy_device_type == PORT_PHY_VSC8512)
            {
                if(p_port->is_combo)
                {
                    if(phy_val->linkup_media == GLB_PORT_TYPE_COPPER)
                        val = 1;
                    else if(phy_val->linkup_media == GLB_PORT_TYPE_FIBER)
                        val = 2;                    
                    mask = (p_card->port_num - port_idx)*2;
                    epld_item_write_mask(0,EPLD_PHY_COMMBO_STAT0,(val<<mask),(3<<mask));
                    LCM_PRINT_CONSOLE("port %d link is up.link_media is %d.\n",port_idx,phy_val->linkup_media);
                }
            }
            else if(phy_hdl->phy_info.phy_device_type == PORT_PHY_NULL)
            {
                val = (phy_val->speed == GLB_SPEED_10G)?1:0;
                mask = 7-(port_idx-8 - p_card->phy_chip_num * 12 - 1);
                epld_item_write_mask(0,EPLD_XG_SPEED,(val<<mask),(1<<mask));
            }
        }
        else
        {

            if(phy_hdl->phy_info.phy_device_type == PORT_PHY_VSC8512)
            {
                if(p_port->is_combo)
                {              
                    mask = (p_card->port_num - port_idx)*2;
                    epld_item_write_mask(0,EPLD_PHY_COMMBO_STAT0,(0<<mask),(3<<mask));
                    LCM_PRINT_CONSOLE("port %d link is down.link_media is %d.\n",port_idx,phy_val->linkup_media);
                }
            }
            else if(phy_hdl->phy_info.phy_device_type == PORT_PHY_NULL)
            {
                mask = 7-(port_idx-8 - p_card->phy_chip_num * 12 - 1);
                epld_item_write_mask(0,EPLD_XG_SPEED,(0<<mask),(1<<mask));
            }                
        }
    }
    /* added end by tongzb */   
}

int32
_lcm_port_link_change_process(uint8 port_idx, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    int32 ret = 0;
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    lcapi_hagt_update_stack_port_t port_info;
    lcapi_lcm_port_info_t panel_port_info;

    p_card= lcm_mgt_get_card();
    p_port = p_card->pp_port[port_idx - 1];
    panel_port_info.port_no = p_port->panel_port_no;
    panel_port_info.sub_port_no = p_port->panel_sub_port_no;
    panel_port_info.slot_no= p_port->panel_slot_no;
    
    /*Link change or speed/duplex change(this may happen when too many port status change) fix bug 14722*/            
    if((-1 != phy_change->action)||(1 == phy_change->speed_change)
        ||(1==phy_change->duplex_change)||(1==phy_change->media_change)
        ||(1 == phy_change->flowctrl_change) ||(1 == phy_change->master_slave_change)
        ||(1 == phy_change->eee_status_change)||(1 == phy_change->cl73_ability_change))
    {
        LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d link change action %d speed %d duplex %d media %d flowctrl %d mode %d eee status %d", 
            port_idx, phy_change->action, phy_change->speed_change, phy_change->duplex_change, phy_change->media_change, 
            phy_change->flowctrl_change, phy_change->master_slave_change, phy_change->eee_status_change);

        _lcm_port_status_update(p_port, phy_val, phy_change);
        #ifdef GG_3AP
        _lcm_port_3ap_process(p_port, phy_val);
        #endif
        _lcm_board_specific_link_change_process(port_idx,p_port,phy_val);
    }
    else
    {
        /*No change*/
        /* if port status have no status change we need to check link status between kernel and hsrvd */
#ifndef CNOS_OS
        ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_SYNC_LINK_STATUS, &panel_port_info);
#endif

        return 0;
    }

    _lcm_port_mgt_led_mode(p_port);

    /*If this port has no phy and has no attach phy, needn't config mac enable, just need cfg flowctrl*/
    if(phy_check_no_phy(port_idx-1) && !phy_check_attach_phy(port_idx-1))
    {
        _lcm_port_process_with_no_phy(p_port, phy_val, phy_change);
    }
    else
    {
        _lcm_port_process_with_phy(p_port, phy_val, phy_change);
    }
    
    /* Update new link status. */
#ifdef CNOS_OS
    ret = lcm_port_notify_port_status(&panel_port_info);
#else
    ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_LINK_STATUS, &panel_port_info);
#endif
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm update port idx %d status fail.", port_idx);
        return 0;
    }

    /* Modified by Percy Wang for bug 25588, 2013-11-12 */
    if (lcm_stack_is_enable() && (-1 != phy_change->action))
    {
        port_info.gport = GLB_TRANS_PORTID_TO_GPORT(p_port->glb_chip_idx, p_port->logic_port_idx);
        port_info.linkup = p_port->port_status.link_up;
        LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_STACK_PORT, &port_info);
    }
    
    return 0;
}

int32 lcm_get_phy_intr_info(int32 *phy_intr_stat)
{
    int32 ret1,ret2;
    int32 phy_intr_stat1=0;
    int32 phy_intr_stat2=0;
    int32 stats=0;
    
    ret1 = epld_item_read(0, EPLD_INT_STAT1, &phy_intr_stat1);
    if(!ret1)
    {
        stats |= phy_intr_stat1;
    }
    ret2 = epld_item_read(0, EPLD_INT_STAT2, &phy_intr_stat2);
    if(!ret2)
    {
        stats |= (phy_intr_stat2<<8);
    }

    if(( ret2)&&(ret1))
    {
        /* if no define EPLD_INT_STAT1 or EPLD_INT_STAT2, then set all phy port get interrupt */
        /* for bug 47517 */ 
        /* p_card->p_port_range must be one item include all phy port */
        *phy_intr_stat = 0x00;
    }
    else
    {
        *phy_intr_stat = stats;
    }
    return 0;
}
/***************************************************************************************************
 * Name         : lcm_port_status_intr 
 * Purpose      : handle interrupt caused by port stauts change          
 * Input        : N/A         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
lcm_port_status_intr (void)
{
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    glb_phy_state_t phy_val;    
    phy_state_change_t phy_change;
    uint8 port_idx;
    int32 ret;
    int32 phy_intr_stat = 0xff;

    int32 start_port = 0;
    int32 end_port = 0;

    if(lcm_mgt_get_bypass_timers())
        return LCM_E_SUCCESS;

    /* add timer control in lcsh */
    if(!lcm_check_timers_mask(PORT_INTR))
        return LCM_E_SUCCESS;
    /*get local line card*/
    p_card= lcm_mgt_get_card();
    if(NULL == p_card)
    {
        LCM_LOG_ERR("Pointer to card is invalid.");
        return LCM_E_INVALID_PTR;
    }

#ifdef _CTC_LOONGSON_
    ret = get_ctc_phy_stat(&phy_intr_stat);
#else
    ret = lcm_get_phy_intr_info(&phy_intr_stat);
#endif
    if(ret < 0)
    {
        LCM_LOG_ERR("Get the phy interrupt status failed.");
        return ret;
    }
    LCM_LOG_DEBUG(LCM_CARD, "Lcm port phy intr status %x\n", phy_intr_stat);
    _lcm_port_get_port_range(p_card, phy_intr_stat, &start_port, &end_port);
    LCM_LOG_DEBUG(LCM_CARD, "Get port range, from %d to %d\n", start_port, end_port);
    for(port_idx = start_port; port_idx <= end_port; port_idx++)
    {
        p_port = p_card->pp_port[port_idx - 1];

        /*modified by xgu for bug 13493, 2011-01-11*/        
        if(!p_port || !p_port->create_done)
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d not create.", port_idx);
            continue;
        }
        
        memset(&phy_val,0,sizeof(glb_phy_state_t));
        memset(&phy_change,0,sizeof(phy_state_change_t));

        if(phy_get_link_interrupt(port_idx - 1, &phy_val, &phy_change))
        {
            /*get interrupt status Error*/
            continue;
        }

        _lcm_port_link_change_process(port_idx, &phy_val, &phy_change);
    }
    if(p_card->phy_interrupt_mode == GLB_PHY_INTERRUPT_MODE_EPLD)
    {
        /* add 'is_gbdemo' flag for some features that gbdemo board owns only, added by qicx, 2013-07-08 */
        /* fix bug 24090, cr7223, qicx, 2013-08-15 */
        epld_cfg_phy_interrupt_mask(0, p_card->is_gbdemo);
    }
    else if(p_card->phy_interrupt_mode == GLB_PHY_INTERRUPT_MODE_CPU)
    {
        ctc_en_phy_irq(NULL);
    }
    else if(p_card->phy_interrupt_mode == GLB_PHY_INTERRUPT_MODE_NULL)
    {
        ctc_en_phy_irq(NULL);//default cpu interrupt mode
    }

    return LCM_E_SUCCESS;
}

/***************************************************************************************************
 * Name         : lcm_port_port_status_timer 
 * Purpose      : update port status for speed, duplex and media type          
 * Input        : N/A         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
void
lcm_port_port_status_timer (void* p_data)
{
    CTC_TASK_GET_MASTER
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    glb_phy_state_t phy_val;
    phy_state_change_t phy_change;
    uint8 port_idx;

    uint32 los; 
    int32 ret = 0;
    uint16 reg_val = 0;
    
    /* modify default port timer interval from 3s to 1s, qicx, 2013-12-18 */
    /* Modified by kcao for bug 47615 2018-08-29, linkagg back-switch will break off for several hundred millisecond */
    CTC_TASK_ADD_TIME_MSEC(lcm_port_port_status_timer, NULL, 200);

    if(lcm_mgt_get_bypass_timers())
        return;

    /* add timer control in lcsh */
    if(!lcm_check_timers_mask(PORT_TIMER))
        return ;

#if 0
    /* for bug 54144, modified by liwh 2019-11-05 */
    /* need notify port status after system configure done in case vlan configure re-configure */
    static int is_cfg_done =0;
    extern int syscmd_file_exist(char *filename);
    if((is_cfg_done == 0)&&(syscmd_file_exist(STARTUP_CFG_DONE_FILE)))
    {
        is_cfg_done = 1;
    }
    if(!is_cfg_done)
        return ;
#endif

    g_port_timer_cnt++;
    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    for(port_idx = 1; port_idx <= p_card->port_num; port_idx++)
    {
        p_port = p_card->pp_port[port_idx - 1];
        /*modified by xgu for bug 13493, 2011-01-11*/        
        if(!p_port || !p_port->create_done)
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d not create.", port_idx);
            continue;
        }
        
        /* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
        /* For port without phy(Interrupt by GB mac), timer interval is 1s;
           for port with phy, timer interval is 3s */
        if (0 != g_port_timer_cnt % 15)/*added by tongzb for port down/up/down when remote port is down, 2019-05-27*/
        {
            if (!phy_check_no_phy(port_idx-1))
            {
                continue;
            }
        }
        /* end of qicx added */

        lcm_fiber_poll_event(p_port);
        
        memset(&phy_val,0,sizeof(glb_phy_state_t));
        memset(&phy_change,0,sizeof(phy_state_change_t));

        /*This code only applies to 65DV1,because it is related to the design of hardware!*/
        if((GLB_SERIES_E350 == p_card->board_type.series)&&(GLB_BOARD_E350_65DV1== p_card->board_type.type))
        {
            /*The first eight ports are 100/1000M optical ports*/
            if(port_idx <= 8)
            {
                /*los==1,means tx los*/
                ret = epld_get_fiber_los(port_idx - 1, &los);
                
                if(RESULT_OK==ret)
                {
                    gephy_page_reg_read(port_idx - 1, PHY_PAGE_EXTENDED, 0x13, &reg_val);

                    /*When los signals are generated, phy's sigdect must be zero to make the port down*/
                    if(los == 1)
                    {
                        if(reg_val == 0)
                        {
                            gephy_page_reg_write(port_idx - 1, PHY_PAGE_EXTENDED, 0x13, 0x1);

                        }
                        /*Read interrupt register values*/
                        gephy_reg_read(port_idx - 1, 0x19, &reg_val);
                        /*The fast link failure interrupt must be mask, otherwise PHY will always report the interrupt.*/
                        if((reg_val & 0x0080) == 0x80)
                        {
                            gephy_reg_write(port_idx - 1, 0x19, reg_val&0xff7f);
                        }

                    }
                    /*If los signal is not generated, phy's sigdect signal must be 1*/
                    else
                    {
                        if(reg_val == 1)
                        {
                            gephy_page_reg_write(port_idx - 1, PHY_PAGE_EXTENDED, 0x13, 0x0);
                        }
                        /*Read interrupt register values*/
                        gephy_reg_read(port_idx - 1, 0x19, &reg_val);
                        /*Open fast link failure interrupt*/
                        if((reg_val & 0x0080) == 0x0)
                        {
                            gephy_reg_write(port_idx - 1, 0x19, reg_val|0x0080);
                        }
                    }
                }
            }
        }

        if(phy_get_link_poll(port_idx - 1, &phy_val, &phy_change))
        {
            /*Poll Error*/
            continue;
        }
        if (phy_change.action != -1)
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port timer port %d action %d.", port_idx, phy_change.action);
        }        
        _lcm_port_link_change_process(port_idx, &phy_val, &phy_change);
    }

    return;
}

/***************************************************************************************************
 * Name         : lcm_port_cl73_auto_timer 
 * Purpose      : trigger CL73 auto-neg periodly          
 * Input        : N/A         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         :  jqiu modified for bug39456 40G/100G auto-neg. 2016-07-08
***************************************************************************************************/
#if (!defined DUET2)&&(!defined TSINGMA)
/*special used for debug. control by lcsh cmd.*/
uint8 g_skip_auto_flag=0;

void
lcm_port_cl73_auto_timer (void* p_data)
{
    CTC_TASK_GET_MASTER
    glb_card_t* p_card = NULL; 
    uint8 port_idx;

    CTC_TASK_ADD_TIME_MSEC(lcm_port_cl73_auto_timer, NULL, LCM_PORT_DEFAULT_CL73_AUTO_INTERVAL);
    if(g_skip_auto_flag)
    {
        return;
    }
    p_card= lcm_mgt_get_card();  
    for(port_idx = 0; port_idx < p_card->port_num; port_idx++)
    {
        phy_do_cl73_auto(port_idx);
    }
    return;
}
#endif
#endif /*_GLB_UML_SYSTEM_*/

/* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
int32
lcm_port_poll_all_ports(void* pv_arg)
{
#ifdef _GGEMU_
// TODO: removed by xgu for gg emualtion board; 2014-6-2
return 0;
#endif
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    glb_phy_state_t phy_val;
    phy_state_change_t phy_change;
    uint8 port_idx;

    LCM_LOG_DEBUG(LCM_CARD, "Lcm   poll all port status.");
    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    for(port_idx = 1; port_idx <= p_card->port_num; port_idx++)
    {
        p_port = p_card->pp_port[port_idx - 1];

        /*modified by xgu for bug 13493, 2011-01-11*/        
        if(!p_port || !p_port->create_done)
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d not create.", port_idx);
            continue;
        }
        
        memset(&phy_val,0,sizeof(glb_phy_state_t));
        memset(&phy_change,0,sizeof(phy_state_change_t));

        if(phy_get_link_poll(port_idx - 1, &phy_val, &phy_change))
        {
            /*Poll Error*/
            continue;
        }
        _lcm_port_link_change_process(port_idx, &phy_val, &phy_change);
    }
#endif   
    return LCM_E_SUCCESS;
}
/* end of qicx added */

#if 0
int32
test_gport_link_flap(uint16 my_gport)
{
    uint16 gport = my_gport;
    lcm_port_poll_port_status(&gport);
    
    return LCM_E_SUCCESS;
}
#endif

int32
lcm_port_poll_port_status(void* pv_arg)
{
#ifdef _GGEMU_
return 0;
#endif
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    glb_phy_state_t phy_val;
    phy_state_change_t phy_change;
    uint8 port_idx;
    uint16 *gport;
    ctc_chip_device_info_t device_info;

    gport = (uint16*)pv_arg;

    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    for(port_idx = 1; port_idx <= p_card->port_num; port_idx++)
    {
        p_port = p_card->pp_port[port_idx - 1];

        /*modified by xgu for bug 13493, 2011-01-11*/        
        if(!p_port || !p_port->create_done)
        {
            LCM_LOG_DEBUG(LCM_CARD, "Lcm port %d not create.", port_idx);
            continue;
        }

        if(*gport == p_port->logic_port_idx)
        {
            memset(&phy_val,0,sizeof(glb_phy_state_t));
            memset(&phy_change,0,sizeof(phy_state_change_t));

            /* for bug 47969, merged from v6.2. For GG1.0, 40G must do polling after intr event reported. add by shil, 2018-07-13 */
            ctc_chip_get_property(0, CTC_CHIP_PROP_DEVICE_INFO, (void*)&device_info);
            if ((sal_strncmp(device_info.chip_name, "CTC8096", 7) == 0)&&(device_info.version_id <= 1)
                && ((p_port->port_speed_ability & (GLB_SUPPORT_SPEED_40G|GLB_SUPPORT_SPEED_100G)) != 0))
            {
                if (phy_get_link_tolerable_time(port_idx - 1)==0)
                {
                    phy_config_link_tolerable_time(port_idx - 1, 5); /* set link tolerable time to 5ms */
                }
            }
            if(phy_get_link_interrupt(port_idx - 1, &phy_val, &phy_change))
            {
                break;;
            }
#ifdef CNOS_OS
            char port_name[GLB_PHY_PORT_FULL_NAME_SZ];
            extern int32 hsrv_carrier_blink_down_process(char *ifname);
            if (phy_change.action == -1)
            {
                sal_memset(port_name, 0, sizeof(port_name));
                if_build_port_fullname_by_slot_port(port_name, p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
                hsrv_carrier_blink_down_process(port_name);
            }
#endif
            if (phy_change.action != -1)
            {
                LCM_LOG_DEBUG(LCM_CARD, "Lcm intr port %d action %d.", port_idx, phy_change.action);
            }
            _lcm_port_link_change_process(port_idx, &phy_val, &phy_change);
            break;
        }
    }

#endif   
    return LCM_E_SUCCESS;
}

/***************************************************************************************************
 * Name         : lcm_port_led_cfg 
 * Purpose      : management port led          
 * Input        : N/A         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
void
lcm_port_led_cfg(void* p_data)
{
#ifndef _GLB_UML_SYSTEM_
    CTC_TASK_GET_MASTER

    CTC_TASK_ADD_TIME_MSEC(lcm_port_led_cfg, NULL, 5000);
    led_mgt_port_led();
#endif
    return;
}

int32 lcm_port_speed_mode_config(glb_speed_mode_t mode)
{
    int i,ret=0;
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    lcapi_lcm_port_info_t panel_port_info;
    p_card= lcm_mgt_get_card();

    for(i=0; i<p_card->port_num; i++)
    {
        p_port = p_card->pp_port[i];
        if(!p_port->speed_mode_cfg_en)
        {
            continue;
        }
        if(p_port->speed_mode == mode)
        {
            continue;
        }
        if(((E_SPEED_1_10G == mode)&(p_port->port_cfg.speed == GLB_SPEED_25G))
         ||((E_SPEED_1_25G == mode)&(p_port->port_cfg.speed == GLB_SPEED_10G))
         ||((E_SPEED_10_25G == mode)&(p_port->port_cfg.speed == GLB_SPEED_1G))
         ||((E_SPEED_1_10G == mode)&(p_port->port_cfg.speed == GLB_SPEED_AUTO)))
        {
            LCM_LOG_ERR("Set port %d speed-mode %d fail", i+1, mode);
            return RESULT_ERROR;
        }
    }

    for(i=0; i<p_card->port_num; i++)
    {
        p_port = p_card->pp_port[i];
        if(!p_port->speed_mode_cfg_en)
        {
            continue;
        }
        if(p_port->speed_mode == mode)
        {
            continue;
        }
        if(E_SPEED_1_10G == mode)
        {
            p_port->port_speed_ability =(GLB_SUPPORT_SPEED_1G |GLB_SUPPORT_SPEED_10G);
            p_port->speed_mode = mode;
        }
        else if(E_SPEED_1_25G == mode)
        {
            p_port->port_speed_ability =(GLB_SUPPORT_SPEED_25G |GLB_SUPPORT_SPEED_1G);
            p_port->speed_mode = mode;
        }
        else if(E_SPEED_10_25G == mode)
        {
            p_port->port_speed_ability =(GLB_SUPPORT_SPEED_10G |GLB_SUPPORT_SPEED_25G);
            p_port->speed_mode = mode;
        }
        //lcm_port_update_speed_mode(p_port);
        if ((ret = lcm_msg_tx_lcPortSpeedMode(p_port, 0))<0)
        {
            return ret;
        }
        panel_port_info.port_no = p_port->panel_port_no;
        panel_port_info.sub_port_no= p_port->panel_sub_port_no;
        panel_port_info.slot_no= p_port->panel_slot_no;
        ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_UPDATE_LINK_STATUS, &panel_port_info);
#if 0 ///TODO: wait for update fiber driver
        if(!lcm_fiber_is_present(i) && lcm_fiber_is_created(i))
        {
            p_port->fiber_triger = 1; /* triger this manu */
            lcm_fiber_poll_event(p_port);
        }
#endif
    }

    return RESULT_OK;
}

