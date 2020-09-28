
#ifndef __TBL_SSM_GLOBAL_DEFINE_H__
#define __TBL_SSM_GLOBAL_DEFINE_H__

/* TBL_SSM_GLOBAL field defines */
typedef enum
{
    TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID      = 0 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE         = 1 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY       = 2 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_OPTION_MODE          = 3 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY       = 4 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY      = 5 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_DEVICE_ROLE          = 6 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_SSM_ENABLE           = 7 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE    = 8 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_HOLDOVER_EN          = 9 ,  /* RW */
    TBL_SSM_GLOBAL_FLD_FREERUN_EN           = 10,  /* RW */
    TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX       = 11,  /* RW */
    TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX = 12,  /* RW */
    TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS  = 13,  /* RW */
    TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT = 14,  /* RW */
    TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT = 15,  /* RW */
    TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT = 16,  /* RW */
    TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX = 17,  /* RW */
    TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN = 18,  /* RW */
    TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN = 19,  /* RW */
    TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN = 20,  /* RW */
    TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME         = 21,  /* RW */
    TBL_SSM_GLOBAL_FLD_WTR_TIME             = 22,  /* RW */
    TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER   = 23,  /* READ */
    TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER       = 24,  /* READ */
    TBL_SSM_GLOBAL_FLD_MAX                  = 25
} tbl_ssm_global_field_id_t;

/* TBL_SSM_GLOBAL defines */
typedef struct
{
    ssm_clock_id_t       device_clock_id;     /* Device clock ID*/
    uint32               synce_enable;        /* Global synce enable*/
    uint32               local_priority;      /* Synce local priority*/
    uint32               option_mode;         /* Synce local quality option mode*/
    uint32               system_quality;      /* device quality, ssm_quality_t*/
    uint32               current_quality;     /* current quality, ssm_quality_t*/
    uint32               device_role;         /* Device role, ssm_device_role_t*/
    uint32               ssm_enable;          /* Global ssm enable*/
    uint32               ssm_extend_enable;   /* Global Ssm extend enable*/
    uint32               holdover_en;         /* Current status is holdover or not*/
    uint32               freerun_en;          /* Current status is freerun or not*/
    uint32               select_ifindex;      /* Select port*/
    uint32               dpll_selected_ifindex; /* Dpll selected port's index */
    uint32               dpll_current_status; /* Dpll current status */
    uint32               force_quality_option1_cnt; /* Force configured quality option1 count*/
    uint32               force_quality_option2_cnt; /* Force configured quality option2 count*/
    uint32               clock_recovery_select; /* clock recovery selected enbale*/
    uint32               next_selected_ifindex; /* The next selected port*/
    uint32               select_if_linkup_restore_en; /* Select port up restore enable*/
    uint32               select_if_priority_change_en; /* Select port priority change enable*/
    uint32               select_if_quality_change_en; /* Select port quality change enable*/
    uint32               holdoff_time;        /* Hold off time*/
    uint32               wtr_time;            /* Wait to retore time*/
    ctc_task_t           *holdoff_send_timer; /* Hold off send timer */
    ctc_task_t           *wtr_send_timer;     /* Wait to retore send timer */
} tbl_ssm_global_t;

#endif /* !__TBL_SSM_GLOBAL_DEFINE_H__ */

