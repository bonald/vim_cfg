
#ifndef __TBL_RMON_HISTORY_DEFINE_H__
#define __TBL_RMON_HISTORY_DEFINE_H__

/* TBL_RMON_HISTORY field defines */
typedef enum
{
    TBL_RMON_HISTORY_FLD_KEY                  = 0 ,  /* READ */
    TBL_RMON_HISTORY_FLD_STATUS               = 1 ,  /* RW */
    TBL_RMON_HISTORY_FLD_OWNER                = 2 ,  /* RW */
    TBL_RMON_HISTORY_FLD_DATA                 = 3 ,  /* RW */
    TBL_RMON_HISTORY_FLD_DATA_SOURCE          = 4 ,  /* RW */
    TBL_RMON_HISTORY_FLD_BUCKETS_REQUESTED    = 5 ,  /* RW */
    TBL_RMON_HISTORY_FLD_BUCKETS_GRANTED      = 6 ,  /* RW */
    TBL_RMON_HISTORY_FLD_INTERVAL             = 7 ,  /* RW */
    TBL_RMON_HISTORY_FLD_CURRENT_SAMPLE_NO    = 8 ,  /* RW */
    TBL_RMON_HISTORY_FLD_T_UNDER_CREATION_TIMER = 9 ,  /* READ */
    TBL_RMON_HISTORY_FLD_T_COLL_HISTORY_TIMER = 10,  /* READ */
    TBL_RMON_HISTORY_FLD_MAX                  = 11
} tbl_rmon_history_field_id_t;

/* TBL_RMON_HISTORY defines */
typedef struct
{
    uint32               index;
} tbl_rmon_history_key_t;

typedef struct
{
    tbl_rmon_history_key_t key;
    uint32               status;              /* 1-Valid, 2-createRequest, *3-UnderCreation, 4- invalid, *5-nonExistent */
    char                 owner[RMON_OWNER_NAME_SIZE+1];
    uint32               data[RMON_HISTORY_CONTROL_MAXSIZE];
    uint32               data_source;         /* As of now it is ifindex */
    uint32               buckets_requested;
    uint32               buckets_granted;
    uint32               interval;
    uint32               current_sample_no;
    ctc_task_t           *t_under_creation_timer;
    ctc_task_t           *t_coll_history_timer;
} tbl_rmon_history_t;

typedef struct
{
    ctclib_hash_t        *rmon_history_hash;
    ctclib_slist_t       *rmon_history_list;
} tbl_rmon_history_master_t;

#endif /* !__TBL_RMON_HISTORY_DEFINE_H__ */

