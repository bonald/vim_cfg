
#ifndef __TBL_OAM_FLOW_INFO_DEFINE_H__
#define __TBL_OAM_FLOW_INFO_DEFINE_H__

/* TBL_OAM_FLOW_INFO field defines */
typedef enum
{
    TBL_OAM_FLOW_INFO_FLD_KEY                  = 0 ,  /* READ */
    TBL_OAM_FLOW_INFO_FLD_TYPE                 = 1 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_NAME                 = 2 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_SESSION_ID           = 3 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_OUTLABEL_FLOW_ID     = 4 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_INLABEL_FLOW_ID      = 5 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_OUTLABEL_GROUP_ID    = 6 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_INLABEL_GROUP_ID     = 7 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_IFINDEX              = 8 ,  /* RW */
    TBL_OAM_FLOW_INFO_FLD_MAX                  = 9 
} tbl_oam_flow_info_field_id_t;

/* TBL_OAM_FLOW_INFO defines */
typedef struct
{
    uint32               flow_id;
} tbl_oam_flow_info_key_t;

typedef struct
{
    tbl_oam_flow_info_key_t key;
    uint32               type;
    char                 name[LSPNAME_SIZE];
    uint32               session_id;
    uint32               outlabel_flow_id;
    uint32               inlabel_flow_id;
    uint32               outlabel_group_id;
    uint32               inlabel_group_id;
    uint32               ifindex;
} tbl_oam_flow_info_t;

typedef struct
{
    ctclib_hash_t        *oam_flow_info_hash;
    ctclib_slist_t       *oam_flow_info_list;
} tbl_oam_flow_info_master_t;

#endif /* !__TBL_OAM_FLOW_INFO_DEFINE_H__ */

