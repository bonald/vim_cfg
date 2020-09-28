
#ifndef __TBL_ACL_WORM_FILTER_DEFINE_H__
#define __TBL_ACL_WORM_FILTER_DEFINE_H__

/* TBL_ACL_WORM_FILTER field defines */
typedef enum
{
    TBL_ACL_WORM_FILTER_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_WORM_FILTER_FLD_SEQ_NUM              = 1 ,  /* RW */
    TBL_ACL_WORM_FILTER_FLD_ENABLE               = 2 ,  /* RW */
    TBL_ACL_WORM_FILTER_FLD_ISTCP                = 3 ,  /* RW */
    TBL_ACL_WORM_FILTER_FLD_DSTPORT              = 4 ,  /* RW */
    TBL_ACL_WORM_FILTER_FLD_ISRANGE              = 5 ,  /* RW */
    TBL_ACL_WORM_FILTER_FLD_STATS_EN             = 6 ,  /* RW */
    TBL_ACL_WORM_FILTER_FLD_MAX                  = 7 
} tbl_acl_worm_filter_field_id_t;

/* TBL_ACL_WORM_FILTER defines */
typedef struct
{
    char                 worm_filter_name[ACL_NAME_FULL_NAME_SIZE+1];
} tbl_acl_worm_filter_key_t;

typedef struct
{
    tbl_acl_worm_filter_key_t key;
    uint32               seq_num;
    uint32               enable;
    uint32               istcp;
    uint32               dstport;
    uint32               isrange;
    uint32               stats_en;
} tbl_acl_worm_filter_t;

typedef struct
{
    ctclib_hash_t        *acl_worm_filter_hash;
    ctclib_slist_t       *acl_worm_filter_list;
} tbl_acl_worm_filter_master_t;

#endif /* !__TBL_ACL_WORM_FILTER_DEFINE_H__ */

