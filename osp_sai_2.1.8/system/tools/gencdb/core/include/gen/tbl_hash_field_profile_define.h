
#ifndef __TBL_HASH_FIELD_PROFILE_DEFINE_H__
#define __TBL_HASH_FIELD_PROFILE_DEFINE_H__

/* TBL_HASH_FIELD_PROFILE field defines */
typedef enum
{
    TBL_HASH_FIELD_PROFILE_FLD_KEY                  = 0 ,  /* READ */
    TBL_HASH_FIELD_PROFILE_FLD_DESCRIPTION          = 1 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DATA          = 2 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_FLAG                 = 3 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_INDEX                = 4 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_L2_SET               = 5 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_IP_SET               = 6 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET             = 7 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_GRE_SET              = 8 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET            = 9 ,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET             = 10,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET             = 11,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET            = 12,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET            = 13,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET             = 14,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC      = 15,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_HASH_SEED_VALUE      = 16,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY        = 17,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE       = 18,  /* RW */
    TBL_HASH_FIELD_PROFILE_FLD_MAX                  = 19
} tbl_hash_field_profile_field_id_t;

/* TBL_HASH_FIELD_PROFILE defines */
typedef struct
{
    char                 name[HASH_FIELD_PROFILE_NAME_SIZE+1];
} tbl_hash_field_profile_key_t;

typedef struct
{
    tbl_hash_field_profile_key_t key;
    char                 description[DESC_SIZE];
    uint32               bitmap_data;
    uint32               flag;
    uint32               index;
    uint32               l2_set;
    uint32               ip_set;
    uint32               ipv6_set;
    uint32               gre_set;
    uint32               nvgre_set;
    uint32               mpls_set;
    uint32               vpws_set;
    uint32               l3vpn_set;
    uint32               vxlan_set;
    uint32               vpls_set;
    uint32               hash_arithmetic;
    uint32               hash_seed_value;
    uint32               mode_symmetry;
    uint32               bitmap_disable;
} tbl_hash_field_profile_t;

typedef struct
{
    ctclib_hash_t        *hash_field_profile_hash;
    ctclib_slist_t       *hash_field_profile_list;
} tbl_hash_field_profile_master_t;

#endif /* !__TBL_HASH_FIELD_PROFILE_DEFINE_H__ */

