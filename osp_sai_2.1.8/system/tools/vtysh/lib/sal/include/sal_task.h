#ifndef __SAL_TASK_H__
#define __SAL_TASK_H__

/**
 * @file sal_task.h
 */

#define SAL_SCHED_RR SCHED_RR
#define SAL_SCHED_FIFO SCHED_FIFO
#define SAL_SCHED_OTHER SCHED_OTHER

/**
 * @defgroup task Tasks
 * @{
 */

/** Task Object */
typedef struct sal_task sal_task_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create a new task
 *
 * @param[out] ptask
 * @param[in]  name
 * @param[in]  stack_size
 * @param[in]  start
 * @param[in]  arg
 *
 * @return
 */
sal_err_t sal_task_create(sal_task_t **ptask,
                          const char *name,
                          size_t stack_size,
                          void (*start)(void *),
                          void *arg);

/**
 * Destroy the task
 *
 * @param[in] task
 */
void sal_task_destroy(sal_task_t *task);

/**
 * Exit the current executing task
 */
void sal_task_exit();

/**
 * Sleep for <em>msec</em> milliseconds
 *
 * @param[in] msec
 */
void sal_task_sleep(uint32_t msec);

/**
 * Yield the processor voluntarily to other tasks
 */
void sal_task_yield();

/**
  * Adjust the task self priority and scheduling policy
  *
  * @param[in] policy - OTHER/RR/FIFO
  * @param[in] prio - 0 ~ 99
  */
void sal_task_adjust_self_priority(int32 policy, int32 prio);

/**
  * Adjust the task priority and scheduling policy
  *
  * @param[in] policy - OTHER/RR/FIFO
  * @param[in] prio - 0 ~ 99
  */
void sal_task_adjust_priority(sal_task_t *task, int32 policy, int32 prio);

/**
  * Get the task self priority and scheduling policy
  *
  * @param[in] pointer to policy
  * @param[in] pointer to prio
  */
void sal_task_get_self_priority(int32 *policy, int32 *prio);

/**
 * Get current time
 *
 * @param tv
 */
void sal_gettimeofday(struct timeval *tv);

/**
 * Get uptime since last startup
 *
 * @param ts
 */
void sal_getuptime(struct timespec *ts);

void sal_udelay(uint32 usec);

void sal_delay(uint32 sec);

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup task */

#endif /* !__SAL_TASK_H__ */

