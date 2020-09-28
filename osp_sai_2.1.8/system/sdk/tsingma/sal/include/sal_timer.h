#ifndef __SAL_TIMER_H__
#define __SAL_TIMER_H__

/**
 * @file sal_timer.h
 */

#ifdef _SAL_LINUX_KM
#   include <linux/time.h>
#   include <linux/rtc.h>
#endif

/**
 * @defgroup timer Timers
 * @{
 */

/** Timer Object */

#ifdef _SAL_LINUX_KM
    typedef struct sal_timer    sal_timer_t;
 /*    typedef struct time_t       sal_time_t;*/
#else
    typedef uint32 sal_timer_t;
#endif

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Initialize timer manager
 */
sal_err_t ctc_sal_timer_init(void);

/**
 * Deinitialize timer manager
 */
void ctc_sal_timer_fini(void);

/**
 * Create a timer
 *
 * @param ptimer
 * @param func
 * @param arg
 *
 * @return
 */
sal_err_t ctc_sal_timer_create(sal_timer_t** ptimer,
                           void (* func)(void*),
                           void* arg);

/**
 * Destroy timer
 *
 * @param timer
 */
void ctc_sal_timer_destroy(sal_timer_t* timer);

/**
 * Start timer
 *
 * @param timer
 * @param timeout
 *
 * @return
 */
sal_err_t ctc_sal_timer_start(sal_timer_t* timer, uint32 timeout);

/**
 * Stop timer
 *
 * @param timer
 *
 * @return
 */
sal_err_t ctc_sal_timer_stop(sal_timer_t* timer);

#if defined(_SAL_LINUX_KM)
sal_time_t ctc_sal_time(sal_time_t*);
char* ctc_sal_ctime(const sal_time_t *clock);
#endif

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup timer */

#endif /* !__SAL_TIMER_H__ */

