#include "sal.h"
#include <pthread.h>

typedef struct vx_mutex_s
{
    int8 mutex_opaque_type;
}* vx_mutex_t;

struct sal_mutex
{
    vx_mutex_t sal_vx_mutex;
};

struct sal_sem
{
    SEM_ID sem;
};

sal_err_t
ctc_sal_mutex_create(sal_mutex_t** pmutex)
{
    SEM_ID sem;
    sal_mutex_t* mutex;

    mutex = (sal_mutex_t*)mem_malloc(MEM_SAL_MODULE, sizeof(sal_mutex_t));
    if (!mutex)
    {
        return -1;
    }

    sem = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
    mutex->sal_vx_mutex = (vx_mutex_t)sem;
    *pmutex = mutex;

    return 0;
}

void
ctc_sal_mutex_destroy(sal_mutex_t* mutex)
{
    SEM_ID sem = (SEM_ID)(mutex->sal_vx_mutex);

    assert(sem);

    semDelete(sem);
    mem_free(mutex);
}

void
ctc_sal_mutex_lock(sal_mutex_t* mutex)
{
    SEM_ID sem = (SEM_ID)(mutex->sal_vx_mutex);

    assert(sem);

    semTake(sem, WAIT_FOREVER);
}

void
ctc_sal_mutex_unlock(sal_mutex_t* mutex)
{
    SEM_ID sem = (SEM_ID)(mutex->sal_vx_mutex);

    semGive(sem);
}

sal_err_t
ctc_sal_sem_create(sal_sem_t** pp_sem, uint32 init_count)
{
    sal_sem_t* p_sem = NULL;

    p_sem = (sal_sem_t*)mem_malloc(MEM_SAL_MODULE, sizeof(sal_sem_t));
    if (!p_sem)
    {
        return ENOMEM;
    }

    p_sem->sem = semCCreate(SEM_Q_FIFO, init_count);
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

    semDelete(p_sem->sem);
    mem_free(p_sem);

    return 0;
}

extern int sysClkRateGet(void);

int
_ctc_sal_usec_to_ticks(uint32 usec)
{
    int32 divisor = 0;

    divisor = 1000000 / sysClkRateGet();
    return (usec + divisor - 1) / divisor;
}

sal_err_t
ctc_sal_sem_take(sal_sem_t* p_sem, int32 msec)
{
    int32 ticks = 0;

    if (!p_sem)
    {
        return EINVAL;
    }

    if (SAL_SEM_FOREVER == msec)
    {
        ticks = WAIT_FOREVER;
    }
    else
    {
        ticks = _ctc_sal_usec_to_ticks(msec * 1000);
    }

    return semTake(p_sem->sem, ticks);
}

sal_err_t
ctc_sal_sem_give(sal_sem_t* p_sem)
{
    if (!p_sem)
    {
        return EINVAL;
    }

    return semGive(p_sem->sem);
}

