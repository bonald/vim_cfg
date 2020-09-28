/**************************************************************************
* lcm_log.c   :   lcm log message define
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* 2011 - 11 - 15
* Revision          R0.1
* Author            zhaow
*                       Email: zhaow@centecnetworks.com
* Date               2011 - 11 - 15
* Reason   :       First Create.
**************************************************************************/
#include "lcm_log.h"
    
log_message_t lcm_log_message[] =
{
    { 0, "Unrecognized Error Code" },
    { LCM_3_UNKNOWN_DAUGHTER_CARD, "%% Unknown daughter card type.\n"},
    { LCM_3_UPDATE_BOOTROM_FAIL, "%% Update bootrom fail.\n"},
    { LCM_3_UPDATE_BOOTROM_FREQUENTLY, "%% Please don't update bootrom frequently, try again after 30 seconds.\n"},
    { LCM_3_UPDATE_EPLD_FAIL, "%% Update epld fail.\n"},
    { LCM_3_CONFIG_BOOTCMD_FAIL, "%% Config bootcmd fail.\n"},
    { LCM_3_UNKNOWN_SLOT_CARD, "%% Unknown card number.\n"},
    { LCM_3_PORT_FEC_CONFLICT, "%% Port %s fec configure is invalid.\n"},
    { LCM_4_UNSUPPORT_BOARD_SERIES_TYPE, "%% Unsupport board series %d type %d.\n"},
    { LCM_4_VCT_TEST_BUT_PORT_SHUTDOWN, "%% VCT test but port is shutdown.\n"},
    { LCM_4_GET_REBOOT_INFO, "%% %s reboot infomation fail\n"}, 
    { LCM_4_SET_SW_VER_FAIL, "%% Set software version fail\n"},
};

