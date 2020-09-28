
#ifndef __TBL_NAT_RULE_DEFINE_H__
#define __TBL_NAT_RULE_DEFINE_H__

/* TBL_NAT_RULE field defines */
typedef enum
{
    TBL_NAT_RULE_FLD_KEY                  = 0 ,  /* READ */
    TBL_NAT_RULE_FLD_HANDLE_POSTROUTING   = 1 ,  /* RW */
    TBL_NAT_RULE_FLD_HANDLE_PREROUTING    = 2 ,  /* RW */
    TBL_NAT_RULE_FLD_CATEGORY_ID          = 3 ,  /* RW */
    TBL_NAT_RULE_FLD_REFER_IIF            = 4 ,  /* RW */
    TBL_NAT_RULE_FLD_REFER_OIF            = 5 ,  /* RW */
    TBL_NAT_RULE_FLD_NFT_CHAIN_ID_ACL     = 6 ,  /* RW */
    TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_PRE = 7 ,  /* RW */
    TBL_NAT_RULE_FLD_NFT_CHAIN_ID_RULE_POST = 8 ,  /* RW */
    TBL_NAT_RULE_FLD_CFG_ORDER            = 9 ,  /* RW */
    TBL_NAT_RULE_FLD_MAX                  = 10
} tbl_nat_rule_field_id_t;

/* TBL_NAT_RULE defines */
typedef nat_rule_key_t tbl_nat_rule_key_t;

typedef struct
{
    tbl_nat_rule_key_t   key;
    uint32               handle_postrouting;
    uint32               handle_prerouting;
    uint32               category_id;         /* oif category id, save it for latter use*/
    char                 refer_iif[IFNAME_SIZE+1]; /* if configured local ip in NAT rule, find the correspoding iif and save it here for later use*/
    char                 refer_oif[IFNAME_SIZE+1]; /* if configured pool in NAT rule, or global IP, find the correspoding oif and save it here for later use*/
    uint32               nft_chain_id_acl;
    uint32               nft_chain_id_rule_pre;
    uint32               nft_chain_id_rule_post;
    uint64               cfg_order;
} tbl_nat_rule_t;

typedef struct
{
    ctclib_hash_t        *nat_rule_hash;
    ctclib_slist_t       *nat_rule_list;
} tbl_nat_rule_master_t;

#endif /* !__TBL_NAT_RULE_DEFINE_H__ */

