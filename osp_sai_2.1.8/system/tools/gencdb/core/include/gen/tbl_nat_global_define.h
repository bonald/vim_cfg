
#ifndef __TBL_NAT_GLOBAL_DEFINE_H__
#define __TBL_NAT_GLOBAL_DEFINE_H__

/* TBL_NAT_GLOBAL field defines */
typedef enum
{
    TBL_NAT_GLOBAL_FLD_TIMEOUT              = 0 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP         = 1 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP          = 2 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP          = 3 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_LIMIT        = 4 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFT_HDL              = 5 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_IF  = 6 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_POST = 7 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFT_HDL_CONNLIMIT_PRE = 8 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFT_HDL_UPSTREAM_FILTER_NAT_RULE = 9 ,  /* RW */
    TBL_NAT_GLOBAL_FLD_RULE_ID_OPB          = 10,  /* READ */
    TBL_NAT_GLOBAL_FLD_SESSION_ID_OPB       = 11,  /* READ */
    TBL_NAT_GLOBAL_FLD_CATEGORY_ID_OPB      = 12,  /* READ */
    TBL_NAT_GLOBAL_FLD_AGING_TIMER          = 13,  /* READ */
    TBL_NAT_GLOBAL_FLD_TIMER_MARK           = 14,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFCT_HANDLE          = 15,  /* RW */
    TBL_NAT_GLOBAL_FLD_RULE_CFG_ORDER       = 16,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_CNT_UPSTREAM = 17,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_CNT_DNSTREAM = 18,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_UPSTREAM = 19,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_FAST_FWD_CNT_DNSTREAM = 20,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_ADD_CNT = 21,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFCT_KERNEL_NOTIFY_DEL_CNT = 22,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_ADD_CNT = 23,  /* RW */
    TBL_NAT_GLOBAL_FLD_NFCT_NOTIFY_KERNEL_DEL_CNT = 24,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_AGING_DEL_CNT = 25,  /* RW */
    TBL_NAT_GLOBAL_FLD_SESSION_AGING_UPD_TTL_CNT = 26,  /* RW */
    TBL_NAT_GLOBAL_FLD_MAX                  = 27
} tbl_nat_global_field_id_t;

/* TBL_NAT_GLOBAL defines */
typedef struct
{
    uint32               timeout;
    uint32               timeout_icmp;
    uint32               timeout_tcp;
    uint32               timeout_udp;
    uint32               session_limit;
    uint32               nft_hdl;
    uint32               nft_hdl_upstream_if;
    uint32               nft_hdl_connlimit_post;
    uint32               nft_hdl_connlimit_pre;
    uint32               nft_hdl_upstream_filter_nat_rule;
    ctclib_opb_t         rule_id_opb;
    ctclib_opb_t         session_id_opb;
    ctclib_opb_t         category_id_opb;
    ctc_task_t           *aging_timer;
    uint32               timer_mark;
    void                 *nfct_handle;
    uint64               rule_cfg_order;
    uint32               session_cnt_upstream;
    uint32               session_cnt_dnstream;
    uint32               session_fast_fwd_cnt_upstream;
    uint32               session_fast_fwd_cnt_dnstream;
    uint32               nfct_kernel_notify_add_cnt;
    uint32               nfct_kernel_notify_del_cnt;
    uint32               nfct_notify_kernel_add_cnt;
    uint32               nfct_notify_kernel_del_cnt;
    uint32               session_aging_del_cnt;
    uint32               session_aging_upd_ttl_cnt;
} tbl_nat_global_t;

#endif /* !__TBL_NAT_GLOBAL_DEFINE_H__ */

