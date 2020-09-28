/****************************************************************************
 * hagt2lcm_talk.h      Hal agent talk to lcm, callback function
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :xgu
 * Date          :2010-08-19.
 * Reason        :First Create.
 ****************************************************************************/
#ifndef __HAGT2LCM_TALK_H__
#define __HAGT2LCM_TALK_H__
#include "sal.h"

typedef int32 (*hagt2lcm_callback_t)(void* );

enum hagt2lcm_cb_msg_type_e
{
    HAGT2LCM_CB_SET_SPEED,
    HAGT2LCM_CB_SET_DUPLEX,
    HAGT2LCM_CB_SET_UNIDIR,
    HAGT2LCM_CB_SET_FEC_ENABLE,
    HAGT2LCM_CB_SET_MEDIA,
    HAGT2LCM_CB_SET_FLOWCTRL, 
    HAGT2LCM_CB_SET_PHY_ENABLE,
    HAGT2LCM_CB_SET_MASTER_SLAVE_MODE,
    HAGT2LCM_CB_GET_PORT_NUM,
    HAGT2LCM_CB_GET_LINK_STATUS,
    //HAGT2LCM_CB_EN_NORMAL_INTR,
    //HAGT2LCM_CB_EN_FATAL_INTR,
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
    //HAGT2LCM_CB_EN_FOAM_INTR,
#endif /*FOAM_SUPPORT*/
    HAGT2LCM_CB_PORT_CREATE_DONE,    /*modified by xgu for bug 13493, 2011-01-11*/
    HAGT2LCM_CB_SET_LPBK, 
    HAGT2LCM_CB_BYPASS_TIMERS,
    HAGT2LCM_CB_ASIC_FATAL,
    //HAGT2LCM_CB_SET_POE_ENABLE,
    //HAGT2LCM_CB_SET_POE_BUDGET,
    //HAGT2LCM_CB_SET_POE_PRIORITY,
    
    HAGT2LCM_CB_SET_SYNCE,
    HAGT2LCM_CB_GET_ACTIVE_SUP_SLOT,
#ifdef DPLL_SUPPORT
    HAGT2LCM_CB_SET_FORCE_IF,
#endif

#ifdef GB_DEMO_BOARD
    //HAGT2LCM_CB_SET_TOD_MODE,  /*modified by wangl for TOD code bug 24378, 2013-8-13*/
    //HAGT2LCM_CB_SET_GB_DEMO_10G_DPLL_MODE,
    //HAGT2LCM_CB_SET_GB_DEMO_CLOCK_RECOVERY_SELECT,
#endif
#ifdef PTN_722_BOARD
    //HAGT2LCM_CB_SET_TOD_MODE_PTN722,
    //HAGT2LCM_CB_PTN722_EPLD_SELECT_SLOT_CLOCK,
    //HAGT2LCM_CB_SET_PTN722_CLOCK_RECOVERY_SELECT,
#endif
    HAGT2LCM_CB_GET_SYSMAC,
    HAGT2LCM_CB_RELOAD,
    
    /* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
    //HAGT2LCM_CB_GET_LOGIC_PORT_ARRAY,
    HAGT2LCM_CB_POLL_ALL_PORTS, /* added by yaom */
    /* end of qicx added */
    HAGT2LCM_CB_POLL_PORT_STATUS,
    /* support eee function, modified by liuht for bug 28298, 2014-04-21 */
    HAGT2LCM_CB_SET_EEE_CAPABILITY,
    //HAGT2LCM_CB_SET_TOD_OUT_PUT_MODE,
    /* end of liuht modified */
    HAGT2LCM_CB_TYPE_MAX
};
typedef enum hagt2lcm_cb_msg_type_e hagt2lcm_cb_msg_type_t;
extern hagt2lcm_callback_t hagt2lcm_cb[HAGT2LCM_CB_TYPE_MAX];

#define HAGT2LCM_CALLBACK_SEND(msg_type, arg)  hagt2lcm_cb[msg_type](arg)
extern int32 hagt2lcm_set_callback(hagt2lcm_cb_msg_type_t msg_type, hagt2lcm_callback_t func);
extern int32 hagt_modules_init(void* arg);
extern int32 hagt_lcm_set_mac_en(void* arg);
extern int32 hagt_lcm_set_cpu_mac_en(void* p_arg);
extern int32 hagt_lcm_set_mac_speed(void* arg);
extern int32 hagt_lcm_set_mac_flowcntrl(void* p_arg);
extern int32 hagt_if_tx_link_status (void* arg);

extern int32 hagt_show_stm_alloc_info(void* arg);

extern int hagt_message_reinit(void* arg);
extern int32 hagt_stack_set_trunk_rchip(void *arg);
extern int32 hagt_stack_update_stack_port(void *arg);
/* modified by yaom for bug 18501 20110306 */   
extern int32 hagt_lcm_update_link_state (void* arg);

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
extern int32 hagt_lcm_set_mac_eee_en(void* p_arg);
/* End of liuht modified */
/* modified by yaom for 10G remote fault detection 20120926 */
extern int32 hagt_lcm_get_remote_fault_status (void* p_arg);
extern int32 hagt_lcm_set_force_tx_en(void* p_arg);
extern int32 hagt_lcm_set_rx_peaking(void* p_arg);
extern int32 hagt_lcm_set_3ap_ffe(void* p_arg);
extern int32 hagt_lcm_set_sum_ffe(void* p_arg);
extern int32 hagt_lcm_set_loopback(void* p_arg);
extern int32 hagt_lcm_set_polarity(void* p_arg);
extern int32 hagt_lcm_get_polarity(void* p_arg);
#endif /*__HAGT2LCM_TALK_H__*/
