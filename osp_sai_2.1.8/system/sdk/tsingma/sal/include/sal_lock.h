#ifndef __SAL_LOCK_H__
#define __SAL_LOCK_H__

/**
 * @file sal_lock.h
 */

/**
 * @defgroup lock LOCKS
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif


/** SpinLock Object */
typedef struct sal_spinlock sal_spinlock_t;


/**
 * Create a new spinlock
 *
 * @param[out] pspinlock
 *
 * @return
 */
sal_err_t ctc_sal_spinlock_create(sal_spinlock_t** pspinlock);

/**
 * Destroy the spinlock
 *
 * @param[in] spinlock
 */
void ctc_sal_spinlock_destroy(sal_spinlock_t* spinlock);

/**
 * Lock the spinlock
 *
 * @param[in] spinlock
 */
void ctc_sal_spinlock_lock(sal_spinlock_t* spinlock);

/**
 * Unlock the spinlock
 *
 * @param[in] spinlock
 */
void ctc_sal_spinlock_unlock(sal_spinlock_t* spinlock);

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup lock */

#endif /* !__SAL_LOCK_H__ */

