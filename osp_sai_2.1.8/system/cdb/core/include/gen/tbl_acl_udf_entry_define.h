
#ifndef __TBL_ACL_UDF_ENTRY_DEFINE_H__
#define __TBL_ACL_UDF_ENTRY_DEFINE_H__

/* TBL_ACL_UDF_ENTRY field defines */
typedef enum
{
    TBL_ACL_UDF_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_UDF_ENTRY_FLD_ACL_REF_CNT          = 1 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_PRIORITY             = 2 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE          = 3 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM           = 4 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_VALID                = 5 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_OFFSET               = 6 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS            = 7 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_ETHTYPE              = 8 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_ETHTYPE_MASK         = 9 ,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_VLANNUM              = 10,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_VLANNUM_MASK         = 11,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_IPPROTO              = 12,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_IPPROTO_MASK         = 13,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_L4_TYPE              = 14,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_L4_TYPE_MASK         = 15,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_SRCPORT              = 16,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_SRCPORT_MASK         = 17,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_DSTPORT              = 18,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_DSTPORT_MASK         = 19,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_MPLSNUM              = 20,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_MPLSNUM_MASK         = 21,  /* RW */
    TBL_ACL_UDF_ENTRY_FLD_MAX                  = 22
} tbl_acl_udf_entry_field_id_t;

/* TBL_ACL_UDF_ENTRY defines */
typedef struct
{
    uint32               id;
} tbl_acl_udf_entry_key_t;

typedef struct
{
    tbl_acl_udf_entry_key_t key;
    uint32               acl_ref_cnt;
    uint16               priority;
    uint8                offset_type;         /* l2-udf: 0, l3-udf: 1, l4_udf: 2 */
    uint8                offset_num;
    uint8                valid[GLB_ACL_UDF_FIELD_NUM_MAX];
    uint8                offset[GLB_ACL_UDF_FIELD_NUM_MAX];
    uint32               key_flags;
    uint32               ethtype;
    uint32               ethtype_mask;
    uint32               vlannum;             /* maximum 3 */
    uint32               vlannum_mask;
    uint32               ipproto;             /* tcp:6 udp:17 gre:47 pim:103.. */
    uint32               ipproto_mask;
    uint32               l4_type;             /* ctc_parser_l4_type_t max 0xf */
    uint32               l4_type_mask;
    uint32               srcport;
    uint32               srcport_mask;
    uint32               dstport;
    uint32               dstport_mask;
    uint32               mplsnum;
    uint32               mplsnum_mask;
} tbl_acl_udf_entry_t;

typedef struct
{
    tbl_acl_udf_entry_t  *acl_udf_entry_array[GLB_ACL_UDF_ENTRY_NUM_MAX+1];
} tbl_acl_udf_entry_master_t;

#endif /* !__TBL_ACL_UDF_ENTRY_DEFINE_H__ */

