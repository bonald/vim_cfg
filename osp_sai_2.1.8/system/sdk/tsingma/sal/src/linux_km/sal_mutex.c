#include "sal.h"
#include <linux/jiffies.h>

struct sal_mutex
{
    struct semaphore sem;
};

struct sal_sem
{
    struct semaphore sem;
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

    sema_init(&mutex->sem, 1);

    *pmutex = mutex;
    return 0;
}

void
ctc_sal_mutex_destroy(sal_mutex_t* mutex)
{
    SAL_FREE(mutex);
}

void
ctc_sal_mutex_lock(sal_mutex_t* mutex)
{
    int ret = 0;

    ret = down_interruptible(&mutex->sem);

    return;
}

void
ctc_sal_mutex_unlock(sal_mutex_t* mutex)
{
    up(&mutex->sem);
}

bool
ctc_sal_mutex_try_lock(sal_mutex_t* mutex)
{
    return down_trylock(&mutex->sem) == 0;
}

sal_err_t
ctc_sal_sem_create(sal_sem_t** pp_sem, uint32 init_count)
{
    sal_sem_t* sem;

    SAL_MALLOC(sem, sal_sem_t*, sizeof(sal_sem_t));
    if (!sem)
    {
        return ENOMEM;
    }

    sema_init(&sem->sem, 0);

    *pp_sem = sem;
    return 0;
}

sal_err_t
ctc_sal_sem_destroy(sal_sem_t* p_sem)
{
    SAL_FREE(p_sem);
    return 0;
}

sal_err_t
ctc_sal_sem_take(sal_sem_t* p_sem, int32 msec)
{
    int ret;

    if (!p_sem)
    {
        CTC_SAL_DEBUG_OUT("Sem is NULL!!!!\n");
        return EINVAL;
    }

    if (msec == SAL_SEM_FOREVER) {
        ret = down_interruptible(&p_sem->sem);
	} else {
		long jiffies = msecs_to_jiffies(msec);

        ret = down_timeout(&p_sem->sem, jiffies);
    }

    return ret;
}

sal_err_t
ctc_sal_sem_give(sal_sem_t* p_sem)
{
    up(&p_sem->sem);
    return 0;
}

