/**
 *  Copyright (C) 2011, 2012, 2013 CentecNetworks, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "sal.h"
#include <limits.h>
#include <pthread.h>
#include <sys/syscall.h>

/*add for loongson cpu*/
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

#define SAL_TASK_MAX_NUM    64
static int32 g_sal_task_count = 0;
sal_task_t *g_sal_task_array[SAL_TASK_MAX_NUM];

struct sal_task
{
    pthread_t pth;
    char* name;
    void (* start)(void*);
    void* arg;
    int prio;
    int tid;
};

#define SAL_SECOND_USEC                   (1000000)

pid_t
sal_gettid()
{
    return syscall(SYS_gettid);
}

static char*
sal_prio_str(int32 priority, char *buf, int32 len)
{
    int32 nice_prio = 0;

    sal_memset(buf, 0, sizeof(len));
    if (0 == priority)
    {
        priority = SAL_TASK_PRIO_DEF;
    }
    
    if (priority >= SAL_TASK_PRIO_RT_HIGH && priority <= SAL_TASK_PRIO_RT_LOW)
    {
        sal_snprintf(buf, len, "%-3d (RT %d)", priority, priority);
    }
    else
    {
        nice_prio = priority - SAL_TASK_PRIO_NICE_HIGH - 20;
        sal_snprintf(buf, len, "%-3d (NI %d)", priority, nice_prio);
    }
    
    return buf;
}

void
sal_task_dump(int (*show_func)(void *, const char *fmt, ...), void *para)
{
    char prio_str[20];
    int32 i = 0;
    sal_task_t* p_task = NULL;
    
    show_func(para, "Total Task Count : %d\n", g_sal_task_count);

    if (g_sal_task_count <= 0)
    {
        return;
    }
    
    show_func(para, "%-5s %-11s %-6s %-15s %-10s %-16s\n", 
        "Index", "Pthread_t", "LWP", "Priority", "Function", "Name");
    show_func(para, "-------------------------------------------------------------------\n");
    for (i = 0; i < g_sal_task_count; i++)
    {
        p_task = g_sal_task_array[i];
        if (p_task)
        {
            show_func(para, "%-5d %-11u %-6d %-15s %-10p %-16s\n", 
                i, (uint32)p_task->pth, p_task->tid, 
                sal_prio_str(p_task->prio, prio_str, sizeof(prio_str)),
                p_task->start, p_task->name ? p_task->name : "");
        }
    }

    return;
}

static void*
_start_thread(void* arg)
{
    sal_task_t* task = (sal_task_t*)arg;
    task->tid = sal_gettid();
    if (task->prio)
    {
        sal_task_set_priority(task->prio);
    }
    (*task->start)(task->arg);

    return 0;
}

static int
_sal_task_add_to_array(sal_task_t* p_task)
{
    g_sal_task_array[g_sal_task_count] = p_task;
    g_sal_task_count++;
    return 0;
}

static int
_sal_task_del_from_array(sal_task_t* p_task)
{
    int i = 0;
    int find = FALSE;

    for (i = 0; i < g_sal_task_count; i++)
    {
        if (p_task == g_sal_task_array[i])
        {
            find = TRUE;
        }
        
    }

    if (!find)
    {
        return -1;
    }

    for (; i < g_sal_task_count-1; i++)
    {
        g_sal_task_array[i] = g_sal_task_array[i+1];
    }
    g_sal_task_count--;
    return 0;
}

sal_err_t
sal_task_create(sal_task_t** ptask,
                char* name,
                size_t stack_size,
                int prio,
                void (* start)(void*),
                void* arg)
{
    sal_task_t* task;
    pthread_attr_t attr;
    int ret;

    if (stack_size == 0)
    {
        stack_size = SAL_DEF_TASK_STACK_SIZE;  /*default stack_size 32k*/
    }

    if (g_sal_task_count >= SAL_TASK_MAX_NUM)
    {
        return ENOMEM;
    }

    SAL_MALLOC(task, sal_task_t*, sizeof(sal_task_t));
    if (!task)
    {
        return ENOMEM;
    }

    task->name = sal_strdup(name);
    task->start = start;
    task->arg = arg;
    task->prio = prio;

    if ((stack_size != 0) && (stack_size < PTHREAD_STACK_MIN))
    {
        stack_size = PTHREAD_STACK_MIN;
    }

    pthread_attr_init(&attr);
    if (stack_size)
    {
        pthread_attr_setstacksize(&attr, stack_size);
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&task->pth, &attr, _start_thread, task);
    if (ret)
    {
        SAL_FREE(task);

        return ret;
    }

    *ptask = task;
    _sal_task_add_to_array(task);
    return 0;
}

void
sal_task_destroy(sal_task_t* task)
{
    if (task->name)
    {
        SAL_FREE(task->name);
    }
    pthread_join(task->pth, NULL);
    _sal_task_del_from_array(task);
    SAL_FREE(task);
}

void
sal_task_exit()
{
    pthread_exit(0);
}

void
sal_task_sleep(uint32_t msec)
{
    struct timespec ts;
    int err;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    while (1) 
    {
        err = nanosleep(&ts, &ts);
        if (err == 0)
        {
            break;
        }

        if (errno != EINTR)
        {
            break;
        }
    }
}

void sal_task_adjust_self_priority(int32 policy, int32 prio)
{
    struct sched_param param;

    param.sched_priority = prio;
    pthread_setschedparam(pthread_self(), policy, &param);
}

void sal_task_adjust_priority(sal_task_t *task, int32 policy, int32 prio)
{
    struct sched_param param;

    param.sched_priority = prio;
    pthread_setschedparam(task->pth, policy, &param);
}

void sal_task_get_self_priority(int32 *policy, int32 *prio)
{
    struct sched_param param;

    pthread_getschedparam(pthread_self(), policy, &param);
    *prio = param.sched_priority;
}

void
sal_task_yield()
{
    sched_yield();
}

void sal_gettimeofday(struct timeval *tv)
{
    gettimeofday(tv, NULL);
}
void
sal_gettime(sal_systime_t* tv)
{
    struct timeval temp_tv;

    sal_memset(&temp_tv, 0, sizeof(temp_tv));

    gettimeofday(&temp_tv, NULL);

    tv->tv_sec = temp_tv.tv_sec;
    tv->tv_usec = temp_tv.tv_usec;
}

void
sal_getuptime(struct timespec* ts)
{
    int fd;
    unsigned long sec, njiffy;
    char buf[64];

    fd = open("/proc/uptime", O_RDONLY);
    if (fd == -1)
    {
        perror("failed to open /proc/uptime:");
        return;
    }

    memset(buf, 0, 64);
    read(fd, buf, 64);
    sscanf(buf, "%lu.%lu", &sec, &njiffy);
    ts->tv_sec = sec;
    ts->tv_nsec = njiffy * 1000000000 / 100;
    close(fd);
}

void sal_udelay(uint32 usec)
{
    struct timeval tv;

    tv.tv_sec = (time_t) (usec / SAL_SECOND_USEC);
    tv.tv_usec = (long) (usec % SAL_SECOND_USEC);
    select(0, NULL, NULL, NULL, &tv);
}

void sal_delay(uint32 sec)
{
    sleep(sec);
}

/**
 * Set task's priority 1-99 for realtime SCHED_RR, 100-139 for SCHED_OTHER
 *
 * @param tv
 */
sal_err_t
sal_task_set_priority(int32 priority)
{
    pthread_t pid = 0;
    struct sched_param param;
    int32 rt_priority = 0;
    int32 nice_prio = 0;
    int32 old_policy = 0;
    sal_err_t ret = 0;

    if (priority < SAL_TASK_PRIO_HIGH || priority > SAL_TASK_PRIO_LOW)
    {
        return -1;
    }

    if (priority >= SAL_TASK_PRIO_RT_HIGH && priority <= SAL_TASK_PRIO_RT_LOW)
    {
        rt_priority = (SAL_TASK_PRIO_RT_LOW + 1) - priority;
    }

    pid = pthread_self();
    ret = pthread_getschedparam(pid, &old_policy, &param);
    if (rt_priority)
    {
        param.sched_priority = rt_priority;
        ret = pthread_setschedparam(pid, SCHED_RR, &param);
    }
    else
    {
        if (SCHED_OTHER != old_policy)
        {
            param.sched_priority = 0;
            ret = pthread_setschedparam(pid, SCHED_OTHER, &param);
        }
        nice_prio = priority - SAL_TASK_PRIO_NICE_HIGH - 20;
        nice(nice_prio);
    }

    return ret;
}
