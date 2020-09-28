
#ifndef __TBL_SNMP_CFG_DEFINE_H__
#define __TBL_SNMP_CFG_DEFINE_H__

/* TBL_SNMP_CFG field defines */
typedef enum
{
    TBL_SNMP_CFG_FLD_ENABLE               = 0 ,  /* RW */
    TBL_SNMP_CFG_FLD_SERVER_ENABLE        = 1 ,  /* RW */
    TBL_SNMP_CFG_FLD_VERSION              = 2 ,  /* RW */
    TBL_SNMP_CFG_FLD_ENGINEID             = 3 ,  /* RW */
    TBL_SNMP_CFG_FLD_CONTACT              = 4 ,  /* RW */
    TBL_SNMP_CFG_FLD_LOCATION             = 5 ,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_COLDSTART = 6 ,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_WARMSTART = 7 ,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKDOWN = 8 ,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_LINKUP   = 9 ,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_SYSTEM   = 10,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_VRRP     = 11,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_MASTER_CHANGE = 12,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_STATE_CHANGE = 13,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_PTP_ACCURACY_CHANGE = 14,  /* RW */
    TBL_SNMP_CFG_FLD_TRAP_ENABLE_ALL      = 15,  /* RW */
    TBL_SNMP_CFG_FLD_WATCH_RESTART_COUNT  = 16,  /* READ */
    TBL_SNMP_CFG_FLD_AGENT_STATS          = 17,  /* RW */
    TBL_SNMP_CFG_FLD_T_SNMP_TRAP_ALL_TIMER = 18,  /* READ */
    TBL_SNMP_CFG_FLD_T_SNMP_ENABLE_TIMER  = 19,  /* READ */
    TBL_SNMP_CFG_FLD_MAX                  = 20
} tbl_snmp_cfg_field_id_t;

/* TBL_SNMP_CFG defines */
typedef struct
{
    uint8                enable;              /* Default process is enable */
    uint8                server_enable;       /* Enable / Disable agent */
    uint8                version;             /* Agent version: all, v1, v2c, v3 */
    char                 engineid[SNMP_ENGINEID_SIZE]; /* engine id hex string */
    snmp_info_t          contact;             /* system contact */
    snmp_info_t          location;            /* system location */
    uint32               trap_enable;         /* bitmap of GLB_TRAP_TYPE_ */
    uint32               watch_restart_count;
    uint32               agent_stats;         /* 0: clear stats, 1: show stats */
    ctc_task_t           *t_snmp_trap_all_timer;
    ctc_task_t           *t_snmp_enable_timer;
} tbl_snmp_cfg_t;

#endif /* !__TBL_SNMP_CFG_DEFINE_H__ */

