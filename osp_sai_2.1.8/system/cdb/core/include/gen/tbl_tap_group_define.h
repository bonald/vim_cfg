
#ifndef __TBL_TAP_GROUP_DEFINE_H__
#define __TBL_TAP_GROUP_DEFINE_H__

/* TBL_TAP_GROUP field defines */
typedef enum
{
    TBL_TAP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_TAP_GROUP_FLD_ID                   = 1 ,  /* RW */
    TBL_TAP_GROUP_FLD_FLAGS                = 2 ,  /* RW */
    TBL_TAP_GROUP_FLD_DESC                 = 3 ,  /* RW */
    TBL_TAP_GROUP_FLD_TRUNCATION_USE       = 4 ,  /* RW */
    TBL_TAP_GROUP_FLD_TAP_GROUP_OID        = 5 ,  /* READ */
    TBL_TAP_GROUP_FLD_MAX                  = 6 
} tbl_tap_group_field_id_t;

/* TBL_TAP_GROUP defines */
typedef struct
{
    char                 name[TAP_NAME_SIZE];
} tbl_tap_group_key_t;

typedef struct
{
    tbl_tap_group_key_t  key;
    uint32               id;
    uint32               flags;
    char                 desc[TAP_DESC_SIZE];
    uint8                truncation_use;
    uint64               tap_group_oid;       /* tap_group id for ASIC mapping*/
} tbl_tap_group_t;

typedef struct
{
    ctclib_hash_t        *tap_group_hash;
    ctclib_slist_t       *tap_group_list;
} tbl_tap_group_master_t;

#endif /* !__TBL_TAP_GROUP_DEFINE_H__ */

