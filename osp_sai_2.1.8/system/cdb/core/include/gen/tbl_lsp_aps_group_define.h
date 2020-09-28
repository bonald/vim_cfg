
#ifndef __TBL_LSP_APS_GROUP_DEFINE_H__
#define __TBL_LSP_APS_GROUP_DEFINE_H__

/* TBL_LSP_APS_GROUP field defines */
typedef enum
{
    TBL_LSP_APS_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_LSP_APS_GROUP_FLD_WORKING_LSP_NAME     = 1 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_PROTECTION_LSP_NAME  = 2 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_REF_CNT              = 3 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_FLOW_FLAG            = 4 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME = 5 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_HOLD_OFF_TIME        = 6 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_IS_REVERTIVE         = 7 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_ACTIVE_PATH          = 8 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_BRIDGE_PERMANENT     = 9 ,  /* RW */
    TBL_LSP_APS_GROUP_FLD_CURRENT_STATE        = 10,  /* RW */
    TBL_LSP_APS_GROUP_FLD_MAX                  = 11
} tbl_lsp_aps_group_field_id_t;

/* TBL_LSP_APS_GROUP defines */
typedef struct
{
    uint32               group_id;
} tbl_lsp_aps_group_key_t;

typedef struct
{
    tbl_lsp_aps_group_key_t key;
    char                 working_lsp_name[LSPNAME_SIZE];
    char                 protection_lsp_name[LSPNAME_SIZE];
    uint32               ref_cnt;
    uint32               flow_flag;           /* If set means aps group id is added by openflow*/
    uint32               wait_to_restore_time; /* Wait-to-restore time value  */
    uint32               hold_off_time;       /* hold-off time value */
    uint32               is_revertive;        /* Revertive (1) or non-revertive (0) */
    uint32               active_path;         /* None, Working, Protection*/
    uint32               bridge_permanent;    /* 0 means 1:1 and 1 means 1+1 */
    uint32               current_state;       /* Current state to be communicated to far end */
} tbl_lsp_aps_group_t;

typedef struct
{
    ctclib_hash_t        *lsp_aps_group_hash;
    ctclib_slist_t       *lsp_aps_group_list;
} tbl_lsp_aps_group_master_t;

#endif /* !__TBL_LSP_APS_GROUP_DEFINE_H__ */

