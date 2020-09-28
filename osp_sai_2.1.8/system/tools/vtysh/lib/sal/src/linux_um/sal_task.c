#include "sal.h"
#include <limits.h>
#include <pthread.h>

/*add by jcao for loongson cpu*/
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

struct sal_task
{
    pthread_t pth;
    void (*start)(void *);
    void *arg;
};

#define SAL_SECOND_USEC                   (1000000)


static void *_sal_start_thread(void *arg)
{
    sal_task_t *task = (sal_task_t *)arg;
    (*task->start)(task->arg);

    return 0;
}

sal_err_t sal_task_create(sal_task_t **ptask,
                          const char *name,
                          size_t stack_size,
                          void (*start)(void *),
                          void *arg)
{
    sal_task_t *task;
    pthread_attr_t attr;
    int ret;

    SAL_MALLOC(task, sal_task_t *, sizeof(sal_task_t));
    if (!task)
        return ENOMEM;
    task->start = start;
    task->arg = arg;

    if ((stack_size != 0) && (stack_size < PTHREAD_STACK_MIN))
        stack_size = PTHREAD_STACK_MIN;

    pthread_attr_init(&attr);
    if (stack_size)
        pthread_attr_setstacksize(&attr, stack_size);

    ret = pthread_create(&task->pth, &attr, _sal_start_thread, task);
    if (ret)
    {
        SAL_FREE(task);

        return ret;
    }

    *ptask = task;
    return 0;
}

void sal_task_destroy(sal_task_t *task)
{
    pthread_join(task->pth, NULL);
    SAL_FREE(task);
}

void sal_task_exit()
{
    pthread_exit(0);
}

void sal_task_sleep(uint32_t msec)
{
    struct timespec ts;
    int err;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    while (1) {
        err = nanosleep(&ts, &ts);
        if (err == 0)
            break;

        if (errno != EINTR)
            break;
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


void sal_task_yield()
{
    sched_yield();
}

void sal_gettimeofday(struct timeval *tv)
{
    gettimeofday(tv, NULL);
}

void sal_getuptime(struct timespec *ts)
{
    int fd;
    unsigned long sec, njiffy;
    char buf[64];

    fd = open("/proc/uptime",O_RDONLY);
    if (fd == -1)
    {
        perror("failed to open /proc/uptime:");
        return ;
    }
    memset(buf,0,64);
    read(fd,buf,64);
    sscanf(buf,"%lu.%lu", &sec, &njiffy);
    ts->tv_sec = sec;
    ts->tv_nsec = njiffy*1000000000/100;
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

