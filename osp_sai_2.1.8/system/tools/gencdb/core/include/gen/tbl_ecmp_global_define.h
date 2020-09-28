
#ifndef __TBL_ECMP_GLOBAL_DEFINE_H__
#define __TBL_ECMP_GLOBAL_DEFINE_H__

/* TBL_ECMP_GLOBAL field defines */
typedef enum
{
    TBL_ECMP_GLOBAL_FLD_HASH_KEY             = 0 ,  /* RW */
    TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM       = 1 ,  /* RW */
    TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE    = 2 ,  /* RW */
    TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE        = 3 ,  /* RW */
    TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM        = 4 ,  /* RW */
    TBL_ECMP_GLOBAL_FLD_MAX                  = 5 
} tbl_ecmp_global_field_id_t;

/* TBL_ECMP_GLOBAL defines */
typedef struct
{
    uint32               hash_key;
    uint32               hash_algorithm;
    uint32               load_balance_mode;
    uint32               dlb_mode_type;
    uint32               rr_prefix_num;
} tbl_ecmp_global_t;

#endif /* !__TBL_ECMP_GLOBAL_DEFINE_H__ */

