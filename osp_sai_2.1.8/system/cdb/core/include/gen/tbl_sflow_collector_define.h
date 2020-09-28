
#ifndef __TBL_SFLOW_COLLECTOR_DEFINE_H__
#define __TBL_SFLOW_COLLECTOR_DEFINE_H__

/* TBL_SFLOW_COLLECTOR field defines */
typedef enum
{
    TBL_SFLOW_COLLECTOR_FLD_KEY                  = 0 ,  /* READ */
    TBL_SFLOW_COLLECTOR_FLD_BUF                  = 1 ,  /* RW */
    TBL_SFLOW_COLLECTOR_FLD_PKT_LEN              = 2 ,  /* RW */
    TBL_SFLOW_COLLECTOR_FLD_SENDFD               = 3 ,  /* RW */
    TBL_SFLOW_COLLECTOR_FLD_TRANSDELAY_TIMER     = 4 ,  /* READ */
    TBL_SFLOW_COLLECTOR_FLD_SEQUENCE             = 5 ,  /* RW */
    TBL_SFLOW_COLLECTOR_FLD_T_LOG_TIMER          = 6 ,  /* READ */
    TBL_SFLOW_COLLECTOR_FLD_MAX                  = 7 
} tbl_sflow_collector_field_id_t;

/* TBL_SFLOW_COLLECTOR defines */
typedef sflow_collector_key_t tbl_sflow_collector_key_t;

typedef struct
{
    tbl_sflow_collector_key_t key;
    uint8                buf[GLB_SFLOW_PKT_BUF+4]; /* sflow packet buffer */
    uint32               pkt_len;             /* sflow packet length */
    uint32               sendfd;              /* collector socket */
    ctc_task_t           *transdelay_timer;   /* sflow trasnmit dealy timer */
    uint32               sequence;            /* sflow packet header sequence */
    ctc_task_t           *t_log_timer;
} tbl_sflow_collector_t;

typedef struct
{
    ctclib_slist_t       *sflow_collector_list;
} tbl_sflow_collector_master_t;

#endif /* !__TBL_SFLOW_COLLECTOR_DEFINE_H__ */

