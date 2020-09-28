/****************************************************************************
 * $Id$
 *  Define unified log interface for log api
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : zhoucheng
 * Date          : 2007-11-1 10:18
 * Reason        : First Create.
 *****************************************************************************/
#ifndef _GENLOG_H_
#define _GENLOG_H_
#include "genlog_id.h"
#include "genlog_usr.h"

#if 0
#define HAVE_GENLOG_DEBUG
#endif

#define INVALID_LOGID           (-1)

/*
 * arguments to setlogmask.
 */
#define LOG_MASK(pri)           (1 << (pri))            /* mask for one priority */
#define LOG_UPTO(pri)           ((1 << ((pri)+1)) - 1)  /* all priorities through pri */

#define LOG_PRIMASK             0x07                    /* mask to extract priority part (internal) */
#define LOG_PRI(p)              ((p) & LOG_PRIMASK)     /* extract priority */
//#define LOG_MAKEPRI(fac, pri)   (((fac) << 3) | (pri))

/*
 * Severity type
 */
typedef enum
{
    E_EMERGENCY     = 0,    /* system is unusable */
    E_ALERT         = 1,    /* action must be taken immediately */
    E_CRITICAL      = 2,    /* critical conditions */
    E_ERROR         = 3,    /* error conditions */
    E_WARNING       = 4,    /* warning conditions */
    E_NOTICE        = 5,    /* normal but significant condition */
    E_INFORMATIONAL = 6,    /* informational */
    E_DEBUG         = 7,    /* debug-level messages */
    E_MAX
} E_SEVERITY;

/* facility codes */
#define LOG_KERN        (0<<3)  /* kernel messages */
#define LOG_USER        (1<<3)  /* random user-level messages */
#define LOG_MAIL        (2<<3)  /* mail system */
#define LOG_DAEMON      (3<<3)  /* system daemons */
#define LOG_AUTH        (4<<3)  /* security/authorization messages */
#define LOG_SYSLOG      (5<<3)  /* messages generated internally by syslogd */
#define LOG_LPR         (6<<3)  /* line printer subsystem */
#define LOG_NEWS        (7<<3)  /* network news subsystem */
#define LOG_UUCP        (8<<3)  /* UUCP subsystem */
#define LOG_CRON        (9<<3)  /* clock daemon */
#define LOG_AUTHPRIV    (10<<3) /* security/authorization messages (private) */
#define LOG_FTP         (11<<3) /* ftp daemon */

/* other codes through 15 reserved for system use */
#define LOG_LOCAL0      (16<<3) /* reserved for local use */
#define LOG_LOCAL1      (17<<3) /* reserved for local use */
#define LOG_LOCAL2      (18<<3) /* reserved for local use */
#define LOG_LOCAL3      (19<<3) /* reserved for local use */
#define LOG_LOCAL4      (20<<3) /* reserved for local use */
#define LOG_LOCAL5      (21<<3) /* reserved for local use */
#define LOG_LOCAL6      (22<<3) /* reserved for local use */
#define LOG_LOCAL7      (23<<3) /* reserved for local use */

#define LOG_NFACILITIES 24  /* current number of facilities */
#define LOG_FACMASK     0x03f8  /* mask to extract facility part */
#define LOG_FAC(p)      (((p) & LOG_FACMASK) >> 3) /* facility of pri */

/*
 * Option flags for openlog.
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define LOG_PID         0x01    /* log the pid with each message */
#define LOG_CONS        0x02    /* log on the console if errors in sending */
#define LOG_ODELAY      0x04    /* delay open until first syslog() (default) */
#define LOG_NDELAY      0x08    /* don't delay open */
#define LOG_NOWAIT      0x10    /* don't wait for console forks: DEPRECATED */
#define LOG_PERROR      0x20    /* log to stderr as well */

/* alarm log prefix */
#define ALRM_LOG_PREFIX "-~ALRM~-"

#define LOG_FUNC_NAME_MAXLEN  128

/*defination for logging interval*/
#define GLB_MAX_LOGGING_INTERVAL 120

#define LOG_MSG_SIZE sizeof(log_message_t)

typedef unsigned int  CMD_LEVEL;

/*modified by weij for bug 18671, 2012-03-20*/
typedef enum
{
    M_LOG_FREQ_INTVAL_1 = 1,    /* 1 seconds */
    M_LOG_FREQ_INTVAL_2 = 30,   /* 30 seconds */
    M_LOG_FREQ_INTVAL_3 = 1800, /* 30 miniutes */
    M_LOG_FREQ_INTVAL_4 = 3600, /* 1 hours */
} E_LOG_RATE_LIMIT_INTVAL;

typedef struct
{
    int     log_type;
    int     log_interval;
    int     next_sec;
    int     suppress_cnt;
} log_message_rate_limit_t;

/* Message structure. */
typedef struct
{
    int key;
    char *str;
} log_message_t;

/*
 * Operating type
 */
typedef enum
{
    OPRT_SUCCESS = 0,
    OPRT_FAIL = 1,
} E_OPRT_RLT;


/*modified by weij for bug 18447, 2012-03-22*/
typedef enum log_rate_limit_type_e
{
    LOG_RATE_LIMIT_TYPE_DEF = 0, /*no limit*/
    LOG_RATE_LIMIT_TYPE_LIB_EXC_MEM_THRESHOLD,
    LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,  /*modified by liuy for bug 19756, 2012-7-4*/
    LOG_RATE_LIMIT_TYPE_MAX
} log_rate_limit_type_t;

/*
 * Macros for module
 * Notice: please keep the order with gModuleNameArr
 */
typedef enum
{
    M_MOD_UNKNOWN = 0,  /* UNKNOWN */
    M_MOD_LOG,          /* LOG */
    M_MOD_LIB,          /* LIB */
    M_MOD_NETLINK,      /* NETLINK */
    
    M_MOD_INTF,         /* INTERFACE */
    M_MOD_VLAN,         /* VLAN */
    M_MOD_FDB,          /* FDB */
    M_MOD_LAG,          /* LAG */
    M_MOD_LACP,         /* LACP */
    M_MOD_MSTP,         /* MSTP */
    M_MOD_ROUTE,        /* ROUTE */
    M_MOD_ARP,          /* ARP */
    M_MOD_IGMP,          /* IGMP */
    M_MOD_IPMC, 		 /* Ipmc */
    M_MOD_IGSP,          /* IGMP Snooping */
    M_MOD_LLDP,          /* LLDP*/
    
    M_MOD_CFG,          /* CFG */
    M_MOD_CDS,          /* CDS */
    M_MOD_CHSM,         /* CHSM */
    M_MOD_FEI,          /* FEI */
    M_MOD_HSRV,         /* HSRV */
    M_MOD_LSRV,         /* LSRV */
    M_MOD_HAGT,         /* HAGT */
    M_MOD_LCM,          /* LCM */
    M_MOD_LCSH,         /* LCSH */
    M_MOD_SDK,          /* SDK */
    M_MOD_DRV,          /* DRV */
    M_MOD_APP,          /* APP */
    M_MOD_IPSOUR,       /* IPSOURCE */
    M_MOD_DHCPRLY,      /* DHCP relay */
    M_MOD_DHCPCLI,      /* DHCP client */
    M_MOD_DHCPSNOOPING, /* DHCP snooping */
    M_MOD_DHCPSNOOPING6, /* DHCP snooping v6 */
    M_MOD_DHCPRLY6,     /* DHCPv6 relay */    
    M_MOD_DHCLIENT,      /* DHCP relay */
    M_MOD_DHCLIENT6,      /* DHCP relay */
    M_MOD_VLAN_SECURITY,/* VLAN Security */
    M_MOD_MLAG,         /* MLAG */
    M_MOD_INTERNAL,         /* INTERNAL */
    M_MOD_SAI,          /* SAI */
    M_MOD_AUTHD,        /* AUTHD */
    M_MOD_BHM,          /* BHM */
    M_MOD_ARPINSPECTION,/* ARP inspection*/
    M_MOD_SFLOW,        /* SFLOW */
    M_MOD_OPENFLOW,     /* OPENFLOW */
    M_MOD_VRRP,         /* VRRP */
    M_MOD_SSM,          /* SSM */
    M_MOD_IPSLA,        /* IPSLA */
    M_MOD_TRACK,        /* TRACK */
    M_MOD_PTP,         /* PTP */
    M_MOD_G8131,       /* G8131 */
    M_MOD_TPOAM,       /* TPOAM */
    M_MOD_ND,             /* Neighbor Discovery */
    M_MOD_NCS,          /* NETCONF Service */
    M_MOD_SNMPAGT,      /* SNMP Agent */
    M_MOD_RMON,      /* RMON */
    M_MOD_DHCPD6,      /* DHCP relay */
    M_MOD_NAT,         /* NAT*/
    M_MOD_G8032,         /* G8032*/
	/*add by zhw for multicast*/
    M_MOD_PIMSM, 		/*PIM-SM*/
    M_MOD_PIMDM,		/*PIM-DM*/
    M_MOD_MAX           /* Max module number */
} E_MODULE;

/* module name pair struct */
typedef struct
{
    E_MODULE eNo; /* module no */
    const char *pszName; /* name */
} S_MODULE_NAME_PAIR;

/* Open&Close the log module for commucination with syslog-ng daemon */
extern void genlog_open(void);
extern void genlog_close(void);

/* Add one system log */
extern void log_sys(E_MODULE eModule, E_SEVERITY eSeverity, const char *fmt, ...);

/* Add one operating log */
extern void log_oper(E_MODULE eModule, E_SEVERITY eSeverity, CMD_LEVEL cl, E_OPRT_RLT rlt, const char *fmt, ...);

/* Add one device user log */
extern void log_diag(E_MODULE eModule, E_SEVERITY eSeverity, const char *fmt, ...);

/* Get the access client ip address and user name */
extern void log_init_line(const char * ip_addr, const char * usr_name);

extern void log_set_nonblocking(int set);

/* Ratelimit */
extern void log_sys_rate_limit(E_MODULE eModule, E_SEVERITY eSeverity, int eFreq, const char *fmt, ...);
extern void log_diag_rate_limit(E_MODULE eModule, E_SEVERITY eSeverity, int rateLimitType, const char *fmt, ...);
extern log_message_rate_limit_t *log_get_rate_limit_arr(void);

/* log_sys with timestamp */
#define LOG_SYS_TS(eModule, eSeverity, fmt, args...) \
do {\
    struct timeval tv; \
    ctc_task_update_boottime(&tv); \
    log_sys(eModule, eSeverity, "[TS %u.%06u] "fmt, (uint32)tv.tv_sec, (uint32)tv.tv_usec, ##args); \
} while (0)

typedef struct
{
    int         logid;
    int         severity;
    char       *logid_str;
    char       *format_str;
} osp_log_info_t;

void
logid_sys(E_LOGID logid, ...);

const char*
logid_get_desc(E_LOGID logid);

void
logid_register_cb_func(LOG_CB_FUNC func);

void logit(int eSeverity, int syserr, const char *fmt, ...);
#endif  /* !_GENLOG_H_ */
