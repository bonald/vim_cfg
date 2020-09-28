
#ifndef __TBL_VCM_DEFINE_H__
#define __TBL_VCM_DEFINE_H__

/* TBL_VCM field defines */
typedef enum
{
    TBL_VCM_FLD_KEY                  = 0 ,  /* READ */
    TBL_VCM_FLD_SLOT                 = 1 ,  /* READ */
    TBL_VCM_FLD_VCM_TYPE             = 2 ,  /* READ */
    TBL_VCM_FLD_VCM_CHANNEL_NUMS     = 3 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME0        = 4 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME1        = 5 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME2        = 6 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME3        = 7 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME4        = 8 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME5        = 9 ,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME6        = 10,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME7        = 11,  /* READ */
    TBL_VCM_FLD_CHANNEL_NAME8        = 12,  /* READ */
    TBL_VCM_FLD_FLAG                 = 13,  /* READ */
    TBL_VCM_FLD_CURR_DEPEND_PIN      = 14,  /* READ */
    TBL_VCM_FLD_VOLT_SPLIT_FACTOR    = 15,  /* READ */
    TBL_VCM_FLD_LOW                  = 16,  /* READ */
    TBL_VCM_FLD_HIGH                 = 17,  /* READ */
    TBL_VCM_FLD_VOLT                 = 18,  /* READ */
    TBL_VCM_FLD_CURR                 = 19,  /* READ */
    TBL_VCM_FLD_TMPR                 = 20,  /* READ */
    TBL_VCM_FLD_MAX                  = 21
} tbl_vcm_field_id_t;

/* TBL_VCM defines */
typedef struct
{
    int32                id;
} tbl_vcm_key_t;

typedef struct
{
    tbl_vcm_key_t        key;
    int32                slot;
    uint8                vcm_type;
    uint8                vcm_channel_nums;
    char                 channel_name0[GLB_NAME_INFO_STR_MAX];
    char                 channel_name1[GLB_NAME_INFO_STR_MAX];
    char                 channel_name2[GLB_NAME_INFO_STR_MAX];
    char                 channel_name3[GLB_NAME_INFO_STR_MAX];
    char                 channel_name4[GLB_NAME_INFO_STR_MAX];
    char                 channel_name5[GLB_NAME_INFO_STR_MAX];
    char                 channel_name6[GLB_NAME_INFO_STR_MAX];
    char                 channel_name7[GLB_NAME_INFO_STR_MAX];
    char                 channel_name8[GLB_NAME_INFO_STR_MAX];
    uint32               flag[MAX_VCM_CHANNEL_NUM];
    uint8                curr_depend_pin[MAX_VCM_CHANNEL_NUM];
    double               volt_split_factor[MAX_VCM_CHANNEL_NUM];
    double               low[MAX_VCM_CHANNEL_NUM];
    double               high[MAX_VCM_CHANNEL_NUM];
    double               volt[MAX_VCM_CHANNEL_NUM];
    double               curr[MAX_VCM_CHANNEL_NUM];
    double               tmpr[MAX_VCM_CHANNEL_NUM];
} tbl_vcm_t;

typedef struct
{
    ctclib_hash_t        *vcm_hash;
    ctclib_slist_t       *vcm_list;
} tbl_vcm_master_t;

#endif /* !__TBL_VCM_DEFINE_H__ */

