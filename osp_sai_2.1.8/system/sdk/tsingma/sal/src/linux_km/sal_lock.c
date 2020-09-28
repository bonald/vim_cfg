#include "sal.h"

struct sal_spinlock
{
    spinlock_t lock;
    unsigned long flags;
};

sal_err_t
ctc_sal_spinlock_create(sal_spinlock_t** pspinlock)
{
    sal_spinlock_t* spinlock = NULL;

    SAL_MALLOC(spinlock, sal_spinlock_t, sizeof(sal_spinlock_t));

    if (!spinlock) {
        return ENOMEM;
    }
    spin_lock_init(&spinlock->lock);
    *pspinlock = spinlock;
    return 0;
}

void
ctc_sal_spinlock_destroy(sal_spinlock_t* spinlock)
{
    SAL_FREE(spinlock);
}

void
ctc_sal_spinlock_lock(sal_spinlock_t* spinlock)
{
    spin_lock_irqsave(&spinlock->lock, spinlock->flags);
}

void
ctc_sal_spinlock_unlock(sal_spinlock_t* spinlock)
{
    spin_unlock_irqrestore(&spinlock->lock, spinlock->flags);
}

