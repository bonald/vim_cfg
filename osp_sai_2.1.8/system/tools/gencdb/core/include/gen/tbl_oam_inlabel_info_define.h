
#ifndef __TBL_OAM_INLABEL_INFO_DEFINE_H__
#define __TBL_OAM_INLABEL_INFO_DEFINE_H__

/* TBL_OAM_INLABEL_INFO field defines */
typedef enum
{
    TBL_OAM_INLABEL_INFO_FLD_KEY                  = 0 ,  /* READ */
    TBL_OAM_INLABEL_INFO_FLD_REF_CNT              = 1 ,  /* RW */
    TBL_OAM_INLABEL_INFO_FLD_MAX                  = 2 
} tbl_oam_inlabel_info_field_id_t;

/* TBL_OAM_INLABEL_INFO defines */
typedef struct
{
    uint32               inlabel;
} tbl_oam_inlabel_info_key_t;

typedef struct
{
    tbl_oam_inlabel_info_key_t key;
    uint32               ref_cnt;             /* indicates whether hold-off expires of not */
} tbl_oam_inlabel_info_t;

typedef struct
{
    ctclib_hash_t        *oam_inlabel_info_hash;
    ctclib_slist_t       *oam_inlabel_info_list;
} tbl_oam_inlabel_info_master_t;

#endif /* !__TBL_OAM_INLABEL_INFO_DEFINE_H__ */

