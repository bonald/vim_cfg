#include "sal.h"
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define SAL_TO_POSIX_NICE_PRIO(p) ((((p) * 39) / 255) - 20)

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

void ctc_sal_udelay(uint32 usec)
{
    udelay(usec);
}

STATIC int
kthread_start(void* arg)
{
    sal_task_t* task = (sal_task_t*)arg;

    /*daemonize(task->name);*/
    complete(&task->started);
    schedule();

 /*    current->security = task;*/
    (*task->start)(task->arg);
 /*    current->security = NULL;*/

    complete(&task->done);

    ctc_sal_task_exit(task);
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

    SAL_MALLOC(task, sal_task_t*, sizeof(sal_task_t));
    if (!task)
    {
        return ENOMEM;
    }

    sal_memcpy(task->name, name, 64);
    task->start = start;
    task->arg = arg;
    init_completion(&task->started);
    init_completion(&task->done);

    task->ktask = kthread_create(kthread_start, task, name);
    if (IS_ERR(task->ktask))
    {
        SAL_FREE(task);
        return -1;
    }
    wake_up_process(task->ktask);
    wait_for_completion(&task->started);
    task->prio = prio;
    set_user_nice(task->ktask, SAL_TO_POSIX_NICE_PRIO(prio));
    *ptask = task;

    _sal_task_add_to_db(task);

    return 0;
}

void
ctc_sal_task_destroy(sal_task_t* task)
{
    wait_for_completion(&task->done);
    _sal_task_del_from_db(task);
    SAL_FREE(task);
}

void
ctc_sal_task_exit(sal_task_t *task)
{
 /*    sal_task_t* task = (sal_task_t*)current->security;*/

 /*    current->security = NULL;*/
    complete_and_exit(&task->done, 0);
}

void
ctc_sal_task_sleep(uint32_t msec)
{
    __set_current_state(TASK_UNINTERRUPTIBLE);
    schedule_timeout(msecs_to_jiffies(msec));
}

void
ctc_sal_task_yield()
{
    yield();
}

void ctc_sal_delay(uint32 sec)
{
    ctc_sal_task_sleep(sec * 1000);
}

void
ctc_sal_gettime(sal_systime_t* tv)
{
    struct timeval temp_tv;

    sal_memset(&temp_tv, 0, sizeof(temp_tv));

    do_gettimeofday(&temp_tv);

    tv->tv_sec = temp_tv.tv_sec;
    tv->tv_usec = temp_tv.tv_usec;
}

void
ctc_sal_getuptime(struct timespec* ts)
{

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

