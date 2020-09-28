/****************************************************************************
 * $Id$
 *  Define unified log interface for l og api
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : zhoucheng
 * Date          : 2007-11-1 10:18
 * Reason        : First Create.
 *****************************************************************************/
 #include <sys/types.h>

#ifndef  _GENLOG_USR_H_
#define  _GENLOG_USR_H_

#define MAX_IP_LEN 256
#define MAX_USR_NAME_LEN 32
#define MAX_LOG_TIME_LEN 64

/* Used by reentrant functions */
struct genlog_data 
{
    int         log_file;
    int         connected;
    int         opened;
    int         log_stat;
    int         is_percent;
    const char *log_tag;
    int         log_fac;
    int         log_mask;
    /*modified by weij for bug 18671, 2012-03-20*/
    char        log_time_str[MAX_LOG_TIME_LEN];
    int         rate_limit_type;
};

/* Log item format result type */
typedef enum eFmtRtn
{
    FMT_FAIL    = -1,
    FMT_SUCCESS = 0,
} E_FMT_RTN;

/* Formatting function pointer type */
typedef E_FMT_RTN(*FMT_FUNC)(char **, size_t *);

/* log callback function */
typedef int (*LOG_CB_FUNC)(int log_id, int severity, char *log);

/* extra log info type */
typedef struct extra_log_info
{
    char        ip_addr[MAX_IP_LEN+1];          /* login ip address */
    char        usr_name[MAX_USR_NAME_LEN+1];   /* login username */
    int         cmd_level;                      /* the operating command level */
    int         oprt_rtn;                       /* operating result */
    FMT_FUNC    fmt_func;                       /* operating message formating function */
    LOG_CB_FUNC cb_func;
} extra_log_info;
    
#define SYSLOG_DATA_INIT        {-1, 0, 0, 0, 0, (const char *)0, LOG_USER, 0xFF, {0}, 0}
#define SYSLOG_OSP_DATA_INIT    {-1, 0, 0, 0, 1, (const char *)0, LOG_USER, 0xFF, {0}, 0}
#define EXTRA_LOG_INFO_INIT {"", "", -1, -1, NULL, NULL}
#define _PATH_LOG           "/dev/log"

#endif /* ! _GENLOG_USR_H_ */

