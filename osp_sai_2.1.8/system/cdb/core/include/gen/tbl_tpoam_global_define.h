
#ifndef __TBL_TPOAM_GLOBAL_DEFINE_H__
#define __TBL_TPOAM_GLOBAL_DEFINE_H__

/* TBL_TPOAM_GLOBAL field defines */
typedef enum
{
    TBL_TPOAM_GLOBAL_FLD_NODE_ID              = 0 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE         = 1 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN          = 2 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_RDI_REASON           = 3 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_RDI_REASON_MASK      = 4 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK       = 5 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_SD_REASON_MASK       = 6 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX    = 7 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_CURR   = 8 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_EVENT_RULE_MAP       = 9 ,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_OAM_GLB_STATS        = 10,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_EXEC                 = 11,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_TRAP_CACHE           = 12,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_TRAP                 = 13,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_LOG                  = 14,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL     = 15,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD   = 16,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL     = 17,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL     = 18,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_LM_LOCAL_THRESHOLD   = 19,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_LM_REMOTE_THRESHOLD  = 20,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL     = 21,  /* RW */
    TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL_TIMER = 22,  /* READ */
    TBL_TPOAM_GLOBAL_FLD_MAX                  = 23
} tbl_tpoam_global_field_id_t;

/* TBL_TPOAM_GLOBAL defines */
typedef struct
{
    uint32               node_id;             /*MPLS-TP NODE-ID, used by MIP. cli mpls-tp node-id A.B.C.D*/
    uint32               channel_type;        /*MPLS-TP OAM Y.1731 channel type, default is 0x8902. cli mpls-tp oam-y1731 channel-type CHANNELTYPE*/
    uint32               fast_aps_en;         /*do APS in HSRV. cli mpls-tp oam-y1731 fast-aps enable*/
    uint32               rdi_reason;          /*GLB_TPOAM_RDI_RSN_LOC. cli mpls-tp oam-y1731 rdi-reason*/
    uint32               rdi_reason_mask;     /*GLB_TPOAM_EVT_XXX. cli mpls-tp oam-y1731 rdi-reason*/
    uint32               sf_reason_mask;      /*GLB_TPOAM_EVT_XXX. cli mpls-tp oam-y1731 sf-reason*/
    uint32               sd_reason_mask;      /*GLB_TPOAM_EVT_XXX. cli mpls-tp oam-y1731 sd-reason*/
    tpoam_rc_count_t     resouce_count_max;   /* MAX resouce count */
    tpoam_rc_count_t     resouce_count_curr;  /* current resouce count *//*section ...*/
    uint32               event_rule_map[TPOAM_EVT_DEST_MAX]; /*cli mpls-tp oam-y1731 event-rule*/
    tpoam_stats_t        oam_glb_stats;
    tpoam_exec_t         exec;                /*current exec oam test*/
    tpoam_trap_cache_t   trap_cache;
    tpoam_ratelimit_t    trap;
    tpoam_ratelimit_t    log;
    uint32               dm_enable_global;
    uint32               dm_delay_threshold;
    uint32               dm_calc_interval;
    uint32               lm_enable_global;
    uint32               lm_local_threshold;
    uint32               lm_remote_threshold;
    uint32               lm_calc_interval;
    ctc_task_t           *lm_calc_interval_timer;
} tbl_tpoam_global_t;

#endif /* !__TBL_TPOAM_GLOBAL_DEFINE_H__ */

