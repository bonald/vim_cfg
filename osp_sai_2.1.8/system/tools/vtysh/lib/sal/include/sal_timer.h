#ifndef __SAL_TIMER_H__
#define __SAL_TIMER_H__

/**
 * @file sal_timer.h
 */

/**
 * @defgroup timer Timers
 * @{
 */

/** Timer Object */
typedef struct sal_timer sal_timer_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialize timer manager
 */
sal_err_t sal_timer_init();

/**
 * Deinitialize timer manager
 */
void sal_timer_fini();

/**
 * Create a timer
 *
 * @param ptimer
 * @param func
 * @param arg
 *
 * @return
 */
sal_err_t sal_timer_create(sal_timer_t **ptimer,
                           void (*func)(void *),
                           void *arg);

/**
 * Destroy timer
 * 
 * @param timer
 */
void sal_timer_destroy(sal_timer_t *timer);

/**
 * Start timer
 *
 * @param timer
 * @param timeout
 *
 * @return
 */
sal_err_t sal_timer_start(sal_timer_t *timer, uint32_t timeout);

/**
 * Stop timer
 *
 * @param timer
 *
 * @return
 */
sal_err_t sal_timer_stop(sal_timer_t *timer);

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup timer */

#endif /* !__SAL_TIMER_H__ */

