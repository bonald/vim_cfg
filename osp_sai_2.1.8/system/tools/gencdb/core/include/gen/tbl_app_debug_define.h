
#ifndef __TBL_APP_DEBUG_DEFINE_H__
#define __TBL_APP_DEBUG_DEFINE_H__

/* TBL_APP_DEBUG field defines */
typedef enum
{
    TBL_APP_DEBUG_FLD_SNMP_AGENT           = 0 ,  /* RW */
    TBL_APP_DEBUG_FLD_RMON_EVENT           = 1 ,  /* RW */
    TBL_APP_DEBUG_FLD_RMON_ALARM           = 2 ,  /* RW */
    TBL_APP_DEBUG_FLD_RMON_STATS           = 3 ,  /* RW */
    TBL_APP_DEBUG_FLD_RMON_HISTORY         = 4 ,  /* RW */
    TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL      = 5 ,  /* RW */
    TBL_APP_DEBUG_FLD_NETCONF_RPC          = 6 ,  /* RW */
    TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR    = 7 ,  /* RW */
    TBL_APP_DEBUG_FLD_NETCONF_DEBUG        = 8 ,  /* RW */
    TBL_APP_DEBUG_FLD_MAX                  = 9 
} tbl_app_debug_field_id_t;

/* TBL_APP_DEBUG defines */
typedef struct
{
    uint32               snmp;                /* bitmap of APPDBG_FLAG_SNMP_ */
    uint32               rmon;                /* bitmap of APPDBG_FLAG_RMON_ */
    uint32               ntp;                 /* bitmap of APPDBG_FLAG_RMON_ */
    uint32               netconf;             /* bitmap of APPDBG_FLAG_NETCONF_ */
} tbl_app_debug_t;

#endif /* !__TBL_APP_DEBUG_DEFINE_H__ */

