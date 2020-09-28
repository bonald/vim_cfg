
#ifndef __TBL_RMON_ALARM_DEFINE_H__
#define __TBL_RMON_ALARM_DEFINE_H__

/* TBL_RMON_ALARM field defines */
typedef enum
{
    TBL_RMON_ALARM_FLD_KEY                  = 0 ,  /* READ */
    TBL_RMON_ALARM_FLD_CONF                 = 1 ,  /* RW */
    TBL_RMON_ALARM_FLD_INTERVAL             = 2 ,  /* RW */
    TBL_RMON_ALARM_FLD_VARIABLE             = 3 ,  /* RW */
    TBL_RMON_ALARM_FLD_VARIABLE_WORD        = 4 ,  /* RW */
    TBL_RMON_ALARM_FLD_SAMPLE_TYPE          = 5 ,  /* RW */
    TBL_RMON_ALARM_FLD_VALUE                = 6 ,  /* RW */
    TBL_RMON_ALARM_FLD_STARTUP_ALARM        = 7 ,  /* RW */
    TBL_RMON_ALARM_FLD_RISING_THRESHOLD     = 8 ,  /* RW */
    TBL_RMON_ALARM_FLD_FALLING_THRESHOLD    = 9 ,  /* RW */
    TBL_RMON_ALARM_FLD_RISING_EVENT_INDEX   = 10,  /* RW */
    TBL_RMON_ALARM_FLD_FALLING_EVENT_INDEX  = 11,  /* RW */
    TBL_RMON_ALARM_FLD_OWNER                = 12,  /* RW */
    TBL_RMON_ALARM_FLD_STATUS               = 13,  /* RW */
    TBL_RMON_ALARM_FLD_LAST_EVENT           = 14,  /* RW */
    TBL_RMON_ALARM_FLD_PREV_VALUE           = 15,  /* RW */
    TBL_RMON_ALARM_FLD_CURR_VALUE           = 16,  /* RW */
    TBL_RMON_ALARM_FLD_T_ALARM_TIMER        = 17,  /* READ */
    TBL_RMON_ALARM_FLD_T_UNDER_CREATION_TIMER = 18,  /* READ */
    TBL_RMON_ALARM_FLD_MAX                  = 19
} tbl_rmon_alarm_field_id_t;

/* TBL_RMON_ALARM defines */
typedef struct
{
    uint32               index;
} tbl_rmon_alarm_key_t;

typedef struct
{
    tbl_rmon_alarm_key_t key;
    uint32               conf;
    uint32               interval;
    uint32               variable[RMON_ALARM_VARIABLE_MAXSIZE];
    char                 variable_word[RMON_ALARM_VAR_WORD_LENGTH+1];
    uint32               sample_type;         /*1- absoluteValue, 2- deltaValue */
    int64                value;
    uint32               startup_alarm;
    int32                rising_threshold;    /* For ether stats type is uint64 */
    int32                falling_threshold;
    uint32               rising_event_index;
    uint32               falling_event_index;
    char                 owner[RMON_OWNER_NAME_SIZE+1];
    uint32               status;              /* 1-Valid, 2-createRequest, *3-UnderCreation, 4- invalid, *5-nonExistent */
    uint32               last_event;
    uint64               prev_value;
    uint64               curr_value;
    ctc_task_t           *t_alarm_timer;
    ctc_task_t           *t_under_creation_timer;
} tbl_rmon_alarm_t;

typedef struct
{
    ctclib_hash_t        *rmon_alarm_hash;
    ctclib_slist_t       *rmon_alarm_list;
} tbl_rmon_alarm_master_t;

#endif /* !__TBL_RMON_ALARM_DEFINE_H__ */

