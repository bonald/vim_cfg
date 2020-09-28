#ifndef __SAL_INTR_H__
#define __SAL_INTR_H__

/**
 * @file sal_intr.h
 */

/**
 * @defgroup intr Interrupt Locking
 * @{
 */

/**
 * Interrupt flags
 */
typedef unsigned long sal_iflags_t;

/**
 * Lock hardware interrupts
 *
 * @return Saved interrupt flags
 */
static inline sal_iflags_t sal_hwi_lock();

/**
 * Unlock hardware interrupts
 *
 * @param iflags Saved interrupt flags returned by sal_hwi_lock()
 */
static inline void sal_hwi_unlock(sal_iflags_t iflags);

/**
 * Lock software interrupts
 */
static inline void sal_swi_lock();

/**
 * Unlock software interrupts
 */
static inline void sal_swi_unlock();

#ifdef _SAL_LINUX_KM
static inline sal_iflags_t sal_hwi_lock()
{
    unsigned long iflags;

    local_irq_save(iflags);

    return iflags;
}

static inline void sal_hwi_unlock(sal_iflags_t iflags)
{
    local_irq_restore(iflags);
}

static inline void sal_swi_lock()
{
    local_bh_disable();
}

static inline void sal_swi_unlock()
{
    local_bh_enable();
}
#else /* _SAL_LINUX_UM */
extern sal_mutex_t* p_sal_async_queue_mutex;
static inline sal_iflags_t sal_hwi_lock()
{
    sal_mutex_lock(p_sal_async_queue_mutex);
    return 0;
}

static inline void sal_hwi_unlock(sal_iflags_t iflags)
{
    sal_mutex_unlock(p_sal_async_queue_mutex);
}

static inline void sal_swi_lock()
{
}

static inline void sal_swi_unlock()
{
}
#endif

/**@}*/ /* End of @defgroup intr */

#endif /* !__SAL_INTR_H__ */

