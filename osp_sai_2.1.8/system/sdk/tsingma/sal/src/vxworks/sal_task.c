#include "sal.h"
#include <limits.h>

extern int sysClkRateGet (void);

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

    CTC_SAL_DEBUG_OUT("%-5s %-10s %-10s %-13s %-16s\n", "Index", "Tid", "Priority", "Function", "Name");
    CTC_SAL_DEBUG_OUT("-------------------------------------------------------------------\n");
    for (loop = 0; loop < g_ctc_sal_task_count; loop++)
    {
        p_task = g_ctc_sal_task_array[loop];
        if (p_task)
        {
            CTC_SAL_DEBUG_OUT("%-5d %-10d %-10d %-13p %-16s\n", loop, p_task->tid, p_task->prio, p_task->start, p_task->name);
        }
    }
    CTC_SAL_DEBUG_OUT("-------------------------------------------------------------------\n");

    return;
}

sal_err_t
ctc_sal_task_create(sal_task_t** ptask,
                char* name,
                size_t stack_size,
                int prio,
                void (* start)(void*),
                void* arg)
{
    int rv;
    sal_task_t* task;
    sigset_t new_mask, orig_mask;

    task = (sal_task_t*)mem_malloc(MEM_SAL_MODULE, sizeof(sal_task_t));
    if (!task)
    {
        return ENOMEM;
    }

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGINT);
    sigprocmask(SIG_BLOCK, &new_mask, &orig_mask);
    rv = taskSpawn(name, prio, (VX_UNBREAKABLE | VX_FP_TASK), stack_size, (FUNCPTR)start,
                   PTR_TO_INT(arg), 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sigprocmask(SIG_SETMASK, &orig_mask, NULL);

    if (rv == ERROR)
    {
        return -1;
    }
    else
    {
        task->pthread = (vx_task_t)INT_TO_PTR(rv);
        sal_memcpy(task->name, name, 64);
        task->start = start;
        task->prio = prio;
        task->tid = rv;
        *ptask = task;

        _sal_task_add_to_db(task);

        return 0;
    }
}

void
ctc_sal_task_destroy(sal_task_t* task)
{
    taskDelete(PTR_TO_INT(task->pthread));
    _sal_task_del_from_db(task);
    mem_free(task);
}

void
ctc_sal_task_exit(int rc)
{
    exit(rc);
}

void
ctc_sal_task_sleep(uint32_t msec)
{
    int divisor = 0;
    int tick = 0;

    divisor = 1000 / sysClkRateGet();

    tick = (msec + divisor - 1) / divisor;

    taskDelay(tick);
}

void
ctc_sal_task_yield()
{
#if 0
    sched_yield();
#endif
}

void
ctc_sal_gettime(sal_systime_t* tv)
{
    struct timespec temp_tv;

    sal_memset(&temp_tv, 0, sizeof(temp_tv));

    clock_gettime(CLOCK_REALTIME, &temp_tv);

    tv->tv_sec = temp_tv.tv_sec;
    tv->tv_usec = temp_tv.tv_nsec / 1000;
}

void
ctc_sal_getuptime(struct timespec* ts)
{
#if 0
    zhuj_debug
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
#endif
}

void ctc_sal_udelay(uint32 usec)
{
    int divisor = 0;
    int tick = 0;

    divisor = 1000000 / sysClkRateGet();

    tick = (usec + divisor - 1) / divisor;

    taskDelay(tick);
}

/**
 * Set thread cpu mask & priority. tid 0 means current task.
 *
 */
sal_err_t
ctc_sal_task_set_affinity_prio(pid_t tid, uint64 cpu_mask, int32 priority)
{
    return 0;
}

/**
 * Get thread cpu mask & priority. tid 0 means current task.
 *
 */
sal_err_t
ctc_sal_task_get_affinity_prio(pid_t tid, uint64* p_cpu_mask, int32* p_priority)
{
    return 0;
}


sal_err_t
ctc_sal_task_set_priority(int32 priority)
{
    return 0;
}

sal_err_t
ctc_sal_task_get_priority(int32* p_priority)
{
    return 0;
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
    return 0;
}
