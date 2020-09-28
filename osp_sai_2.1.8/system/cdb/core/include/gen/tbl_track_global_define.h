
#ifndef __TBL_TRACK_GLOBAL_DEFINE_H__
#define __TBL_TRACK_GLOBAL_DEFINE_H__

/* TBL_TRACK_GLOBAL field defines */
typedef enum
{
    TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM     = 0 ,  /* READ */
    TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL = 1 ,  /* RW */
    TBL_TRACK_GLOBAL_FLD_T_TRACK_TIMER        = 2 ,  /* READ */
    TBL_TRACK_GLOBAL_FLD_MAX                  = 3 
} tbl_track_global_field_id_t;

/* TBL_TRACK_GLOBAL defines */
typedef struct
{
    uint16               track_object_num;    /* configured track object number */
    uint16               track_timer_interval; /* track timer update interval */
    ctc_task_t           *t_track_timer;      /* TRACK update timer */
} tbl_track_global_t;

#endif /* !__TBL_TRACK_GLOBAL_DEFINE_H__ */

