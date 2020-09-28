/**************************************************************************
* drv_log.h   :   drv header for logging
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
#ifndef __DRV_LOG_H__
#define __DRV_LOG_H__
#include "sal_common.h"
#include "genlog.h"

extern log_message_t drv_log_message[];

enum drv_log_key_s
{
    DRV_3_FAN_TYPE_UNKNOWN=1,
    DRV_3_PSU_TYPE_UNKNOWN,
    DRV_3_VSC3308_USED_TYPE_UNKNOWN,
    DRV_6_LOG_KEY_MAX,
};
typedef enum drv_log_key_s drv_log_key_t;

#define DRV_LOG_USER(eSeverity, code, ...)\
do {\
    if(code < DRV_6_LOG_KEY_MAX)\
    {\
         log_sys(M_MOD_DRV, eSeverity, drv_log_message[code].str, ##__VA_ARGS__); \
    } \
}while(0)

#endif /* __DRV_LOG_H__ */

