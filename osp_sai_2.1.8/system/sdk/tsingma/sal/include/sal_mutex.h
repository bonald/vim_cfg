#ifndef __SAL_MUTEX_H__
#define __SAL_MUTEX_H__

/**
 * @file sal_mutex.h
 */

/**
 * @defgroup mutex Mutexes
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif


/** Mutex Object */
typedef struct sal_mutex sal_mutex_t;

#define SAL_SEM_FOREVER     (-1)
/** Sem Object */
typedef struct sal_sem sal_sem_t;

/**
 * Create a new mutex
 *
 * @param[out] pmutex
 *
 * @return
 */
sal_err_t ctc_sal_mutex_create(sal_mutex_t** pmutex);

/**
 * Destroy the mutex
 *
 * @param[in] mutex
 */
void ctc_sal_mutex_destroy(sal_mutex_t* mutex);

/**
 * Lock the mutex
 *
 * @param[in] mutex
 */
void ctc_sal_mutex_lock(sal_mutex_t* mutex);

/**
 * Unlock the mutex
 *
 * @param[in] mutex
 */
void ctc_sal_mutex_unlock(sal_mutex_t* mutex);

/**
 * Try to lock the mutex
 *
 * @param[in] mutex
 *
 * @return
 */
bool ctc_sal_mutex_try_lock(sal_mutex_t* mutex);

/**
 * Create a new sem
 *
 * @param[out] pp_sem
 *
 * @param[in] init count
 *
 * @return
 */
sal_err_t ctc_sal_sem_create(sal_sem_t** pp_sem, uint32 init_count);

/**
 * Destroy the sem
 *
 * @param[in] p_sem
 *
 * @return
 */
sal_err_t ctc_sal_sem_destroy(sal_sem_t* p_sem);

/**
 * Task the sem
 *
 * @param[in] p_sem
 *
 * @param[in] usec
 *
 * @return
 */
sal_err_t ctc_sal_sem_take(sal_sem_t* p_sem, int32 msec);

/**
 * Give the sem
 *
 * @param[in] p_sem
 *
 * @return
 */
sal_err_t ctc_sal_sem_give(sal_sem_t* p_sem);

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup mutex */

#endif /* !__SAL_MUTEX_H__ */

