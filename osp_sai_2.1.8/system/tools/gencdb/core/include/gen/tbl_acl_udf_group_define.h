
#ifndef __TBL_ACL_UDF_GROUP_DEFINE_H__
#define __TBL_ACL_UDF_GROUP_DEFINE_H__

/* TBL_ACL_UDF_GROUP field defines */
typedef enum
{
    TBL_ACL_UDF_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_UDF_GROUP_FLD_GROUP_TYPE           = 1 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_IP_VERSION           = 2 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_L3_PROTOCOL          = 3 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_OFFSET               = 4 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_UDF_MATCH_OID        = 5 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_UDF_OID              = 6 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_UDF_GROUP_OID        = 7 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_CNT                  = 8 ,  /* RW */
    TBL_ACL_UDF_GROUP_FLD_MAX                  = 9 
} tbl_acl_udf_group_field_id_t;

/* TBL_ACL_UDF_GROUP defines */
typedef struct
{
    uint32               id;
} tbl_acl_udf_group_key_t;

typedef struct
{
    tbl_acl_udf_group_key_t key;
    uint32               group_type;          /* l3-udf: 1, l4_udf: 2 */
    uint32               ip_version;          /* ipv4: 1, ipv6: 2 */
    uint32               l3_protocol;         /* 0-255 */
    uint8                offset;
    uint64               udf_match_oid;
    uint64               udf_oid;
    uint64               udf_group_oid;
    uint32               cnt;
} tbl_acl_udf_group_t;

typedef struct
{
    tbl_acl_udf_group_t  *acl_udf_group_array[GLB_ACL_UDF_GROUP_NUM_MAX];
} tbl_acl_udf_group_master_t;

#endif /* !__TBL_ACL_UDF_GROUP_DEFINE_H__ */

