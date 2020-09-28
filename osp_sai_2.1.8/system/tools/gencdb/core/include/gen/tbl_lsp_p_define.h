
#ifndef __TBL_LSP_P_DEFINE_H__
#define __TBL_LSP_P_DEFINE_H__

/* TBL_LSP_P field defines */
typedef enum
{
    TBL_LSP_P_FLD_KEY                  = 0 ,  /* READ */
    TBL_LSP_P_FLD_REF_CNT              = 1 ,  /* RW */
    TBL_LSP_P_FLD_WEST_INLABEL         = 2 ,  /* RW */
    TBL_LSP_P_FLD_WEST_OUTLABEL        = 3 ,  /* RW */
    TBL_LSP_P_FLD_WEST_IFINDEX         = 4 ,  /* RW */
    TBL_LSP_P_FLD_WEST_VID             = 5 ,  /* RW */
    TBL_LSP_P_FLD_WEST_NEXTHOP_MAC     = 6 ,  /* RW */
    TBL_LSP_P_FLD_WEST_OUTLABEL_EXP    = 7 ,  /* RW */
    TBL_LSP_P_FLD_WEST_OUTLABEL_EXP_IS_SET = 8 ,  /* RW */
    TBL_LSP_P_FLD_WEST_OUTLABEL_TTL    = 9 ,  /* RW */
    TBL_LSP_P_FLD_EAST_INLABEL         = 10,  /* RW */
    TBL_LSP_P_FLD_EAST_OUTLABEL        = 11,  /* RW */
    TBL_LSP_P_FLD_EAST_IFINDEX         = 12,  /* RW */
    TBL_LSP_P_FLD_EAST_VID             = 13,  /* RW */
    TBL_LSP_P_FLD_EAST_OUTLABEL_EXP    = 14,  /* RW */
    TBL_LSP_P_FLD_EAST_OUTLABEL_EXP_IS_SET = 15,  /* RW */
    TBL_LSP_P_FLD_EAST_OUTLABEL_TTL    = 16,  /* RW */
    TBL_LSP_P_FLD_EAST_NEXTHOP_MAC     = 17,  /* RW */
    TBL_LSP_P_FLD_OAM_SESSION_ID       = 18,  /* RW */
    TBL_LSP_P_FLD_MAX                  = 19
} tbl_lsp_p_field_id_t;

/* TBL_LSP_P defines */
typedef struct
{
    char                 name[LSPNAME_SIZE];
} tbl_lsp_p_key_t;

typedef struct
{
    tbl_lsp_p_key_t      key;
    uint32               ref_cnt;
    uint32               west_inlabel;
    uint32               west_outlabel;
    uint32               west_ifindex;
    uint32               west_vid;
    mac_addr_t           west_nexthop_mac;
    uint32               west_outlabel_exp;
    uint32               west_outlabel_exp_is_set;
    uint32               west_outlabel_ttl;
    uint32               east_inlabel;
    uint32               east_outlabel;
    uint32               east_ifindex;
    uint32               east_vid;
    uint32               east_outlabel_exp;
    uint32               east_outlabel_exp_is_set;
    uint32               east_outlabel_ttl;
    mac_addr_t           east_nexthop_mac;
    uint32               oam_session_id;
} tbl_lsp_p_t;

typedef struct
{
    ctclib_hash_t        *lsp_p_hash;
    ctclib_slist_t       *lsp_p_list;
} tbl_lsp_p_master_t;

#endif /* !__TBL_LSP_P_DEFINE_H__ */

