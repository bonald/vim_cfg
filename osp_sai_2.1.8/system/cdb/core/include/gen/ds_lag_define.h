
#ifndef __DS_LAG_DEFINE_H__
#define __DS_LAG_DEFINE_H__

/* DS_LAG field defines */
typedef enum
{
    DS_LAG_FLD_MODE                 = 0 ,  /* READ */
    DS_LAG_FLD_LAG_ID               = 1 ,  /* READ */
    DS_LAG_FLD_OPER_STATE           = 2 ,  /* READ */
    DS_LAG_FLD_MISMATCH             = 3 ,  /* READ */
    DS_LAG_FLD_BUNDLE_PORTS_COUNT   = 4 ,  /* READ */
    DS_LAG_FLD_LOAD_BALANCE_MODE    = 5 ,  /* RW */
    DS_LAG_FLD_IS_ACTIVE_BACKUP_MODE = 6 ,  /* RW */
    DS_LAG_FLD_MAX_ACTIVE_NUMBER    = 7 ,  /* RW */
    DS_LAG_FLD_CUR_ACTIVE_NUMBER    = 8 ,  /* RW */
    DS_LAG_FLD_CUR_ACTIVE_MAX_PRIORITY = 9 ,  /* RW */
    DS_LAG_FLD_MEMBER_PORTS         = 10,  /* READ */
    DS_LAG_FLD_MAX                  = 11
} ds_lag_field_id_t;

/* DS_LAG defines */
typedef struct
{
    uint8                mode;                /* glb_agg_mode_t */
    uint8                lag_id;
    uint8                oper_state;
    uint8                mismatch;            /* oper speed mismatch */
    int8                 bundle_ports_count;
    uint8                load_balance_mode;
    uint8                is_active_backup_mode;
    uint32               max_active_number;
    uint32               cur_active_number;
    uint32               cur_active_max_priority;
    cdb_reference_list_t member_ports;        /* only for LAG group */
} ds_lag_t;

#endif /* !__DS_LAG_DEFINE_H__ */

