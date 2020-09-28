/****************************************************************************
 * logging.h: 
 *   1) contorl syslog-ng to send log messages
 *   2) implement log buffer
 *   3) send log to vty
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : Alexander Liu
 * Date         : 2006-12-07
 * Reason       : First Create.
 ****************************************************************************/
#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <bsd_queue.h>

/* logging configuration file */
#define LOGGING_CFGFILE_DEFAULT     CTC_ETC_BASE_DIR"/etc/syslog-ng.conf"
#define LOGGING_SYSLOGNG_PIDFILE    "/var/run/syslog-ng.pid"
#define LOGGING_LOGFILE_PATH        "/var/log"
#define LOGGING_LOGFILE_DEFAULT     LOGGING_LOGFILE_PATH"/messages"

/* receive log from syslog-ng */
#define LOGGING_LOG_PATH            "/tmp/.logbuf_line"
#define LOGGING_LOG_TRAP_PATH       "/tmp/.logtrap" /* Added by zhoucheng 2007-11-27 13:34 */
/* added by chenzhonghua for bug 16508, 2011-09-26*/
#define LOGGING_LOG_DIAG_PATH       LOGGING_LOGFILE_PATH"/diag_messages"
/* Added by chenzhonghua for bug 14173*/
#define LOGGING_LOG_REDIRECT_PATH   "/mnt/flash/cold/log/startup_info.log"

/* server numbers */
#define LOGGING_SERVER_MAX  3

/* log buffer line number */
#define LOGGING_BUF_LINENUM_DEFAULT     500
#define LOGGING_BUF_LINENUM_MAX         1000
#define LOGGING_BUF_LINENUM_MIN         10

/* show log buffer line number */
#define LOGGING_BUF_SHOWNUM_DEFAULT     20
#define LOGGING_BUF_SHOWNUM_MAX         LOGGING_BUF_LINENUM_MAX
#define LOGGING_BUF_SHOWNUM_MIN         (-(LOGGING_BUF_LINENUM_MAX))

/* output facility & severity */
#define LOGGING_SRV_SEVERITY_DEFAULT    LOG_WARNING
#define LOGGING_MOD_SEVERITY_DEFAULT    LOG_DEBUG
#define LOGGING_FILE_SEVERITY_DEFAULT   LOG_INFO
#define LOGGING_TRAP_SEVERITY_DEFAULT   LOG_ALERT /* Added by zhoucheng 2007-11-29 15:37 */

/* file sync interval (unit is minute) */
#define LOGGING_FILESYNC_DEFAULT        60
#define LOGGING_FILESYNC_MAX            1440
#define LOGGING_FILESYNC_MIN            1

/* maximal message size */
#define LOGGING_MAX_MSG_SIZE            1024
#define LOGGING_MAX_MSG_LEN             1024

/* Added by zhoucheng 2007-12-10 21:35 */
#define LOGGING_MERGE_DEFAULT_STATE         1
#define LOGGING_MERGE_DEFAULT_FIFO_SIZE     1024
#define LOGGING_MERGE_MAX_FIFO_SIZE         10240
#define LOGGING_MERGE_MIN_FIFO_SIZE         100
#define LOGGING_MERGE_DEFAULT_TIMEOUT       10000
#define LOGGING_MERGE_MAX_TIMEOUT           300000
#define LOGGING_MERGE_MIN_TIMEOUT           1000

/* Added by zhoucheng for bug 3974 */
#define LOGGING_MERGE_DEFAULT_TIMEFROM      1

/* action type */
typedef enum eActionType
{
    E_ACTION_RUNNING = 0,
    E_ACTION_SHUTDOWN,
    E_ACTION_DIAG,
    E_ACTION_DIAG_SHUTDOWN
} E_ActionType;

/* logging buffer statistics */
typedef struct tagLogStat
{
    unsigned long ulCurLogCnts; /* current log counts */
    unsigned long long ullTotalLogCnts; /* total log counts */
    unsigned long long ullDropLogCnts; /* droped log counts */
    unsigned long long ullMergeLogCnts; /* merged log counts */
} S_LogStatistics;

/* log item structure */
typedef struct tagLogItem
{
    TAILQ_ENTRY(tagLogItem) entry;
    
    char *pszLog; /* log string */
    size_t nLen; /* length of the buffer */
} S_LogItem;

/* log buffer head */
typedef TAILQ_HEAD(tagLogQueue, tagLogItem) S_LogBuffer;

/* terminal monitor item structure */
typedef struct tagTerminalMonitor
{
    TAILQ_ENTRY(tagTerminalMonitor) entry;
    
    char *pszTty;
} S_TerminalMontior;

/* log buffer head */
typedef TAILQ_HEAD(tagTerminalMonitorQueue, tagTerminalMonitor) S_TerminalMonitorQueue;

int logging_terminal_monitor(int onoff, char *pszTty);

const char *logging_get_level_name(unsigned long nSevirity);
const char *logging_get_facility_name(unsigned long nFacility);
S_LogStatistics* logging_get_stat();

typedef int logs_iter_fn_t(void *pArg, const char *pszMsg);
void logging_show_records(int nLines, logs_iter_fn_t pCB, void *pArg);
int logging_clear_buf();
int logging_set_bufsz(int lines);

int logging_server_add_server_address(tbl_syslog_cfg_t * pCfg, 
    int vrf, char *ipaddr, char *pszErrMsg, size_t nSize);
int logging_server_del_server_address(tbl_syslog_cfg_t * pCfg, 
    int vrf, char *ipaddr, char *pszErrMsg, size_t nSize);

#endif /* !_LOGGING_H_ */
