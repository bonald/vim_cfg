
#include <stdio.h>
#include <stdlib.h>
#include "sal.h"
#include "sal_types.h"
#include "ctc_task.h"
#include "iv.h"
#include "iv_event.h"

#define TIME_SECOND_PER_DAY      86400
#define TIME_HOUR_PER_DAY        24
#define TIME_MINUTE_PER_HOUR     60
#define TIME_SECOND_PER_MINUTE   60
#define TIME_SECOND_PER_HOUR     3600
#define TIME_DAY_PER_YEAR        365
#define TIME_DAY_PER_MONTH       30
#define TIME_MONTH_PER_YEAR      12

#define TASK_UNUSED_MAX 512

#define ONE_DAY_SECOND      (60*60*24)

#define CTC_TASK_LOG    printf

#define ctc_task_list_for_each(N, L) \
    for ((N) = (L)->head; (N); (N) = (N)->next)

#define ctc_task_list_for_each_del(N, NN, L) \
    if ((L) && (L)->head) \
        for ((N)=(L)->head, NN=((N)!=NULL)?(N)->next:NULL; \
             (N); \
             (N)=(NN), NN=((N)!=NULL)?(N)->next:NULL)


static CTC_TASK_BF_POLL_CB g_ctc_task_before_poll_cb = NULL;
static ctc_task_master_t* _g_task_master = NULL;

/* get boot time */
sal_time_t _g_ctclib_last_upsec = 0;
struct sal_timeval _g_ctclib_last_boottime = {0, 0};

static inline struct sal_timeval *
_ctclib_time_boottime(struct sal_timeval *tv)
{
    /* modified by kcao for bug 13579, 2010-11-23 
     * (struct timezone *)-1 means get relative time/uptime 
     * refer to kernel/time.c sys_gettimeofday()
     */
    #ifndef _GLB_UML_SYSTEM_
    /* modified by jqiu 2013-03-06
     * some platform don't support param -1 in gettimeofday.
     * use another sys call clock_gettime() */ 
    sal_ctc_clock_gettime(tv);
    #else
#ifdef _KERNEL_VER_2_6_    
    gettimeofday(tv, (struct timezone *)-1);
#else
    struct timespec temp_tv;
    sal_memset(&temp_tv, 0, sizeof(temp_tv));
    clock_gettime(CLOCK_MONOTONIC, &temp_tv);
    tv->tv_sec = temp_tv.tv_sec;
    tv->tv_usec = temp_tv.tv_nsec / 1000;
#endif
    #endif
    
    _g_ctclib_last_upsec = tv->tv_sec;
    _g_ctclib_last_boottime.tv_sec = tv->tv_sec;
    _g_ctclib_last_boottime.tv_usec = tv->tv_usec;
    return tv;
}

void
_ctc_task_update_boottime_cb()
{
    struct sal_timeval tv;
    _ctclib_time_boottime(&tv);
}

static void
_ctc_task_list_add_node(ctc_task_list_t *p_list, ctc_task_t *p_task)
{
    p_task->next = NULL;
    p_task->prev = p_list->tail;
    if (p_list->tail)
    {
        p_list->tail->next = p_task;
    }
    else
    {
        p_list->head = p_task;
    }
    p_list->tail = p_task;
    p_list->count++;

    if (p_list->count > p_list->max_count)
    {
        p_list->max_count = p_list->count;
    }
}

ctc_task_t*
ctc_task_list_delete_node(ctc_task_list_t *p_list, ctc_task_t *p_task)
{
    if (p_task->next)
    {
        p_task->next->prev = p_task->prev;
    }
    else
    {
        p_list->tail = p_task->prev;
    }
    
    if (p_task->prev)
    {
        p_task->prev->next = p_task->next;
    }
    else
    {
        p_list->head = p_task->next;
    }
    p_task->next = p_task->prev = NULL;
    p_list->count--;
    return p_task;
}

static ctc_task_t *
_ctc_task_trim_head(ctc_task_list_t *p_list)
{
    if (p_list->head)
    {
        return ctc_task_list_delete_node(p_list, p_list->head);
    }
    return NULL;
}

static ctc_task_t*
_ctc_task_get_new(ctc_task_master_t *p_master, int32 prio, int32 type)
{
    ctc_task_t *p_task = NULL;

    if (p_master->unused.head)
    {
        p_task = _ctc_task_trim_head(&p_master->unused);
    }
    else
    {
        p_task = sal_calloc(sizeof(ctc_task_t));
        if (NULL == p_task)
        {
            return NULL;
        }
        p_master->alloc++;
    }
    p_task->type = type;
    p_task->priority = prio;
    p_task->p_master = p_master;
    
    return p_task;
}

void
ctc_task_add_unuse(ctc_task_master_t *p_master, ctc_task_t *p_task)
{
    SAL_ASSERT(TASK_UNUSED == p_task->type);
    p_task->type = TASK_UNUSED;
    if (p_master->unused.count >= TASK_UNUSED_MAX)
    {
        sal_free(p_task);
    }
    else
    {
        _ctc_task_list_add_node(&p_master->unused, p_task);
    }
}

ctc_task_t*
ctc_task_add_timer(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 sec)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t *p_task = NULL;
    struct iv_timer *p_timer = NULL;
    struct sal_timeval tv_now;
    struct timespec *iv_tv;
    
    p_task = _ctc_task_get_new(p_master, prio, TASK_TIMER);
    if (NULL == p_task)
    {
        return NULL;
    }
    
    p_timer = (struct iv_timer*)p_task->p_iv;

    /* Init IV timer */
    IV_TIMER_INIT(p_timer);
#if 0
    iv_validate_now();
    p_timer->expires = iv_now;
#else
    _ctclib_time_boottime(&tv_now);
    p_timer->expires.tv_nsec = tv_now.tv_usec * 1000;
    p_timer->expires.tv_sec = tv_now.tv_sec;
    iv_tv = __iv_now_location();
    *iv_tv = p_timer->expires;
#endif
    p_timer->expires.tv_sec += sec;
    p_timer->cookie = (void *)p_arg;
    p_timer->handler = handler;
    iv_timer_register(p_timer);

    _ctc_task_list_add_node(&p_master->timer, p_task);
    return p_task;
}

ctc_task_t*
ctc_task_add_timer_msec(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 msec)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t *p_task = NULL;
    struct iv_timer *p_timer = NULL;
    struct sal_timeval tv_now;
    struct timespec *iv_tv;
    
    p_task = _ctc_task_get_new(p_master, prio, TASK_TIMER);
    if (NULL == p_task)
    {
        return NULL;
    }
    
    p_timer = (struct iv_timer*)p_task->p_iv;
    
    /* Init IV timer */
    IV_TIMER_INIT(p_timer);
#if 0
    iv_validate_now();
    p_timer->expires = iv_now;
#else
    _ctclib_time_boottime(&tv_now);
    p_timer->expires.tv_nsec = tv_now.tv_usec * 1000;
    p_timer->expires.tv_sec = tv_now.tv_sec;
    iv_tv = __iv_now_location();
    *iv_tv = p_timer->expires;
#endif
    p_timer->expires.tv_nsec += 1000000 * (msec%1000);
    p_timer->expires.tv_sec += msec/1000;
    if (p_timer->expires.tv_nsec > 1000000000)
    {
        p_timer->expires.tv_nsec -= 1000000000;
        p_timer->expires.tv_sec++;
    }
    p_timer->cookie = (void *)p_arg;
    p_timer->handler = handler;
    iv_timer_register(p_timer);

    _ctc_task_list_add_node(&p_master->timer, p_task);
    return p_task;
}

ctc_task_t*
ctc_task_add_read(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 fd)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t *p_task = NULL;
    struct iv_fd *p_fd = NULL;

    if (fd < 0)
    {
        return NULL;
    }
    
    p_task = _ctc_task_get_new(p_master, prio, TASK_FD_READ);
    if (NULL == p_task)
    {
        return NULL;
    }
    
    p_fd = (struct iv_fd*)p_task->p_iv;

    /* Init IV fd */
    IV_FD_INIT(p_fd);
    p_fd->fd = fd;
    p_fd->cookie = (void *)p_arg;
    p_fd->handler_in = handler;
    iv_fd_register(p_fd);

    _ctc_task_list_add_node(&p_master->read, p_task);
    return p_task;
}

ctc_task_t*
ctc_task_add_write(int32 prio, CTC_TASK_HANDLER handler, void *p_arg, int32 fd)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t *p_task = NULL;
    struct iv_fd *p_fd = NULL;

    if (fd < 0)
    {
        return NULL;
    }
    
    p_task = _ctc_task_get_new(p_master, prio, TASK_FD_WRITE);
    if (NULL == p_task)
    {
        return NULL;
    }
    
    p_fd = (struct iv_fd*)p_task->p_iv;

    /* Init IV fd */
    IV_FD_INIT(p_fd);
    p_fd->fd = fd;
    p_fd->cookie = (void *)p_arg;
    p_fd->handler_out = handler;
    iv_fd_register(p_fd);

    _ctc_task_list_add_node(&p_master->read, p_task);
    return p_task;
}

int32
ctc_task_unset_write(ctc_task_t *p_task)
{
    struct iv_fd *p_fd = NULL;

    if (TASK_FD_READ != p_task->type)
    {
        return -1;
    }
    p_fd = (struct iv_fd*)p_task->p_iv;
    iv_fd_set_handler_out(p_fd, NULL);

    return 0;
}

int32
ctc_task_set_write(ctc_task_t *p_task, CTC_TASK_HANDLER handler)
{
    struct iv_fd *p_fd = NULL;

    if (TASK_FD_READ != p_task->type)
    {
        return -1;
    }
    p_fd = (struct iv_fd*)p_task->p_iv;
    iv_fd_set_handler_out(p_fd, handler);

    return 0;
}

ctc_task_t*
ctc_task_add_event(int32 prio, CTC_TASK_HANDLER handler, void *p_arg)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t *p_task = NULL;
    struct iv_event *p_event = NULL;
    
    p_task = _ctc_task_get_new(p_master, prio, TASK_EVENT);
    if (NULL == p_task)
    {
        return NULL;
    }
    
    p_event = (struct iv_event*)p_task->p_iv;

    /* Init IV event */
    p_event->cookie = (void *)p_arg;
    p_event->handler = handler;
    iv_event_register(p_event);

    _ctc_task_list_add_node(&p_master->event, p_task);
    return p_task;
}

ctc_task_t*
ctc_task_add_task(int32 prio, CTC_TASK_HANDLER handler, void *p_arg)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t *p_task = NULL;
    struct iv_task *p_iv_task = NULL;
    
    p_task = _ctc_task_get_new(p_master, prio, TASK_TASK);
    if (NULL == p_task)
    {
        return NULL;
    }
    
    p_iv_task = (struct iv_task*)p_task->p_iv;

    /* Init IV task */
    IV_TASK_INIT(p_iv_task);
    p_iv_task->cookie = (void *)p_arg;
    p_iv_task->handler = handler;
    iv_task_register(p_iv_task);

    _ctc_task_list_add_node(&p_master->task, p_task);
    return p_task;
}

void
ctc_task_delete_timer(ctc_task_t *p_task)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    struct iv_timer *p_timer = NULL;
    int index = 0;

    if (TASK_UNUSED == p_task->type)
    {
        return;
    }
    
    p_timer = (struct iv_timer*)p_task->p_iv;

    index = iv_timer_get_index(p_timer);
    if (0 == index)
    {
        /* time is in expired list */
        iv_timer_unregister(p_timer);
        p_task->type = TASK_UNUSED;
        ctc_task_add_unuse(p_master, p_task);
    }
    else if (-1 == index)
    {
        /* time is in expired list, and is removed */
    }
    else
    {
        iv_timer_unregister(p_timer);
        ctc_task_list_delete_node(&p_master->timer, p_task);
        p_task->type = TASK_UNUSED;
        ctc_task_add_unuse(p_master, p_task);
    }
}

void
ctc_task_delete_read(ctc_task_t *p_task)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    struct iv_fd *p_fd = NULL;

    if (TASK_UNUSED == p_task->type)
    {
        return;
    }
    
    p_fd = (struct iv_fd*)p_task->p_iv;
    iv_fd_unregister(p_fd);
    ctc_task_list_delete_node(&p_master->read, p_task);
    p_task->type = TASK_UNUSED;
    ctc_task_add_unuse(p_master, p_task);
}

void
ctc_task_delete_write(ctc_task_t *p_task)
{
    ctc_task_delete_read(p_task);
}

void
ctc_task_delete_event(ctc_task_t *p_task)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    struct iv_event *p_iv_event = NULL;

    if (TASK_UNUSED == p_task->type)
    {
        return;
    }
    
    p_iv_event = (struct iv_event*)p_task->p_iv;
    iv_event_unregister(p_iv_event);
    ctc_task_list_delete_node(&p_master->task, p_task);
    p_task->type = TASK_UNUSED;
    ctc_task_add_unuse(p_master, p_task);
}

void
ctc_task_delete_task(ctc_task_t *p_task)
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    struct iv_task *p_iv_task = NULL;
    
    if (TASK_UNUSED == p_task->type)
    {
        return;
    }
    
    p_iv_task = (struct iv_task*)p_task->p_iv;
    iv_task_unregister(p_iv_task);
    ctc_task_list_delete_node(&p_master->task, p_task);
    p_task->type = TASK_UNUSED;
    ctc_task_add_unuse(p_master, p_task);
}

void
ctc_task_main()
{
    iv_register_run_cb(_ctc_task_update_boottime_cb);
    iv_register_before_poll_cb(g_ctc_task_before_poll_cb);
    iv_main();
}

void ctc_task_main_cowork(struct timeval *(*coworkcb)(struct timeval *))
{
    iv_main_cowork(coworkcb);
}


ctc_task_master_t*
ctc_task_init()
{
    iv_init();
    _g_task_master = (ctc_task_master_t*)sal_calloc(sizeof(ctc_task_master_t));
    return _g_task_master;
}

void
ctc_task_deinit()
{
    ctc_task_master_t *p_master = ctc_task_get_master();
    ctc_task_t* p_task = NULL;
    ctc_task_t* p_task2 = NULL;
    
    iv_deinit();
    
    ctc_task_list_for_each_del(p_task, p_task2, &p_master->timer)
    {
        sal_free(p_task);
    }    
    ctc_task_list_for_each_del(p_task, p_task2, &p_master->event)
    {
        sal_free(p_task);
    }    
    ctc_task_list_for_each_del(p_task, p_task2, &p_master->read)
    {
        sal_free(p_task);
    }    
    ctc_task_list_for_each_del(p_task, p_task2, &p_master->write)
    {
        sal_free(p_task);
    }    
    ctc_task_list_for_each_del(p_task, p_task2, &p_master->task)
    {
        sal_free(p_task);
    }
    ctc_task_list_for_each_del(p_task, p_task2, &p_master->unused)
    {
        sal_free(p_task);
    }
    sal_free(p_master);
}

ctc_task_master_t*
ctc_task_get_master()
{
    return _g_task_master;
}

void
ctc_task_dump_list(ctc_task_list_t *p_list)
{
    ctc_task_t* p_task = NULL;
    
    ctc_task_list_for_each(p_task, p_list)
    {
        CTC_TASK_LOG("  %p", p_task);
    }
    CTC_TASK_LOG("\n");
}

void
ctc_task_dump()
{
    ctc_task_master_t* p_master = ctc_task_get_master();

    CTC_TASK_LOG("Total Task :      %d\n", p_master->alloc);
    CTC_TASK_LOG("Timer      :      %d\n", p_master->timer.count);
    ctc_task_dump_list(&p_master->timer);
    CTC_TASK_LOG("FD Read    :      %d\n", p_master->read.count);
    ctc_task_dump_list(&p_master->read);
    CTC_TASK_LOG("FD Write   :      %d\n", p_master->write.count);
    ctc_task_dump_list(&p_master->write);
    CTC_TASK_LOG("Event      :      %d\n", p_master->event.count);
    ctc_task_dump_list(&p_master->event);
    CTC_TASK_LOG("Unused     :      %d\n", p_master->unused.count);
    ctc_task_dump_list(&p_master->unused);
}

struct sal_timeval*
ctc_task_update_boottime(struct sal_timeval *tv)
{
    return _ctclib_time_boottime(tv);
}

struct sal_timeval *
ctc_task_boottime(struct sal_timeval *tv)
{
    tv->tv_sec = _g_ctclib_last_boottime.tv_sec;
    tv->tv_usec = _g_ctclib_last_boottime.tv_usec;
    return tv;
}

sal_time_t
ctc_time_boottime_sec(sal_time_t *tp)
{
    struct sal_timeval tv;

    ctc_task_boottime(&tv);
    if (tp)
    {
        *tp = (sal_time_t)tv.tv_sec;
    }
  
    return (sal_time_t)tv.tv_sec;
}

sal_time_t
ctc_time_boottime_msec(sal_time_t *tp)
{
    struct sal_timeval tv;

    ctc_task_boottime(&tv);
    if (tp)
    {
        *tp =  (sal_time_t)(tv.tv_sec * 1000) + (sal_time_t)(tv.tv_usec / 1000);
    }

    return ((sal_time_t)(tv.tv_sec * 1000) + (sal_time_t)(tv.tv_usec / 1000));
}

int32
ctc_timer_get_remain_sec(const ctc_task_t *p_task)
{
    int32 delta = 0;
    struct iv_timer *p_timer = NULL;

    if (TASK_TIMER != p_task->type)
    {
        SAL_ASSERT(0);
    }

    p_timer = (struct iv_timer*)p_task->p_iv;
    delta = (p_timer->expires.tv_sec - _g_ctclib_last_boottime.tv_sec);

    return delta;
}

int32
ctc_timer_get_remain_msec(const ctc_task_t *p_task)
{
    int32 delta = 0;
    int32 delta_ms_tmp = 0;
    int32 delta_ms = 0;
    
    struct iv_timer *p_timer = NULL;

    if (TASK_TIMER != p_task->type)
    {
        SAL_ASSERT(0);
    }

    p_timer = (struct iv_timer*)p_task->p_iv;

    delta = (p_timer->expires.tv_sec - _g_ctclib_last_boottime.tv_sec);
    delta_ms_tmp = (p_timer->expires.tv_nsec/1000000 - _g_ctclib_last_boottime.tv_usec/1000);
    delta_ms = delta * 1000 + delta_ms_tmp;
    return delta_ms;
}

char*
ctc_time_seconds_to_ymd_hms(uint32 seconds, char* str, uint32 str_len)
{
    uint32 year = 0;
    uint32 month = 0;
    uint32 day = 0;
    uint32 hour = 0;
    uint32 minute = 0;
    uint32 second = 0;
    uint32 day_count = 0;
    uint32 month_count = 0;

    str[0] = '\0';
    
    if (seconds < TIME_SECOND_PER_DAY)
    {
        hour = (seconds / TIME_SECOND_PER_HOUR) % TIME_HOUR_PER_DAY;
        minute = (seconds / TIME_SECOND_PER_MINUTE) % TIME_MINUTE_PER_HOUR;
        second = seconds % TIME_SECOND_PER_MINUTE;
        sal_snprintf(str, str_len, "%02u:%02u:%02u", hour, minute, second);
    }
    else if (seconds < TIME_SECOND_PER_DAY*TIME_DAY_PER_MONTH)
    {
        day_count = seconds / TIME_SECOND_PER_DAY;
        day  = day_count % TIME_DAY_PER_MONTH;
        hour = (seconds / TIME_SECOND_PER_HOUR) % TIME_HOUR_PER_DAY;
        minute = (seconds / TIME_SECOND_PER_MINUTE) % TIME_MINUTE_PER_HOUR;
        sal_snprintf(str, str_len, "%ud%uh%um", day, hour, minute);
    }
    else if (seconds < TIME_SECOND_PER_DAY*TIME_DAY_PER_YEAR)
    {
        day_count = seconds / TIME_SECOND_PER_DAY;
        month_count = (day_count / TIME_DAY_PER_MONTH);
        month  = month_count % TIME_MONTH_PER_YEAR;
        day  = day_count % TIME_DAY_PER_MONTH;
        hour = (seconds / TIME_SECOND_PER_HOUR) % TIME_HOUR_PER_DAY;
        sal_snprintf(str, str_len, "%um%ud%uh", month, day, hour);
    }
    else
    {
        day_count = seconds / TIME_SECOND_PER_DAY;
        year = day_count / TIME_DAY_PER_YEAR;

        day_count = day_count % TIME_DAY_PER_YEAR;
        month_count = (day_count / TIME_DAY_PER_MONTH);
        month  = month_count % TIME_MONTH_PER_YEAR;
        day  = day_count % TIME_DAY_PER_MONTH;
        sal_snprintf(str, str_len, "%uy%um%ud", year, month, day);
    }

    return str;
}

char*
ctc_task_uptime_to_str(sal_time_t time, char *str, uint32 str_len)
{
    sal_time_t now;
    sal_time_t delta;
    int32 hours;
    int32 minutes;
    int32 seconds;

    str[0] = 0;

    /* If uptime is ZERO, just return blank field */
    if (time == 0)
    {
        sal_snprintf(str, str_len, "%s", "N/A");
        return str;
    }

    /* Get current time */
    now = ctc_time_boottime_sec(NULL);
  
    delta = now - time;

    minutes = delta / 60;
    seconds = delta % 60;
    hours = minutes /60;
    minutes = minutes % 60;

    /* Making formatted timer strings */
    if (delta < ONE_DAY_SECOND)
    {
        sal_snprintf(str, str_len, "%02d:%02d:%02d", hours, minutes, seconds);
    }

    return str;
}

char*
ctc_task_time_to_str(sal_time_t time, char *str, uint32 str_len)
{
    int32 hours;
    int32 minutes;
    int32 seconds;

    str[0] = 0;

    /* If uptime is ZERO, just return blank field */
    if (time == 0)
    {
        sal_snprintf(str, str_len, "%s", "N/A");
        return str;
    }

    minutes = time / 60;
    seconds = time % 60;
    hours = minutes /60;
    minutes = minutes % 60;

    /* Making formatted timer strings */
    if (time < ONE_DAY_SECOND)
    {
        sal_snprintf(str, str_len, "%02d:%02d:%02d", hours, minutes, seconds);
    }

    return str;
}

char*
ctc_task_val2str(char *str, uint32 str_len, const ctc_task_t *p_task)
{
    struct iv_timer *p_timer = NULL;
    int32 delta = 0;

    if (NULL == p_task)
    {
        sal_strncpy(str, "null", str_len);
        return str;
    }
    
    str[0] = '\0';
    switch (p_task->type)
    {
    case TASK_FD_READ:
        sal_strncpy(str, "read", str_len);
        break;
    case TASK_TIMER:
        p_timer = (struct iv_timer*)p_task->p_iv;
        delta = (p_timer->expires.tv_sec - _g_ctclib_last_boottime.tv_sec) * 1000000 
            + ((uint32)p_timer->expires.tv_nsec/1000 - _g_ctclib_last_boottime.tv_usec);
        sal_snprintf(str, str_len, "timer#%d.%06d", 
            delta/1000000, delta%100000);
        break;
    case TASK_EVENT:
        sal_strncpy(str, "event", str_len);
        break;
    case TASK_TASK:
        sal_strncpy(str, "task", str_len);
        break;
    default:
        sal_strncpy(str, "unused", str_len);
    }

    return str;
}

int32
ctc_task_str2val(char *str, ctc_task_t *p_task)
{
    return 0;
}


void
ctc_task_register_before_poll_cb(CTC_TASK_BF_POLL_CB cb)
{
    g_ctc_task_before_poll_cb = cb;
}

