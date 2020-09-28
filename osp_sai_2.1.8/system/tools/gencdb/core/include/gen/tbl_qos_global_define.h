
#ifndef __TBL_QOS_GLOBAL_DEFINE_H__
#define __TBL_QOS_GLOBAL_DEFINE_H__

/* TBL_QOS_GLOBAL field defines */
typedef enum
{
    TBL_QOS_GLOBAL_FLD_QOS_ENABLE           = 0 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_PHB_ENABLE           = 1 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE = 2 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_POLICER_STATS_ENABLE = 3 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_CUR_CPU_RATE         = 4 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_DEF_CPU_RATE         = 5 ,  /* READ */
    TBL_QOS_GLOBAL_FLD_LATENCY_RANGE        = 6 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE  = 7 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH    = 8 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_LOW     = 9 ,  /* RW */
    TBL_QOS_GLOBAL_FLD_CHECK_QOS_ENABLE     = 10,  /* RW */
    TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE       = 11,  /* RW */
    TBL_QOS_GLOBAL_FLD_IN_PORT_POLICER_RES_CNT = 12,  /* RW */
    TBL_QOS_GLOBAL_FLD_EG_PORT_POLICER_RES_CNT = 13,  /* RW */
    TBL_QOS_GLOBAL_FLD_IN_FLOW_POLICER_RES_CNT = 14,  /* RW */
    TBL_QOS_GLOBAL_FLD_EG_FLOW_POLICER_RES_CNT = 15,  /* RW */
    TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE   = 16,  /* RW */
    TBL_QOS_GLOBAL_FLD_MAX                  = 17
} tbl_qos_global_field_id_t;

/* TBL_QOS_GLOBAL defines */
typedef struct
{
    uint8                qos_enable;
    uint8                phb_enable;
    uint8                port_policer_first_enable;
    uint8                policer_stats_enable;
    uint32               cur_cpu_rate;        /* total cpu rate*/
    uint32               def_cpu_rate;        /* total cpu rate*/
    uint32               latency_range[GLB_MONITOR_LATENCY_RANGE_ID_MAX]; /* latency event 8 ranges */
    uint32               buffer_event_enable; /* enable buffer event */
    uint32               buffer_event_high;   /* set buffer event high threshold  */
    uint32               buffer_event_low;    /* set buffer event low threshold  */
    uint32               check_qos_enable;    /*check qos enable*/
    uint8                qos_ipg_enable;      /* enable qos policing/shaping include ipg */
    uint32               in_port_policer_res_cnt; /* tbl_qos_policer_res_t */
    uint32               eg_port_policer_res_cnt; /* tbl_qos_policer_res_t */
    uint32               in_flow_policer_res_cnt; /* tbl_qos_policer_res_t */
    uint32               eg_flow_policer_res_cnt; /* tbl_qos_policer_res_t */
    uint32               queue_shape_enable;  /* queue shape enable */
} tbl_qos_global_t;

#endif /* !__TBL_QOS_GLOBAL_DEFINE_H__ */

