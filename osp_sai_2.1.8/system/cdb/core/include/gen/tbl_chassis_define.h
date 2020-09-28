
#ifndef __TBL_CHASSIS_DEFINE_H__
#define __TBL_CHASSIS_DEFINE_H__

/* TBL_CHASSIS field defines */
typedef enum
{
    TBL_CHASSIS_FLD_TYPE                 = 0 ,  /* READ */
    TBL_CHASSIS_FLD_SLOT_NUM             = 1 ,  /* READ */
    TBL_CHASSIS_FLD_CUR_STM_MODE         = 2 ,  /* RW */
    TBL_CHASSIS_FLD_NEXT_STM_MODE        = 3 ,  /* RW */
    TBL_CHASSIS_FLD_MAC_NUM              = 4 ,  /* RW */
    TBL_CHASSIS_FLD_SYS_MAC              = 5 ,  /* RW */
    TBL_CHASSIS_FLD_LOCATION_LED         = 6 ,  /* RW */
    TBL_CHASSIS_FLD_RESET_REBOOT_INFO    = 7 ,  /* RW */
    TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO  = 8 ,  /* RW */
    TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM = 9 ,  /* READ */
    TBL_CHASSIS_FLD_SCHEDULE_REBOOT_TIMER = 10,  /* RW */
    TBL_CHASSIS_FLD_SCHEDULE_REBOOT_ALARM_TIMER = 11,  /* READ */
    TBL_CHASSIS_FLD_SCHEDULE_REBOOT      = 12,  /* READ */
    TBL_CHASSIS_FLD_MAX                  = 13
} tbl_chassis_field_id_t;

/* TBL_CHASSIS defines */
typedef struct
{
    int32                type;
    int32                slot_num;
    int32                cur_stm_mode;
    int32                next_stm_mode;
    int32                mac_num;
    mac_addr_t           sys_mac;
    uint32               location_led;        /* use to locate switch in lab, 0 means off, 1 menas on*/
    uint32               reset_reboot_info;   /* reset event*/
    uint32               support_reboot_info; /* if this card support save reboot info, 1 means yes*/
    uint32               one_minute_reboot_alarm; /* if updated, means system will reboot after 1 minute */
    ctc_task_t           *schedule_reboot_timer; /* schedule reboot timer */
    ctc_task_t           *schedule_reboot_alarm_timer; /* schedule reboot alarm timer */
    uint32               schedule_reboot;     /* if set, means system in schedule reboot */
} tbl_chassis_t;

#endif /* !__TBL_CHASSIS_DEFINE_H__ */

