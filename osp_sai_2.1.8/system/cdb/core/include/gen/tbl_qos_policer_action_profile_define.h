
#ifndef __TBL_QOS_POLICER_ACTION_PROFILE_DEFINE_H__
#define __TBL_QOS_POLICER_ACTION_PROFILE_DEFINE_H__

/* TBL_QOS_POLICER_ACTION_PROFILE field defines */
typedef enum
{
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_KEY                  = 0 ,  /* READ */
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_NAME                 = 1 ,  /* RW */
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_CONFIRM_ACTION       = 2 ,  /* RW */
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_EXCEED_ACTION        = 3 ,  /* RW */
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_VIOLATE_ACTION       = 4 ,  /* RW */
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_REF_CNT              = 5 ,  /* RW */
    TBL_QOS_POLICER_ACTION_PROFILE_FLD_MAX                  = 6 
} tbl_qos_policer_action_profile_field_id_t;

/* TBL_QOS_POLICER_ACTION_PROFILE defines */
typedef struct
{
    uint32               id;
} tbl_qos_policer_action_profile_key_t;

typedef struct
{
    tbl_qos_policer_action_profile_key_t key;
    char                 name[QOS_NAME_SIZE+1];
    glb_qos_color_action_t confirm_action;      /* green color action */
    glb_qos_color_action_t exceed_action;       /* yellow color action */
    glb_qos_color_action_t violate_action;      /* red color action */
    uint32               ref_cnt;
} tbl_qos_policer_action_profile_t;

typedef struct
{
    tbl_qos_policer_action_profile_t *qos_policer_action_profile_array[GLB_QOS_POLICER_ACTION_PROFILE_NUM];
} tbl_qos_policer_action_profile_master_t;

#endif /* !__TBL_QOS_POLICER_ACTION_PROFILE_DEFINE_H__ */

