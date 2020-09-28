
#ifndef __TBL_PW_TPE_DEFINE_H__
#define __TBL_PW_TPE_DEFINE_H__

/* TBL_PW_TPE field defines */
typedef enum
{
    TBL_PW_TPE_FLD_KEY                  = 0 ,  /* READ */
    TBL_PW_TPE_FLD_REF_CNT              = 1 ,  /* RW */
    TBL_PW_TPE_FLD_INLABEL              = 2 ,  /* RW */
    TBL_PW_TPE_FLD_OUTLABEL             = 3 ,  /* RW */
    TBL_PW_TPE_FLD_OUTLABEL_EXP         = 4 ,  /* RW */
    TBL_PW_TPE_FLD_OUTLABEL_EXP_IS_SET  = 5 ,  /* RW */
    TBL_PW_TPE_FLD_OUTLABEL_TTL         = 6 ,  /* RW */
    TBL_PW_TPE_FLD_TUNNEL_TYPE          = 7 ,  /* RW */
    TBL_PW_TPE_FLD_LSP_NAME             = 8 ,  /* RW */
    TBL_PW_TPE_FLD_LSP_APS_GROUP_ID     = 9 ,  /* RW */
    TBL_PW_TPE_FLD_PW_APS_GROUP_ID      = 10,  /* RW */
    TBL_PW_TPE_FLD_OAM_SESSION_ID       = 11,  /* RW */
    TBL_PW_TPE_FLD_MAX                  = 12
} tbl_pw_tpe_field_id_t;

/* TBL_PW_TPE defines */
typedef struct
{
    char                 name[LSPNAME_SIZE];
} tbl_pw_tpe_key_t;

typedef struct
{
    tbl_pw_tpe_key_t     key;
    uint32               ref_cnt;
    uint32               inlabel;
    uint32               outlabel;
    uint32               outlabel_exp;
    uint32               outlabel_exp_is_set;
    uint32               outlabel_ttl;
    uint32               tunnel_type;
    char                 lsp_name[LSPNAME_SIZE];
    uint32               lsp_aps_group_id;
    uint32               pw_aps_group_id;
    uint32               oam_session_id;
} tbl_pw_tpe_t;

typedef struct
{
    ctclib_hash_t        *pw_tpe_hash;
    ctclib_slist_t       *pw_tpe_list;
} tbl_pw_tpe_master_t;

#endif /* !__TBL_PW_TPE_DEFINE_H__ */

