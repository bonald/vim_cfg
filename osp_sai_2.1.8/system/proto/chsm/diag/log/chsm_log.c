
/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "chsm_inc.h"
#include "chsm/diag/log/chsm_log.h"

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

/****************************************************************************
 *  
* Function
*
*****************************************************************************/

log_message_t chsm_log_message[] =
{
    { 0, "Unrecognized Error Code" },
    { CHSM_0_LC_ASIC_FATAL, "%% %s\n"}, 
    { CHSM_3_TCAM_SRAM_MATCH_FAIL, "%% TCAM type and SRAM type does not match, line card attach failed\n"},
    { CHSM_4_TMPR_ALARM, "System notice: temperature %d is %s\n"},  
    { CHSM_4_FAN_ABSENT, "System notice: Fan tray index %d is %s\n"},
    { CHSM_4_FAN_FAIL, "System notice: Fan index %d-%d is %s\n"},
    { CHSM_4_TRANSCEIVER_INSERT, "System notice: eth-%d-%d insert transceiver %s\n"},
    { CHSM_4_TRANSCEIVER_REMOVE, "System notice: eth-%d-%d remove transceiver\n"},
    { CHSM_4_LC_ATTACH, "Switch %d attach, stm mode=%d, tcam type=%d, sram type=%d, board series %x, board type %x\n"},
    { CHSM_4_LC_DEATTACH, "Switch %d deattach\n"},
    { CHSM_4_PSU_ABSENT, "System notice: Psu index %d is %s\n"},
    { CHSM_4_PSU_FAN_FAIL, "System notice: Psu index %d fan is %s\n"},
    { CHSM_4_PSU_WORK_STATUS, "System notice: Psu index %d work is %s\n"},
    { CHSM_4_PSU_ALERT, "System notice: Psu index %d %s\n"},
    { CHSM_4_FIBER_LOG, "Transceiver eth-%d-%d %s\n"},
    { CHSM_6_POE_ENABLE, "POE port %d is %s\n"},
};

