
/* Task library.
 * Copyright (C) 2013 Centec Networks Inc.  All rights reserved.
 */

#ifndef __CTC_TASK_H__
#define __CTC_TASK_H__

#define TIMESTAMP_DECLARE() \
    struct timeval _tv1, _tv2;

#define TIMESTAMP_TS1() \
    gettimeofday(&_tv1, NULL);

#define TIMESTAMP_TS2() \
    gettimeofday(&_tv2, NULL);

#define TIMESTAMP_LOG(_STR_) \
    log_sys(M_MOD_LIB, E_ERROR, "%s timestamp delta is %u us", _STR_, (_tv2.tv_sec - _tv1.tv_sec)*1000000 + (_tv2.tv_usec - _tv1.tv_usec));

#define TIMESTAMP_TS2_LOG(_STR_) \
    TIMESTAMP_TS2(); \
    TIMESTAMP_LOG(_STR_);

/*job scheduler api related*/
#define TASK_PRI_HIGH     0   /* highest priority */
#define TASK_PRI_NORMAL   1
#define TASK_PRI_LOW      2
#define TASK_PRI_NUM      3

#define TASK_FD_READ      0
#define TASK_FD_WRITE     1
#define TASK_TIMER        2
#define TASK_EVENT        3
#define TASK_TASK         4
#define TASK_UNUSED       5

#define CTC_TASK_IS_RUNNING(_TASK) (_TASK && (_TASK->type != TASK_UNUSED))

#define CTC_TASK_STOP_TIMER(_TIMER) \
do { \
    if ((_TIMER)) \
    { \
        ctc_task_delete_timer((_TIMER)); \
        (_TIMER) = NULL; \
    } \
} while (0)

typedef void (*CTC_TASK_HANDLER)(void* p_data);

#define CTC_TASK_GET_MASTER
#define CTC_TASK_ADD_TIME(_FUNC_, _PTR_, _MS_)      ctc_task_add_timer(TASK_PRI_NORMAL, _FUNC_, (void*)_PTR_, _MS_)
#define CTC_TASK_ADD_TIME_MSEC(_FUNC_, _PTR_, _MS_) ctc_task_add_timer_msec(TASK_PRI_NORMAL, _FUNC_, (void*)_PTR_, _MS_)
#define CTC_TASK_DEL_TIME(_TASK_) \
do { \
    ctc_task_delete_timer((_TASK_)); \
    (_TASK_) = NULL; \
} while (0)
#define CTC_TASK_ADD_READ(_FUNC_, _PTR_, _FD_)      ctc_task_add_read(TASK_PRI_NORMAL, _FUNC_, (void*)_PTR_, _FD_)
#define CTC_TASK_ADD_READ_HIGH(_FUNC_, _PTR_, _FD_) ctc_task_add_read(TASK_PRI_HIGH, _FUNC_, (void*)_PTR_, _FD_)
#define CTC_TASK_DEL_READ(_TASK_) \
do { \
    ctc_task_delete_read((_TASK_)); \
    (_TASK_) = NULL; \
} while (0)

struct ctc_task_master_s;

typedef struct ctc_task_s
{
    struct ctc_task_s          *next;
    struct ctc_task_s          *prev;
    
    int32                       type;
    struct ctc_task_master_s   *p_master;
    int32                       priority;
    void                       *p_iv[16];   /* ivykis data */
} ctc_task_t;

typedef struct ctc_task_hdr_s
{
    struct ctc_task_s          *next;
    struct ctc_task_s          *prev;
    
    int32                       type;
    struct ctc_task_master_s   *p_master;
    int32                       priority;
} ctc_task_hdr_t;

typedef struct
{
    struct ctc_task_s* head;
    struct ctc_task_s* tail;
    int32  count;
    int32  max_count;
} ctc_task_list_t;

typedef struct ctc_task_master_s
{
    ctc_task_list_t timer;
    ctc_task_list_t event;
    ctc_task_list_t read;
    ctc_task_list_t write;
    ctc_task_list_t task;
    
    /**/
    ctc_task_list_t queue[TASK_PRI_NUM];
    ctc_task_list_t unused;
    int32       alloc;
} ctc_task_master_t;

ctc_task_master_t*
ctc_task_init();

ctc_task_master_t*
ctc_task_get_master();

ctc_task_t*
ctc_task_add_timer(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 sec);
ctc_task_t*
ctc_task_add_timer_msec(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 msec);
ctc_task_t*
ctc_task_add_read(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 fd);
ctc_task_t*
ctc_task_add_write(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 fd);
int32
ctc_task_set_write(ctc_task_t *p_task, CTC_TASK_HANDLER handler);
ctc_task_t*
ctc_task_add_event(int32 prio, CTC_TASK_HANDLER handler, void *p_arg);
ctc_task_t*
ctc_task_add_task(int32 prio, CTC_TASK_HANDLER handler, void *p_arg);

void
ctc_task_delete_timer(ctc_task_t *p_task);
void
ctc_task_delete_read(ctc_task_t *p_task);
void
ctc_task_delete_write(ctc_task_t *p_task);
void
ctc_task_delete_event(ctc_task_t *p_task);
void
ctc_task_delete_task(ctc_task_t *p_task);

void
ctc_task_main();

/* Added by Alexander 2014-12-30 14:22 */
void ctc_task_main_cowork(struct timeval *(*coworkcb)(struct timeval *));

void
ctc_task_deinit();

void
ctc_task_dump();

struct sal_timeval*
ctc_task_update_boottime(struct sal_timeval *tv);

struct sal_timeval *
ctc_task_boottime(struct sal_timeval *tv);

sal_time_t
ctc_time_boottime_sec(sal_time_t *tp);

sal_time_t
ctc_time_boottime_msec(sal_time_t *tp);

int32
ctc_timer_get_remain_sec(const ctc_task_t *p_task);

int32
ctc_timer_get_remain_msec(const ctc_task_t *p_task);

char*
ctc_time_seconds_to_ymd_hms(uint32 seconds, char* str, uint32 str_len);

char*
ctc_task_uptime_to_str(sal_time_t time, char *str, uint32 str_len);

char*
ctc_task_time_to_str(sal_time_t time, char *str, uint32 str_len);

char*
ctc_task_val2str(char *str, uint32 str_len, const ctc_task_t *p_task);

int32
ctc_task_str2val(char *str, ctc_task_t *p_task);

typedef void (*CTC_TASK_BF_POLL_CB)(void);
void
ctc_task_register_before_poll_cb(CTC_TASK_BF_POLL_CB cb);

#endif /* __CTC_TASK_H__ */

