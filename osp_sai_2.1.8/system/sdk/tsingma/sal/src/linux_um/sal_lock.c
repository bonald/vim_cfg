#if defined(LINUX) || defined(linux) || defined(__linux__)
#define _XOPEN_SOURCE 600
#endif
#include "sal.h"
#include <pthread.h>

struct sal_spinlock
{
    pthread_spinlock_t lock;
};

sal_err_t
ctc_sal_spinlock_create(sal_spinlock_t** pspinlock)
{
    sal_spinlock_t* spinlock = NULL;

    SAL_MALLOC(spinlock, sal_spinlock_t, sizeof(sal_spinlock_t));

    if (!spinlock) {
        return ENOMEM;
    }
    pthread_spin_init(&spinlock->lock, PTHREAD_PROCESS_SHARED);
    *pspinlock = spinlock;
    return 0;
}

void
ctc_sal_spinlock_destroy(sal_spinlock_t* spinlock)
{
    pthread_spin_destroy(&spinlock->lock);
    SAL_FREE(spinlock);
}

void
ctc_sal_spinlock_lock(sal_spinlock_t* spinlock)
{
    pthread_spin_lock(&spinlock->lock);
}

void
ctc_sal_spinlock_unlock(sal_spinlock_t* spinlock)
{
    pthread_spin_unlock(&spinlock->lock);
}

