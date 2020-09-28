
#ifndef __TBL_LOG_DEFINE_H__
#define __TBL_LOG_DEFINE_H__

/* TBL_LOG field defines */
typedef enum
{
    TBL_LOG_FLD_KEY                  = 0 ,  /* READ */
    TBL_LOG_FLD_LOG_ID               = 1 ,  /* READ */
    TBL_LOG_FLD_SEVERITY             = 2 ,  /* READ */
    TBL_LOG_FLD_TIMESTAMP            = 3 ,  /* READ */
    TBL_LOG_FLD_DATA                 = 4 ,  /* READ */
    TBL_LOG_FLD_MAX                  = 5 
} tbl_log_field_id_t;

/* TBL_LOG defines */
typedef struct
{
    uint32               sequence_id;
} tbl_log_key_t;

typedef struct
{
    tbl_log_key_t        key;
    uint16               log_id;
    uint16               severity;
    timestamp_t          timestamp;
    char                 data[LOG_DATA_LEN];
} tbl_log_t;

typedef struct
{
    tbl_log_t            *array[LOG_ITEM_COUNT];
    uint32               curr_index;
    uint32               curr_sequence_id;
} tbl_log_master_t;

#endif /* !__TBL_LOG_DEFINE_H__ */

