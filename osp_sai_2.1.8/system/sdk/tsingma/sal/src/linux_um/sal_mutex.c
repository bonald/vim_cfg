#include "sal.h"
#include <pthread.h>

struct sal_mutex
{
    pthread_mutex_t lock;
};

struct sal_sem
{
    sem_t sem;
};

sal_err_t
ctc_sal_mutex_create(sal_mutex_t** pmutex)
{
    sal_mutex_t* mutex;

    SAL_MALLOC(mutex, sal_mutex_t*, sizeof(sal_mutex_t));
    if (!mutex)
    {
        return ENOMEM;
    }

    pthread_mutex_init(&mutex->lock, NULL);

    *pmutex = mutex;
    return 0;
}

void
ctc_sal_mutex_destroy(sal_mutex_t* mutex)
{
    pthread_mutex_destroy(&mutex->lock);
    SAL_FREE(mutex);
}

void
ctc_sal_mutex_lock(sal_mutex_t* mutex)
{
    pthread_mutex_lock(&mutex->lock);
}

void
ctc_sal_mutex_unlock(sal_mutex_t* mutex)
{
    pthread_mutex_unlock(&mutex->lock);
}

bool
ctc_sal_mutex_try_lock(sal_mutex_t* mutex)
{
    return pthread_mutex_trylock(&mutex->lock) != EBUSY;
}

sal_err_t
ctc_sal_sem_create(sal_sem_t** pp_sem, uint32 init_count)
{
    sal_sem_t* p_sem = NULL;

    SAL_MALLOC(p_sem, sal_sem_t*, sizeof(sal_sem_t));
    if (!p_sem)
    {
        return ENOMEM;
    }

    sem_init(&p_sem->sem, 0, init_count);
    *pp_sem = p_sem;

    return 0;
}

sal_err_t
ctc_sal_sem_destroy(sal_sem_t* p_sem)
{
    if (!p_sem)
    {
        return EINVAL;
    }

    sem_destroy(&p_sem->sem);
    SAL_FREE(p_sem);

    return 0;
}

sal_err_t
ctc_sal_sem_take(sal_sem_t* p_sem, int32 msec)
{
    int32 ret = 0;

    if (!p_sem)
    {
        return EINVAL;
    }

    if (SAL_SEM_FOREVER == msec)
    {
        do
        {
            ret = sem_wait(&p_sem->sem);
        }
        while (ret != 0 && EINTR == errno);
    }
    else
    {
        int32 time_wait = 1;

        for (;;)
        {
            if (sem_trywait(&p_sem->sem) == 0)
            {
                ret = 0;
                break;
            }

            if (errno != EAGAIN && errno != EINTR)
            {
                ret = errno;
                break;
            }

            if (time_wait > msec)
            {
                time_wait = msec;
            }

            ctc_sal_task_sleep(time_wait);
            msec -= time_wait;

            if (msec == 0)
            {
                ret = ETIMEDOUT;
                break;
            }

            if ((time_wait *= 2) > 100)
            {
                time_wait = 100;
            }
        }
    }

    return ret;
}

sal_err_t
ctc_sal_sem_give(sal_sem_t* p_sem)
{
    if (!p_sem)
    {
        return EINVAL;
    }

    return sem_post(&p_sem->sem);
}

