
#ifndef __TBL_OAM_SESSION_DEFINE_H__
#define __TBL_OAM_SESSION_DEFINE_H__

/* TBL_OAM_SESSION field defines */
typedef enum
{
    TBL_OAM_SESSION_FLD_KEY                  = 0 ,  /* READ */
    TBL_OAM_SESSION_FLD_CREAT_BY_FLOW        = 1 ,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_TYPE             = 2 ,  /* RW */
    TBL_OAM_SESSION_FLD_PW_TYPE              = 3 ,  /* RW */
    TBL_OAM_SESSION_FLD_MP_TYPE              = 4 ,  /* RW */
    TBL_OAM_SESSION_FLD_SERVICE_IFINDEX      = 5 ,  /* RW */
    TBL_OAM_SESSION_FLD_SERVICE_NAME         = 6 ,  /* RW */
    TBL_OAM_SESSION_FLD_MEGID_STR            = 7 ,  /* RW */
    TBL_OAM_SESSION_FLD_MEPID                = 8 ,  /* RW */
    TBL_OAM_SESSION_FLD_RMEPID               = 9 ,  /* RW */
    TBL_OAM_SESSION_FLD_CC_ENABLE            = 10,  /* RW */
    TBL_OAM_SESSION_FLD_CC_INTERVAL          = 11,  /* RW */
    TBL_OAM_SESSION_FLD_CC_INTERVAL_CFG      = 12,  /* RW */
    TBL_OAM_SESSION_FLD_LM_ENABLE            = 13,  /* RW */
    TBL_OAM_SESSION_FLD_DM_ENABLE            = 14,  /* RW */
    TBL_OAM_SESSION_FLD_EXP                  = 15,  /* RW */
    TBL_OAM_SESSION_FLD_LEVEL                = 16,  /* RW */
    TBL_OAM_SESSION_FLD_TTL                  = 17,  /* RW */
    TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID     = 18,  /* RW */
    TBL_OAM_SESSION_FLD_BIND_OAM_IFINDEX     = 19,  /* RW */
    TBL_OAM_SESSION_FLD_ICCLEN               = 20,  /* RW */
    TBL_OAM_SESSION_FLD_DESCRIPTION          = 21,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_DM               = 22,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_LM               = 23,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_LB               = 24,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_EVENT            = 25,  /* RW */
    TBL_OAM_SESSION_FLD_P_T_SEND_DM_TIMER    = 26,  /* READ */
    TBL_OAM_SESSION_FLD_DM_ACTIVE            = 27,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_STATUS           = 28,  /* RW */
    TBL_OAM_SESSION_FLD_SIGNAL_STATUS        = 29,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_STATS            = 30,  /* RW */
    TBL_OAM_SESSION_FLD_OAM_LIST             = 31,  /* RW */
    TBL_OAM_SESSION_FLD_APS_GROUP_ID         = 32,  /* RW */
    TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW        = 33,  /* RW */
    TBL_OAM_SESSION_FLD_MAX                  = 34
} tbl_oam_session_field_id_t;

/* TBL_OAM_SESSION defines */
typedef struct
{
    uint32               session_id;
} tbl_oam_session_key_t;

typedef struct
{
    tbl_oam_session_key_t key;
    uint32               creat_by_flow;       /*flag for judge delete oam session is allowed*/
    uint32               oam_type;            /*lsp / pw/ section ....add by chenc*//*GLB_OAM_TYPE_MPLS_SECTION*/
    uint32               pw_type;             /*vpws/vpls/spe/tpe. add by chenc*//*TPOAM_PW_TYPE_TPE*/
    uint32               mp_type;             /*mep or mip. add by chenc*//*TPOAM_MP_TYPE_MEP*/
    uint32               service_ifindex;     /*for section oam*/
    char                 service_name[LSPNAME_SIZE]; /*for lsp oam and pw oam*/
    char                 megid_str[GLB_TPOAM_MEGID_STR_LEN];
    uint32               mepid;
    uint32               rmepid;
    uint32               cc_enable;
    uint32               cc_interval;
    uint32               cc_interval_cfg;
    uint32               lm_enable;
    uint32               dm_enable;
    uint8                exp;                 /*for lmep, add by chenc*/
    uint8                level;
    uint8                ttl;
    uint32               bind_oam_flow_id;
    uint32               bind_oam_ifindex;
    uint32               icclen;
    char                 description[TPOAM_DESCRIPTION_SIZE];
    tpoam_dm_t           oam_dm;
    tpoam_lm_t           oam_lm;
    tpoam_lb_t           oam_lb;
    tpoam_event_t        oam_event;
    ctc_task_t           *p_t_send_dm_timer;
    uint32               dm_active;
    uint32               oam_status;          /* glb_tpoam_evt_t bitmap */
    uint32               signal_status;       /*for CRC trigger AIS*/
    tpoam_stats_t        oam_stats;
    tpoam_list_t         oam_list;
    uint32               aps_group_id;        /*lsp aps group id or pw aps group id*/
    uint32               flag_add_flow;       /*flag for opm to init para*/
} tbl_oam_session_t;

typedef struct
{
    ctclib_hash_t        *oam_session_hash;
    ctclib_slist_t       *oam_session_list;
} tbl_oam_session_master_t;

#endif /* !__TBL_OAM_SESSION_DEFINE_H__ */

