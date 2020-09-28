
#ifndef __TBL_IGSP_GLOBAL_DEFINE_H__
#define __TBL_IGSP_GLOBAL_DEFINE_H__

/* TBL_IGSP_GLOBAL field defines */
typedef enum
{
    TBL_IGSP_GLOBAL_FLD_ENABLE               = 0 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_FAST_LEAVE           = 1 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN      = 2 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS      = 3 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_VERSION              = 4 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME = 5 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL       = 6 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_LMQI                 = 7 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_LMQC                 = 8 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR       = 9 ,  /* RW */
    TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER    = 10,  /* RW */
    TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER    = 11,  /* RW */
    TBL_IGSP_GLOBAL_FLD_TCN_ENABLE           = 12,  /* RW */
    TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT      = 13,  /* RW */
    TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL   = 14,  /* RW */
    TBL_IGSP_GLOBAL_FLD_TCN_QUERY_CURRENT_COUNT = 15,  /* READ */
    TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME = 16,  /* RW */
    TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE     = 17,  /* RW */
    TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM      = 18,  /* RW */
    TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC           = 19,  /* RW */
    TBL_IGSP_GLOBAL_FLD_ALL_HOSTS            = 20,  /* READ */
    TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS          = 21,  /* READ */
    TBL_IGSP_GLOBAL_FLD_V3_ROUTERS           = 22,  /* READ */
    TBL_IGSP_GLOBAL_FLD_T_GROUP_EXPIRE       = 23,  /* READ */
    TBL_IGSP_GLOBAL_FLD_T_TCN_QUERY          = 24,  /* READ */
    TBL_IGSP_GLOBAL_FLD_MAX                  = 25
} tbl_igsp_global_field_id_t;

/* TBL_IGSP_GLOBAL defines */
typedef struct
{
    uint8                enable;
    uint8                fast_leave;
    uint8                discard_unknown;
    uint8                report_suppress;
    uint32               version;
    uint32               querier_max_response_time; /* querier - max-response-time */
    uint32               query_interval;      /* querier - query internal */
    uint32               lmqi;                /* querier - last member query interval */
    uint32               lmqc;                /* querier - last member query count */
    uint32               robustness_var;
    uint32               max_member_number;
    uint32               curr_group_member;   /* current group member count */
    uint32               tcn_enable;
    uint32               tcn_query_count;
    uint32               tcn_query_interval;
    uint32               tcn_query_current_count;
    uint32               tcn_query_max_response_time;
    uint32               host_join_enable;    /* allow host join/leave */
    uint32               host_joined_num;     /* allow host join/leave */
    addr_ipv4_t          global_src;
    addr_ipv4_t          all_hosts;
    addr_ipv4_t          all_routers;
    addr_ipv4_t          v3_routers;
    ctc_task_t           *t_group_expire;     /* 1 seconds timer for group expire */
    ctc_task_t           *t_tcn_query;        /* TCN query timer */
} tbl_igsp_global_t;

#endif /* !__TBL_IGSP_GLOBAL_DEFINE_H__ */

