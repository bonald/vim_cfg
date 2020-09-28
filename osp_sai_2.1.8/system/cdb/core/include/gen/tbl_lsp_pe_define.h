
#ifndef __TBL_LSP_PE_DEFINE_H__
#define __TBL_LSP_PE_DEFINE_H__

/* TBL_LSP_PE field defines */
typedef enum
{
    TBL_LSP_PE_FLD_KEY                  = 0 ,  /* READ */
    TBL_LSP_PE_FLD_REF_CNT              = 1 ,  /* RW */
    TBL_LSP_PE_FLD_INLABEL              = 2 ,  /* RW */
    TBL_LSP_PE_FLD_OUTLABEL             = 3 ,  /* RW */
    TBL_LSP_PE_FLD_IFINDEX              = 4 ,  /* RW */
    TBL_LSP_PE_FLD_VID                  = 5 ,  /* RW */
    TBL_LSP_PE_FLD_OUTLABEL_EXP         = 6 ,  /* RW */
    TBL_LSP_PE_FLD_OUTLABEL_EXP_IS_SET  = 7 ,  /* RW */
    TBL_LSP_PE_FLD_OUTLABEL_TTL         = 8 ,  /* RW */
    TBL_LSP_PE_FLD_OAM_SESSION_ID       = 9 ,  /* RW */
    TBL_LSP_PE_FLD_LSP_APS_GROUP_ID     = 10,  /* RW */
    TBL_LSP_PE_FLD_NEXTHOP_MAC          = 11,  /* RW */
    TBL_LSP_PE_FLD_MAX                  = 12
} tbl_lsp_pe_field_id_t;

/* TBL_LSP_PE defines */
typedef struct
{
    char                 name[LSPNAME_SIZE];
} tbl_lsp_pe_key_t;

typedef struct
{
    tbl_lsp_pe_key_t     key;
    uint32               ref_cnt;
    uint32               inlabel;
    uint32               outlabel;
    uint32               ifindex;
    uint32               vid;
    uint32               outlabel_exp;
    uint32               outlabel_exp_is_set;
    uint32               outlabel_ttl;
    uint32               oam_session_id;
    uint32               lsp_aps_group_id;
    mac_addr_t           nexthop_mac;
} tbl_lsp_pe_t;

typedef struct
{
    ctclib_hash_t        *lsp_pe_hash;
    ctclib_slist_t       *lsp_pe_list;
} tbl_lsp_pe_master_t;

#endif /* !__TBL_LSP_PE_DEFINE_H__ */

