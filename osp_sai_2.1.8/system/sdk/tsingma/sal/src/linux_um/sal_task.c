#define _GNU_SOURCE

#include <sched.h>
#include "sal.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>

/*add for loongson cpu*/
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

#define SAL_SECOND_USEC                   (1000000)

pid_t
_ctc_sal_gettid()
{
    return syscall(SYS_gettid);
}

void
ctc_sal_task_dump()
{
    int32 loop = 0;
    sal_task_t* p_task = NULL;

    CTC_SAL_DEBUG_OUT("Total Task Count: %d\n", g_ctc_sal_task_count);

    if (g_ctc_sal_task_count <= 0)
    {
        return;
    }

    CTC_SAL_DEBUG_OUT("%-5s %-13s %-10s %-10s %-13s %-16s\n", "Index", "Pthread_t", "Tid", "Priority", "Function", "Name");
    CTC_SAL_DEBUG_OUT("-------------------------------------------------------------------\n");
    for (loop = 0; loop < g_ctc_sal_task_count; loop++)
    {
        p_task = g_ctc_sal_task_array[loop];
        if (p_task)
        {
            CTC_SAL_DEBUG_OUT("%-5d 0x%-11x %-10d %-10d %-13p %-16s\n", loop, (uint32)p_task->pth, p_task->tid,
                p_task->prio, p_task->start, p_task->name);
        }
    }
    CTC_SAL_DEBUG_OUT("-------------------------------------------------------------------\n");

    return;
}

sal_task_t *
_ctc_sal_task_lookup(pid_t tid)
{
    int32 loop = 0;
    sal_task_t* p_task = NULL;

    for (loop = 0; loop < g_ctc_sal_task_count; loop++)
    {
        p_task = g_ctc_sal_task_array[loop];
        if (p_task && p_task->tid == tid)
        {
            return p_task;
        }
    }

    return NULL;
}

STATIC void*
start_thread(void* arg)
{
    sal_task_t* task = (sal_task_t*)arg;
    task->tid = _ctc_sal_gettid();
    if (task->prio)
    {
        ctc_sal_task_set_priority(task->prio);
    }
    (*task->start)(task->arg);
    task->done = 1;

    return 0;
}

sal_err_t
ctc_sal_task_create(sal_task_t** ptask,
                char* name,
                size_t stack_size,
                int prio,
                void (* start)(void*),
                void* arg)
{
    sal_task_t* task;
    pthread_attr_t attr;
    int ret;
    int i = 0;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    cpu_set_t cpuset;

    if (stack_size == 0)
    {
        stack_size = SAL_DEF_TASK_STACK_SIZE;  /*default stack_size 32k*/
    }

    SAL_MALLOC(task, sal_task_t*, sizeof(sal_task_t));
    if (!task)
    {
        return ENOMEM;
    }

    sal_memcpy(task->name, name, 64);
    task->start = start;
    task->arg = arg;
    task->prio = prio;
    task->done = 0;

    if(*ptask)
    {
        task->cpu_mask = (**ptask).cpu_mask;
        task->task_type = (**ptask).task_type;
    }
    else
    {
        task->cpu_mask = 0;
        task->task_type = 0xffff;
    }
    
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

    ret = pthread_create(&task->pth, &attr, start_thread, task);
    if (ret)
    {
        SAL_FREE(task);

        return ret;
    }

    pthread_attr_destroy(&attr);

    /*bind cpu*/
    if(task->cpu_mask)
    {
        CPU_ZERO(&cpuset);
        for (i = 0; i < cpu_num; i++)
        {
            if ((((task->cpu_mask) & (1 << (i))) ? 1 : 0))
            {
                CPU_SET(i, &cpuset);
            }    
        }
        ret = sched_setaffinity(task->pth, sizeof(cpu_set_t), &cpuset);
    }
    
    *ptask = task;
    _sal_task_add_to_db(task);

    return 0;
}

void
ctc_sal_task_destroy(sal_task_t* task)
{
    uint16 count = 0;

    if (task)
    {
        while ((count < 5000) && ( 0 == task->done))
        {
            count++;
            ctc_sal_task_sleep(1);
        }
        _sal_task_del_from_db(task);
        SAL_FREE(task);
    }
}

/**
 * Set thread cpu mask & priority. tid 0 means current task.
 *
 */
sal_err_t
ctc_sal_task_set_affinity_prio(pid_t tid, uint64 cpu_mask, int32 priority)
{
    struct sched_param param;
    int32 rt_priority = 0;
    int32 nice_prio = 0;
    int32 old_policy = 0;
    sal_err_t ret = 0;
    int i = 0;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    cpu_set_t cpuset;
    sal_task_t* db_task = NULL;
    pthread_t thread = 0;

    if (!cpu_mask)
    {
        return -1;
    }

    tid = (tid == _ctc_sal_gettid()) ? 0 : tid;

    if(tid)
    {
        db_task = _ctc_sal_task_lookup(tid);
        if(NULL == db_task)
        {
            return -1;
        }
    }

    thread = (NULL == db_task) ? pthread_self(): db_task->pth;
    
    if(priority > 0)
    {
        if (priority < SAL_TASK_PRIO_HIGH || priority > SAL_TASK_PRIO_LOW)
        {
            return -1;
        }

        if (priority >= SAL_TASK_PRIO_RT_HIGH && priority <= SAL_TASK_PRIO_RT_LOW)
        {
            rt_priority = (SAL_TASK_PRIO_RT_LOW + 1) - priority;
        }

        ret = pthread_getschedparam(thread, &old_policy, &param);
        if (rt_priority)
        {
            param.sched_priority = rt_priority;
            ret = pthread_setschedparam(thread, SCHED_RR, &param);
        }
        else
        {
            if (SCHED_OTHER != old_policy)
            {
                param.sched_priority = 0;
                ret = pthread_setschedparam(thread, SCHED_OTHER, &param);
            }
            nice_prio = priority - SAL_TASK_PRIO_NICE_HIGH - 20;
            nice(nice_prio);
        }
        if (ret)
        {
            return ret;
        }
        else
        {
            if(db_task)
            {
                db_task->prio = priority;
            }
        }
    }

    if(cpu_mask)
    {
        CPU_ZERO(&cpuset);
        for (i = 0; i < cpu_num; i++)
        {
            if (cpu_mask & (1LL << i))
            {
                CPU_SET(i, &cpuset);
            }    
        }
        ret = sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
        if(ret < 0)
        {
            perror("sched_setaffinity");
        }
        else
        {
            if(db_task)
            {
                db_task->cpu_mask = cpu_mask;
            }
        }        
    }
    return ret;
}

/**
 * Get thread cpu mask & priority. tid 0 means current task.
 *
 */
sal_err_t
ctc_sal_task_get_affinity_prio(pid_t tid, uint64* p_cpu_mask, int32* p_priority)
{
    sal_err_t ret = 0;
    int i = 0;
    uint64 cpu_mask = 0;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    cpu_set_t cpuset;
    int32 policy = 0;
    int32 priority = 0;
    sal_task_t* db_task = NULL;
    pthread_t thread = 0;
    
    if(tid)
    {
        db_task = _ctc_sal_task_lookup(tid);
        if(NULL == db_task)
        {
            return -1;
        }
    }
    
    thread = (NULL == db_task) ? pthread_self(): db_task->pth;
    ret = pthread_getschedparam(thread , &policy, (struct sched_param *)&priority);

    if(!ret)
    {
        if(SCHED_RR == policy)
        {
            *p_priority = (SAL_TASK_PRIO_RT_LOW + 1) - priority;
        }
        else
        {
            priority = getpriority(PRIO_PROCESS, _ctc_sal_gettid());
            *p_priority = priority + SAL_TASK_PRIO_NICE_HIGH + 20;           
        }
    }


    CPU_ZERO(&cpuset);
    ret = sched_getaffinity(tid , sizeof(cpu_set_t), &cpuset);
    if (ret < 0)
    {
        perror("sched_setaffinity");
        return ret;
            
    }
    for (i = 0; i < cpu_num; i++)
    {
        if (CPU_ISSET(i, &cpuset))
        {
            cpu_mask |= (uint64)0x01 << i;
        }    
    }
    
    *p_cpu_mask = cpu_mask;

    return ret;
}

void
ctc_sal_task_exit()
{
    pthread_exit(0);
}

void
ctc_sal_task_sleep(uint32_t msec)
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

void
ctc_sal_task_yield()
{
    sched_yield();
}

void
ctc_sal_gettime(sal_systime_t* tv)
{
    struct timeval temp_tv;

    sal_memset(&temp_tv, 0, sizeof(temp_tv));

    gettimeofday(&temp_tv, NULL);

    tv->tv_sec = temp_tv.tv_sec;
    tv->tv_usec = temp_tv.tv_usec;
}

void
ctc_sal_getuptime(struct timespec* ts)
{
    int fd;
    unsigned long sec, njiffy;
    char buf[64];
    int ret = 0;

    fd = open("/proc/uptime", O_RDONLY);
    if (fd == -1)
    {
        perror("failed to open /proc/uptime:");
        return;
    }

    memset(buf, 0, 64);
    ret = read(fd, buf, 64);
    if(!ret)
    {
        close(fd);
        return;
    }
    sscanf(buf, "%lu.%lu", &sec, &njiffy);
    ts->tv_sec = sec;
    ts->tv_nsec = njiffy * 1000000000 / 100;
    close(fd);
}

void ctc_sal_udelay(uint32 usec)
{
    struct timeval tv;

    tv.tv_sec = (time_t) (usec / SAL_SECOND_USEC);
    tv.tv_usec = (long) (usec % SAL_SECOND_USEC);
    select(0, NULL, NULL, NULL, &tv);
}

void ctc_sal_delay(uint32 sec)
{
    sleep(sec);
}


/**
 * Set task's priority 1-99 for realtime SCHED_RR, 100-139 for SCHED_OTHER
 *
 * @param tv
 */
sal_err_t
ctc_sal_task_set_priority(int32 priority)
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

/**
 * Get task's priority 1-99 for realtime SCHED_RR, 100-139 for SCHED_OTHER
 *
 * @param tv
 */
sal_err_t
ctc_sal_task_get_priority(int32* p_priority)
{
    pthread_t pid = 0;
    int32 policy = 0;
    int32 priority = 0;
    sal_err_t ret = 0;

    pid = pthread_self();
    ret = pthread_getschedparam(pid , &policy, (struct sched_param *)&priority);

    if(!ret)
    {
        if(SCHED_RR == policy)
        {
            *p_priority = (SAL_TASK_PRIO_RT_LOW + 1) - priority;
        }
        else
        {
            priority = getpriority(PRIO_PROCESS, _ctc_sal_gettid());
            *p_priority = priority + SAL_TASK_PRIO_NICE_HIGH + 20;           
        }
    }

    return ret;
}

/**
 * Get task  list, 
 * @param count: array buffer length
 * @array: pointer for a array buffer
 * @return: 0 success, -1 array is NULL, when count<task number, buffer is not enought, return value is -(task number).
 */
sal_err_t
ctc_sal_get_task_list(uint8 lchip,  uint8 count, sal_task_t* array)
{
    uint8 i = 0, j = 0;
    int task_chip = 0;
    uint8 name[64] = {0};
    sal_task_t* p_task = NULL;

    if(array == NULL || count < 1)
    {
        return -1;
    }
    
    for (i = 0; i < g_ctc_sal_task_count; i++)
    {
        p_task = g_ctc_sal_task_array[i];
        if (p_task->name[1] == ' ')
        {
            sal_sscanf(p_task->name, "%d %s",&task_chip, name);
        }
        else
        {
            sal_memcpy(name, p_task->name, sizeof(name));
        }
        
        if (task_chip != lchip)
        {
            continue;
        }
        if(j > count)
        {
            j++;
            continue;
        }
        else
        {
            sal_memcpy((array + j), p_task, sizeof(sal_task_t));
            sal_memcpy((array + j)->name, name, sizeof(name));
            j++;
        }
    }
    if ((j+1)>count)
    {
        return -(j+1);
    }

    (array + j)->tid = _ctc_sal_gettid();
    (array + j)->task_type = 0; /*SAL_TASK_TYPE_MAIN */
    sal_strcpy((array + j)->name, "sdk_main_pthread");
    ctc_sal_task_get_affinity_prio(0, &((array + j)->cpu_mask), &((array + j)->prio));
    (array + j)->pth = pthread_self();

    return 0;
}


