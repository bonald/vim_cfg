/**
 *  Copyright (C) 2011, 2012, 2013 CentecNetworks, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __SAL_MUTEX_H__
#define __SAL_MUTEX_H__

/**
 * @file sal_mutex.h
 */

/**
 * @defgroup mutex Mutexes
 * @{
 */

/** Mutex Object */
typedef struct sal_mutex sal_mutex_t;

typedef struct sal_cond sal_cond_t;

#define SAL_SEM_FOREVER     (-1)
/** Sem Object */
typedef struct sal_sem sal_sem_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create a new recursive mutex
 *
 * @param[out] pmutex
 *
 * @return
 */
sal_err_t sal_mutex_recursive_create(sal_mutex_t **pmutex);

/**
 * Create a new mutex
 *
 * @param[out] pmutex
 *
 * @return
 */
sal_err_t sal_mutex_create(sal_mutex_t **pmutex);

/**
 * Create a new condition
 *
 * @param[out] pcond
 *
 * @return
 */
sal_err_t sal_cond_create(sal_cond_t** pcond);

/**
 * Destroy the mutex
 *
 * @param[in] mutex
 */
void sal_mutex_destroy(sal_mutex_t *mutex);

/**
 * Lock the mutex
 *
 * @param[in] mutex
 */
void sal_mutex_lock(sal_mutex_t *mutex);

/**
 * Unlock the mutex
 *
 * @param[in] mutex
 */
void sal_mutex_unlock(sal_mutex_t *mutex);

/**
 * Try to lock the mutex
 *
 * @param[in] mutex
 *
 * @return
 */
bool sal_mutex_try_lock(sal_mutex_t *mutex);

/**
 * Signal a condition
 */
void sal_task_cond_signal(sal_cond_t* p_cond);

/**
 * Wait on a condition
 */
void sal_task_cond_wait(sal_mutex_t* p_mutex, sal_cond_t* p_cond);

/**
 * Create a new sem
 *
 * @param[out] pp_sem
 *
 * @param[in] init count
 *
 * @return
 */
sal_err_t sal_sem_create(sal_sem_t **pp_sem, uint32 init_count);

/**
 * Destroy the sem
 *
 * @param[in] p_sem
 *
 * @return
 */
sal_err_t sal_sem_destroy(sal_sem_t *p_sem);

/**
 * Task the sem
 *
 * @param[in] p_sem
 *
 * @param[in] usec
 *
 * @return
 */
sal_err_t sal_sem_take(sal_sem_t *p_sem, int32 msec);

/**
 * Give the sem
 *
 * @param[in] p_sem
 *
 * @return
 */
sal_err_t sal_sem_give(sal_sem_t *p_sem);
#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup mutex */

#endif /* !__SAL_MUTEX_H__ */
