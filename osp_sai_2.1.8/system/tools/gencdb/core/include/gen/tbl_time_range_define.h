
#ifndef __TBL_TIME_RANGE_DEFINE_H__
#define __TBL_TIME_RANGE_DEFINE_H__

/* TBL_TIME_RANGE field defines */
typedef enum
{
    TBL_TIME_RANGE_FLD_KEY                  = 0 ,  /* RW */
    TBL_TIME_RANGE_FLD_TIME                 = 1 ,  /* RW */
    TBL_TIME_RANGE_FLD_REF_CNT              = 2 ,  /* READ */
    TBL_TIME_RANGE_FLD_RUNNING_TIMER        = 3 ,  /* READ */
    TBL_TIME_RANGE_FLD_MAX                  = 4 
} tbl_time_range_field_id_t;

/* TBL_TIME_RANGE defines */
typedef struct
{
    char                 name[TIME_RANGE_NAME_SIZE+1];
} tbl_time_range_key_t;

typedef struct
{
    tbl_time_range_key_t key;
    time_range_value_t   time;
    uint32               ref_cnt;             /* ace using this time-range, no matter is ace is appiled, for CLI check */
    time_range_timer_t   running_timer;       /*time-range running ace list*/
} tbl_time_range_t;

typedef void (*TIME_RANGE_CB)(void* arg, bool invalid);

typedef struct
{
    ctclib_hash_t        *time_range_hash;
    ctclib_slist_t       *time_range_list;
    TIME_RANGE_CB        invalid_func[TIME_RANGE_ARG_TYPE_MAX];
} tbl_time_range_master_t;

#endif /* !__TBL_TIME_RANGE_DEFINE_H__ */

