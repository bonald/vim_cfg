
#ifndef __TBL_LAG_GLOBAL_DEFINE_H__
#define __TBL_LAG_GLOBAL_DEFINE_H__

/* TBL_LAG_GLOBAL field defines */
typedef enum
{
    TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY      = 0 ,  /* RW */
    TBL_LAG_GLOBAL_FLD_SYSTEM_ID            = 1 ,  /* READ */
    TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY = 2 ,  /* READ */
    TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID       = 3 ,  /* READ */
    TBL_LAG_GLOBAL_FLD_LOAD_BALANCE         = 4 ,  /* RW */
    TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN = 5 ,  /* RW */
    TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC      = 6 ,  /* RW */
    TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE    = 7 ,  /* RW */
    TBL_LAG_GLOBAL_FLD_MAX                  = 8 
} tbl_lag_global_field_id_t;

/* TBL_LAG_GLOBAL defines */
typedef struct
{
    uint16               system_priority;
    mac_addr_t           system_id;
    uint16               mlag_system_priority;
    mac_addr_t           mlag_system_id;
    uint32               load_balance;
    uint32               load_balance_inner_field_en;
    uint8                hash_arithmetic;
    uint8                load_balance_mode[GLB_LAG_GROUP_NUM+1];
} tbl_lag_global_t;

#endif /* !__TBL_LAG_GLOBAL_DEFINE_H__ */

