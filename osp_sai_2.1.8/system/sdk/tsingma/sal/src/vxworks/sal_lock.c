#include "sal.h"
#include <sysLib.h>
#include <intLib.h>

#if VX_VERSION >= 66
#include <spinLockLib.h>
#endif

struct sal_spinlock
{
#if VX_VERSION >=66
    spinlock_t lock;
#else
    int il;
#endif
};

sal_err_t
ctc_sal_spinlock_create(sal_spinlock_t** pspinlock)
{
    sal_spinlock_t* spinlock = NULL;

    spinlock = (sal_spinlock_t*)mem_malloc(MEM_SAL_MODULE, sizeof(sal_spinlock_t));
    if (!spinlock)
    {
        return -1;
    }

#if VX_VERSION >= 66
        spinLockIsrInit(&spinlock->spinlock, 0);
#else
        spinlock->il = 0;
#endif
    *pspinlock = spinlock;
    return 0;
}

void
ctc_sal_spinlock_destroy(sal_spinlock_t* spinlock)
{
    mem_free(spinlock);
}

void
ctc_sal_spinlock_lock(sal_spinlock_t* spinlock)
{
#if VX_VERSION >= 66
    spinLockIsrTake(&spinlock->lock);
#else
    spinlock->il = intLock();
#endif
}

void
ctc_sal_spinlock_unlock(sal_spinlock_t* spinlock)
{
#if VX_VERSION >= 66
    spinLockIsrGive(&spinlock->lock);
#else
    intUnlock(spinlock->il);
#endif;
}

