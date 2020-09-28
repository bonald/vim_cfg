
#ifndef __STBL_ACL_UDF_ENTRY_DEFINE_H__
#define __STBL_ACL_UDF_ENTRY_DEFINE_H__

/* STBL_ACL_UDF_ENTRY field defines */
typedef enum
{
    STBL_ACL_UDF_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_GROUP_TYPE           = 1 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_GROUP_LENGTH         = 2 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_OFFSET_TYPE          = 3 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_OFFSET_NUM           = 4 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_OFFSET               = 5 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_INSTALLED            = 6 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_KEY_FLAGS            = 7 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_ETHTYPE              = 8 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_VLANNUM              = 9 ,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_IPPROTO              = 10,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_L4_TYPE              = 11,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_SRCPORT              = 12,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_DSTPORT              = 13,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_MPLSNUM              = 14,  /* READ */
    STBL_ACL_UDF_ENTRY_FLD_MAX                  = 15
} stbl_acl_udf_entry_field_id_t;

/* STBL_ACL_UDF_ENTRY defines */
typedef struct
{
    uint32               id;
} stbl_acl_udf_entry_key_t;

typedef struct
{
    stbl_acl_udf_entry_key_t key;
    uint8                group_type;          /* GENERIC: 0   HASH: 1 */
    uint8                group_length;        /* Duet2: 4   GG: 1 */
    uint8                offset_type;         /* l2-udf: 0, l3-udf: 1, l4_udf: 2 */
    uint8                offset_num;
    uint8                offset[GLB_ACL_UDF_FIELD_NUM_MAX];
    uint8                installed;           /* installed to asic or not */
    uint32               key_flags;
    uint32               ethtype[2];
    uint32               vlannum[2];          /* maximum 3 */
    uint32               ipproto[2];          /* tcp:6 udp:17 gre:47 pim:103.. */
    uint32               l4_type[2];          /* no support ctc_parser_l4_type_t max 0xf */
    uint32               srcport[2];
    uint32               dstport[2];
    uint32               mplsnum[2];
} stbl_acl_udf_entry_t;

typedef struct
{
    stbl_acl_udf_entry_t *array[GLB_ACL_UDF_ENTRY_NUM_MAX+1];
} stbl_acl_udf_entry_master_t;

#endif /* !__STBL_ACL_UDF_ENTRY_DEFINE_H__ */

