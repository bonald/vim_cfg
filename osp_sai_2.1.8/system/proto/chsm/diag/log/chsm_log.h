/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

#ifndef __CHSM_LOG_H__
#define __CHSM_LOG_H__
#include "sal.h"
#include "genlog.h"

extern log_message_t chsm_log_message[];

enum chsm_log_key_s
{
    CHSM_0_LC_ASIC_FATAL = 1, 
    CHSM_3_TCAM_SRAM_MATCH_FAIL,
    CHSM_4_TMPR_ALARM,
    CHSM_4_FAN_ABSENT,
    CHSM_4_FAN_FAIL,
    CHSM_4_TRANSCEIVER_INSERT,
    CHSM_4_TRANSCEIVER_REMOVE,
    CHSM_4_LC_ATTACH,
    CHSM_4_LC_DEATTACH,
    CHSM_4_PSU_ABSENT,
    CHSM_4_PSU_FAN_FAIL,
    CHSM_4_PSU_WORK_STATUS,
    CHSM_4_PSU_ALERT,  
    CHSM_4_FIBER_LOG,  
    CHSM_6_POE_ENABLE,
    CHSM_6_LOG_KEY_MAX,
};
typedef enum chsm_log_key_s chsm_log_key_t;

#define CHSM_LOG_USER(eSeverity, code, ...)\
do {\
    if(code < CHSM_6_LOG_KEY_MAX)\
    {\
         log_sys(M_MOD_CHSM, eSeverity, chsm_log_message[code].str, ##__VA_ARGS__); \
    } \
}while(0)

#endif /* __CHSM_LOG_H__ */
