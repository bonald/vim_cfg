
#ifndef __TBL_TAP_GROUP_EGRESS_DEFINE_H__
#define __TBL_TAP_GROUP_EGRESS_DEFINE_H__

/* TBL_TAP_GROUP_EGRESS field defines */
typedef enum
{
    TBL_TAP_GROUP_EGRESS_FLD_KEY                  = 0 ,  /* READ */
    TBL_TAP_GROUP_EGRESS_FLD_TIMESTAMP_EN         = 1 ,  /* RW */
    TBL_TAP_GROUP_EGRESS_FLD_TAP_GROUP_MEMBER_OID = 2 ,  /* RW */
    TBL_TAP_GROUP_EGRESS_FLD_MAX                  = 3 
} tbl_tap_group_egress_field_id_t;

/* TBL_TAP_GROUP_EGRESS defines */
typedef tap_group_egress_key_t tbl_tap_group_egress_key_t;

typedef struct
{
    tbl_tap_group_egress_key_t key;
    uint32               timestamp_en;
    uint64               tap_group_member_oid; /* tap_group_member_oid id for ASIC mapping*/
} tbl_tap_group_egress_t;

typedef struct
{
    ctclib_hash_t        *tap_group_egress_hash;
    ctclib_slist_t       *tap_group_egress_list;
} tbl_tap_group_egress_master_t;

#endif /* !__TBL_TAP_GROUP_EGRESS_DEFINE_H__ */

