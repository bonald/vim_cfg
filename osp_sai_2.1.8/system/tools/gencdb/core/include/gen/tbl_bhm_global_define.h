
#ifndef __TBL_BHM_GLOBAL_DEFINE_H__
#define __TBL_BHM_GLOBAL_DEFINE_H__

/* TBL_BHM_GLOBAL field defines */
typedef enum
{
    TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FD       = 0 ,  /* READ */
    TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN  = 1 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE   = 2 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE   = 3 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_REACTIVE             = 4 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_IS_RELOAD            = 5 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_IS_SHUTDOWN          = 6 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_IS_WARNING           = 7 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_REBOOT               = 8 ,  /* RW */
    TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_TIMER    = 9 ,  /* READ */
    TBL_BHM_GLOBAL_FLD_MAX                  = 10
} tbl_bhm_global_field_id_t;

/* TBL_BHM_GLOBAL defines */
typedef struct
{
    int32                hw_watchdog_fd;
    uint32               hw_watchdog_feed_en;
    uint32               hw_watchdog_enable;
    uint32               sw_watchdog_enable;
    uint32               reactive;            /* glb_bhm_action_t for sw_watchdog */
    uint32               is_reload;
    uint32               is_shutdown;
    uint32               is_warning;
    uint32               reboot;              /* glb_warmboot_type_t */
    ctc_task_t           *sw_watchdog_timer;
} tbl_bhm_global_t;

#endif /* !__TBL_BHM_GLOBAL_DEFINE_H__ */

