/**************************************************************************
* lcm_log.h   :   lcm header for logging
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
#ifndef __LCM_LOG_H__
#define __LCM_LOG_H__
#include "sal_common.h"
#include "genlog.h"
    
extern log_message_t lcm_log_message[];

enum lcm_log_key_s
{
    LCM_3_UNKNOWN_DAUGHTER_CARD = 1,
    LCM_3_UPDATE_BOOTROM_FAIL,
    LCM_3_UPDATE_BOOTROM_FREQUENTLY,
    LCM_3_UPDATE_EPLD_FAIL,
    LCM_3_CONFIG_BOOTCMD_FAIL = 5,
    LCM_3_UNKNOWN_SLOT_CARD,  /* Added by qicx for GB Demo board, 2012-11-19 */
    LCM_3_PORT_FEC_CONFLICT, 
    LCM_4_UNSUPPORT_BOARD_SERIES_TYPE,
    LCM_4_VCT_TEST_BUT_PORT_SHUTDOWN,
    LCM_4_GET_REBOOT_INFO,  /* Added by liuht for bug27036, 2014-02-21 */ 
    LCM_4_SET_SW_VER_FAIL, /*fix bug39335, support to define sw_ver, liangf, 2016-07-06*/
    LCM_6_LOG_KEY_MAX,
};
typedef enum lcm_log_key_s lcm_log_key_t;

#define LCM_LOG_USER(eSeverity, code, ...)\
do {\
    if(code < LCM_6_LOG_KEY_MAX)\
    {\
         log_sys(M_MOD_LCM, eSeverity, lcm_log_message[code].str, ##__VA_ARGS__); \
    } \
}while(0)

#define LCM_LOG_CONSOLE(fmt, args...)                        \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "a+");                  \
    sal_fprintf(fp_console, fmt, ##args);                      \
    fclose(fp_console);                                        \
}

#define LCM_LOG_PTS(fmt, args...)                        \
do{                                                        \
    FILE * fp_pts = NULL;                                  \
    int pts_cnt = 0;                                        \
    char pts_name[32];                                      \
    struct stat pts_stat_buf;                               \
    for(pts_cnt=0;pts_cnt<10;pts_cnt++)                     \
    {                                                       \
        sal_sprintf(pts_name, "/dev/pts/%d", pts_cnt);      \
        pts_name[10] = '\0';                                \
        if (stat(pts_name, &pts_stat_buf) == 0)             \
        {                                                   \
            fp_pts = fopen(pts_name, "a+");                 \
            sal_fprintf(fp_pts, fmt, ##args);               \
            fclose(fp_pts);                                 \
        }                                                   \
    }                                                       \
}while(0)

#endif /* __LCM_LOG_H__ */

