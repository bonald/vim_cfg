
#ifndef __TBL_FEA_LAG_DEFINE_H__
#define __TBL_FEA_LAG_DEFINE_H__

/* TBL_FEA_LAG field defines */
typedef enum
{
    TBL_FEA_LAG_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_LAG_FLD_LAG_ID               = 1 ,  /* RW */
    TBL_FEA_LAG_FLD_MAX                  = 2 
} tbl_fea_lag_field_id_t;

/* TBL_FEA_LAG defines */
typedef struct
{
    uint32               id;
} tbl_fea_lag_key_t;

typedef struct
{
    tbl_fea_lag_key_t    key;
    uint64               lag_id;
} tbl_fea_lag_t;

typedef struct
{
    ctclib_hash_t        *fea_lag_hash;
} tbl_fea_lag_master_t;

#endif /* !__TBL_FEA_LAG_DEFINE_H__ */

