/**************************************************************************
* drv_log.c   :   drv log message define
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
#include "drv_log.h"

log_message_t drv_log_message[] =
{
    { 0, "Unrecognized Error Code" },
    { DRV_3_FAN_TYPE_UNKNOWN, "Unsupport fan chip type %d\n"},
    { DRV_3_PSU_TYPE_UNKNOWN, "Unsupport power chip type %d\n"}, 
    { DRV_3_VSC3308_USED_TYPE_UNKNOWN, "Unsupport vsc3308 used chip type %d\n"}, 
};

