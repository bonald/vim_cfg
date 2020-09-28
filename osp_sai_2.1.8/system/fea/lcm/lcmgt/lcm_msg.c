/**************************************************************************
* lcm_msg.c   :   lcm msg api
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* 
* Revision  :        R0.1
* Author    :        tongzb
* Date      :        2019 - 09 - 28
* Reason    :        First Create.
**************************************************************************/
#include "sal_common.h"
#include "glb_hw_define.h"
#include "lcapi.h"
#include "lcm_client.h"
#include "lcm_mgt.h"
#include "lcm_msg.h"
#include "lcm_card.h"
#include "lcm_error.h"
#include "lcm_debug.h"
#include "lcm_hagttlk.h"
#include "lcm_port.h"
#include "ctc_pci.h"
#include "eeprom_api.h"
#include "glb_stm_define.h"
#include "led_api.h"
#include "sensor_api.h"
#include "sensor_drv.h"
#include "epld_api.h"
#include "power_api.h"
#include "fan_api.h"
#include "mux_api.h"
#include "bootrom_drv.h"
#include "ctclib_show.h"
#include "fiber_api.h"
#include "epld_api.h"
#include "phy_api.h"
#include "lcm_log.h"
#include "dal.h"
#include "sysenv_api.h"
#include "glb_distribute_system_define.h"
#include "glb_if_define.h"
#include "ctclib_sys_cmd.h"

//#include "stack.h"
#include "ctc_api.h"
#include "ctc_chip.h"

struct vct_msg_s
{
    uint32 port_id;
};
typedef struct vct_msg_s vct_msg_t;

void lcm_vct_async_timeout(void* p_data);
LCM_DEFINE_TASK_ENCAP(lcm_vct_async_timeout); 

extern int32 lcm_stack_tx_lcm_chsm(LcmMsg_t *msg);
extern int32 lcm_stack_tx_lcm_local_chsm(LcmMsg_t *msg);
extern int32 if_build_port_shortname_by_slot_port (char * ifname, uint16 slot_id, uint16 port_id, uint16 sub_port_id);

int32 lcm_msg_tx(LcmMsg_t *msg,int tx_attribute)
{
    int32 ret = LCM_E_SUCCESS;

    lcm_clnt_t *clnt = lcm_get_lcm_client();
    if ((ret = lcm_clnt_send(clnt, msg)) < 0)
    {
        LCM_LOG_ERR( "%d", ret);
        return ret;
    }

    return LCM_E_SUCCESS;
}

void
lcm_vct_async_timeout(void* p_data)
{
#ifndef _GLB_UML_SYSTEM_          
    CTC_TASK_GET_MASTER
    int32 ret;
    uint32 port_id;
    glb_port_cable_info_t port_cable_info;
    vct_msg_t* p_vct_msg = (vct_msg_t* )p_data;
        
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm VCT async timeout!\n");
    port_id = p_vct_msg->port_id;

    ret = phy_get_cable_info(port_id, &port_cable_info);
    if(ret != GLB_VCT_OK)
    {
        if(ret == GLB_VCT_WAIT)
        {
            CTC_TASK_ADD_TIME_MSEC(lcm_vct_async_timeout, (void*)p_vct_msg, GLB_VCT_DETECT_TIME);
            return;
        }
        LCM_LOG_ERR("Lcm get VCT cable info fail!\n");
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm get VCT cable info: pair_a = %d, pair_b = %d, pair_c = %d, pair_d = %d\n",
                   port_cable_info.pair_A.pair_length, port_cable_info.pair_B.pair_length,
                   port_cable_info.pair_C.pair_length, port_cable_info.pair_D.pair_length);
    ret = lcm_msg_tx_lcChsmVCT(port_id, &port_cable_info, 0);

    if (NULL != p_vct_msg)
    {
        LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_vct_msg);
    }
#endif    
    return;
}

int32
lcm_vct_start_timer(void* arg)
{
    CTC_TASK_GET_MASTER
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm VCT start timer!\n");
    
    CTC_TASK_ADD_TIME_MSEC(lcm_vct_async_timeout, arg, GLB_VCT_DETECT_TIME);
    return LCM_E_SUCCESS;
}


#if SEP("TX Function")
int32
lcm_msg_tx_lcChsmKeepaliveAck(void)
{
    LcmMsg_t msg;
    int32 ret;
    glb_card_t* p_card;
    p_card = lcm_mgt_get_card();
    
    memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmKeepaliveAck;
    msg.op.lcChsmKeepaliveAck.slot = p_card->logic_slot_no;
    
    if ((ret = lcm_msg_tx(&msg,0)) < 0)
    {
        LCM_LOG_ERR( "%d", ret);
        return ret;
    }
    return 0;
}








int32 
lcm_msg_tx_lcPortInfo(glb_port_t* p_port, int tx_attribute)
{
    LcmMsg_t msg;
    int32 logic_slot;
    unsigned char mac[6];
    int mac_length;
    char ifname[GLB_PHY_PORT_FULL_NAME_SZ];
    
    LCM_MGT_CHK_SLOT(logic_slot);
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcPortInfo;
    msg.op.lcPortInfo.portIdx = p_port->port_idx;
    msg.op.lcPortInfo.portLogicIdx = p_port->logic_port_idx;
    msg.op.lcPortInfo.portMedia = p_port->port_cfg.media;
    msg.op.lcPortInfo.portPhyType = p_port->phy_type;
    /*Fix bug 14686. jqiu 2011-06-15*/
    msg.op.lcPortInfo.portPanelSlot = p_port->panel_slot_no;
    msg.op.lcPortInfo.portPanelPort = p_port->panel_port_no;
    msg.op.lcPortInfo.subportPanelPort = p_port->panel_sub_port_no;

    /*Fix bug 18162. zhangjx 2012-05-30*/
    msg.op.lcPortInfo.poesupport = p_port->poe_support;
    msg.op.lcPortInfo.portspeedability = p_port->port_speed_ability;
    msg.op.lcPortInfo.portspeedreal = p_port->port_speed_real;
    msg.op.lcPortInfo.lpsupport = p_port->lp_support;
    msg.op.lcPortInfo.iscombo = p_port->is_combo;
    /* Modified by liuht for bug 29005, 2014-06-17 */
    msg.op.lcPortInfo.eeesupport = p_port->eee_support;
    msg.op.lcPortInfo.unidirsupport = p_port->unidir_support;
    /* End of liuht modified */
    /* for bug 47572,add by tongzb, support speed 1G/10G/25G */
    msg.op.lcPortInfo.speedmodecfgen = p_port->speed_mode_cfg_en;    
    msg.op.lcPortInfo.speedmode = p_port->speed_mode;    
    msg.op.lcPortInfo.mediaSwitchSupport = p_port->support_media_switch;
    if_build_port_shortname_by_slot_port(ifname, p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
    lcm_mgt_get_if_hwaddr(ifname, (unsigned char *)mac, &mac_length);
    if (mac_length)
    {
        msg.op.lcPortInfo.mac.buf = (uint8_t *)mac;
        msg.op.lcPortInfo.mac.size = mac_length;
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK,    "Line card send port %d logic_port %d media %d max speed %d.", 
                  p_port->port_idx, p_port->logic_port_idx, p_port->port_cfg.media, p_port->port_speed_ability);
    return lcm_msg_tx(&msg,tx_attribute);
}

int32 
lcm_msg_tx_lcFiberAttach(glb_port_t *p_port, int tx_attribute)
{
    LcmMsg_t msg;
    
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcFiberModuleAttach;
    msg.op.lcFiberModuleAttach.slot = p_port->logic_slot_idx;
    msg.op.lcFiberModuleAttach.port = p_port->port_idx;
    msg.op.lcFiberModuleAttach.type = (((fiber_info_t*)p_port->p_fiber)->fiber_type);
    msg.op.lcFiberModuleAttach.fiberflag = (((fiber_info_t*)p_port->p_fiber)->fiber_flag);
    msg.op.lcFiberModuleAttach.iscopper = (((fiber_info_t*)p_port->p_fiber)->is_copper);
    msg.op.lcFiberModuleAttach.channelidx = (((fiber_info_t*)p_port->p_fiber)->channel_idx);
    msg.op.lcFiberModuleAttach.channelnum = (((fiber_info_t*)p_port->p_fiber)->channel_num);
    msg.op.lcFiberModuleAttach.name.size = FIBER_VENDOR_NAME_LEN+1;
    msg.op.lcFiberModuleAttach.name.buf = (uint8*)(((fiber_info_t*)p_port->p_fiber)->name);
    msg.op.lcFiberModuleAttach.pn.size = FIBER_VENDOR_PN_LEN+1;
    msg.op.lcFiberModuleAttach.pn.buf = (uint8*)(((fiber_info_t*)p_port->p_fiber)->pn);
    msg.op.lcFiberModuleAttach.sn.size = FIBER_VENDOR_SN_LEN+1;
    msg.op.lcFiberModuleAttach.sn.buf = (uint8*)(((fiber_info_t*)p_port->p_fiber)->sn);
    msg.op.lcFiberModuleAttach.wavelength.size = sizeof(((fiber_info_t*)p_port->p_fiber)->wavelength);
    msg.op.lcFiberModuleAttach.wavelength.buf = ((fiber_info_t*)p_port->p_fiber)->wavelength;
    msg.op.lcFiberModuleAttach.length.size = sizeof(((fiber_info_t*)p_port->p_fiber)->length);
    msg.op.lcFiberModuleAttach.length.buf = ((fiber_info_t*)p_port->p_fiber)->length;
    msg.op.lcFiberModuleAttach.complianceCode.size = sizeof(((fiber_info_t*)p_port->p_fiber)->compliance_code);
    msg.op.lcFiberModuleAttach.complianceCode.buf = ((fiber_info_t*)p_port->p_fiber)->compliance_code;
    msg.op.lcFiberModuleAttach.ddm = ((fiber_info_t*)p_port->p_fiber)->ddm_support;
    if(msg.op.lcFiberModuleAttach.ddm)
    {
        msg.op.lcFiberModuleAttach.txPwr.curr1          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleAttach.txPwr.curr2          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleAttach.txPwr.curr3          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleAttach.txPwr.curr4          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleAttach.txPwr.highAlarm      = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.txPwr.lowAlarm       = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.txPwr.highWarn       = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleAttach.txPwr.lowWarn        = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleAttach.rxPwr.curr1          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleAttach.rxPwr.curr2          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleAttach.rxPwr.curr3          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleAttach.rxPwr.curr4          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleAttach.rxPwr.highAlarm      = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.rxPwr.lowAlarm       = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.rxPwr.highWarn       = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleAttach.rxPwr.lowWarn        = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleAttach.tmpr.curr1            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleAttach.tmpr.curr2            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleAttach.tmpr.curr3            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleAttach.tmpr.curr4            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleAttach.tmpr.highAlarm       = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.tmpr.lowAlarm        = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.tmpr.highWarn        = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleAttach.tmpr.lowWarn         = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleAttach.vcc.curr1         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleAttach.vcc.curr2         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleAttach.vcc.curr3         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleAttach.vcc.curr4         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleAttach.vcc.highAlarm    = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.vcc.lowAlarm     = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.vcc.highWarn     = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleAttach.vcc.lowWarn      = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleAttach.bias.curr1        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleAttach.bias.curr2        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleAttach.bias.curr3        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleAttach.bias.curr4        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleAttach.bias.highAlarm    = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.bias.lowAlarm     = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleAttach.bias.highWarn     = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleAttach.bias.lowWarn      = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_LOW_WARN_VALUE];
    }
    msg.op.lcFiberModuleAttach.tunableinfo.tunablesupport = ((fiber_info_t*)p_port->p_fiber)->tunable_support;
    if(msg.op.lcFiberModuleAttach.tunableinfo.tunablesupport)
    {
        msg.op.lcFiberModuleAttach.tunableinfo.tunemode           = ((fiber_info_t*)p_port->p_fiber)->tune_mode;
        msg.op.lcFiberModuleAttach.tunableinfo.firstfreq           = ((fiber_info_t*)p_port->p_fiber)->first_freq;
        msg.op.lcFiberModuleAttach.tunableinfo.lastfreq           = ((fiber_info_t*)p_port->p_fiber)->last_freq;
        msg.op.lcFiberModuleAttach.tunableinfo.stepfreq           = ((fiber_info_t*)p_port->p_fiber)->step_freq;
        msg.op.lcFiberModuleAttach.tunableinfo.tunablewavelength  = ((fiber_info_t*)p_port->p_fiber)->tunable_wavelength;
        msg.op.lcFiberModuleAttach.tunableinfo.dithersupport      = ((fiber_info_t*)p_port->p_fiber)->dither_support;
        msg.op.lcFiberModuleAttach.tunableinfo.wavechannels       = ((fiber_info_t*)p_port->p_fiber)->wave_channels;
        msg.op.lcFiberModuleAttach.tunableinfo.wavechannelidx    = ((fiber_info_t*)p_port->p_fiber)->wave_channel_idx;
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send Fiber attach on port %d.", p_port->port_idx);
    
    return lcm_msg_tx(&msg,tx_attribute);
}

int32 
lcm_msg_tx_lcFiberDeAttach(glb_port_t *p_port, int tx_attribute)
{
    LcmMsg_t msg;
        
    memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcFiberModuleDeattach;
    msg.op.lcFiberModuleDeattach.slot = p_port->logic_slot_idx;
    msg.op.lcFiberModuleDeattach.port = p_port->port_idx;    

    return lcm_msg_tx(&msg,tx_attribute);
}

int32
lcm_msg_tx_lcFiberUpdate(glb_port_t *p_port, int tx_attribute)
{
    LcmMsg_t msg;
        
    memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcFiberModuleUpdate;
    msg.op.lcFiberModuleUpdate.slot = p_port->logic_slot_idx;
    msg.op.lcFiberModuleUpdate.port = p_port->port_idx;
    msg.op.lcFiberModuleUpdate.ddm = ((fiber_info_t*)p_port->p_fiber)->ddm_support;
    if(msg.op.lcFiberModuleUpdate.ddm)
    {
        msg.op.lcFiberModuleUpdate.txPwr.curr1          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleUpdate.txPwr.curr2          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleUpdate.txPwr.curr3          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleUpdate.txPwr.curr4          = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleUpdate.txPwr.highAlarm      = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.txPwr.lowAlarm       = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.txPwr.highWarn       = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleUpdate.txPwr.lowWarn        = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleUpdate.rxPwr.curr1          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleUpdate.rxPwr.curr2          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleUpdate.rxPwr.curr3          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleUpdate.rxPwr.curr4          = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleUpdate.rxPwr.highAlarm      = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.rxPwr.lowAlarm       = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.rxPwr.highWarn       = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleUpdate.rxPwr.lowWarn        = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleUpdate.tmpr.curr1            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleUpdate.tmpr.curr2            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleUpdate.tmpr.curr3            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleUpdate.tmpr.curr4            = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleUpdate.tmpr.highAlarm       = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.tmpr.lowAlarm        = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.tmpr.highWarn        = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleUpdate.tmpr.lowWarn         = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleUpdate.vcc.curr1         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleUpdate.vcc.curr2         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleUpdate.vcc.curr3         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleUpdate.vcc.curr4         = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleUpdate.vcc.highAlarm    = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.vcc.lowAlarm     = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.vcc.highWarn     = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleUpdate.vcc.lowWarn      = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_LOW_WARN_VALUE];

        msg.op.lcFiberModuleUpdate.bias.curr1        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE1];
        msg.op.lcFiberModuleUpdate.bias.curr2        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE2];
        msg.op.lcFiberModuleUpdate.bias.curr3        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE3];
        msg.op.lcFiberModuleUpdate.bias.curr4        = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE4];
        msg.op.lcFiberModuleUpdate.bias.highAlarm    = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_HIGH_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.bias.lowAlarm     = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_LOW_ALARM_VALUE];
        msg.op.lcFiberModuleUpdate.bias.highWarn     = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_HIGH_WARN_VALUE];
        msg.op.lcFiberModuleUpdate.bias.lowWarn      = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_LOW_WARN_VALUE];
    }
    msg.op.lcFiberModuleUpdate.tunableinfo.tunablesupport = ((fiber_info_t*)p_port->p_fiber)->tunable_support;
    if(msg.op.lcFiberModuleUpdate.tunableinfo.tunablesupport)
    {
        msg.op.lcFiberModuleUpdate.tunableinfo.tunemode           = ((fiber_info_t*)p_port->p_fiber)->tune_mode;
        msg.op.lcFiberModuleUpdate.tunableinfo.firstfreq           = ((fiber_info_t*)p_port->p_fiber)->first_freq;
        msg.op.lcFiberModuleUpdate.tunableinfo.lastfreq           = ((fiber_info_t*)p_port->p_fiber)->last_freq;
        msg.op.lcFiberModuleUpdate.tunableinfo.stepfreq           = ((fiber_info_t*)p_port->p_fiber)->step_freq;
        msg.op.lcFiberModuleUpdate.tunableinfo.tunablewavelength  = ((fiber_info_t*)p_port->p_fiber)->tunable_wavelength;
        msg.op.lcFiberModuleUpdate.tunableinfo.dithersupport      = ((fiber_info_t*)p_port->p_fiber)->dither_support;
        msg.op.lcFiberModuleUpdate.tunableinfo.wavechannels       = ((fiber_info_t*)p_port->p_fiber)->wave_channels;
        msg.op.lcFiberModuleUpdate.tunableinfo.wavechannelidx    = ((fiber_info_t*)p_port->p_fiber)->wave_channel_idx;
    }

    return lcm_msg_tx(&msg,tx_attribute); 
}

int32 
lcm_msg_tx_lcUptimeGetAck(int tx_attribute)
{
    LcmMsg_t msg;
    time_t diff = 0;
	struct sal_timeval uptime;
	
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send Uptime");
	
#ifndef _GLB_UML_SYSTEM_
    /* get system uptime */
    /* modified by jqiu 2013-03-06
     * some platform don't support param -1 in gettimeofday.
     * use another sys call clock_gettime() */
    sal_ctc_clock_gettime(&uptime);
#else
    gettimeofday(&uptime, (struct sal_timezone *)-1);
#endif
    diff = uptime.tv_sec;
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcUptimeGetAck;

    msg.op.lcUptimeGetAck.day  = diff/86400;
    msg.op.lcUptimeGetAck.hour = (diff/3600)%24;
    msg.op.lcUptimeGetAck.min  = (diff/60)%60;
	
    return lcm_msg_tx(&msg,tx_attribute); 
}

int32 
lcm_msg_tx_lcTmprThreshold(int32 low, int32 high, int32 crit, int tx_attribute)
{
    LcmMsg_t msg;
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm tx ChsmTmprGet ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }    

    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcTmprThreshold;
    msg.op.lcTmprThreshold.slot = p_card->logic_slot_no;
    msg.op.lcTmprThreshold.low = low;
    msg.op.lcTmprThreshold.high = high;
    msg.op.lcTmprThreshold.crit = crit;

    return lcm_msg_tx(&msg,tx_attribute); 
}

int32 
lcm_msg_tx_lcOemInfo(int tx_attribute)
{
    LcmMsg_t msg;
    uint8 buf[MAX_EEPROM_OEM_INFO_LEN+1];
    
    sal_memset(buf, 0, sizeof(buf));
    lcm_mgt_oem_info_get((char *)buf);

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send lcOemInfoGetAck");
        
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcOemInfoGetAck;
    msg.op.lcOemInfoGetAck.oemInfo.buf = buf;
    msg.op.lcOemInfoGetAck.oemInfo.size = MAX_EEPROM_OEM_INFO_LEN;
    LCM_LOG_BRINGUP("Line card send OemInfo");
    
    return lcm_msg_tx(&msg,tx_attribute); 
}

int32 
lcm_msg_tx_lcChsmTmprGetAck(int tx_attribute)
{
    LcmMsg_t msg;
    int32 ret;
    uint32 sensor_sum, i = 0;
    TmprValue_t tmpr_value[MAX_TMPR_SENSOR_NUM];
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm tx ChsmTmprGet ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }    

    sensor_sum = p_card->sensor_num + p_card->chip_sensor_num;
    for (i = 0; i < sensor_sum; i++)
    {
        tmpr_value[i].index= i; 
        tmpr_value[i].value= p_card->tmpr_status[i].tmpr_val;
        tmpr_value[i].pos = p_card->tmpr_status[i].pos;
    }
        
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmTmprGetAck;
    msg.op.lcChsmTmprGetAck.count = sensor_sum;
    for (i = 0; i < sensor_sum; i++)
    {
        ASN_SET_ADD(&(msg.op.lcChsmTmprGetAck.tmpr.list), &tmpr_value[i]);
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send sensor temperature.");

    ret = lcm_msg_tx(&msg,tx_attribute);     
    asn_set_empty(&(msg.op.lcChsmTmprGetAck.tmpr.list));
    return ret;
}

int32 
lcm_msg_tx_lcchsmGetFanStatusAck(int tx_attribute)
{
    LcmMsg_t msg;
    int32 ret;
    uint8 i = 0;
    glb_card_t *p_card = NULL;
    FanValue_t fan_val[MAX_FAN_TRAY_NUM];

    sal_memset(&fan_val, 0, (sizeof(FanValue_t))*MAX_FAN_TRAY_NUM);
    
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm tx chsmGetFanStatus ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    for(i=0; i<p_card->fan_module_num; i++)
    {
        fan_val[i].index = i;
        fan_val[i].num = p_card->fan_status[i].num;
        fan_val[i].speedAdjust = p_card->fan_status[i].speed_adjust;
        fan_val[i].present = p_card->fan_status[i].present;
        fan_val[i].status = p_card->fan_status[i].status;
        fan_val[i].speedLevel = p_card->fan_status[i].speed_level;
        fan_val[i].speedRate = p_card->fan_status[i].speed_rate;
    }
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmFanStatusGetAck;
    msg.op.lcChsmFanStatusGetAck.count = p_card->fan_module_num;
    for (i = 0; i < p_card->fan_module_num; i++)
    {
        ASN_SET_ADD(&(msg.op.lcChsmFanStatusGetAck.fan.list), &fan_val[i]);
    }
    
    LCM_LOG_DEBUG(LCM_CHSMTLK,    "Line card send fan status.");

    ret = lcm_msg_tx(&msg,tx_attribute); 
    asn_set_empty(&(msg.op.lcChsmFanStatusGetAck.fan.list));
    return ret;
}

int32 
lcm_msg_tx_lcchsmGetPsuStatusAck(int tx_attribute)
{
    LcmMsg_t msg;
    int32 ret;
    uint32 i = 0;
    glb_card_t *p_card = NULL;
    PsuValue_t psu_val[MAX_PSU_NUM];

    sal_memset(&psu_val, 0, (sizeof(PsuValue_t))*MAX_PSU_NUM);
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm tx chsmGetPsuStatus ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    } 

    for (i = 0; i < p_card->psu_module_num; i++)
    {
        psu_val[i].index = i;
        psu_val[i].psuMode = p_card->psu_status[i].psu_mode;
        psu_val[i].psuWorkStatus = p_card->psu_status[i].psu_work_status;
        psu_val[i].psuAlert = p_card->psu_status[i].psu_alert;
        psu_val[i].fixedstatus = p_card->psu_status[i].fixed_status;
        psu_val[i].psuAbsent = p_card->psu_status[i].psu_absent;
        psu_val[i].psuType = p_card->psu_status[i].psu_type;
        psu_val[i].psuHealthStatusChange = p_card->psu_status[i].psu_health_status_change;

        if (psu_val[i].psuHealthStatusChange)
        {
            psu_val[i].psuPmbusSupport = p_card->psu_status[i].psu_pmbus_status.support_pmbus;
            psu_val[i].psuPmbusStatusByte = p_card->psu_status[i].psu_pmbus_status.status_byte;
            psu_val[i].psuPmbusStatusWord = p_card->psu_status[i].psu_pmbus_status.status_word;
            psu_val[i].psuPmbusStatusVout = p_card->psu_status[i].psu_pmbus_status.status_vout;
            psu_val[i].psuPmbusStatusIout = p_card->psu_status[i].psu_pmbus_status.status_iout;
            psu_val[i].psuPmbusStatusInput = p_card->psu_status[i].psu_pmbus_status.status_input;
            psu_val[i].psuPmbusStatusTemperature = p_card->psu_status[i].psu_pmbus_status.status_temperature;
            psu_val[i].psuPmbusStatusCml = p_card->psu_status[i].psu_pmbus_status.status_cml;
            psu_val[i].psuPmbusStatusOther = p_card->psu_status[i].psu_pmbus_status.status_other;
            psu_val[i].psuPmbusStatusMfrSpecific = p_card->psu_status[i].psu_pmbus_status.status_mfr_specific;
            psu_val[i].psuPmbusStatusFan12 = p_card->psu_status[i].psu_pmbus_status.status_fans_1_2;
            psu_val[i].psuPmbusStatusFan34 = p_card->psu_status[i].psu_pmbus_status.status_fans_3_4;
        }
    }

    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmPsuStatusGetAck;
    msg.op.lcChsmPsuStatusGetAck.count = p_card->psu_module_num;
    
    for (i = 0; i < p_card->psu_module_num; i++)
    {
        ASN_SET_ADD(&(msg.op.lcChsmPsuStatusGetAck.psu.list), &psu_val[i]);
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send psu status.");
    
    ret = lcm_msg_tx(&msg,tx_attribute);
    asn_set_empty(&(msg.op.lcChsmPsuStatusGetAck.psu.list));
    return ret;
}

int32 
lcm_msg_tx_lcchsmGetVcmCfgAck(int tx_attribute)
{
    LcmMsg_t msg;
    int32 ret;
    uint32 i = 0;
    uint32 j = 0;
    VcmCfg_t vcmCfg[MAX_VCM_MODULE_NUM];
    VcmChannelCfg_t vcmChannelCfg[MAX_VCM_MODULE_NUM][MAX_VCM_CHANNEL_NUM];
    glb_card_t* p_card = NULL;
    
    p_card = lcm_mgt_get_card();
    sal_memset(&vcmCfg, 0, (sizeof(VcmCfg_t))*MAX_VCM_MODULE_NUM);
    
    for (i = 0; i < p_card->vcm_module_num; i++)
    {
        sal_memset(&vcmChannelCfg[i], 0, (sizeof(VcmChannelCfg_t))*MAX_VCM_CHANNEL_NUM);
        for(j = 0; j< MAX_VCM_CHANNEL_NUM; j++)
        {
            vcmChannelCfg[i][j].flag = p_card->vcm_cfg[i].channel_cfg[j].flag;
            vcmChannelCfg[i][j].currDependPin = p_card->vcm_cfg[i].channel_cfg[j].curr_depend_pin;
            vcmChannelCfg[i][j].voltSplitFactor = p_card->vcm_cfg[i].channel_cfg[j].volt_split_factor;
            vcmChannelCfg[i][j].high = p_card->vcm_cfg[i].channel_cfg[j].high;
            vcmChannelCfg[i][j].low = p_card->vcm_cfg[i].channel_cfg[j].low;
            vcmChannelCfg[i][j].name.buf = p_card->vcm_cfg[i].channel_cfg[j].name;
            vcmChannelCfg[i][j].name.size = MAX_VCM_CHANNEL_LEN;
            ASN_SET_ADD(&(vcmCfg[i].vcmChannelCfg.list), &vcmChannelCfg[i][j]);
            if(p_card->vcm_cfg[i].channel_cfg[j].flag & GLB_PIN_FLAG_OVER)
                break;
        }
        LCM_LOG_DEBUG(LCM_CHSMTLK,  "line card get config of vcm %d",i);
        vcmCfg[i].channelNum = j;
    }
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmVcmCfgGetAck;
    msg.op.lcChsmVcmCfgGetAck.count = p_card->vcm_module_num;
    for (i = 0; i < p_card->vcm_module_num; i++)
    {
        ASN_SET_ADD(&(msg.op.lcChsmVcmCfgGetAck.vcmCfg.list), &vcmCfg[i]);
    }

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send vcm config.");
    
    ret = lcm_msg_tx(&msg,tx_attribute);
    asn_set_empty(&(msg.op.lcChsmVcmCfgGetAck.vcmCfg.list));
    return ret;
}

int32 
lcm_msg_tx_lcchsmGetVcmStatusAck(int tx_attribute)
{
    LcmMsg_t msg;
    int32 ret;
    uint32 i = 0;
    uint32 j = 0;
    VcmStatus_t vcmstatus[MAX_VCM_MODULE_NUM];
    VcmMonitor_t vcm_monitor[MAX_VCM_MODULE_NUM][MAX_VCM_CHANNEL_NUM];
    glb_card_t* p_card = NULL;

    p_card = lcm_mgt_get_card();
    sal_memset(&vcmstatus, 0, (sizeof(VcmStatus_t))*MAX_VCM_MODULE_NUM);
    
    for (i = 0; i < p_card->vcm_module_num; i++)
    {
        sal_memset(&vcm_monitor[i], 0, (sizeof(VcmMonitor_t))*MAX_VCM_CHANNEL_NUM);
        for(j = 0; j< MAX_VCM_CHANNEL_NUM; j++)
        {
            vcm_monitor[i][j].volt = p_card->vcm_status[i].volt[j];
            vcm_monitor[i][j].curr = p_card->vcm_status[i].curr[j];
            vcm_monitor[i][j].temperature = p_card->vcm_status[i].tmpr[j];
            ASN_SET_ADD(&(vcmstatus[i].vcmMonitor.list), &vcm_monitor[i][j]);
            if(p_card->vcm_cfg[i].channel_cfg[j].flag & GLB_PIN_FLAG_OVER)
                break;
        }
        vcmstatus[i].channelNum = j;
        LCM_LOG_DEBUG(LCM_CHSMTLK,"line card get vcm status of vcm %d",i);
    }
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmVcmStatusGetAck;
    msg.op.lcChsmVcmStatusGetAck.count = p_card->vcm_module_num;
    
    for (i = 0; i < p_card->vcm_module_num; i++)
    {
        ASN_SET_ADD(&(msg.op.lcChsmVcmStatusGetAck.vcmStatus.list), &vcmstatus[i]);
    }

    LCM_LOG_DEBUG(LCM_CHSMTLK,  "Line card send vcm status.");

    ret = lcm_msg_tx(&msg,tx_attribute);    
    asn_set_empty(&(msg.op.lcChsmVcmStatusGetAck.vcmStatus.list));
    return ret;
}

int32
lcm_msg_tx_lcChsmVCT(uint32 port_id, glb_port_cable_info_t* p_cable_info, int tx_attribute)
{
    LcmMsg_t msg;

    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm send VCT message!\n");

    msg.op.present = op_PR_lcChsmVCTGetInfo;
    msg.op.lcChsmVCTGetInfo.portId     = port_id;
    msg.op.lcChsmVCTGetInfo.remotePair = p_cable_info->remote_pair;
    msg.op.lcChsmVCTGetInfo.portSpeed  = p_cable_info->cable_speed;
    msg.op.lcChsmVCTGetInfo.portLinkup = p_cable_info->port_link_up;
    msg.op.lcChsmVCTGetInfo.portEnable = p_cable_info->port_enable;
    msg.op.lcChsmVCTGetInfo.pairA.pairLength = p_cable_info->pair_A.pair_length;
    msg.op.lcChsmVCTGetInfo.pairB.pairLength = p_cable_info->pair_B.pair_length;
    msg.op.lcChsmVCTGetInfo.pairC.pairLength = p_cable_info->pair_C.pair_length;
    msg.op.lcChsmVCTGetInfo.pairD.pairLength = p_cable_info->pair_D.pair_length;
    msg.op.lcChsmVCTGetInfo.pairA.pairState = p_cable_info->pair_A.pair_status;
    msg.op.lcChsmVCTGetInfo.pairB.pairState = p_cable_info->pair_B.pair_status;
    msg.op.lcChsmVCTGetInfo.pairC.pairState = p_cable_info->pair_C.pair_status;
    msg.op.lcChsmVCTGetInfo.pairD.pairState = p_cable_info->pair_D.pair_status;
    msg.op.lcChsmVCTGetInfo.pairA.pairAccuracy = p_cable_info->pair_A.pair_accuracy;
    msg.op.lcChsmVCTGetInfo.pairB.pairAccuracy = p_cable_info->pair_B.pair_accuracy;
    msg.op.lcChsmVCTGetInfo.pairC.pairAccuracy = p_cable_info->pair_C.pair_accuracy;
    msg.op.lcChsmVCTGetInfo.pairD.pairAccuracy = p_cable_info->pair_D.pair_accuracy;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm get VCT cable info: pair_A = %x, pair_B = %x, pair_C = %x, pair_D = %x\n",
                    p_cable_info->pair_A.pair_length, p_cable_info->pair_B.pair_length, p_cable_info->pair_C.pair_length,
                    p_cable_info->pair_D.pair_length);

    return lcm_msg_tx(&msg,tx_attribute);
}

int32
lcm_msg_tx_lcPortSpeedMode(glb_port_t* p_port, int tx_attribute)
{
    LcmMsg_t msg;
    int32 logic_slot;
    
    LCM_MGT_CHK_SLOT(logic_slot);
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcPortSpeedMode;
    msg.op.lcPortSpeedMode.portIdx = p_port->port_idx;
    msg.op.lcPortSpeedMode.portLogicIdx = p_port->logic_port_idx;
    /*Fix bug 14686. jqiu 2011-06-15*/
    msg.op.lcPortSpeedMode.portPanelSlot = p_port->panel_slot_no;
    msg.op.lcPortSpeedMode.portPanelPort = p_port->panel_port_no;
    msg.op.lcPortSpeedMode.subportPanelPort = p_port->panel_sub_port_no;
    msg.op.lcPortSpeedMode.portspeedability = p_port->port_speed_ability;
    msg.op.lcPortSpeedMode.speedmodecfgen = p_port->speed_mode_cfg_en;
    msg.op.lcPortSpeedMode.speedmode = p_port->speed_mode;
    LCM_LOG_DEBUG(LCM_CHSMTLK,    "Line card send port %d logic_port %d max speed %d speed_mode %d.", 
                  p_port->port_idx, p_port->logic_port_idx, p_port->port_speed_ability,p_port->speed_mode);

    return lcm_msg_tx(&msg,tx_attribute); 
}

int32
lcm_msg_tx_lcRebootInfo(int tx_attribute)
{
    glb_card_t *p_card = NULL;
    LcmMsg_t msg;
    int32 ret;
    int32 i = 0;
    struct RebootInfo *p_msg_rebootinfo = NULL;	
    
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm tx lc get reboot info : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    if(0 == p_card->support_reboot_info)
    {
        return LCM_E_SUCCESS;
    }

    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmRebootInfo;
    msg.op.lcChsmRebootInfo.count  = p_card->reboot_times;

    if (p_card->reboot_times > 0)
    {
        p_msg_rebootinfo = LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(struct RebootInfo) * p_card->reboot_times);
        if (!p_msg_rebootinfo)
        {
            LCM_LOG_ERR( "Lcm no alloc memory fail.");
            return LCM_E_NO_MEMORY;
        }
    }

    for (i = 0; i < p_card->reboot_times; i++)
    {
        p_msg_rebootinfo[i].rebootType.buf= (uint8 *)p_card->reboot_info[i].reboot_type;
        p_msg_rebootinfo[i].rebootType.size= sizeof(p_card->reboot_info[i].reboot_type);	
        p_msg_rebootinfo[i].rebootTimes.buf= (uint8 *)p_card->reboot_info[i].reboot_time;
        p_msg_rebootinfo[i].rebootTimes.size= sizeof(p_card->reboot_info[i].reboot_time);			
        ASN_SET_ADD(&(msg.op.lcChsmRebootInfo.rebootInfo.list), &p_msg_rebootinfo[i]);
    }
    LCM_LOG_BRINGUP("Line card send RebootInfo");

    ret = lcm_msg_tx(&msg,tx_attribute); 

    asn_set_empty(&(msg.op.lcDiagResult.items.list));
    LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_msg_rebootinfo);
    p_msg_rebootinfo = NULL;

    if(ret < 0)
        return ret;
 
    return LCM_E_SUCCESS;
}

int32
lcm_msg_tx_lcchsmReset(int tx_attribute)
{
    LcmMsg_t msg;
    glb_card_t* p_card = NULL;

    /*get local line card*/
    p_card= lcm_mgt_get_card();
    if(NULL == p_card)
    {
        LCM_LOG_ERR( "Pointer to card is invalid.");
        return LCM_E_INVALID_PTR;
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK, "LCM reset begin.\n");    

    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_chsmRecoverSystem;
    msg.op.chsmRecoverSystem.reset = TRUE;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send reset board info.");

    return lcm_msg_tx(&msg,tx_attribute);
}

int32
lcm_msg_tx_lcchsmDyingGasp(int tx_attribute)
{
    LcmMsg_t msg;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "LCM dying gasp begin.\n");    

    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_chsmDyingGasp;
    msg.op.chsmDyingGasp.dyingGasp = TRUE;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send power down info.");
    
    return lcm_msg_tx(&msg,tx_attribute);
}

int32 
lcm_msg_tx_lcOnline(int tx_attribute)
{
    LcmMsg_t msg;
    glb_card_t *p_card = NULL;
    int32 ret = 0;
    ctc_chip_device_info_t device_info;
    
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm send lcOnline : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    sal_memset(&msg, 0, sizeof(LcmMsg_t));

    msg.op.present = op_PR_lcOnline;

    msg.op.lcOnline.tmprLimit.boardLow = p_card->tmpr_limit.board_low;
    msg.op.lcOnline.tmprLimit.boardHigh = p_card->tmpr_limit.board_high;
    msg.op.lcOnline.tmprLimit.boardCrit = p_card->tmpr_limit.board_crit;   

    msg.op.lcOnline.hwVer = p_card->hw_ver;
    msg.op.lcOnline.epldVer = p_card->epld_ver;
    msg.op.lcOnline.epldDate = p_card->epld_date;
    msg.op.lcOnline.epldTime = p_card->epld_time;
    /* for bug 53288, added by tongzb, 2019-10-14, for board have no epld */
    msg.op.lcOnline.epldstatus = p_card->epld_status;
    msg.op.lcOnline.flashSize = p_card->flash_size;
    msg.op.lcOnline.dramSize = p_card->dram_size;

    lcm_mgt_get_sw_info(p_card);
    msg.op.lcOnline.swVer.buf = (uint8 *)p_card->sw_ver;
    msg.op.lcOnline.swVer.size = strlen(p_card->sw_ver);
    msg.op.lcOnline.platformType = p_card->platform_type;

    ctc_chip_get_property(0, CTC_CHIP_PROP_DEVICE_INFO, (void*)&device_info);
    msg.op.lcOnline.chipDeviceId = device_info.device_id;
    msg.op.lcOnline.chipVersionId = device_info.version_id;
    msg.op.lcOnline.chipName.buf = (uint8 *)device_info.chip_name;
    msg.op.lcOnline.chipName.size = strlen(device_info.chip_name);

    lcm_mgt_get_current_bootromver(p_card->bootrom_ver);
    msg.op.lcOnline.bootromVer.buf = (uint8 *)p_card->bootrom_ver;
    msg.op.lcOnline.bootromVer.size = strlen(p_card->bootrom_ver);

    lcm_mgt_get_current_serialno(p_card->serial_no);
    msg.op.lcOnline.serialNo.buf = (uint8 *)p_card->serial_no;
    msg.op.lcOnline.serialNo.size = strlen(p_card->serial_no);
    
    if((ret = lcm_msg_tx(&msg,tx_attribute))<0)
    {
        return ret;
    }
    
    lcm_msg_tx_lcRebootInfo(tx_attribute);

    return LCM_E_SUCCESS;
}

#endif


#if SEP("RX Function")
int32 
lcm_msg_rx_chsmLcAttachAck(LcmMsg_t* msg)
{
    glb_card_t *p_card = NULL;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "lcm_msg_rx_chsmLcAttachAck");
    lcm_clnt_t *clnt = lcm_get_lcm_client();

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv lcAttach ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }    
    p_card->card_state = msg->op.chsmLcAttachAck.state;
    clnt->keepalive_cnt = 0;
    lcm_mgt_set_priority_flag(msg->op.chsmLcAttachAck.lcFlag);
    lcm_mgt_set_err_reboot_flag(msg->op.chsmLcAttachAck.errReboot);

    return LCM_E_SUCCESS;
}








int32 
lcm_msg_rx_chsmReloadLc(LcmMsg_t *msg)
{
    int32 ret = LCM_E_SUCCESS;

    LCM_LOG_ERR( "Recieve reload message, reloading...\n");
    ret = lcm_reload();
    return ret;
}

int32 
lcm_msg_rx_chsmTmprThreshold(LcmMsg_t *msg)
{
    int32 ret =0;
    int32 low, high, crit;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx set sensor temperature threshold message.");
    
    low = msg->op.chsmTmprThreshold.low;
    high = msg->op.chsmTmprThreshold.high;
    crit = msg->op.chsmTmprThreshold.crit;
    ret = lcm_mgt_set_tmpr_threshold(low, high, crit);

    //ret += lcm_msg_tx_lcTmprThreshold(low, high, crit, 1);

    return ret;
}

int32 
lcm_msg_rx_chsmOemInfoSet(LcmMsg_t *msg)
{
    int32 ret = 0;
    uint8 buf[MAX_EEPROM_OEM_INFO_LEN+1];

    sal_memcpy(buf, msg->op.chsmOemInfoSet.oemInfo.buf, MAX_EEPROM_OEM_INFO_LEN);    
    ret = lcm_mgt_oem_info_set(buf);

    return ret;
}

int32 
lcm_msg_rx_chsmOemInfoGet(LcmMsg_t *msg)
{
    int32 ret = 0;

    ret = lcm_msg_tx_lcOemInfo(1);

    return ret;
}

int32 
lcm_msg_rx_chsmPortInfoSet(LcmMsg_t *msg)
{
    return lcm_mgt_set_port_info((char *)msg->op.chsmPortInfoSet.portInfo.buf);
}

int32 
lcm_msg_rx_chsmLedSet(LcmMsg_t *msg)
{
    glb_led_type_t type;
    glb_led_stat_t status;
    int32 ret;

    type = msg->op.chsmLedSet.reserved;
    status = msg->op.chsmLedSet.ledStatus;
    ret = lcm_mgt_led_set(type, status);

    return ret;
}

int32 
lcm_msg_rx_chsmTmprGet(LcmMsg_t *msg)
{
    int32 ret =0;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx get sensor temperature message.");

    ret = lcm_msg_tx_lcChsmTmprGetAck(0);   
    return ret;
}

int32 
lcm_msg_rx_chsmGetFanStatus(LcmMsg_t *msg)
{
    int32 ret =0;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx get fan status message.");

    ret = lcm_msg_tx_lcchsmGetFanStatusAck(0);   
    return ret;
}

int32 
lcm_msg_rx_chsmGetPsuStatus(LcmMsg_t *msg)
{
    int32 ret =0;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx get psu status message.");

    ret = lcm_msg_tx_lcchsmGetPsuStatusAck(0);   
    return ret;
}

int32 
lcm_msg_rx_chsmGetVcmCfg(LcmMsg_t *msg)
{
    int32 ret =0;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx get vcm config message.");

    ret = lcm_msg_tx_lcchsmGetVcmCfgAck(0);   
    return ret;
}

int32 
lcm_msg_rx_chsmGetVcmStatus(LcmMsg_t *msg)
{
    int32 ret =0;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx get vcm status message.");

    ret = lcm_msg_tx_lcchsmGetVcmStatusAck(0);   
    return ret;
}

int32 
lcm_msg_rx_chsmResetRebootInfo(LcmMsg_t *msg)
{
    return lcm_mgt_reset_reboot_info();
}

int32
lcm_msg_rx_chsmSetVctMod(LcmMsg_t *msg)
{
#ifndef _GLB_UML_SYSTEM_          
    int32 ret;
    int32 slot_id = 0xff;
    int32 port_id = 0xff;
    vct_msg_t* p_vct_msg;
    glb_card_t* p_card = NULL;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm rx set VCT mod message!\n");
    slot_id = msg->op.chsmSysSetVCT.slot;
    port_id = msg->op.chsmSysSetVCT.port;
    
    p_vct_msg = (vct_msg_t*)LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(vct_msg_t));
    if (NULL == p_vct_msg)
    {
        return LCM_E_NO_MEMORY;
    }
    p_vct_msg->port_id = port_id;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm set VCT mod slot = %d,port = %d\n", slot_id, port_id);
    
    ret = lcm_vct_start_timer(p_vct_msg);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm set VCT start timer fail!\n");
        LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_vct_msg);
        return LCM_E_CFG_GEN;
    }
    
    p_card = lcm_mgt_get_card();
    if ((port_id+1) > p_card->port_num)
    {
        LCM_LOG_ERR("Port index %d is out of range, port numbers is %d",
                    port_id, p_card->port_num);
        return LCM_E_INVALID_PORT;
    }
    /* fix bug: port can't work on shutdown mode, otherwise the port works abnormal */
    if(!p_card->pp_port[port_id]->port_cfg.enable)
    {
        LCM_LOG_ERR("slot = %d,port = %d VCT test, but port is shutdown!\n", slot_id, port_id);
        LCM_LOG_USER(E_WARNING,LCM_4_VCT_TEST_BUT_PORT_SHUTDOWN);
        return LCM_E_CFG_GEN;
    }

    ret = phy_set_vct_mod(port_id);
    if(ret < 0)
    {
        LCM_LOG_ERR("Lcm set VCT mod fail!\n");
        LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_vct_msg);
        return LCM_E_CFG_GEN;
    }
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Lcm set VCT mod end\n");
#endif 
    return LCM_E_SUCCESS;   
}

int32 
lcm_msg_rx_chsmConfigSpeedMode(LcmMsg_t *msg)
{
    int ret;
    //uint16 group_id;
    glb_speed_mode_t mode;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Config Speed Mode message.");

    mode = msg->op.chsmConfigSpeedMode.mode;
    //group_id = msg->op.chsmConfigSpeedMode.groupid;

    ret = lcm_port_speed_mode_config(mode);
    if(ret != RESULT_OK)
    {
        LCM_LOG_ERR("LCM ConfigSpeedMode: failed.\n");
        return ret;
    }
    return LCM_E_SUCCESS;
}

int32  
lcm_msg_rx_chsmUpdatebootrom(LcmMsg_t *msg)
{
    int ret;
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Updatebootrom message.");
    ret = lcm_mgt_update_bootrom((char *)msg->op.chsmUpdatebootrom.dstfile.buf);///TODO:size is not the same ?

    return ret;
}

int32  
lcm_msg_rx_chsmUpdateEpld(LcmMsg_t *msg)
{
    int ret;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Update EPLD message.");
    ret = lcm_mgt_update_epld((char *)msg->op.chsmUpdatebootrom.dstfile.buf);///TODO:size is not the same ?

    return ret;
}

int32
lcm_msg_rx_chsmLoadPhyFirmware(LcmMsg_t *msg)
{
    int ret;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Update EPLD message.");
    ret = lcm_mgt_update_phy_firmware((int32)msg->op.chsmLoadPhyFirmware.portid, (char *)msg->op.chsmLoadPhyFirmware.dstfile.buf);///TODO:size is not the same ?

    return ret;
}

int32
lcm_msg_rx_chsmConfigbootcmd(LcmMsg_t *msg)
{
    int ret;
    glb_card_t *p_card = NULL;
    char bootcmd[M_FULLPATH_MAX_LEN+1]; /*max len is 18+128+10+8+15+15=194 byte.*/
     
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv chsmConfigbootcmd : card is NULL.");
        return LCM_E_INVALID_PTR;
    }    
    sal_strncpy(bootcmd, (char *)msg->op.chsmConfigbootcmd.bootcmd.buf, msg->op.chsmConfigbootcmd.bootcmd.size);

    if(p_card->platform_type == 0)
    {
#if (defined _CTC_ARM_LS1023A_ || defined _CTC_ARM_CTC5236_)
        ret = lcm_mgt_set_configbootcmd_exec(bootcmd);
#else
        ret = lcm_mgt_set_configbootcmd_normal(bootcmd);
#endif
    }
    else
    {
        ret = lcm_mgt_set_configbootcmd_exec(bootcmd);
    }
     
    if(ret < 0)
    {
        //LCM_LOG_USER(E_ERROR,LCM_3_CONFIG_BOOTCMD_FAIL);
        LCM_LOG_ERR( "Config bootcmd fail.");
        return LCM_E_HW_UPDATEBTROM_GEN;
    }
    return LCM_E_SUCCESS;
}

int32 
lcm_msg_rx_chsmMgmtCfgSet(LcmMsg_t *msg)
{    
    int32 ret =0;
#ifndef _GLB_UML_SYSTEM_   
    uint8 buf[25];

    assert(24 == msg->op.chsmMgmtCfgSet.cfgs.size);
    sal_memcpy(buf, msg->op.chsmMgmtCfgSet.cfgs.buf, msg->op.chsmMgmtCfgSet.cfgs.size);
    buf[msg->op.chsmMgmtCfgSet.cfgs.size] = '\0';  
    
    ret = lcm_mgt_mgmt_cfg_set(buf);
#endif    
    return ret;
}

int32 
lcm_msg_rx_chsmFanSpeedRate(LcmMsg_t *msg)
{
    int32 ret =0;
#ifndef _GLB_UML_SYSTEM_ 
    uint16 bottomspeed,lowspeed, highspeed, fullspeed;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx set fan speed message.");
    
    bottomspeed = msg->op.chsmFanSpeedRate.bottomspeedrate;
    lowspeed = msg->op.chsmFanSpeedRate.lowspeedrate;
    highspeed = msg->op.chsmFanSpeedRate.highspeedrate;
    fullspeed = msg->op.chsmFanSpeedRate.fullspeedrate;

    lcm_mgt_config_fan_speed_rate(bottomspeed,lowspeed, highspeed, fullspeed);    
#endif
    return ret;
}

#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
int32
lcm_msg_rx_chsmSmartCfgSet(LcmMsg_t *msg)
{
    int32 ret =LCM_E_SUCCESS;
    ret = lcm_mgt_smartcfg_set(msg->op.chsmSmartCfgSet.flags);
    return ret;
}
#endif /* !HAVE_SMARTCFG */

int32 
lcm_msg_rx_chsmSetSwVer(LcmMsg_t *msg)
{
    int32 ret = LCM_E_SUCCESS;
    ret = lcm_mgt_set_swver(msg->op.chsmSetSwVer.swVer.buf);
    return ret;
}

#endif

