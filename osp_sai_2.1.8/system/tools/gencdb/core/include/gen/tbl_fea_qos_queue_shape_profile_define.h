
#ifndef __TBL_FEA_QOS_QUEUE_SHAPE_PROFILE_DEFINE_H__
#define __TBL_FEA_QOS_QUEUE_SHAPE_PROFILE_DEFINE_H__

/* TBL_FEA_QOS_QUEUE_SHAPE_PROFILE field defines */
typedef enum
{
    TBL_FEA_QOS_QUEUE_SHAPE_PROFILE_FLD_KEY                  = 0 ,  /* RW */
    TBL_FEA_QOS_QUEUE_SHAPE_PROFILE_FLD_SAI_SCHEDULER_ID     = 1 ,  /* RW */
    TBL_FEA_QOS_QUEUE_SHAPE_PROFILE_FLD_MAX                  = 2 
} tbl_fea_qos_queue_shape_profile_field_id_t;

/* TBL_FEA_QOS_QUEUE_SHAPE_PROFILE defines */
typedef struct
{
    uint32               id;
} tbl_fea_qos_queue_shape_profile_key_t;

typedef struct
{
    tbl_fea_qos_queue_shape_profile_key_t key;
    uint64               sai_scheduler_id;
} tbl_fea_qos_queue_shape_profile_t;

typedef struct
{
    tbl_fea_qos_queue_shape_profile_t *fea_qos_queue_shape_profile_array[GLB_QOS_SHAPE_PROFILE_NUM];
} tbl_fea_qos_queue_shape_profile_master_t;

#endif /* !__TBL_FEA_QOS_QUEUE_SHAPE_PROFILE_DEFINE_H__ */

