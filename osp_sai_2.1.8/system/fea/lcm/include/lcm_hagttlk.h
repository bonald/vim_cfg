/****************************************************************************
 * hagt2lcm_talk.h      lcm talk to hal agent, callback function
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :xgu
 * Date          :2010-08-19.
 * Reason        :First Create.
 ****************************************************************************/
#ifndef __LCM_HAGTTLK_H__
#define __LCM_HAGTTLK_H__

typedef int32 (*lcm2hagt_callback_t)(void* );

enum lcm2hagt_cb_msg_type_e
{
    LCM2HAGT_CB_INIT_HAGT,
    LCM2HAGT_CB_INIT_QUEUE,
    LCM2HAGT_CB_SET_MAC_EN,
    LCM2HAGT_CB_GET_MAC_LINK_STATUS,  /* Added by qicx to get mac link status: 2012-11-20 */
    LCM2HAGT_CB_SET_MAC_SPEED,
    LCM2HAGT_CB_SET_MAC_FLOWCNTRL,
    LCM2HAGT_CB_UPDATE_LINK_STATUS,
    LCM2HAGT_CB_SYNC_LINK_STATUS,
    LCM2HAGT_CB_SET_CPU_MAC_EN,

    LCM2HAGT_CB_SHOW_SDK_ALLOC_INFO,
    LCM2HAGT_CB_MESSAGE_REINIT,
     LCM2HAGT_CB_UPDATE_DPLL_STATE, /* modified by yaom for bug 18501 20110306 */
    LCM2HAGT_CB_SET_STACK_SWITCH_TX_TRUNK,
    LCM2HAGT_CB_UPDATE_STACK_PORT,
    /* modified by yaom for 10G remote fault detection 20120926 */
    LCM2HAGT_CB_GET_REMOTE_FAULT_STATUS,
    LCM2HAGT_CB_SET_FORCE_TX_EN,
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    LCM2HAGT_CB_SET_EEE_CAPABILITY,
    /* End of liuht modified */
    /* dying gasp intr, notify event to hagt */
    LCM2HAGT_CB_NOTIFY_DYING_GASP,
    /* special for 28G serdes param adjust*/
    LCM2HAGT_CB_SET_RX_PEAKING,
    /* 802.3AP train enable*/
    LCM2HAGT_CB_SET_3AP_TRAINING,
    /* 802.3AP force FFE*/
    LCM2HAGT_CB_SET_3AP_FFE,
    LCM2HAGT_CB_SET_SUM_FFE,
    LCM2HAGT_CB_SET_LOOPBACK,
    LCM2HAGT_CB_GET_POLARITY,
    LCM2HAGT_CB_SET_POLARITY,
    LCM2HAGT_CB_GET_SDK_PROFILE,
    LCM2HAGT_CB_TYPE_MAX
};
typedef enum lcm2hagt_cb_msg_type_e lcm2hagt_cb_msg_type_t;
extern lcm2hagt_callback_t lcm2hagt_cb[LCM2HAGT_CB_TYPE_MAX];

int32 lcm2hagt_set_callback(lcm2hagt_cb_msg_type_t msg_type, lcm2hagt_callback_t func);
#define LCM2HAGT_CALLBACK_SEND(msg_type, arg)  lcm2hagt_cb[msg_type](arg)

#endif /*__LCM_HAGTTLK_H__*/
