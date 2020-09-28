
#ifndef __STBL_ACL_TBL_DEFINE_H__
#define __STBL_ACL_TBL_DEFINE_H__

/* STBL_ACL_TBL field defines */
typedef enum
{
    STBL_ACL_TBL_FLD_KEY                  = 0 ,  /* READ */
    STBL_ACL_TBL_FLD_PRIORITY             = 1 ,  /* READ */
    STBL_ACL_TBL_FLD_FIELD_BITMAP         = 2 ,  /* READ */
    STBL_ACL_TBL_FLD_STAGE                = 3 ,  /* READ */
    STBL_ACL_TBL_FLD_MAX                  = 4 
} stbl_acl_tbl_field_id_t;

/* STBL_ACL_TBL defines */
typedef struct
{
    uint32               tbl_id;
} stbl_acl_tbl_key_t;

typedef struct
{
    stbl_acl_tbl_key_t   key;
    uint32               priority;
    uint32               field_bitmap;
    uint32               stage;               /* sai_acl_stage_t */
} stbl_acl_tbl_t;

typedef struct
{
    stbl_acl_tbl_t       *array[GLB_SAI_ACL_MAX];
} stbl_acl_tbl_master_t;

#endif /* !__STBL_ACL_TBL_DEFINE_H__ */

