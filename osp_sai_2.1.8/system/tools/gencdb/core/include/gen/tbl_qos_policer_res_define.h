
#ifndef __TBL_QOS_POLICER_RES_DEFINE_H__
#define __TBL_QOS_POLICER_RES_DEFINE_H__

/* TBL_QOS_POLICER_RES field defines */
typedef enum
{
    TBL_QOS_POLICER_RES_FLD_KEY                  = 0 ,  /* READ */
    TBL_QOS_POLICER_RES_FLD_REF_CNT              = 1 ,  /* RW */
    TBL_QOS_POLICER_RES_FLD_MAX                  = 2 
} tbl_qos_policer_res_field_id_t;

/* TBL_QOS_POLICER_RES defines */
typedef qos_policer_res_key_t tbl_qos_policer_res_key_t;

typedef struct
{
    tbl_qos_policer_res_key_t key;
    uint32               ref_cnt;
} tbl_qos_policer_res_t;

typedef struct
{
    ctclib_hash_t        *qos_policer_res_hash;
} tbl_qos_policer_res_master_t;

#endif /* !__TBL_QOS_POLICER_RES_DEFINE_H__ */

