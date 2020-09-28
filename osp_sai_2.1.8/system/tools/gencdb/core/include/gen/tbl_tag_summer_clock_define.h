
#ifndef __TBL_TAG_SUMMER_CLOCK_DEFINE_H__
#define __TBL_TAG_SUMMER_CLOCK_DEFINE_H__

/* TBL_TAG_SUMMER_CLOCK field defines */
typedef enum
{
    TBL_TAG_SUMMER_CLOCK_FLD_TMSTART              = 0 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_TMEND                = 1 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET           = 2 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING         = 3 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY         = 4 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY           = 5 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME          = 6 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM           = 7 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START        = 8 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END          = 9 ,  /* RW */
    TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER  = 10,  /* READ */
    TBL_TAG_SUMMER_CLOCK_FLD_MAX                  = 11
} tbl_tag_summer_clock_field_id_t;

/* TBL_TAG_SUMMER_CLOCK defines */
typedef struct
{
    summer_time_t        tmstart;
    summer_time_t        tmend;
    int32                iDstOffset;
    int32                is_recurring;
    int32                nStartFebDay;
    int32                nEndFebDay;
    char                 strZoneName[33];
    int32                is_wday_tm;
    which_summer_time_t  wday_tm_start;
    which_summer_time_t  wday_tm_end;
    ctc_task_t           *t_summer_time_timer;
} tbl_tag_summer_clock_t;

#endif /* !__TBL_TAG_SUMMER_CLOCK_DEFINE_H__ */

