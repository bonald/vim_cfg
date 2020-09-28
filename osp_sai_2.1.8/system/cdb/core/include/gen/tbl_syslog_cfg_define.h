
#ifndef __TBL_SYSLOG_CFG_DEFINE_H__
#define __TBL_SYSLOG_CFG_DEFINE_H__

/* TBL_SYSLOG_CFG field defines */
typedef enum
{
    TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER     = 0 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE       = 1 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP       = 2 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG       = 3 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_LOGGING_LINES        = 4 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_SERVER_ADDR          = 5 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_SERVER_FACILITY      = 6 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY      = 7 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_TRAP_FACILITY        = 8 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY        = 9 ,  /* RW */
    TBL_SYSLOG_CFG_FLD_TIMESTAMP            = 10,  /* RW */
    TBL_SYSLOG_CFG_FLD_FILE_SEVERITY        = 11,  /* RW */
    TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY      = 12,  /* RW */
    TBL_SYSLOG_CFG_FLD_ENABLE_MERGE         = 13,  /* RW */
    TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE       = 14,  /* RW */
    TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT        = 15,  /* RW */
    TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE       = 16,  /* RW */
    TBL_SYSLOG_CFG_FLD_MAX                  = 17
} tbl_syslog_cfg_field_id_t;

/* TBL_SYSLOG_CFG defines */
typedef struct
{
    uint8                enable_to_server;    /* enable send log to server */
    uint8                enable_to_file;      /* enable send log to local file */
    uint8                enable_to_trap;      /* enable send log to trap */
    uint8                enable_to_diag;      /* enable send log to diag */
    uint16               logging_lines;       /* logging buffer size <10-1000> */
    char                 server_addr[M_SRV_ADDR_BUF_SZ]; /* remote server ip address, format: vrfid,addr;vrfid2,addr2;vrfid3,addr3 */
    uint8                server_facility;     /* facility for logs send to server */
    uint8                server_severity;     /* severity for logs send to server */
    uint8                trap_facility;       /* facility for traps send to server */
    uint8                trap_severity;       /* severity for logs for trap */
    uint8                timestamp;           /* timestamp option of the log */
    uint8                file_severity;       /* severity for logs send to local file */
    uint8                module_severity;     /* severity for logs send to logging buffer */
    uint8                enable_merge;        /* enable logging merge */
    uint8                enable_operate;      /* operate enable */
    uint16               merge_timeout;       /* merge timeout <1-300> seconds */
    uint16               merge_fifosize;      /* merge fifo-size <100-10240> */
} tbl_syslog_cfg_t;

#endif /* !__TBL_SYSLOG_CFG_DEFINE_H__ */

