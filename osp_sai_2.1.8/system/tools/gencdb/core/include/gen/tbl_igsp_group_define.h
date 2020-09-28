
#ifndef __TBL_IGSP_GROUP_DEFINE_H__
#define __TBL_IGSP_GROUP_DEFINE_H__

/* TBL_IGSP_GROUP field defines */
typedef enum
{
    TBL_IGSP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_IGSP_GROUP_FLD_RETX_GROUP_LMQC      = 1 ,  /* READ */
    TBL_IGSP_GROUP_FLD_RETX_GROUP_SOURCE_LMQC = 2 ,  /* READ */
    TBL_IGSP_GROUP_FLD_LIVENESS             = 3 ,  /* READ */
    TBL_IGSP_GROUP_FLD_HAS_RX_LEAVE         = 4 ,  /* READ */
    TBL_IGSP_GROUP_FLD_LAST_REPORTER_ADDRESS = 5 ,  /* READ */
    TBL_IGSP_GROUP_FLD_UPTIME               = 6 ,  /* READ */
    TBL_IGSP_GROUP_FLD_UPDATE_TIME          = 7 ,  /* READ */
    TBL_IGSP_GROUP_FLD_V1_UPDATE_TIME       = 8 ,  /* READ */
    TBL_IGSP_GROUP_FLD_IS_V1_HOST_EXIST     = 9 ,  /* READ */
    TBL_IGSP_GROUP_FLD_TYPE                 = 10,  /* READ */
    TBL_IGSP_GROUP_FLD_INACTIVE             = 11,  /* READ */
    TBL_IGSP_GROUP_FLD_IS_V3_LEAVE          = 12,  /* READ */
    TBL_IGSP_GROUP_FLD_T_RETX_QUERY         = 13,  /* READ */
    TBL_IGSP_GROUP_FLD_MAX                  = 14
} tbl_igsp_group_field_id_t;

/* TBL_IGSP_GROUP defines */
typedef igsp_group_key_t tbl_igsp_group_key_t;

typedef struct
{
    tbl_igsp_group_key_t key;
    uint32               retx_group_lmqc;
    uint32               retx_group_source_lmqc;
    uint32               liveness;
    uint32               has_rx_leave;        /* has received leave */
    addr_ipv4_t          last_reporter_address; /* last reporter address */
    sal_time_t           uptime;              /* first create time */
    sal_time_t           update_time;         /* last update time */
    sal_time_t           v1_update_time;      /* v1 host last update time */
    uint8                is_v1_host_exist;    /* host running igmpv1 exist */
    uint8                type;                /* igmp_type_t */
    uint8                inactive;            /* when global disable or vlan disable, static group should be inactive */
    uint8                is_v3_leave;         /* leave from v3 */
    ctc_task_t           *t_retx_query;
} tbl_igsp_group_t;

typedef struct
{
    ctclib_hash_t        *group_hash;
} tbl_igsp_group_master_t;

#endif /* !__TBL_IGSP_GROUP_DEFINE_H__ */

