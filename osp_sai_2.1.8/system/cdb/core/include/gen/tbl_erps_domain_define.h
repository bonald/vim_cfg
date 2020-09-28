
#ifndef __TBL_ERPS_DOMAIN_DEFINE_H__
#define __TBL_ERPS_DOMAIN_DEFINE_H__

/* TBL_ERPS_DOMAIN field defines */
typedef enum
{
    TBL_ERPS_DOMAIN_FLD_KEY                  = 0 ,  /* READ */
    TBL_ERPS_DOMAIN_FLD_NAME                 = 1 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_INSTANCE_ID_NUM      = 2 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_INSTANCE_ID          = 3 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_PRIMARY_CONTROL_VLAN_ID = 4 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_SUB_CONTROL_VLAN_ID  = 5 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_HELLO_TIMER_INTERVAL = 6 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_FAILURE_TIMER_INTERVAL = 7 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_PRE_FORWARDING_TIMER_INTERVAL = 8 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_FLUSH_TIMER_INTERVAL = 9 ,  /* RW */
    TBL_ERPS_DOMAIN_FLD_EDGE_HELLO_TIMER_INTERVAL = 10,  /* RW */
    TBL_ERPS_DOMAIN_FLD_EDGE_FAILURE_TIMER_INTERVAL = 11,  /* RW */
    TBL_ERPS_DOMAIN_FLD_ERPS_ENABLE          = 12,  /* RW */
    TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST       = 13,  /* RW */
    TBL_ERPS_DOMAIN_FLD_MAX                  = 14
} tbl_erps_domain_field_id_t;

/* TBL_ERPS_DOMAIN defines */
typedef struct
{
    uint32               domain_id;
} tbl_erps_domain_key_t;

typedef struct
{
    tbl_erps_domain_key_t key;
    char                 name[ERPS_MAX_NAME_LEN];
    uint32               instance_id_num;
    uint32               instance_id[ERPS_MAX_INSTANCE];
    uint32               primary_control_vlan_id;
    uint32               sub_control_vlan_id;
    uint32               hello_timer_interval;
    uint32               failure_timer_interval;
    uint32               pre_forwarding_timer_interval;
    uint32               flush_timer_interval;
    uint32               edge_hello_timer_interval;
    uint32               edge_failure_timer_interval;
    uint32               erps_enable;
    void                 *erps_primary_ring;  /* refer to tbl_erps_ring_t */
    cdb_reference_list_t erps_ring_list;      /* list of erps rings for this domain */
} tbl_erps_domain_t;

typedef struct
{
    tbl_erps_domain_t    *domain_array[GLB_MAX_ERPS_DOMAIN];
} tbl_erps_domain_master_t;

#endif /* !__TBL_ERPS_DOMAIN_DEFINE_H__ */

