/****************************************************************************
* $Id$
*  line card port information
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2010-08-16
* Reason        : First Create.
****************************************************************************/
#ifndef __LCM_PORT_H__
#define __LCM_PORT_H__

#include "sal_common.h"
#include "lcm_specific.h"
#include "glb_hw_define.h"

typedef struct
{
    char        ifname[33];
    uint32      ifindex;
    uint32      speed;
    uint32      duplex;
    uint32      link_up;
    uint32      flowctrl_send;
    uint32      flowctrl_recv;
    uint32      media;
} lcm_if_phy_state_notify_t;

extern int32
lcm_port_set_speed(void* pv_arg);
extern int32
lcm_port_set_fec_en (void* pv_arg);
extern int32
lcm_port_set_poe (void* pv_arg);
extern int32
lcm_port_set_ffe(void* pv_arg);
extern int32
lcm_port_set_master_slave_mode(void* pv_arg);
extern int32
lcm_port_set_duplex (void* pv_arg);
extern int32
lcm_port_set_media(void* pv_arg);
extern int32
lcm_port_set_link_tolerable_time(void* pv_arg);
extern int32
lcm_port_set_flowctrl(void* pv_arg);
/* unsupport poe , remove poe driver and interface */
#if 0
extern int32
lcm_port_set_poe_enable(void* pv_arg);
extern int32
lcm_port_set_poe_budget(void* pv_arg);
extern int32
lcm_port_set_poe_priority(void* pv_arg);
#endif
extern int32
lcm_port_set_enable(void* pv_arg);
extern int32
lcm_port_get_port_num (void* pv_arg);
extern int32
lcm_port_get_port_status (void* pv_arg);
extern int32
lcm_port_get_port_link_up_status (void* pv_arg);

/* modified by yaom for 10G remote fault detection 20120926 */
int32 lcm_fiber_port_status_timer();

int32 lcm_port_status_intr (void);
void lcm_port_copper_sfp_init(void* p_data);
//int32 lcm_poe_status_intr (void);
extern int32
lcm_port_set_create_done(void* pv_arg);
extern int32
lcm_port_set_lpbk(void* pv_arg);
extern int32
lcm_port_set_syncE_enable(void * pv_arg);

/* added by qicx for bug26201 cr8236 for GB fiber port intr, 2013-12-18 */
extern int32
lcm_get_logic_port_array(void* pv_arg);
extern int32
lcm_port_poll_all_ports(void* pv_arg);
extern int32
lcm_port_poll_port_status(void* pv_arg);
/* support eee function, modified by liuht for bug 28298, 2014-04-21 */
extern int32
lcm_port_set_eee_capability(void* pv_arg);
extern int32
lcm_port_set_unidir (void* pv_arg);
extern int32
lcm_port_set_wavelength(void* pv_arg);
extern int32
lcm_port_set_bypass(void* pv_arg);
#if 0
#ifdef GB_DEMO_BOARD
extern int32
lcm_port_tod_set_mode(void *pv_arg);
extern int32
lcm_10g_port_dpll_ic_set_mode(void* pv_arg);
extern int32
lcm_clock_recovery_select(void* pv_arg);
#endif
#ifdef PTN_722_BOARD
extern int32
lcm_port_tod_set_mode_ptn722(void* pv_arg);
extern int32
lcm_port_ptn722_select_slot_clock(void* pv_arg);
extern int32
lcm_clock_recovery_select(void* pv_arg);
#endif
#endif
#ifdef _CTC_DUBLIN_
extern int32
lcm_port_flf_intr(void); /* For DUBLIN board */
#endif

void lcm_port_led_cfg(void* p_data);
void lcm_port_port_status_timer(void* p_data);
void lcm_port_cl73_auto_timer(void* p_data);

int32 lcm_port_speed_mode_config(glb_speed_mode_t mode);

LCM_DECLARE_TASK_ENCAP(lcm_port_led_cfg)
LCM_DECLARE_TASK_ENCAP(lcm_port_port_status_timer)
LCM_DECLARE_TASK_ENCAP(lcm_port_cl73_auto_timer)

#endif /*__LCM_PORT_H__*/
