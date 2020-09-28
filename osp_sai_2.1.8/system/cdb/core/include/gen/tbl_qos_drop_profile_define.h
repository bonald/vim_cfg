
#ifndef __TBL_QOS_DROP_PROFILE_DEFINE_H__
#define __TBL_QOS_DROP_PROFILE_DEFINE_H__

/* TBL_QOS_DROP_PROFILE field defines */
typedef enum
{
    TBL_QOS_DROP_PROFILE_FLD_KEY                  = 0 ,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_NAME                 = 1 ,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MAX_THRESHOLD = 2 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_MIN_THRESHOLD = 3 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_GREEN_PROBABILTY = 4 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MAX_THRESHOLD = 5 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_MIN_THRESHOLD = 6 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_YELLOW_PROBABILTY = 7 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MAX_THRESHOLD = 8 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_MIN_THRESHOLD = 9 ,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_RED_PROBABILTY = 10,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_FLAGS_RANDOM_DETECT  = 11,  /* READ */
    TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD = 12,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD = 13,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY    = 14,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD = 15,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD = 16,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY   = 17,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD   = 18,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD   = 19,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY      = 20,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_REF_CNT              = 21,  /* RW */
    TBL_QOS_DROP_PROFILE_FLD_MAX                  = 22
} tbl_qos_drop_profile_field_id_t;

/* TBL_QOS_DROP_PROFILE defines */
typedef struct
{
    uint32               id;
} tbl_qos_drop_profile_key_t;

typedef struct
{
    tbl_qos_drop_profile_key_t key;
    char                 name[QOS_NAME_SIZE+1];
    uint32               flags;               /* GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD */
    uint32               green_max_threashold;
    uint32               green_min_threashold;
    uint32               green_probability;
    uint32               yellow_max_threashold;
    uint32               yellow_min_threashold;
    uint32               yellow_probability;
    uint32               red_max_threashold;
    uint32               red_min_threashold;
    uint32               red_probability;
    uint32               ref_cnt;
} tbl_qos_drop_profile_t;

typedef struct
{
    tbl_qos_drop_profile_t *qos_drop_profile_array[GLB_QOS_DROP_PROFILE_NUM];
} tbl_qos_drop_profile_master_t;

#endif /* !__TBL_QOS_DROP_PROFILE_DEFINE_H__ */

