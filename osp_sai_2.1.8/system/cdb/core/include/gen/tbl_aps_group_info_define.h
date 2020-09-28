
#ifndef __TBL_APS_GROUP_INFO_DEFINE_H__
#define __TBL_APS_GROUP_INFO_DEFINE_H__

/* TBL_APS_GROUP_INFO field defines */
typedef enum
{
    TBL_APS_GROUP_INFO_FLD_KEY                  = 0 ,  /* READ */
    TBL_APS_GROUP_INFO_FLD_WORKING_NAME         = 1 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME      = 2 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_SELECT_NAME          = 3 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_APS_GROUP_TYPE       = 4 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME = 5 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME        = 6 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE         = 7 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH          = 8 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT     = 9 ,  /* RW */
    TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT        = 10,  /* RW */
    TBL_APS_GROUP_INFO_FLD_DFOP_REASON          = 11,  /* RW */
    TBL_APS_GROUP_INFO_FLD_SC_APS_INTERVEL      = 12,  /* RW */
    TBL_APS_GROUP_INFO_FLD_FAST_APS_EN          = 13,  /* RW */
    TBL_APS_GROUP_INFO_FLD_APS_CHANNEL          = 14,  /* RW */
    TBL_APS_GROUP_INFO_FLD_REQUESTED_SIGNAL     = 15,  /* RW */
    TBL_APS_GROUP_INFO_FLD_DIRECTION            = 16,  /* RW */
    TBL_APS_GROUP_INFO_FLD_BRIDGE_TYPE          = 17,  /* RW */
    TBL_APS_GROUP_INFO_FLD_LAST_STATE           = 18,  /* RW */
    TBL_APS_GROUP_INFO_FLD_CURRENT_STATE        = 19,  /* RW */
    TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT   = 20,  /* RW */
    TBL_APS_GROUP_INFO_FLD_WTR_TIMER            = 21,  /* READ */
    TBL_APS_GROUP_INFO_FLD_SF_HOLD_OFF_TIMER    = 22,  /* READ */
    TBL_APS_GROUP_INFO_FLD_SFP_HOLD_OFF_TIMER   = 23,  /* READ */
    TBL_APS_GROUP_INFO_FLD_SD_HOLD_OFF_TIMER    = 24,  /* READ */
    TBL_APS_GROUP_INFO_FLD_SDP_HOLD_OFF_TIMER   = 25,  /* READ */
    TBL_APS_GROUP_INFO_FLD_SC_OUT_APS_TIMER     = 26,  /* READ */
    TBL_APS_GROUP_INFO_FLD_APS_TX_TIMER         = 27,  /* READ */
    TBL_APS_GROUP_INFO_FLD_SC_APS_OUT_PDU_COUNT = 28,  /* RW */
    TBL_APS_GROUP_INFO_FLD_HOLD_OFF_EXPIRED     = 29,  /* RW */
    TBL_APS_GROUP_INFO_FLD_MAX                  = 30
} tbl_aps_group_info_field_id_t;

/* TBL_APS_GROUP_INFO defines */
typedef struct
{
    uint32               aps_group_id;
} tbl_aps_group_info_key_t;

typedef struct
{
    tbl_aps_group_info_key_t key;
    char                 working_name[LSPNAME_SIZE];
    char                 protection_name[LSPNAME_SIZE];
    char                 select_name[LSPNAME_SIZE];
    char                 aps_group_type;      /* PW or LSP*/
    uint32               wait_to_restore_time; /* Wait-to-restore time value  */
    uint32               hold_off_time;       /* hold-off time value */
    uint32               is_revertive;        /* Revertive (1) or non-revertive (0) */
    uint32               active_path;         /* None, Working, Protection*/
    uint32               bridge_permanent;    /* 0 means 1:1 and 1 means 1+1 */
    uint32               current_event;       /* current event  -- incoming event. can be APS request, CFM trigger  */
    uint32               dfop_reason;         /* reason for dfop */
    uint32               sc_aps_intervel;     /* interval for out going APS PDU when state change happens */
    uint32               fast_aps_en;         /* fast aps en */
    uint32               aps_channel;         /* Whether APS is enabled or not ( With APS or Without APS */
    uint32               requested_signal;    /* Requested signal NULL (0) /NORMAL (1)*/
    uint32               direction;           /* Unidirectioanal (0) or bidirectional  (1) */
    uint32               bridge_type;         /* T bit value */
    uint32               last_state;          /* last state as per the FSM */
    uint32               current_state;       /* Current state to be communicated to far end */
    uint32               far_end_last_event;  /* Remote end last state */
    ctc_task_t           *wtr_timer;          /* wait-to-restore timer */
    ctc_task_t           *sf_hold_off_timer;  /* Signal fail hold-off timer */
    ctc_task_t           *sfp_hold_off_timer; /* Signal fail to protection timer */
    ctc_task_t           *sd_hold_off_timer;  /* Signal Degress hold-off timer */
    ctc_task_t           *sdp_hold_off_timer; /* Signal Degress to protection timer*/
    ctc_task_t           *sc_out_aps_timer;   /* Three consecutive time each with configured APS PDU interval for state change and exists*/
    ctc_task_t           *aps_tx_timer;       /* Retransmission timer */
    uint32               sc_aps_out_pdu_count; /* APS PDU's sent after state change, need to send 3 consecutive APS PDU's*/
    uint32               hold_off_expired;    /* indicates whether hold-off expires of not */
} tbl_aps_group_info_t;

typedef struct
{
    ctclib_hash_t        *aps_group_info_hash;
    ctclib_slist_t       *aps_group_info_list;
} tbl_aps_group_info_master_t;

#endif /* !__TBL_APS_GROUP_INFO_DEFINE_H__ */

