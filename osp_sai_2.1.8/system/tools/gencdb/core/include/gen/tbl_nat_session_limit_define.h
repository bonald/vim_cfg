
#ifndef __TBL_NAT_SESSION_LIMIT_DEFINE_H__
#define __TBL_NAT_SESSION_LIMIT_DEFINE_H__

/* TBL_NAT_SESSION_LIMIT field defines */
typedef enum
{
    TBL_NAT_SESSION_LIMIT_FLD_KEY                  = 0 ,  /* READ */
    TBL_NAT_SESSION_LIMIT_FLD_LIMIT                = 1 ,  /* RW */
    TBL_NAT_SESSION_LIMIT_FLD_COUNT_UPSTREAM       = 2 ,  /* RW */
    TBL_NAT_SESSION_LIMIT_FLD_COUNT_DNSTREAM       = 3 ,  /* RW */
    TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_POST         = 4 ,  /* RW */
    TBL_NAT_SESSION_LIMIT_FLD_NFT_HDL_PRE          = 5 ,  /* RW */
    TBL_NAT_SESSION_LIMIT_FLD_MAX                  = 6 
} tbl_nat_session_limit_field_id_t;

/* TBL_NAT_SESSION_LIMIT defines */
typedef addr_t tbl_nat_session_limit_key_t;

typedef struct
{
    tbl_nat_session_limit_key_t key;
    uint32               limit;
    uint32               count_upstream;
    uint32               count_dnstream;
    uint32               nft_hdl_post;
    uint32               nft_hdl_pre;
} tbl_nat_session_limit_t;

typedef struct
{
    ctclib_hash_t        *nat_session_limit_hash;
    ctclib_slist_t       *nat_session_limit_list;
} tbl_nat_session_limit_master_t;

#endif /* !__TBL_NAT_SESSION_LIMIT_DEFINE_H__ */

