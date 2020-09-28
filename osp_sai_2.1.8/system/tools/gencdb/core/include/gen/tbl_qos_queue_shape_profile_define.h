
#ifndef __TBL_QOS_QUEUE_SHAPE_PROFILE_DEFINE_H__
#define __TBL_QOS_QUEUE_SHAPE_PROFILE_DEFINE_H__

/* TBL_QOS_QUEUE_SHAPE_PROFILE field defines */
typedef enum
{
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_KEY                  = 0 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_NAME                 = 1 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR                  = 2 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_CIR                  = 3 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE                 = 4 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT         = 5 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_REF_CNT              = 6 ,  /* RW */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_PIR            = 7 ,  /* READ */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_CIR            = 8 ,  /* READ */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_MODE           = 9 ,  /* READ */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_FLAGS_WEIGHT         = 10,  /* READ */
    TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MAX                  = 11
} tbl_qos_queue_shape_profile_field_id_t;

/* TBL_QOS_QUEUE_SHAPE_PROFILE defines */
typedef struct
{
    uint32               id;
} tbl_qos_queue_shape_profile_key_t;

typedef struct
{
    tbl_qos_queue_shape_profile_key_t key;
    char                 name[QOS_NAME_SIZE+1];
    uint32               pir;                 /* kbits/s */
    uint32               cir;                 /* kbits/s */
    uint32               mode;                /* queue class */
    uint32               queue_weight;        /* queue weight */
    uint32               ref_cnt;
    uint32               flags;               /* GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR */
} tbl_qos_queue_shape_profile_t;

typedef struct
{
    tbl_qos_queue_shape_profile_t *qos_queue_shape_profile_array[GLB_QOS_SHAPE_PROFILE_NUM];
} tbl_qos_queue_shape_profile_master_t;

#endif /* !__TBL_QOS_QUEUE_SHAPE_PROFILE_DEFINE_H__ */

