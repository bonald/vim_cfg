/****************************************************************************
 * lc.c   :      linecard main
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Zhuj
 * Date         :       2010-08-11
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "lcapi.h"
#include "lcm_port.h"
#include "lcm_hagttlk.h"
#include "hagt_lcmtlk.h"
#include "lcm_mgt.h"
#include "lcm_specific.h"
#include "lcm_error.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
lcm2hagt_callback_t lcm2hagt_cb[LCM2HAGT_CB_TYPE_MAX] = {0};

struct lcapi_master_s
{
    void *p_thread_master;
};
typedef struct lcapi_master_s lcapi_master_t;

static lcapi_master_t *g_p_lc_master = NULL; 


/****************************************************************************
 *  
* Function
*
*****************************************************************************/
int32 
lcm2hagt_set_callback(lcm2hagt_cb_msg_type_t msg_type, lcm2hagt_callback_t func)
{
    if(msg_type >= LCM2HAGT_CB_TYPE_MAX)
        return LCM_E_INVALID_PARAM;

    lcm2hagt_cb[msg_type] = func;

    return LCM_E_SUCCESS;
}

#ifndef CNOS_OS
static int32
_lcapi_install_hagt_cb()
{    
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_SPEED, lcm_port_set_speed);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_DUPLEX, lcm_port_set_duplex);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_MEDIA, lcm_port_set_media);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_FLOWCTRL, lcm_port_set_flowctrl);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_PHY_ENABLE, lcm_port_set_enable);
    /* Modified by liuht for bug 28155, 2014-04-17 */
    /* Support force master or slave mode */
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_MASTER_SLAVE_MODE, lcm_port_set_master_slave_mode);
    /* End of liuht modified */
    hagt2lcm_set_callback(HAGT2LCM_CB_GET_PORT_NUM, lcm_port_get_port_num);
    hagt2lcm_set_callback(HAGT2LCM_CB_GET_LINK_STATUS, lcm_port_get_port_status);
    hagt2lcm_set_callback(HAGT2LCM_CB_GET_LINK_UP_STATUS, lcm_port_get_port_link_up_status);
    //hagt2lcm_set_callback(HAGT2LCM_CB_EN_NORMAL_INTR, lcm_mgt_enable_asic_normal_intr);
    //hagt2lcm_set_callback(HAGT2LCM_CB_EN_FATAL_INTR, lcm_mgt_enable_asic_fatal_intr);
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
    //hagt2lcm_set_callback(HAGT2LCM_CB_EN_FOAM_INTR, lcm_mgt_enable_foam_normal_intr);
#endif /*FOAM_SUPPORT*/
    /*modified by xgu for bug 13493, 2011-01-11*/
    hagt2lcm_set_callback(HAGT2LCM_CB_PORT_CREATE_DONE, lcm_port_set_create_done);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_LPBK, lcm_port_set_lpbk);
    hagt2lcm_set_callback(HAGT2LCM_CB_BYPASS_TIMERS, lcm_mgt_bypass_timers);
    hagt2lcm_set_callback(HAGT2LCM_CB_ASIC_FATAL, lcm_mgt_asic_fatal);
    //hagt2lcm_set_callback(HAGT2LCM_CB_SET_POE_ENABLE, lcm_port_set_poe_enable);
    //hagt2lcm_set_callback(HAGT2LCM_CB_SET_POE_BUDGET, lcm_port_set_poe_budget);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_SYNCE, lcm_port_set_syncE_enable);
    //hagt2lcm_set_callback(HAGT2LCM_CB_SET_POE_PRIORITY, lcm_port_set_poe_priority);
    /*modified by wangl for TOD code bug 24378, 2013-8-13*/
#if 0
#ifdef GB_DEMO_BOARD
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_TOD_MODE, lcm_port_tod_set_mode);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_GB_DEMO_10G_DPLL_MODE, lcm_10g_port_dpll_ic_set_mode);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_GB_DEMO_CLOCK_RECOVERY_SELECT, lcm_clock_recovery_select);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_TOD_OUT_PUT_MODE, lcm_port_tod_set_out_put_mode);
#endif
#ifdef PTN_722_BOARD
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_TOD_MODE_PTN722, lcm_port_tod_set_mode_ptn722);
    hagt2lcm_set_callback(HAGT2LCM_CB_PTN722_EPLD_SELECT_SLOT_CLOCK, lcm_port_ptn722_select_slot_clock);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_PTN722_CLOCK_RECOVERY_SELECT, lcm_clock_recovery_select);
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_TOD_OUT_PUT_MODE, lcm_port_tod_set_out_put_mode);
#endif
#endif
    hagt2lcm_set_callback(HAGT2LCM_CB_GET_ACTIVE_SUP_SLOT, lcm_mgt_get_active_sup_slot);
    hagt2lcm_set_callback(HAGT2LCM_CB_GET_SYSMAC, lcm_mgt_get_sysmac);
    hagt2lcm_set_callback(HAGT2LCM_CB_RELOAD, lcm_mgt_reload);

    /* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
    //hagt2lcm_set_callback(HAGT2LCM_CB_GET_LOGIC_PORT_ARRAY, lcm_get_logic_port_array);
    hagt2lcm_set_callback(HAGT2LCM_CB_POLL_ALL_PORTS, lcm_port_poll_all_ports);  /* added by yaom */    
    //hagt2lcm_set_callback(HAGT2LCM_CB_POLL_PORT_STATUS, lcm_port_poll_port_status);/* move to lcm */
    /* end of qicx added */
    /* support eee function, modified by liuht for bug 28298, 2014-04-21 */
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_EEE_CAPABILITY, lcm_port_set_eee_capability);
    /* end of liuht modified */ 
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_UNIDIR, lcm_port_set_unidir);
    /*added by liuyang 2016-5-4*/
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_FEC_ENABLE, lcm_port_set_fec_en);
    /*added by tongzb 2018-11-19*/
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_FFE, lcm_port_set_ffe);       
    /* added by tongzb for bug 43937 support tunable fiber *////TODO:need add 
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_WAVELENGTH, lcm_port_set_wavelength); 
    hagt2lcm_set_callback(HAGT2LCM_CB_SET_CARRIER_DOWN_HOLDTIME, lcm_port_set_link_tolerable_time); 
    return 0;
}

static int32
_lcapi_install_lcm_cb()
{     
    lcm2hagt_set_callback(LCM2HAGT_CB_INIT_HAGT, hagt_modules_init);
    //lcm2hagt_set_callback(LCM2HAGT_CB_SET_MAC_EN, hagt_lcm_set_mac_en);//do it juss call sdk api in lcm
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_MAC_SPEED, hagt_lcm_set_mac_speed);
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_MAC_FLOWCNTRL, hagt_lcm_set_mac_flowcntrl);
    lcm2hagt_set_callback(LCM2HAGT_CB_UPDATE_LINK_STATUS, hagt_if_tx_link_status); //TODO, with linkstatus up/down
    lcm2hagt_set_callback(LCM2HAGT_CB_SYNC_LINK_STATUS, hagt_if_sync_link_status); //synce link status between hsrvd  and kernel
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_CPU_MAC_EN, hagt_lcm_set_cpu_mac_en);

    /* modified by yaom for bug 18501 20110306 */
    lcm2hagt_set_callback(LCM2HAGT_CB_UPDATE_DPLL_STATE, hagt_lcm_update_link_state);
    lcm2hagt_set_callback(LCM2HAGT_CB_MESSAGE_REINIT, hagt_message_reinit);
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_STACK_SWITCH_TX_TRUNK, hagt_stack_set_trunk_rchip);
    lcm2hagt_set_callback(LCM2HAGT_CB_UPDATE_STACK_PORT, hagt_stack_update_stack_port);
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    //lcm2hagt_set_callback(LCM2HAGT_CB_SET_EEE_CAPABILITY, hagt_lcm_set_mac_eee_en);
    /* End of liuht modified */
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_FORCE_TX_EN, hagt_lcm_set_force_tx_en);
    /* modified by yaom for 10G remote fault detection 20120926 */
    lcm2hagt_set_callback(LCM2HAGT_CB_GET_REMOTE_FAULT_STATUS, hagt_lcm_get_remote_fault_status);
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_RX_PEAKING, hagt_lcm_set_rx_peaking);
    //lcm2hagt_set_callback(LCM2HAGT_CB_SET_3AP_FFE, hagt_lcm_set_3ap_ffe);
    //lcm2hagt_set_callback(LCM2HAGT_CB_SET_SUM_FFE, hagt_lcm_set_sum_ffe);
    lcm2hagt_set_callback(LCM2HAGT_CB_SET_LOOPBACK, hagt_lcm_set_loopback);
    //lcm2hagt_set_callback(LCM2HAGT_CB_GET_POLARITY, hagt_lcm_get_polarity);
    //lcm2hagt_set_callback(LCM2HAGT_CB_SET_POLARITY, hagt_lcm_set_polarity);

    lcm2hagt_set_callback(LCM2HAGT_CB_GET_SDK_PROFILE, hagt_lcm_get_sdk_profile);
    return 0;
}

ctclib_thread_master_t*
lc_get_thread_master(void)
{
    return g_p_lc_master->p_thread_master;
}

#endif


int32
lc_start(void *p_thread_master)
{
    lcapi_master_t *p_lc_master;
    p_lc_master = LCAPI_MALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(lcapi_master_t));
    if(NULL == p_lc_master)
    {
        return -1;
    }
    g_p_lc_master = p_lc_master;

#ifndef CNOS_OS
    p_lc_master->p_thread_master = p_thread_master;
    /*1. Set callback function in hagt&lcm*/
    _lcapi_install_hagt_cb();
    _lcapi_install_lcm_cb();

    /*2. hagt module start*/
    /* only on IPIOS */
    //hagt_modules_start((ctclib_thread_master_t *)p_thread_master);
#endif    

    /*3. lcm module start*/
    //lcm_mgt_init();

    return 0;
}

