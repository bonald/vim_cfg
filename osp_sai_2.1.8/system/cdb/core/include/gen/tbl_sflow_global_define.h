
#ifndef __TBL_SFLOW_GLOBAL_DEFINE_H__
#define __TBL_SFLOW_GLOBAL_DEFINE_H__

/* TBL_SFLOW_GLOBAL field defines */
typedef enum
{
    TBL_SFLOW_GLOBAL_FLD_ENABLE               = 0 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_VERSION              = 1 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_AGENT                = 2 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_AGENT_IPV6           = 3 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL     = 4 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_COUNTER_PORT_NUM     = 5 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_COUNTER_TIMER        = 6 ,  /* READ */
    TBL_SFLOW_GLOBAL_FLD_COUNTER_NEXT_PORT    = 7 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_SFLOW_ALL            = 8 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_SFLOW_COUNTER        = 9 ,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_SFLOW_SAMPLE         = 10,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_SFLOW_PACKET         = 11,  /* RW */
    TBL_SFLOW_GLOBAL_FLD_MAX                  = 12
} tbl_sflow_global_field_id_t;

/* TBL_SFLOW_GLOBAL defines */
typedef struct
{
    uint32               enable;              /* global sflow enable */
    uint32               version;             /* sflow version,v4 or v5*/
    addr_t               agent;               /* agent ip address */
    addr_t               agent_ipv6;          /* agent ipv6 address */
    uint32               counter_interval;    /* sflow counter polling interval*/
    uint32               counter_port_num;    /* sflow counter sampling enabled port num*/
    ctc_task_t           *counter_timer;      /* sflow counter timer */
    uint32               counter_next_port;   /* get next port stats when sflow_counter_timer*/
    uint32               sflow;               /* bitmap of SFLOW_FLAG_ */
} tbl_sflow_global_t;

#endif /* !__TBL_SFLOW_GLOBAL_DEFINE_H__ */

