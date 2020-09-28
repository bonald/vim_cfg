
#ifndef __TBL_QOS_POLICER_PROFILE_DEFINE_H__
#define __TBL_QOS_POLICER_PROFILE_DEFINE_H__

/* TBL_QOS_POLICER_PROFILE field defines */
typedef enum
{
    TBL_QOS_POLICER_PROFILE_FLD_KEY                  = 0 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_NAME                 = 1 ,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_POLICER_AGP    = 2 ,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_ALGORITHM_MODE = 3 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_COLOR_MODE     = 4 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CIR            = 5 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_CBS            = 6 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EIR_OR_PIR     = 7 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_EBS_OR_PBS     = 8 ,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DROP_COLOR     = 9 ,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_USE_L3_LEGTH   = 10,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS          = 11,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_FLAGS_DUMP_STATS     = 12,  /* READ */
    TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE       = 13,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE           = 14,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_DROP_COLOR           = 15,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_CIR                  = 16,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_CBS                  = 17,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR           = 18,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS           = 19,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_PKT          = 20,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_CONFIRM_BYTE         = 21,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_EXCEED_PKT           = 22,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_EXCEED_BYTE          = 23,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_PKT          = 24,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_VIOLATE_BYTE         = 25,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_IS_PPS               = 26,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME  = 27,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_REF_CNT              = 28,  /* RW */
    TBL_QOS_POLICER_PROFILE_FLD_MAX                  = 29
} tbl_qos_policer_profile_field_id_t;

/* TBL_QOS_POLICER_PROFILE defines */
typedef struct
{
    uint32               id;
} tbl_qos_policer_profile_key_t;

typedef struct
{
    tbl_qos_policer_profile_key_t key;
    char                 name[QOS_NAME_SIZE+1];
    uint32               flags;               /* GLB_POLICER_PROFILE_FLAGS_AGP */
    uint8                apply_dir;           /* for aggreate policer checking*/
    uint8                algorithm_mode;
    uint8                color_mode;          /* 0:color-blind; 1:color-aware */
    uint8                drop_color;          /* 1: red; 2:yellow, refer to glb_qos_color_t*/
    uint64               cir;                 /* kbits/s */
    uint32               cbs;
    uint64               eir_or_pir;          /* depend on algorithm mode */
    uint32               ebs_or_pbs;
    uint64               confirm_pkt;         /* used for dump */
    uint64               confirm_byte;        /* used for dump */
    uint64               exceed_pkt;          /* used for dump */
    uint64               exceed_byte;         /* used for dump */
    uint64               violate_pkt;         /* used for dump */
    uint64               violate_byte;        /* used for dump */
    uint8                is_pps;              /* 0:bps mode; 1:pps mode */
    char                 action_profile_name[QOS_NAME_SIZE+1]; /* tbl_qos_policer_action_profile_t */
    uint32               ref_cnt;
} tbl_qos_policer_profile_t;

typedef struct
{
    tbl_qos_policer_profile_t *qos_policer_profile_array[GLB_QOS_POLICER_PROFILE_NUM];
} tbl_qos_policer_profile_master_t;

#endif /* !__TBL_QOS_POLICER_PROFILE_DEFINE_H__ */

