/****************************************************************************
 * logging.c: 
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

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include <syslog.h>
#include <assert.h>
#include <arpa/inet.h>

#include "proto.h"
#include "cdb_const.h"
#include "cdb_ds.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "logging.h"
#include "param_check.h"
#include "lib_fs.h"
#include "ctc_sock.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/
extern int app_is_startup_done();
int redirect_startup_log(void);

#ifdef _GLB_UML_SYSTEM_
#define M_SYNC_SCRIPT       "/centec_switch/sbin/logsync.sh"
#define M_DIAGSYNC_SCRIPT   "/centec_switch/sbin/diaglogsync.sh"
#else
#define M_SYNC_SCRIPT       SWITCH_PREFIX"/usr/sbin/logsync.sh"
#define M_DIAGSYNC_SCRIPT   SWITCH_PREFIX"/usr/sbin/diaglogsync.sh"
#endif

#define LOGGING_LOGCNT_TOTAL_VAR g_loggingStat.ullTotalLogCnts
#define LOGGING_LOGCNT_DROP_VAR  g_loggingStat.ullDropLogCnts

#define LOGGING_LOGCNT_VAR       g_loggingStat.ulCurLogCnts
#define LOGGING_LOGCNT_INC       g_loggingStat.ulCurLogCnts++
#define LOGGING_LOGCNT_DEC       g_loggingStat.ulCurLogCnts--
#define LOGGING_LOGCNT_ZERO      g_loggingStat.ulCurLogCnts = 0l

/*add by chenzhonghua for bug 14173£¬ 2011-05-04*/
#define LOGGING_SYSTEM_START_UP_FALSE       (0)
#define LOGGING_SYSTEM_START_UP_TRUE        (1)

/* logging buffer line size */
#define LOGGING_LINEBUF_SIZE        8192
#define MGMT_VRF                    64
#define COMM_VRF                    65

/* default timestamp format */
#define LOGGING_DEFAULT_TS_FORMAT       "rfc3164"

/* filter name for severity */
#define LOGGING_SFNAME_FILE                 "f_file_severity"
#define LOGGING_SFNAME_MOD                  "f_mod_severity"
#define LOGGING_SFNAME_SRV                  "f_srv_severity"
#define LOGGING_SFNAME_TRAP                 "f_trap_filter"
#define LOGGING_SFNAME_OPER                 "f_oper_facility"
#define LOGGING_SFNAME_DIAG_FACILITY        "f_diag_facility"
#define LOGGING_SFNAME_DIAG_NOT_FACILITY    "f_diag_not_facility"

#define LOGGING_OPERATE_DEFAULT_STATE       0
#define SYSLOG_NG_PATH_PIDFILE              "/var/run/syslog-ng.pid"

/* syslog-ng configuration file strings */
#define LOGGING_CFGFILE_LOG_STRING  \
"\n# Log to %s\n" \
"log\n" \
"{\n" \
"    source(%s);\n" \
"    filter(%s);\n" \
"    destination(%s);\n" \
"};\n"

#define LOGGING_CFGFILE_LOG_STRING_2_FILTER  \
"\n# Log to %s\n" \
"log\n" \
"{\n" \
"    source(%s);\n" \
"    filter(%s);\n" \
"    filter(%s);\n" \
"    destination(%s);\n" \
"};\n"

#define LOGGING_CFGFILE_LOG_STRING_3_FILTER  \
"\n# Log to %s\n" \
"log\n" \
"{\n" \
"    source(%s);\n" \
"    filter(%s);\n" \
"    filter(%s);\n" \
"    filter(%s);\n" \
"    destination(%s);\n" \
"};\n"

/* timestamp type string for syslog-ng */
#define LOGGING_TS_FORMAT_NONE      ""
#define LOGGING_TS_FORMAT_RISO      "$R_ISODATE "
#define LOGGING_TS_FORMAT_RFULL     "$R_FULLDATE "
#define LOGGING_TS_FORMAT_RBSD      "$R_DATE "

/* template string for log output */
#define LOGGING_TEMPLATE_STRING \
"template(\"%s$HOST $PROGRAM-$LEVEL_NUM$CTCBRIEF: $MSGONLY\\n\")"

/* Added by zhoucheng */
#define LOGGING_TRAP_TEMPLATE_STRING \
"template(\"$MSGONLY\\n\")"

/* Added by zhoucheng */
#define LOGGING_FILE_TEMPLATE_STRING \
"template(\"%s$PROGRAM-$LEVEL_NUM$CTCBRIEF: $MSGONLY\\n\")"

/* template string from remote log */
#define LOGGING_REMOTELOG_TMPLATE_NAME   "template_logremote"
#define LOGGING_REMOTELOG_TMPLATE_STRING  \
"# template for logging to remote server\n" \
"template %s { template(\"<$CTCPRI>%s$HOST $PROGRAM-$LEVEL_NUM$CTCBRIEF: $MSGONLY\\n\"); template_ctcfac(%d); };" \
"\n\n"

/* destination name for syslog-ng */
#define LOGGING_DEST_BUFFER_NAME    "d_logbuffer"
#define LOGGING_DEST_FILE_NAME      "d_logfile"
#define LOGGING_DEST_REMOTE_PREFIX  "d_remote_"
#define LOGGING_DEST_TRAP_NAME      "d_logtrap"
#define LOGGING_DEST_DIAG_NAME      "d_logdiag"


/* destinaiton strings */
#define LOGGING_CFGFILE_MONITOR_NAME    \
"monitor_%d"

#define LOGGING_CFGFILE_DEST_M_STRING \
"# To monitor\n" \
"destination monitor_%d { file(\"%s\", %s); };\n" \
"\n"

#define LOGGING_CFGFILE_DEST_M_STRING_CONSOLE \
"# To monitor\n" \
"destination monitor_%d { file(\"%s\" log_fifo_size(500), %s); };\n" \
"\n"

#define LOGGING_CFGFILE_DEST_B_STRING \
"# To logging buffer\n" \
"destination %s { unix-stream(\"%s\", %s); };\n" \
"\n"

#define LOGGING_CFGFILE_DEST_F_STRING \
"# To logging file\n" \
"destination %s { file(\"%s\", %s\nsizelimitation(524288) syncscript(\"%s\")); };\n" \
"\n"

#define LOGGING_CFGFILE_DEST_F_N_SYNC_STRING \
"# To logging file\n" \
"destination %s { file(\"%s\", %s); };\n" \
"\n"

#define LOGGING_CFGFILE_DEST_R_STRING   \
"# To remote server\n" \
"destination %s%u { udp%s(\"%s\", template(%s)); };\n" \
"\n"

#define LOGGING_CFGFILE_DEST_T_STRING \
"# To logging trap\n" \
"destination %s { unix-stream(\"%s\", %s); };\n" \
"\n"

/* configuration file header and source */
#define LOGGING_CFGFILE_OPTION_STRING   \
"#\n" \
"# Configuration file for syslog-ng\n" \
"#\n" \
"# (C) Copyright Centec Networks Inc.  All rights reserved.\n" \
"#\n" \
"# Authors: Alexander Liu.\n" \
"\n" \
"######\n" \
"# options\n" \
"\n" \
"options {\n" \
"        # disable the chained hostname format in logs\n" \
"        # (default is enabled)\n" \
"        chain_hostnames(0);\n" \
"\n" \
"        # the time to wait before a died connection is re-established\n" \
"        # (default is 60)\n" \
"        time_reopen(10);\n" \
"\n" \
"        # the time to wait before an idle destination file is closed\n" \
"        # (default is 60)\n" \
"        time_reap(360);\n" \
"\n" \
"        # timestamp\n" \
"        # default is rfc3164.\n" \
"        ts_format(\"%s\");\n" \
"\n" \
"        # the number of lines buffered before written to file\n" \
"        # you might want to increase this if your disk isn't catching with\n" \
"        # all the log messages you get or if you want less disk activity\n" \
"        # (say on a laptop)\n" \
"        # (default is 0)\n" \
"        #sync(0);\n" \
"\n" \
"        # the number of lines fitting in the output queue\n" \
"        log_fifo_size(2048);\n" \
"\n" \
"        # enable or disable directory creation for destination files\n" \
"        create_dirs(yes);\n" \
"\n" \
"        # default owner, group, and permissions for log files\n" \
"        # (defaults are 0, 0, 0600)\n" \
"        #owner(root);\n" \
"        #group(root);\n" \
"        perm(0640);\n" \
"\n" \
"        # default owner, group, and permissions for created directories\n" \
"        # (defaults are 0, 0, 0700)\n" \
"        #dir_owner(root);\n" \
"        #dir_group(root);\n" \
"        dir_perm(0755);\n" \
"\n" \
"        # enable or disable DNS usage\n" \
"        # syslog-ng blocks on DNS queries, so enabling DNS may lead to\n" \
"        # a Denial of Service attack\n" \
"        # (default is yes)\n" \
"        use_dns(no);\n" \
"\n" \
"        # maximum length of message in bytes\n" \
"        # this is only limited by the program listening on the /dev/log Unix\n" \
"        # socket, glibc can handle arbitrary length log messages, but -- for\n" \
"        # example -- syslogd accepts only 1024 bytes\n" \
"        # (default is 2048)\n" \
"        #log_msg_size(2048);\n" \
"\n" \
"        # enable merge\n" \
"        merge_enable(%s);\n" \
"\n" \
"        # merge fifo size (default is 1024)\n" \
"        merge_fifo_size(%s);\n" \
"\n" \
"        # merge timeout(default is 10000 millisecond)\n" \
"        merge_timeout(%s);\n" \
"\n" \
"        # merge timefrom\n" \
"        merge_timefrom(%s);\n" \
"};\n\n" 

/* source part */
#define LOGGING_SRC_NAME    "src_all"
#define LOGGING_CFGFILE_SRC_STRING \
"\n" \
"# all known message sources\n" \
"source %s {\n" \
"        # message generated by Syslog-NG\n" \
"        internal();\n" \
"        # standard Linux log source (this is the default place for the syslog()\n" \
"        # function to send logs to)\n" \
"        unix-dgram(\"/dev/log\");\n" \
"        # messages from the kernel\n" \
"        file(\"/proc/kmsg\" log_prefix(\"kernel: \"));\n" \
"        # use the above line if you want to receive remote UDP logging messages\n" \
"        # (this is equivalent to the \"-r\" syslogd flag)\n" \
"        #udp(ip(0.0.0.0) port(514)); # message from line cards\n" \
"};\n\n"

/* severity struct */
typedef struct tagSeverity
{
    unsigned long nValue; /* severity value */
    char *pszName; /* name */
    char *pszSyslogngName; /* name suitable for syslog-ng */
    char *pszNote; /* note */
    char *pszMapedLevel; /* maped level for alarm trap */
} S_Severity;

/* facility */
typedef struct tagFacility
{
    char *pszName; /* name */
    u_int32_t nValue; /* facility value */
    syslog_facility_t nCdbValue; /* facility value in cdb */
    char *pszNote; /* note */
} S_Facility;

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
static int g_bInitialized = 0; /* is logging module initialized? */
/* log buffer queue header */
static S_LogBuffer g_loggingBufferHead = TAILQ_HEAD_INITIALIZER(g_loggingBufferHead);
static char *g_pszRecBuf = NULL; /* log record buffer */
static int g_nLeftLen = 0;
static S_LogStatistics g_loggingStat; /* logging statistics */
static int g_serverCnt = 0; /* configured vrfid & server addr pair count */
static int g_sockIdx = -1; /* socket index */

/* severity array */
static S_Severity g_severityArr[] =
{
    {LOG_EMERG,   "emergency",   "emerg",  "system is unusable", "high"},
    {LOG_ALERT,   "alert",       "alert",  "action must be taken immediately", "middle"},
    {LOG_CRIT,    "critical",    "crit",   "critical conditions", "lower"},
    {LOG_ERR,     "error",       "err",    "error conditions", "minor"},
    {LOG_WARNING, "warning",     "warn",   "warning conditions", ""},
    {LOG_NOTICE,  "notice",      "notice", "normal but significant condition", ""},
    {LOG_INFO,    "information", "info",   "informational", ""},
    {LOG_DEBUG,   "debug",       "debug",  "debug-level messages", ""},
};

/* facility array */
static S_Facility g_facilityArr[] =
{
    { "kern",     LOG_KERN, SYSLOG_FACILITY_KERN, "kernel messages"},
    { "user",     LOG_USER, SYSLOG_FACILITY_USER, "random user-level messages"},
    { "mail",     LOG_MAIL, SYSLOG_FACILITY_MAIL, "mail system"},
    { "daemon",   LOG_DAEMON, SYSLOG_FACILITY_DAEMON, "system daemons"},
    { "auth",     LOG_AUTH, SYSLOG_FACILITY_AUTH, "security/authorization messages"},
    { "syslog",   LOG_SYSLOG, SYSLOG_FACILITY_SYSLOG, "messages generated internally by syslogd"},
    { "lpr",      LOG_LPR, SYSLOG_FACILITY_LPR, "line printer subsystem"},
    { "news",     LOG_NEWS, SYSLOG_FACILITY_NEWS, "network news subsystem"},
    { "uucp",     LOG_UUCP, SYSLOG_FACILITY_UUCP, "UUCP subsystem"},
    { "cron",     LOG_CRON, SYSLOG_FACILITY_CRON, "clock daemon"},
    { "authpriv", LOG_AUTHPRIV, SYSLOG_FACILITY_AUTHPRIV, "security/authorization messages (private)"},
    { "ftp",      LOG_FTP, SYSLOG_FACILITY_FTP, "ftp daemon"},   
    { "local0",   LOG_LOCAL0, SYSLOG_FACILITY_LOCAL0, "reserved for local use 0"},
    { "local1",   LOG_LOCAL1, SYSLOG_FACILITY_LOCAL1, "reserved for local use 1"},
    { "local2",   LOG_LOCAL2, SYSLOG_FACILITY_LOCAL2, "reserved for local use 2"},
    { "local3",   LOG_LOCAL3, SYSLOG_FACILITY_LOCAL3, "reserved for local use 3"},
    { "local4",   LOG_LOCAL4, SYSLOG_FACILITY_LOCAL4, "reserved for local use 4"},
    { "local5",   LOG_LOCAL5, SYSLOG_FACILITY_LOCAL5, "reserved for local use 5"},
    { "local6",   LOG_LOCAL6, SYSLOG_FACILITY_LOCAL6, "reserved for local use 6"},
    { "local7",   LOG_LOCAL7, SYSLOG_FACILITY_LOCAL7, "reserved for local use 7"},
};

S_TerminalMonitorQueue g_terminalMonitorHdr = TAILQ_HEAD_INITIALIZER(g_terminalMonitorHdr);

/****************************************************************************
 *  
* Function
*
*****************************************************************************/
const char *logging_get_syslogng_level_name(unsigned long nFacility);
const char *logging_get_facility_name(unsigned long nFacility);

static void _logging_terminal_monitor_free()
{
    S_TerminalMontior *pItem = NULL;

    while (!TAILQ_EMPTY(&g_terminalMonitorHdr)) {
        pItem = TAILQ_FIRST(&g_terminalMonitorHdr);
        TAILQ_REMOVE(&g_terminalMonitorHdr, pItem, entry);
        free(pItem->pszTty);
        free(pItem);
    }
    return;
}

static int _logging_terminal_monitor_enable(char *pszTty)
{
    S_TerminalMontior *pItem = NULL;

    TAILQ_FOREACH(pItem, &g_terminalMonitorHdr, entry) {
        if (!strcasecmp(pItem->pszTty, pszTty)) {
            return -2;
        }
    }

    pItem = malloc(sizeof(S_TerminalMontior));
    if (NULL == pItem) {
        return -1;
    }

    pItem->pszTty = strdup(pszTty);
    if (NULL == pItem->pszTty) {
        free(pItem);
        return -1;
    }

    TAILQ_INSERT_TAIL(&g_terminalMonitorHdr, pItem, entry);
    return 0;
}

static int _logging_terminal_monitor_disable(char *pszTty)
{
    S_TerminalMontior *pItem = NULL;
    
    TAILQ_FOREACH(pItem, &g_terminalMonitorHdr, entry) {
        if (!strcasecmp(pItem->pszTty, pszTty)) {
            TAILQ_REMOVE(&g_terminalMonitorHdr, pItem, entry);
            free(pItem->pszTty);
            free(pItem);
            return 0;
        }
    }
    
    return -2;
}

int logging_terminal_monitor(int onoff, char *pszTty)
{
    if (onoff) {
        return _logging_terminal_monitor_enable(pszTty);
    }
    
    return _logging_terminal_monitor_disable(pszTty); 
}

int logging_get_standard_facility_value(syslog_facility_t nCdbFac)
{
    unsigned long ret = LOG_LOCAL7;
    int nCnt = sizeof(g_facilityArr) / sizeof(g_facilityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++) {
        if (g_facilityArr[i].nCdbValue == nCdbFac) {
            ret = g_facilityArr[i].nValue;
            break;
        }
    }

    return ret;
}

/*******************************************************************************
 * Name: logging_get_timestamp_name
 * Purpose: get timestamp name by value
 * Input: timestamp value
 * Output: N/A
 * Return: 
 *   Success: timestamp name
 *   Failed:  "invalid"
 * Note: 
 ******************************************************************************/
const char *logging_get_timestamp_name(syslog_timestamp_t eTsFormat)
{
    return cdb_enum_val2str(syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX, eTsFormat);
}

/*******************************************************************************
 * Name: logging_get_timestamp_macro
 * Purpose: get timestamp macro by value
 * Input: timestamp macro value
 * Output: N/A
 * Return: 
 *   Success: timestamp macro name
 * Note: 
 ******************************************************************************/
const char *logging_get_timestamp_macro(syslog_timestamp_t eTsFormat)
{
    switch (eTsFormat)
    {
    case SYSLOG_TIMESTAMP_BSD:
        return LOGGING_TS_FORMAT_RBSD;
    
    case SYSLOG_TIMESTAMP_RISO:
        return LOGGING_TS_FORMAT_RISO;

    case SYSLOG_TIMESTAMP_RFULL:
        return LOGGING_TS_FORMAT_RFULL;
        
    default:
        return LOGGING_TS_FORMAT_NONE;
    }

    /* never retached */
    return LOGGING_TS_FORMAT_NONE;
}


/*******************************************************************************
 * Name: logging_get_timestamp
 * Purpose: get timestamp value by name
 * Input: timestamp style name
 * Output: N/A
 * Return: 
 *   Success: timestamp value
 *   Failed: SYSLOG_TIMESTAMP_MAX
 * Note: 
 ******************************************************************************/
int logging_get_timestamp(const char *pszStyle)
{
    if (!strcasecmp(pszStyle, "rfc3164") || !strcasecmp(pszStyle, "bsd")) {
        return SYSLOG_TIMESTAMP_BSD;
    }

    if (!strcasecmp(pszStyle, "rfc3339") || !strcasecmp(pszStyle, "iso")) {
        return SYSLOG_TIMESTAMP_RISO;
    }

    if (!strcasecmp(pszStyle, "date"))
    {
        return SYSLOG_TIMESTAMP_RFULL;
    }

    return SYSLOG_TIMESTAMP_MAX;
}

/*******************************************************************************
 * Name: logging_cfgfile_write_header
 * Purpose: write options & source for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   mg_en: enable merge or not
 *   mg_fifo_size: merge fifo size
 *   mg_timeout: merge timeout
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_cfgfile_write_header(FILE *pFile, 
        const char * mg_en,
        const char * mg_fifo_size,
        const char * mg_timeout,
        const char * mg_timefrom)
{
    int nRet = 0;
    nRet = fprintf(pFile, LOGGING_CFGFILE_OPTION_STRING, 
                   LOGGING_DEFAULT_TS_FORMAT, mg_en, mg_fifo_size, mg_timeout, mg_timefrom);
    if (nRet <= 0)
    {
        return -1;
    }

    nRet = fprintf(pFile, LOGGING_CFGFILE_SRC_STRING, LOGGING_SRC_NAME);
    return nRet;
}

/*******************************************************************************
 * Name: logging_write_filter_item
 * Purpose: write one filter item
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   pszName: severity filter name
 *   nSeverity: severity level
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_write_filter_item(FILE *pFile, char *pszName, unsigned long nSeverity)
{
    int nRet = 0;

    /* write name */
    nRet = fprintf(pFile, "filter %s { level(", pszName);
    if (nRet <= 0)
    {
        return -1;
    }

    /* write severitys */
    if (0 == nSeverity)
    {
        nRet = fprintf(pFile, "%s); };\n\n", 
                          logging_get_syslogng_level_name(nSeverity));
    }
    else
    {
        nRet = fprintf(pFile, "%s..%s); };\n\n", 
                           logging_get_syslogng_level_name(nSeverity), 
                           logging_get_syslogng_level_name(0));
    }
    return nRet;
}

/*******************************************************************************
 * Name: logging_cfgfile_write_filter
 * Purpose: write filter for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_cfgfile_write_filter(tbl_syslog_cfg_t *pCfg, FILE *pFile)
{
    /* write file severity filter */
    if (logging_write_filter_item(pFile, LOGGING_SFNAME_FILE,
                                  pCfg->file_severity) < 0)
    {
        return -1;
    }

    /* write sever severity filter */
    if (logging_write_filter_item(pFile, LOGGING_SFNAME_SRV,
                                  pCfg->server_severity) < 0)
    {
        return -1;
    }

    /* write module severity filter */
    if (logging_write_filter_item(pFile, LOGGING_SFNAME_MOD,
                                  pCfg->module_severity) < 0)
    {
        return -1;
    }

    if (fprintf(pFile, "filter %s { level(%s..emerg) and facility(%s); };\n\n",
                    LOGGING_SFNAME_TRAP,
                    logging_get_syslogng_level_name(pCfg->trap_severity),
                    logging_get_facility_name(LOG_LOCAL6)) < 0)
    {
        return -1;
    }

    /* Added by zhoucheng 2007-12-23 20:15, bug 3867 */
    if (fprintf(pFile, "filter %s { not facility(local5); };\n\n", LOGGING_SFNAME_OPER) < 0)
    {
        return -1;
    }

    /* add by chenzh for bug 16508, 2011-09-26*/
    if (fprintf(pFile, "filter %s { facility(local7); };\n\n", LOGGING_SFNAME_DIAG_FACILITY) < 0)
    {
        return -1;
    }

    if (fprintf(pFile, "filter %s {not facility(local7); };\n\n", LOGGING_SFNAME_DIAG_NOT_FACILITY) < 0)
    {
        return -1;
    }

    return 0;
}

/*******************************************************************************
 * Name: logging_cfgfile_write_monitor_dest
 * Purpose: write destination for monitor for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   eActionType: action type
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_cfgfile_write_monitor_dest(tbl_syslog_cfg_t *pCfg, FILE *pFile, E_ActionType eActionType)
{
    S_TerminalMontior *pItem;
    
#define LOGGING_TIMESTAMP_BUFSIZE   128
    char szBuf[LOGGING_TIMESTAMP_BUFSIZE];
    int idx = 0;

    /* genrate timestamp string */
    snprintf(szBuf, LOGGING_TIMESTAMP_BUFSIZE, LOGGING_TEMPLATE_STRING,
                 logging_get_timestamp_macro(pCfg->timestamp));

    TAILQ_FOREACH(pItem, &g_terminalMonitorHdr, entry) {           
        /* write logging buffer dest */
        if (!strcasecmp(pItem->pszTty, "/dev/console")) {
            fprintf(pFile, LOGGING_CFGFILE_DEST_M_STRING_CONSOLE, 
                    idx, pItem->pszTty, szBuf);
        } else {
            fprintf(pFile, LOGGING_CFGFILE_DEST_M_STRING, 
                    idx, pItem->pszTty, szBuf);
        }
        idx++;
    } 

    return 0;
}

static int
__write_dest(void *pArg, void *pArg2, int vrf, const char *pszAddr, int nIdx)
{
    FILE *pFile = (FILE *)pArg2;
    int af;

    if (strchr(pszAddr, '.')) {
        af = AF_INET;
    } else {
        af = AF_INET6;
    }

    fprintf(pFile, LOGGING_CFGFILE_DEST_R_STRING, 
           LOGGING_DEST_REMOTE_PREFIX, nIdx, 
           (AF_INET == af) ? "" : "6",
           pszAddr, LOGGING_REMOTELOG_TMPLATE_NAME);
    return 0;
}

/*******************************************************************************
 * Name: logging_cfgfile_write_dest
 * Purpose: write destination for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   eActionType: action type
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_cfgfile_write_dest(tbl_syslog_cfg_t *pCfg, FILE *pFile, E_ActionType eActionType)
{
#define LOGGING_TIMESTAMP_BUFSIZE   128
    char szBuf[LOGGING_TIMESTAMP_BUFSIZE];
    int nRet = 0;

    /* genrate timestamp string */
    snprintf(szBuf, LOGGING_TIMESTAMP_BUFSIZE, LOGGING_TEMPLATE_STRING,
             logging_get_timestamp_macro(pCfg->timestamp));

    /* write logging buffer dest */
    nRet = fprintf(pFile, LOGGING_CFGFILE_DEST_B_STRING, 
                       LOGGING_DEST_BUFFER_NAME, 
                       LOGGING_LOG_PATH, szBuf);
    if (nRet < 0) {
        return nRet;
    }

    snprintf(szBuf, LOGGING_TIMESTAMP_BUFSIZE, LOGGING_FILE_TEMPLATE_STRING,
                 logging_get_timestamp_macro(pCfg->timestamp));
    if (pCfg->enable_to_file) {
        nRet = fprintf(pFile, LOGGING_CFGFILE_DEST_F_STRING, 
                           LOGGING_DEST_FILE_NAME, 
                           LOGGING_LOGFILE_DEFAULT, szBuf,
                           M_SYNC_SCRIPT);
        if (nRet < 0) {
            return nRet;
        }
    }

    /* write remote log server dest */
    if (pCfg->enable_to_server && '\0' != pCfg->server_addr[0]) {
        /* write remote log template */
        nRet = fprintf(pFile, LOGGING_REMOTELOG_TMPLATE_STRING,
                LOGGING_REMOTELOG_TMPLATE_NAME,
                logging_get_timestamp_macro(pCfg->timestamp),
                (logging_get_standard_facility_value(pCfg->server_facility) >> 3));
        if (nRet < 0) {
            return nRet;
        }

        logging_server_addr_iter(pCfg->server_addr, __write_dest, pCfg, pFile);
    }

    /* Added by zhoucheng 2007-11-27 13:39 */
    /* write snmp-trap dest */
    snprintf(szBuf, LOGGING_TIMESTAMP_BUFSIZE, LOGGING_TRAP_TEMPLATE_STRING);
    if (pCfg->enable_to_trap) {
        nRet = fprintf(pFile,  LOGGING_CFGFILE_DEST_T_STRING,
                           LOGGING_DEST_TRAP_NAME, 
                           LOGGING_LOG_TRAP_PATH, szBuf);
        if (nRet < 0) {
            return nRet;
        }
    }

    /* add by chenzh for bug 16508, 2011-09-26*/
    snprintf(szBuf, LOGGING_TIMESTAMP_BUFSIZE, LOGGING_FILE_TEMPLATE_STRING,
             logging_get_timestamp_macro(pCfg->timestamp));
    if (pCfg->enable_to_diag) {
        nRet = fprintf(pFile, LOGGING_CFGFILE_DEST_F_STRING, 
                           LOGGING_DEST_DIAG_NAME, 
                           LOGGING_LOG_DIAG_PATH, szBuf, M_DIAGSYNC_SCRIPT);
        if (nRet < 0)
        {
            return nRet;
        }
    }
    
    return logging_cfgfile_write_monitor_dest(pCfg, pFile, eActionType);
}

/*******************************************************************************
 * Name: logging_cfgfile_write_monitor_log
 * Purpose: write log item for monitor for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   eActionType: action type
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
 int logging_cfgfile_write_diag_log(tbl_syslog_cfg_t *pCfg, FILE *pFile, E_ActionType eActionType)
 {
#if 0
#define LOGGING_LOGNAME_BUF 128
     char szBuf[LOGGING_LOGNAME_BUF];
     struct lib_globals *pzg = pCfg->pzg;
     int nRet = 0;
     struct imi_server *is = NULL;
     struct imi_master *im = NULL;
     struct imi_server_entry *ise;
     struct line *line;
     int type;
     int i;

     if (NULL == pzg || NULL == pzg->imh 
         || NULL == pzg->imh->info
     || NULL == pzg->vr_in_cxt
     || NULL == pzg->vr_in_cxt->proto)
     {
         return 0;
     }
        
     is = pzg->imh->info;
     im = pzg->vr_in_cxt->proto;
     if (E_ACTION_DIAG_SHUTDOWN == eActionType)
     {
         pCfg->nDiagOperateEnable = 0;
         return 0;
     }

     if ((E_ACTION_DIAG != eActionType)&& (!pCfg->nDiagOperateEnable))
     {
         return 0;
     }

     /* Line info.  */
     for (type = LINE_TYPE_CONSOLE; type < LINE_TYPE_MAX; type++)
     {
         for (i = 0; i < vector_max ((vector)im->lines[type]); i++)
         {
             if ((line = vector_slot ((vector)im->lines[type], i)) == NULL)
             {
                 continue;
             }

             if (!CHECK_FLAG (line->flags, LINE_FLAG_UP))
             {
                 continue;
             }
                
             if ((ise = imi_line_entry_lookup (is, type, i)) == NULL)
             {
                 continue;
             }

             snprintf(szBuf, LOGGING_LOGNAME_BUF, LOGGING_CFGFILE_MONITOR_NAME,
                          LINE_TYPE_INDEX(type, i));
             
             nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_2_FILTER, "diag",
                                LOGGING_SRC_NAME, LOGGING_SFNAME_MOD, LOGGING_SFNAME_DIAG_FACILITY, szBuf);

             pCfg->nDiagOperateEnable = 1;
             if (nRet < 0)
             {
                 return nRet;
             }
         }        
     } /* for (i = 0 */
#endif  
     return 0;
}


/*******************************************************************************
 * Name: logging_cfgfile_write_monitor_log
 * Purpose: write log item for monitor for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   eActionType: action type
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_cfgfile_write_monitor_log(tbl_syslog_cfg_t *pCfg, FILE *pFile, E_ActionType eActionType)
{
#define LOGGING_LOGNAME_BUF 128
    char szBuf[LOGGING_LOGNAME_BUF];
    S_TerminalMontior *pItem;
    int idx = 0;


    TAILQ_FOREACH(pItem, &g_terminalMonitorHdr, entry) {
        snprintf(szBuf, LOGGING_LOGNAME_BUF, LOGGING_CFGFILE_MONITOR_NAME, idx);
        if (pCfg->enable_operate)
        {
            fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_2_FILTER, "monitor",
                    LOGGING_SRC_NAME, LOGGING_SFNAME_MOD, 
                    LOGGING_SFNAME_DIAG_NOT_FACILITY, szBuf);
        }
        else
        {
            fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_3_FILTER, "monitor",
                    LOGGING_SRC_NAME, LOGGING_SFNAME_MOD, 
                    LOGGING_SFNAME_OPER, LOGGING_SFNAME_DIAG_NOT_FACILITY, szBuf);
        }
        
        idx++;
    }
    return 0;
}

static int
__write_log(void *pArg, void *pArg2, int vrf, const char *pszAddr, int nIdx)
{
    FILE *pFile = (FILE *)pArg2;
    char szBuf[GENSOCK_ADDRSTRLEN + 1];

    snprintf(szBuf, GENSOCK_ADDRSTRLEN + 1, "%s%u",
             LOGGING_DEST_REMOTE_PREFIX, nIdx);
    fprintf(pFile, LOGGING_CFGFILE_LOG_STRING, "remote server",
                   LOGGING_SRC_NAME, LOGGING_SFNAME_SRV, szBuf);
    return 0;
}

/*******************************************************************************
 * Name: logging_cfgfile_write_log
 * Purpose: write log output option for configuration file
 * Input: 
 *   cli: client 
 *   pFile: file pointer
 *   eActionType: action type
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_cfgfile_write_log(tbl_syslog_cfg_t *pCfg, FILE *pFile, E_ActionType eActionType)
{
    int nRet = 0;
#if 0
    /* Modified by zhoucheng 2007-12-23 20:16, bug 3867 */
    /* write log buffer item */
    if (pCfg->nOperateEnable)
    {
        nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_2_FILTER, "logging buffer",
                       LOGGING_SRC_NAME, LOGGING_SFNAME_MOD, LOGGING_SFNAME_DIAG_NOT_FACILITY, LOGGING_DEST_BUFFER_NAME);
        if (nRet < 0)
        {
            return nRet;
        }
    }
    else
#endif
    {
        if (pCfg->enable_operate)
        {
            nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_2_FILTER, "logging buffer",
                           LOGGING_SRC_NAME, LOGGING_SFNAME_MOD, LOGGING_SFNAME_DIAG_NOT_FACILITY, LOGGING_DEST_BUFFER_NAME);
        }
        else
        {
            nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_3_FILTER, "logging buffer",
                           LOGGING_SRC_NAME, LOGGING_SFNAME_MOD, LOGGING_SFNAME_OPER, LOGGING_SFNAME_DIAG_NOT_FACILITY, LOGGING_DEST_BUFFER_NAME);
        }
        if (nRet < 0)
        {
            return nRet;
        }
    }

    /* write log file item */
    if (pCfg->enable_to_file) {
        nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_2_FILTER, "logging file",
                       LOGGING_SRC_NAME, LOGGING_SFNAME_FILE, 
                       LOGGING_SFNAME_DIAG_NOT_FACILITY, LOGGING_DEST_FILE_NAME);
        if (nRet < 0) {
            return nRet;
        }
    }

    /* write remote log server item */
    if (pCfg->enable_to_server && '\0' != pCfg->server_addr[0]) {
        logging_server_addr_iter(pCfg->server_addr, __write_log, pCfg, pFile);
    }

    /* write log trap item */
    if (pCfg->enable_to_trap) {
        nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING, "logging trap",
                      LOGGING_SRC_NAME, LOGGING_SFNAME_TRAP,
                      LOGGING_DEST_TRAP_NAME);
        if (nRet < 0) {
            return nRet;
        }
    }

    /* write log diag item */
    if (pCfg->enable_to_diag)
    {
        nRet = fprintf(pFile, LOGGING_CFGFILE_LOG_STRING_2_FILTER, "logging diag",
                       LOGGING_SRC_NAME, LOGGING_SFNAME_MOD,
                       LOGGING_SFNAME_DIAG_FACILITY, LOGGING_DEST_DIAG_NAME);
        if (nRet < 0)
        {
            return nRet;
        }
    }

    nRet = logging_cfgfile_write_monitor_log(pCfg, pFile, eActionType);
    if (nRet < 0) {
        return nRet;
    }

    nRet = logging_cfgfile_write_diag_log(pCfg, pFile, eActionType);
    if (nRet < 0) {
        return nRet;
    }

    return 0;
}

/*******************************************************************************
 * Name: logging_syslogng_reload
 * Purpose: let syslog-ng reload configuration
 * Input: cli: client 
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_syslogng_reload(void)
{
    int ret = system("kill -1 `cat /var/run/syslog-ng.pid`");
    return (0 == ret) ? ret : -1;
}

/*******************************************************************************
 * Name: logging_process_cfgfile
 * Purpose: process config file
 * Input: 
 *   cli: client 
 *   eActionType: occasion type: running or shutdown
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed:  -1
 * Note: 
 ******************************************************************************/
int logging_process_cfgfile(tbl_syslog_cfg_t *pCfg, E_ActionType eActionType)
{
    FILE *pFile = NULL;
    char mg_state[8] = {0};
    char mg_fifo_size[32] = {0};
    char mg_timeout[8] = {0};
    char mg_timefrom[8] = {0};

    sprintf(mg_state, "%s", (pCfg->enable_merge) ? "yes" : "no");
    sprintf(mg_fifo_size, "%d", pCfg->merge_fifosize);
    sprintf(mg_timeout, "%d", pCfg->merge_timeout * 1000);
    sprintf(mg_timefrom, "%s", "yes");
    
    pFile = fopen(LOGGING_CFGFILE_DEFAULT, "w+");
    if (NULL == pFile) {
        return -1;
    }

    /* write config file */
    if (logging_cfgfile_write_header(pFile, mg_state, mg_fifo_size, mg_timeout, mg_timefrom) < 0 
        || logging_cfgfile_write_filter(pCfg, pFile) < 0
        || logging_cfgfile_write_dest(pCfg, pFile, eActionType) < 0
        || logging_cfgfile_write_log(pCfg, pFile, eActionType) < 0) {
        fclose(pFile);
        return -1;
    }
        
    fclose(pFile);
    return 0;
}

/*******************************************************************************
 * Name: logging_process_diag
 * Purpose: process monitor for IMISH login/out & IMI shutdown
 * Input: N/A
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed:  -1
 * Note: 
 ******************************************************************************/
int logging_process_diag(E_ActionType eActionType)
{
    logging_process_cfgfile(NULL, eActionType);
    return 0;
}

/*******************************************************************************
 * Name: logging_process_monitor
 * Purpose: process monitor for IMISH login/out & IMI shutdown
 * Input: N/A
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed:  -1
 * Note: 
 ******************************************************************************/
int logging_process_monitor(void)
{
    logging_process_cfgfile(NULL, E_ACTION_RUNNING);
    return 0;
}
 
/*******************************************************************************
 * Name: logging_get_severity
 * Purpose: get severity number by name
 * Input: pszName: severity name
 * Output: N/A
 * Return:
 *   Success: severity value
 *   Failed:  LOG_DEBUG
 * Note: 
 ******************************************************************************/
unsigned long logging_get_severity(const char *pszName)
{
    int nCnt = sizeof(g_severityArr) / sizeof(g_severityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (!strcasecmp(pszName, g_severityArr[i].pszName))
        {
            return g_severityArr[i].nValue;
        }
    }
    
    return LOG_DEBUG;
}
 
/*******************************************************************************
 * Name: logging_get_syslogng_level_name
 * Purpose: get severity name by no for syslog-ng
 * Input: nFacility: severity number
 * Output: N/A
 * Return:
 *   Success: severity name
 *   Failed:  "unknown"
 * Note: 
 ******************************************************************************/
const char *logging_get_syslogng_level_name(unsigned long nFacility)
{
    int nCnt = sizeof(g_severityArr) / sizeof(g_severityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (nFacility == g_severityArr[i].nValue)
        {
            return g_severityArr[i].pszSyslogngName;
        }
    }
    
    return "unknown";
}

/*******************************************************************************
 * Name: logging_get_severity_name
 * Purpose: get severity name by no
 * Input: nFacility: severity number
 * Output: N/A
 * Return:
 *   Success: severity name
 *   Failed:  "unknown"
 * Note: 
 ******************************************************************************/
const char *logging_get_severity_name(unsigned long nFacility)
{
    int nCnt = sizeof(g_severityArr) / sizeof(g_severityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (nFacility == g_severityArr[i].nValue)
        {
            return g_severityArr[i].pszName;
        }
    }
    
    return "unknown";
}

/*******************************************************************************
 * Name: logging_get_level_name
 * Purpose: get alarm_level_name  by no
 * Input: nSevirity: severity number
 * Output: N/A
 * Return:
 *   Success: maped alarm_level_name
 *   Failed:  "unknown"
 * Note: 
 ******************************************************************************/
const char *logging_get_level_name(unsigned long nSevirity)
{
    int nCnt = sizeof(g_severityArr) / sizeof(g_severityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (nSevirity == g_severityArr[i].nValue)
        {
            return g_severityArr[i].pszMapedLevel;
        }
    }
    
    return "unknown";
}

/*******************************************************************************
 * Name: logging_get_facility_name
 * Purpose: get facility name by no
 * Input: nFacility: facility number
 * Output: N/A
 * Return:
 *   Success: facility name
 *   Failed:  NULL
 * Note: 
 ******************************************************************************/
const char *logging_get_facility_name(unsigned long nFacility)
{
    int nCnt = sizeof(g_facilityArr) / sizeof(g_facilityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (nFacility == g_facilityArr[i].nValue)
        {
            return g_facilityArr[i].pszName;
        }
    }

    return "unknown";
}

/*******************************************************************************
 * Name: logging_get_severity_desc
 * Purpose: get severity describle by no
 * Input: nFacility: severity number
 * Output: N/A
 * Return:
 *   Success: note string
 *   Failed:  NULL
 * Note: 
 ******************************************************************************/
const char *logging_get_severity_desc(unsigned long nFacility)
{
    int nCnt = sizeof(g_severityArr) / sizeof(g_severityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (nFacility == g_severityArr[i].nValue)
        {
            return g_severityArr[i].pszNote;
        }
    }

    return "unknown";
}

/*******************************************************************************
 * Name: logging_get_facility
 * Purpose: get facility number by name
 * Input: pszName: facility name
 * Output: N/A
 * Return:
 *   Success: facility value
 *   Failed:  LOG_DEBUG
 * Note: 
 ******************************************************************************/
unsigned long logging_get_facility(const char *pszName)
{
    int nCnt = sizeof(g_facilityArr) / sizeof(g_facilityArr[0]);
    int i;

    for (i = 0; i < nCnt; i++)
    {
        if (!strcasecmp(pszName, g_facilityArr[i].pszName))
        {
            return g_facilityArr[i].nValue;
        }
    }
    
    return LOG_KERN;
}
 
/*******************************************************************************
 * Name: logging_malloc_logitem
 * Purpose: 
 *   malloc one log item and it's log string buffer
 *   and add 1 byte to hold '\0' at the end
 * Input: nLen: record length
 * Output: N/A
 * Return: 
 *   Success: log item pointer
 *   Failed:  NULL
 * Note: 
 ******************************************************************************/
S_LogItem *logging_malloc_logitem(size_t nLen)
{
    S_LogItem *item = NULL;

    if (nLen < 0 || nLen >= LOGGING_MAX_MSG_LEN) {
        return NULL;
    }
    
    item = malloc(sizeof(S_LogItem));
    if (NULL == item) {
        return NULL;
    }

    item->pszLog = malloc(nLen + 1);
    if (NULL == item->pszLog) {
        free(item);
        return NULL;
    }
    
    item->nLen = (nLen + 1);
    return item;
}

/*******************************************************************************
 * Name: logging_free_logitem
 * Purpose: free log item and it's log buffer
 * Input: pItem: log item pointer
 * Output: N/A
 * Return: N/A
 * Note: 
 ******************************************************************************/
void logging_free_logitem(S_LogItem *pItem)
{
    if (NULL != pItem)
    {
        if (NULL != pItem->pszLog)
        {
            free(pItem->pszLog);
        }
        free(pItem);
    }
    return;
}

/*******************************************************************************
 * Name: logging_free_queue
 * Purpose: free log item queue
 * Input: N/A
 * Output: N/A
 * Return: N/A
 * Note: 
 ******************************************************************************/
void logging_free_queue(void)
{
    S_LogItem *pItem = NULL;
    while (!TAILQ_EMPTY(&g_loggingBufferHead))
    {
        pItem = TAILQ_FIRST(&g_loggingBufferHead);
        TAILQ_REMOVE(&g_loggingBufferHead, pItem, entry);
        logging_free_logitem(pItem);
    }
    LOGGING_LOGCNT_ZERO;
    return;
}

/*******************************************************************************
 * Name: logging_adjust_queue_buffer_size
 * Purpose: adjust buffer size
 * Input: N/A
 * Output: N/A
 * Return: N/A
 * Note: 
 ******************************************************************************/
void logging_adjust_queue_buffer_size(unsigned long nValue)
{
    S_LogItem *pItem = NULL;

    /* Added by George Wei 2007-7-12 for bug 2787 {*/
    while (LOGGING_LOGCNT_VAR > nValue)
    {
        /* delete the oldest one */
        pItem = TAILQ_FIRST(&g_loggingBufferHead);
        TAILQ_REMOVE(&g_loggingBufferHead, pItem, entry);

        /* free it */
        logging_free_logitem(pItem);
        pItem = NULL;

        LOGGING_LOGCNT_DEC;
    }
}

/*******************************************************************************
 * Name: logging_insert_record
 * Purpose: insert record into log queue
 * Input: 
 *   pRec: record buffer
 *   nLen: record length
 * Output:
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_insert_record(char *pRec, size_t nLen)
{
    tbl_syslog_cfg_t *pCfg = tbl_syslog_cfg_get_syslog_cfg();
    S_LogItem *pItem = NULL;

    if (NULL == pRec || nLen <= 0)
    {
        return -1;
    }
    
    LOGGING_LOGCNT_TOTAL_VAR++;
    if (LOGGING_LOGCNT_VAR >= pCfg->logging_lines)
    {
        /* too many log items, erase out the oldest one */
        pItem = TAILQ_FIRST(&g_loggingBufferHead);
        TAILQ_REMOVE(&g_loggingBufferHead, pItem, entry);
        if (pItem->nLen <= nLen)
        {
            /* can't reuse this log item */
            logging_free_logitem(pItem);
            pItem = NULL;
        }
        LOGGING_LOGCNT_DEC;
    }

    /* malloc log item if needed */
    if (NULL == pItem && (NULL == (pItem = logging_malloc_logitem(nLen))))
    {
        LOGGING_LOGCNT_DROP_VAR++;
        return -1;
    }

    /* we have malloced 1 more byte to hold the '\0' */
    memcpy(pItem->pszLog, pRec, nLen);
    pItem->pszLog[nLen] = '\0';

    /* insert record to log buffer queue */
    TAILQ_INSERT_TAIL(&g_loggingBufferHead, pItem, entry);
    LOGGING_LOGCNT_INC;
    return 0;
}

/*******************************************************************************
 * Name:    logging_copy_meta_str
 * Purpose: copy str with % as it's origin meaning
 * Input: 
 *   dest: destination
 *   src: source
 *   length: copy length 
 * Output: 
 * Return: 
 *   success: 0 
 *   failed : -1 
 * Note: 
 ******************************************************************************/
int
logging_copy_meta_str(char* dest, const char * src, int length)
{
    const char *p_src = src;
    char *p_dest = dest;

    if (!src || !dest || length<2*strlen(src)+1)
    {
        return -1;
    }
    
    while (*p_src)
    {
        if ('%' == *p_src)
        {
            *p_dest++ = *p_src;
        }
        *p_dest++ = *p_src++;
    }
    *p_dest = '\0';
    return 0;
}

/*******************************************************************************
 * Name: logging_show_records
 * Purpose: show logging records
 * Input: 
 *   cli: client
 *   nLines: line number
 * Output: N/A
 * Return: N/A
 * Note: N/A
 ******************************************************************************/
void logging_show_records(int nLines, logs_iter_fn_t pCB, void *pArg)
{
    S_LogItem *pItem = NULL;
    int nCnt = 0;

    /* Modified by zhoucheng 2008-01-21 20:18, bug 4129 */
    char real_log[2*LOGGING_MAX_MSG_SIZE+1] = {'\0'};

    if (0 == nLines)
    {
        return;
    }

    if (nLines < 0)
    {
        nCnt = -nLines;
        TAILQ_FOREACH(pItem, &g_loggingBufferHead, entry)
        {         
            if (-1 == logging_copy_meta_str(real_log, pItem->pszLog, 2*LOGGING_MAX_MSG_SIZE+1))
            {
                continue;
            }

            pCB(pArg, real_log);
            nCnt--;
            if (nCnt <= 0)
            {
                break;
            }
        }
    }
    else
    {
        nCnt = nLines;
        TAILQ_FOREACH_REVERSE(pItem, &g_loggingBufferHead, tagLogQueue, entry)
        {        
            if (-1 == logging_copy_meta_str(real_log, pItem->pszLog, 2*LOGGING_MAX_MSG_SIZE+1))
            {
                continue;
            }
            pCB(pArg, real_log);
            nCnt--;
            if (nCnt <= 0)
            {
                break;
            }
        }
    }
    return;
}

/*******************************************************************************
 * Name: logging_parse_msgline 
 * Purpose: parse one log record, format is: <...>...
 * Input: 
 *   pzg:  lib globals
 *   pLine: message line
 *   nLen:  record length
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_parse_msgline(char *pLine, size_t nLen)
{
    char *p = NULL;
    char *pRec = NULL;
    size_t nRecLen = 0;

    if (NULL == pLine || nLen <= 0)
    {
        return -1;
    }

    /* skip facility/severity field */
    if ('<' == *pLine && (p = strchr(pLine, '>')) != NULL)
    {
        pRec = pLine + (p - pLine) + 1;
        nRecLen = nLen - ((p - pLine) + 1);
    }
    else
    {
        /* message format error, ignore it */
        pRec = pLine;
        nRecLen = nLen;
    }

    logging_insert_record(pRec, nRecLen);
    return 0;
}

/*******************************************************************************
 * Name: logging_parse_msgs
 * Purpose: parse logging messages, note that may exists more than one record
 * Input: 
 *   pzg:  lib globals
 *   pBuf: message buffer
 *   nLen: buffer length
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_parse_msgs(char *pBuf, size_t nLen)
{
    char *p = NULL;
    char *pRec = pBuf;
    size_t nLeftLen = nLen;
    size_t nRecLen = 0;

    if (NULL == pBuf || nLen <= 0)
    {
        return -1;
    }
    
    /* process it line by line */
    do
    {
        if ((p = strchr(pRec, '\n')) != NULL)
        {
            nRecLen = (p - pRec) + 1; /* add 1 for '\n' */
        }
        else
        {
            /* some chars is the next log */
            g_nLeftLen = nLeftLen;
            break;
        }

        logging_parse_msgline(pRec, nRecLen);
        nLeftLen -= nRecLen;
        pRec += nRecLen;
    } while (nLeftLen > 0);
    return 0;
}
 
/*******************************************************************************
 * Name: logging_server_read
 * Purpose: 
 *   Read log message and enter it to log buffer
 *   This function should not be called directly. Message library call this.
 * Input:
 *   pArg: session
 * Output: N/A
 * Return: N/A
 * Note: 
 ******************************************************************************/
void logging_server_read(void *pArg)
{
    ctc_sock_session_t *p_session = (ctc_sock_session_t *)pArg;
    int sock = p_session->fd;
    int nRead = 0;
    int nTotalLen = 0;

    if (0 == g_bInitialized)
    {
        return;
    }
    
    while (1)
    {
        nRead = read(sock, g_pszRecBuf + g_nLeftLen, LOGGING_LINEBUF_SIZE - g_nLeftLen);
        if (nRead > 0)
        {
            break;
        }

        if (0 == nRead)
        {
            /* Connection closed */
            _ctc_sock_session_close(p_session);
            return;
        }

        switch (errno)
        {
        case EINTR:
        case EAGAIN:
        case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)        
        case EWOULDBLOCK:
#endif
            continue;
        }

        return;
    }

    /* We have malloced 1 more bytes, so can add the '\0' at the end 
     * this can make parse easy.
     */
    nTotalLen = nRead + g_nLeftLen;
    g_nLeftLen = 0;
    g_pszRecBuf[nTotalLen] = '\0';
    //printf("Log: %s\r\n", g_pszRecBuf);
    logging_parse_msgs(g_pszRecBuf, nTotalLen);
    return;
}

int redirect_kernel_log()
{
    /*add by chenzhonghua for bug 14173£¬ 2011-05-04*/
    /*the follow method is used to redirect the kernel log into /mnt/flash/cold/log/startup_info.log*/
    int ret = redirect_startup_log();
    if( ret != 0)
    {
        log_sys(M_MOD_CFG, E_WARNING, "redirect startup log failure.");
    }
    logging_free_queue();
    return 0;
}

/*******************************************************************************
 * Name: logging_server_init
 * Purpose: initialize log buffer server socket
 * Input:  pzg
 * Output: N/A
 * Return:
 *   Success: 0
 *   Fail:    -1
 * Note: 
 ******************************************************************************/
int logging_server_init()
{
    if (g_sockIdx >= 0) {
        return -1;
    }

    g_sockIdx = ctc_socket(CTC_SOCK_IPC, 0, "loggingServer");
    if (-1 == g_sockIdx) {
        return -1;
    }

    ctc_sock_register_read_fn(g_sockIdx, logging_server_read);
    ctc_sock_bind(g_sockIdx, "/tmp/.logbuf_line");
    return 0;
}

/*******************************************************************************
 * Name: logging_server_free
 * Purpose: relase logging server message handler
 * Input: pzg: lib globals
 * Output: N/A
 * Return:
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_server_free()
{
    if (g_sockIdx >= 0) {
        ctc_sock_close(g_sockIdx);
        g_sockIdx = -1;
    }
    return 0;
}

/*******************************************************************************
 * Name:    logging_check_ip
 * Purpose: Check IP address
 * Input: 
 *   ipaddr: IP address
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
logging_check_ip(gensock_t *su)
{
    char szAddrBuf[GENSOCK_ADDRSTRLEN + 1];
    struct in6_addr addr6;

    if (AF_INET == su->sa.sa_family) {
        u_char *p =  (u_char *)&su->sin.sin_addr;
        
        /* filtrate ip with 0.0.0.0 */
        if (0 == *p) {
            return -1;
        }
        
        /* filtrate ip with 255.255.255.255 */
        if (255 == p[0] && 255 == p[1] && 255 == p[2] && 255 == p[3]) {
            return -1;
        }
        
        /* filtrate multicast addresses */
        if (IN_MULTICAST(ntohl(*(int*)&su->sin.sin_addr))
			 || IN_LOOPBACK(ntohl(*(int*)&su->sin.sin_addr))) {
            return -1;
        }

        return 0;
    }

    gensock2str(su, szAddrBuf, GENSOCK_ADDRSTRLEN + 1);
    inet_pton (AF_INET6, szAddrBuf, &addr6);
    
    if (IN6_IS_ADDR_MULTICAST (&addr6)
            || IN6_IS_ADDR_LOOPBACK (&addr6)
            || IN6_IS_ADDR_UNSPECIFIED (&addr6)) {
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * Name: logging_module_free
 * Purpose: deinitialize logging module
 * Input: pzg
 * Output: N/A
 * Return: 
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_module_free()
{    
    if (NULL != g_pszRecBuf)
    {
        free(g_pszRecBuf);
        g_pszRecBuf = NULL;
    }
    
    logging_free_queue();
    LOGGING_LOGCNT_ZERO;
    logging_server_free();
    g_bInitialized = 0;
    _logging_terminal_monitor_free();
    logging_process_cfgfile(NULL, E_ACTION_SHUTDOWN);
    return 0;
}

/*******************************************************************************
 * Name: logging_module_init
 * Purpose: initialize logging module
 * Input: pzg
 * Output: N/A
 * Return: 
 *   Success: 0
 *   Failed: -1
 * Note: 
 ******************************************************************************/
int logging_module_init()
{
    if (1 == g_bInitialized) {
        return -1;
    }

    TAILQ_INIT(&g_loggingBufferHead);
    memset(&g_loggingStat, 0x00, sizeof(S_LogStatistics));

#if 0
    pCfg->nFileSyncInterval = LOGGING_FILESYNC_DEFAULT;
    pCfg->nDiagFacility = LOG_LOCAL7;
    pCfg->nDiagSeverity = LOG_WARNING;
    
    /* Modified by zhoucheng 2007-12-23 20:17, bug 3867 */
    pCfg->nOperateEnable = LOGGING_OPERATE_DEFAULT_STATE;
#endif

    /* malloc 1 more byte to hold '\0' */
    g_pszRecBuf = malloc(LOGGING_LINEBUF_SIZE + 1);
    if (NULL == g_pszRecBuf) {
        return -1;
    }
    
    if (logging_server_init() != 0) {
        logging_module_free();
        log_sys(M_MOD_CFG, E_ERROR,"initialize unix socket failed.");
        return -1;
    }
    
    g_bInitialized = 1;
    return 0;
}

/*******************************************************************************
 * Name:    logging_merge_sync
 * Purpose: synchronize message from syslog-ng merge buffer
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int logging_merge_sync()
{
    FILE * fp = NULL;
    char pid_buf[32] = {0};
    int pid = 0;

    /* first we get the syslog-ng daemon's pid */
    if (NULL == (fp = fopen(SYSLOG_NG_PATH_PIDFILE, "r"))) {
        return -1;
    }
    if (NULL != fgets(pid_buf, 32, fp)) {
        if ((pid = atoi(pid_buf)) != 0) {
            /* send a SIGUSR2 signal to syslog-ng daemon,
               then the syslog-ng will catch and handle it */
            kill(pid, SIGUSR2);
        }
    }
    fclose(fp);

    return 0;
}

/*add by chenzhonghua for bug 14173£¬ 2011-05-04*/
/*******************************************************************************
 * Name:    redirect_startup_log
 * Purpose: this function is used to redirect the system start up log into
 *          another path
 * Input: void
 * Output: N/A
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int redirect_startup_log(void)
{
    const char function[128] = "redirect_startup_log";
    S_LogItem *pItem = NULL;
    FILE * fp = NULL;
    char real_log[2*LOGGING_MAX_MSG_SIZE+1];

    fp = fopen(LOGGING_LOG_REDIRECT_PATH, "wb");
    if (NULL == fp) {
        log_sys(M_MOD_CFG, E_WARNING, "[%s] open system-start-up log failure.", function);
        return -1;
    }

    TAILQ_FOREACH(pItem, &g_loggingBufferHead, entry) {
        if (-1 == logging_copy_meta_str(real_log, pItem->pszLog, 
                        2*LOGGING_MAX_MSG_SIZE+1)) {
            continue;
        }        
        fputs(real_log, fp);
    }
    fclose(fp);

    return 0;
}

int logging_server_addr_find(tbl_syslog_cfg_t *pCfg, int vrfid, const char *pSrvAddr)
{
    char szBuf[M_SRV_ADDR_BUF_SZ];

    assert (NULL != pSrvAddr);

    if ('\0' == pCfg->server_addr[0]) {
        return 0;
    }
    
    snprintf(szBuf, M_SRV_ADDR_BUF_SZ, "%d,%s;", vrfid, pSrvAddr);
    if (NULL != strstr(pCfg->server_addr, szBuf)) {
        return 1;
    }
    
    return 0;
}

int logging_server_addr_add(tbl_syslog_cfg_t *pCfg, int vrfid, const char *pSrvAddr)
{
    char szBuf[M_SRV_ADDR_BUF_SZ];
    int len;

    assert (NULL != pSrvAddr);
    snprintf(szBuf, M_SRV_ADDR_BUF_SZ, "%d,%s;", vrfid, pSrvAddr);
    if ('\0' != pCfg->server_addr[0]) {    
        if (NULL != strstr(pCfg->server_addr, szBuf)) {
            return -1;
        }
        len = strlen(pCfg->server_addr);
    } else { 
        len = 0;
    }

    snprintf(pCfg->server_addr + len, M_SRV_ADDR_BUF_SZ - len, "%s", szBuf);
    return 0;
}

int logging_server_addr_del(tbl_syslog_cfg_t *pCfg, int vrfid, const char *pSrvAddr)
{
    char szBuf[M_SRV_ADDR_BUF_SZ];
    char *p = NULL;
    int totallen;
    int len;
    
    assert (NULL != pSrvAddr);

    if ('\0' == pCfg->server_addr[0]) {
        return -1;
    }
    len = snprintf(szBuf, M_SRV_ADDR_BUF_SZ, "%d,%s;", vrfid, pSrvAddr);
    p = strstr(pCfg->server_addr, szBuf);
    if (NULL == p) {
        return -1;
    }
    totallen = strlen(pCfg->server_addr);
    if (len == totallen) {
        /* only the last record */
//        pCfg->server_addr[0] = '\0';
        memset(pCfg->server_addr, 0x00, strlen(pCfg->server_addr));
        return 0;
    } 

    if (p == pCfg->server_addr) {
        /* at the header */
        snprintf(szBuf, M_SRV_ADDR_BUF_SZ, "%s", p + len);
        snprintf(pCfg->server_addr, M_SRV_ADDR_BUF_SZ, "%s", szBuf);
    } else if ((p + len) == (pCfg->server_addr + totallen)) {
        /* at the tailer */
        pCfg->server_addr[p - pCfg->server_addr] = '\0';
    } else {
        snprintf(szBuf, M_SRV_ADDR_BUF_SZ, "%s", p + len);
        pCfg->server_addr[p - pCfg->server_addr] = '\0';
        snprintf(pCfg->server_addr + (p - pCfg->server_addr), 
            M_SRV_ADDR_BUF_SZ - (p - pCfg->server_addr),
            "%s", szBuf);
    }
    
    return 0;
}

/*
 * return values:
 *   0: success
 *  -1: error
 */
int 
logging_server_add_server_address(tbl_syslog_cfg_t * pCfg, 
    int vrf, char *ipaddr, char *pszErrMsg, size_t nSize)
{
    gensock_t su;
    
    assert(vrf >= 0 && NULL != ipaddr);
    if (g_serverCnt >= LOGGING_SERVER_MAX) {
        snprintf(pszErrMsg, nSize, "Too many logging server address");
        return -1;
    }

    if (-1 == str2gensock(&su, ipaddr)) {
        snprintf(pszErrMsg, nSize, "Invalid IP address");
        return -1;
    }
    
    /* Added by zhoucheng 2008-04-02 16:38, bug 4825 */
    if (logging_check_ip(&su)) {
        snprintf(pszErrMsg, nSize, "Not supported IP address");
        return -1;
    }

    /* is it exists already? */
    if (logging_server_addr_find(pCfg, vrf, (const char *)ipaddr)) {
        snprintf(pszErrMsg, nSize, "Server IP address exist error");
        return -1;
    }

    logging_server_addr_add(pCfg, vrf, (const char *)ipaddr);
    g_serverCnt++;
    return 0;
}

int 
logging_server_del_server_address(tbl_syslog_cfg_t * pCfg, 
    int vrf, char *ipaddr, char *pszErrMsg, size_t nSize)
{
    gensock_t su;

    if (-1 == str2gensock(&su, ipaddr)) {
        snprintf(pszErrMsg, nSize, "Invalid IP address");
        return -1;
    }


    if (logging_server_addr_del(pCfg, vrf, (const char*)ipaddr) != 0) {
        snprintf(pszErrMsg, nSize, "No such address");
        return -1;
    }
    
    g_serverCnt--;
    return 0;
}


S_LogStatistics*
logging_get_stat()
{
    return &g_loggingStat;
}

int logging_clear_buf()
{
    logging_free_queue();
    memset(&g_loggingStat, 0x00, sizeof(S_LogStatistics));
    return 0;
}

int logging_set_bufsz(int lines)
{
    S_LogItem *pItem = NULL;
    
    while (LOGGING_LOGCNT_VAR > lines)
    {
        /* delete the oldest one */
        pItem = TAILQ_FIRST(&g_loggingBufferHead);
        TAILQ_REMOVE(&g_loggingBufferHead, pItem, entry);

        /* free it */
        logging_free_logitem(pItem);
        pItem = NULL;

        LOGGING_LOGCNT_DEC;
    }

    return 0;
}
