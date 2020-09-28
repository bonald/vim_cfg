
#ifndef __TBL_FEA_ACL_DEFINE_H__
#define __TBL_FEA_ACL_DEFINE_H__

/* TBL_FEA_ACL field defines */
typedef enum
{
    TBL_FEA_ACL_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_ACL_FLD_IS_MAC_ACL           = 1 ,  /* RW */
    TBL_FEA_ACL_FLD_SAI_ACL_ID           = 2 ,  /* RW */
    TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV4      = 3 ,  /* RW */
    TBL_FEA_ACL_FLD_SAI_ACL_ID_IPV6      = 4 ,  /* RW */
    TBL_FEA_ACL_FLD_COUNTER_ID           = 5 ,  /* RW */
    TBL_FEA_ACL_FLD_STATS_PACKET         = 6 ,  /* RW */
    TBL_FEA_ACL_FLD_STATS_BYTE           = 7 ,  /* RW */
    TBL_FEA_ACL_FLD_MAX                  = 8 
} tbl_fea_acl_field_id_t;

/* TBL_FEA_ACL defines */
typedef struct
{
    uint64               acl_id;
} tbl_fea_acl_key_t;

typedef struct
{
    tbl_fea_acl_key_t    key;
    uint8                is_mac_acl;
    uint64               sai_acl_id;
    uint64               sai_acl_id_ipv4;
    uint64               sai_acl_id_ipv6;
    uint64               counter_id;
    uint64               stats_packet;
    uint64               stats_byte;
} tbl_fea_acl_t;

typedef struct
{
    ctclib_hash_t        *fea_acl_hash;
} tbl_fea_acl_master_t;

#endif /* !__TBL_FEA_ACL_DEFINE_H__ */

