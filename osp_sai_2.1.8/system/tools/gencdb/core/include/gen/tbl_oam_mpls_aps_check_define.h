
#ifndef __TBL_OAM_MPLS_APS_CHECK_DEFINE_H__
#define __TBL_OAM_MPLS_APS_CHECK_DEFINE_H__

/* TBL_OAM_MPLS_APS_CHECK field defines */
typedef enum
{
    TBL_OAM_MPLS_APS_CHECK_FLD_KEY                  = 0 ,  /* READ */
    TBL_OAM_MPLS_APS_CHECK_FLD_LSP_LABEL            = 1 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_IS_APS_ACTION        = 2 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_REF_CNT              = 3 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_IFINDEX              = 4 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_NEXTHOP_MAC          = 5 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_VID                  = 6 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP         = 7 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_EXP_IS_SET  = 8 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_OUTLABEL_TTL         = 9 ,  /* RW */
    TBL_OAM_MPLS_APS_CHECK_FLD_MAX                  = 10
} tbl_oam_mpls_aps_check_field_id_t;

/* TBL_OAM_MPLS_APS_CHECK defines */
typedef struct
{
    char                 name[LSPNAME_SIZE];
} tbl_oam_mpls_aps_check_key_t;

typedef struct
{
    tbl_oam_mpls_aps_check_key_t key;
    uint32               lsp_label;
    uint32               is_aps_action;
    uint32               ref_cnt;
    uint32               ifindex;
    mac_addr_t           nexthop_mac;
    uint32               vid;
    uint32               outlabel_exp;
    uint32               outlabel_exp_is_set;
    uint32               outlabel_ttl;
} tbl_oam_mpls_aps_check_t;

typedef struct
{
    ctclib_hash_t        *oam_mpls_aps_check_hash;
    ctclib_slist_t       *oam_mpls_aps_check_list;
} tbl_oam_mpls_aps_check_master_t;

#endif /* !__TBL_OAM_MPLS_APS_CHECK_DEFINE_H__ */

