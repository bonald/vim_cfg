#include "sal.h"
#include "sal_task.h"
#include "sal_mutex.h"
#include "sal_event.h"
#include "sal_mem.h"

#ifdef _SAL_LINUX_UM

#else

enum timer_state
{
    TIMER_RUNNING,
    TIMER_STOPPED
};
typedef enum timer_state timer_state_t;

struct sal_timer
{
    timer_state_t state;
    struct timeval end;
    int32 timeout;
    void (* timer_handler)(void*);
    void* user_param;
    sal_timer_t* next;
    sal_timer_t* prev;
};

#ifndef _SAL_VXWORKS
static sal_mutex_t* timer_mutex;
static sal_event_t* timer_event;
static sal_timer_t* head;
static sal_task_t* timer_task;
static bool is_exit;
static bool is_inited;
#endif

int32
ctc_sal_timer_compare(struct timeval timer1, struct timeval timer2)
{
#ifndef _SAL_VXWORKS
    int32 ret;
    ret = (int32)timer1.tv_sec - (int32)timer2.tv_sec;
    if (ret != 0)
    {
        return ret;
    }
    else
    {
        ret = (int32)timer1.tv_usec - (int32)timer2.tv_usec;
        return ret;
    }

#endif
    return 0;
}

void
ctc_sal_timer_check(void* param)
{
#ifndef _SAL_VXWORKS
    struct timeval now;
    sal_systime_t ts;
    int32 timeout;
    int32 ret;
    uint32 sec, usec;
    uint32 exp_sec, exp_usec;
    sal_timer_t* temp_timer;
    sal_timer_t* timer;

    while (1)
    {
        if (is_exit == TRUE)
        {
            sal_task_exit(timer_task);
        }

        sal_mutex_lock(timer_mutex);
        if (head != NULL)
        {
            ctc_sal_gettime(&ts);
            now.tv_sec = ts.tv_sec;
            now.tv_usec = ts.tv_usec;
            ret = ctc_sal_timer_compare(now, head->end);

            if (ret >= 0)
            {
                head->timer_handler(head->user_param);
                sec = head->timeout / 1000;
                usec = (head->timeout % 1000) * 1000;
                exp_sec = head->end.tv_sec + sec;
                exp_usec = head->end.tv_usec + usec;

                if (exp_usec >= 1000000)
                {
                    exp_usec -= 1000000;
                    exp_sec++;
                }

                head->end.tv_sec = exp_sec;
                head->end.tv_usec = exp_usec;
                if (head->next != NULL
                    && ctc_sal_timer_compare(head->end, head->next->end) >= 0)
                {
                    timer = head;
                    head = head->next;
                    head->prev = NULL;
                    temp_timer = head;
                    timer->prev = NULL;
                    timer->next = NULL;

                    while (1)
                    {
                        ret = ctc_sal_timer_compare(timer->end, temp_timer->end);
                        if (ret < 0)
                        {
                            temp_timer->prev->next = timer;
                            timer->prev = temp_timer->prev;
                            temp_timer->prev = timer;
                            timer->next = temp_timer;
                            break;
                        }
                        else if (temp_timer->next != NULL)
                        {
                            temp_timer = temp_timer->next;
                        }
                        else
                        {
                            temp_timer->next = timer;
                            timer->prev = temp_timer;
                            break;
                        }
                    }
                }

                ctc_sal_mutex_unlock(timer_mutex);

            }
            else
            {
                if (head->end.tv_sec != now.tv_sec)
                {
                    if (head->end.tv_usec >= now.tv_sec)
                    {
                        timeout = (head->end.tv_sec - now.tv_sec) * 1000 +
                            (head->end.tv_usec - now.tv_usec) / 1000;
                    }
                    else
                    {
                        timeout = (head->end.tv_sec - now.tv_sec - 1) * 1000 + 1000 -
                            (now.tv_usec - head->end.tv_usec) / 1000;
                    }
                }
                else
                {
                    timeout = (head->end.tv_usec - now.tv_usec) / 1000;
                }

                ctc_sal_mutex_unlock(timer_mutex);
                ctc_sal_event_wait(timer_event, timeout);
            }
        }
        else
        {
            ctc_sal_mutex_unlock(timer_mutex);
            ctc_sal_event_wait(timer_event, -1);
        }
    }

#endif
}

/**
 * Initialize timer manager
 */
sal_err_t
ctc_sal_timer_init()
{
#ifndef _SAL_VXWORKS
    if (!is_inited)
    {
        ctc_sal_mutex_create(&timer_mutex);
        ctc_sal_event_create(&timer_event, TRUE);
        is_exit = FALSE;
        ctc_sal_task_create(&timer_task, "ctcTimer", 0, 0, ctc_sal_timer_check, NULL);
        is_inited = TRUE;
        SAL_LOG_INFO("time manager is opened sucessfully \n");
    }
    else
    {
        SAL_LOG_INFO("Notice!!!time manager has been opened\n");
    }

    return 0;
#endif
    return 0;
}

/**
 * Deinitialize timer manager
 */
void
ctc_sal_timer_fini()
{
#ifndef _SAL_VXWORKS
    if (is_inited)
    {
        if (head == NULL)
        {
            is_exit = TRUE;
            ctc_sal_event_set(timer_event);
            ctc_sal_task_destroy(timer_task);
            ctc_sal_mutex_destroy(timer_mutex);
            ctc_sal_event_destroy(timer_event);
            is_inited = FALSE;
            SAL_LOG_INFO("time manager is closed sucessfully");
        }
        else
        {
            SAL_LOG_INFO("Notice!!!Please close all timers before closing timer manager");
        }
    }
    else
    {
        SAL_LOG_INFO("Notice!!!no manager is opened");
    }

#endif
}

/**
 * Create a timer
 *
 * @param ptimer
 * @param func
 * @param arg
 *
 * @return
 */
sal_err_t
ctc_sal_timer_create(sal_timer_t** ptimer,
                 void (* func)(void*),
                 void* arg)
{
#ifndef _SAL_VXWORKS
    sal_timer_t* temp_timer;
    temp_timer = (sal_timer_t*)mem_malloc(MEM_SAL_MODULE, sizeof(sal_timer_t));
    if (!temp_timer)
    {
        return ENOMEM;
    }

    temp_timer->state = TIMER_STOPPED;
    temp_timer->timer_handler = func;
    temp_timer->user_param = arg;
    temp_timer->next = NULL;
    temp_timer->prev = NULL;
    *ptimer = temp_timer;
#endif
    return 0;
}

/**
 * Destroy timer
 *
 * @param timer
 */
void
ctc_sal_timer_destroy(sal_timer_t* timer)
{
#ifndef _SAL_VXWORKS
    if (timer->state == TIMER_RUNNING)
    {
        ctc_sal_timer_stop(timer);
    }

    mem_free(timer);
#endif
}

/**
 * Start timer
 *
 * @param timer
 * @param timeout
 *
 * @return
 */
sal_err_t
ctc_sal_timer_start(sal_timer_t* timer, uint32_t timeout)
{
#ifndef _SAL_VXWORKS
    struct timeval now;
    sal_systime_t ts;
    int32 ret;
    uint32 sec, usec;
    uint32 exp_sec, exp_usec;
    sal_timer_t* temp_timer;
    if (timer != NULL)
    {
        if (timer->state != TIMER_RUNNING)
        {
            ctc_sal_gettime(&ts);
            now.tv_sec = ts.tv_sec;
            now.tv_usec = ts.tv_usec;
            sec = timeout / 1000;
            usec = (timeout % 1000) * 1000;
            exp_sec = now.tv_sec + sec;
            exp_usec = now.tv_usec + usec;
            if (exp_usec >= 1000000)
            {
                exp_usec -= 1000000;
                exp_sec++;
            }

            timer->end.tv_sec = exp_sec;
            timer->end.tv_usec = exp_usec;
            timer->timeout = timeout;
            ctc_sal_mutex_lock(timer_mutex);
            if (head == NULL)
            {
                head = timer;
                ctc_sal_event_set(timer_event);
            }
            else
            {
                temp_timer = head;

                while (1)
                {
                    ret = ctc_sal_timer_compare(timer->end, temp_timer->end);
                    if (ret < 0)
                    {
                        if (temp_timer->prev == NULL)
                        {
                            timer->next = temp_timer;
                            temp_timer->prev = timer;
                            head = timer;
                            ctc_sal_event_set(timer_event);
                            break;
                        }
                        else
                        {
                            temp_timer->prev->next = timer;
                            timer->prev = temp_timer->prev;
                            temp_timer->prev = timer;
                            timer->next = temp_timer;
                            break;
                        }
                    }
                    else if (temp_timer->next != NULL)
                    {
                        temp_timer = temp_timer->next;
                    }
                    else
                    {
                        temp_timer->next = timer;
                        timer->prev = temp_timer;
                        break;
                    }
                }
            }

            timer->state = TIMER_RUNNING;
            ctc_sal_mutex_unlock(timer_mutex);
        }
    }
    else
    {
        SAL_LOG_INFO("invalid timer pointer");
    }

#endif
    return 0;
}

/**
 * Stop timer
 *
 * @param timer
 *
 * @return
 */
sal_err_t
ctc_sal_timer_stop(sal_timer_t* timer)
{
#ifndef _SAL_VXWORKS
    sal_timer_t* temp_timer;
    if (timer != NULL)
    {
        if (timer->state == TIMER_RUNNING)
        {
            ctc_sal_mutex_lock(timer_mutex);
            temp_timer = head;

            while (1)
            {
                if (temp_timer == timer)
                {
                    if (temp_timer->prev == NULL)
                    {
                        if (temp_timer->next == NULL)
                        {
                            head = NULL;
                            break;
                        }
                        else
                        {
                            head = temp_timer->next;
                            head->prev = NULL;
                            break;
                        }
                    }
                    else if (temp_timer->next == NULL)
                    {
                        temp_timer->prev->next = NULL;
                        break;
                    }
                    else
                    {
                        temp_timer->next->prev = temp_timer->prev;
                        temp_timer->prev->next = temp_timer->next;
                        break;
                    }
                }
                else
                {
                    temp_timer = temp_timer->next;
                }
            }

            ctc_sal_mutex_unlock(timer_mutex);
            timer->next = NULL;
            timer->prev = NULL;
            timer->state = TIMER_STOPPED;
        }
    }
    else
    {
        SAL_LOG_INFO("invalid timer pointer");
    }

#endif
    return 0;
}

sal_time_t ctc_sal_time(sal_time_t *tv)
{
    struct timeval ltv;

    sal_memset(&ltv,0,sizeof(struct timeval));

    if(!tv)
    {
        return ltv.tv_sec;
    }

    do_gettimeofday(&ltv);

    *tv = ltv.tv_sec;

    return *tv;
}

char* ctc_sal_ctime(const sal_time_t *clock)
{
    static char time_buf[64] = "";
    struct rtc_time tm;

    sal_memset(time_buf,0,sizeof(time_buf));
    rtc_time_to_tm(*clock, &tm);

    sal_snprintf(time_buf,
                 sizeof(time_buf),
                 "%4d-%02d-%02d %02d:%02d:%02d\n",
                 1900 + tm.tm_year,
                 tm.tm_mon,
                 tm.tm_mday,
                 tm.tm_hour,
                 tm.tm_min,
                 tm.tm_sec);

    return time_buf;
}

#endif

