#ifndef __SAL_EVENT_H__
#define __SAL_EVENT_H__

/**
 * @file sal_event.h
 */

/**
 * @defgroup event Events
 * @{
 */

/**
 * The sal_event_t object is used to signal between threads that a particular
 * event has occurred
 */
typedef struct sal_event sal_event_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Create a new event object
 *
 * @param[out] pevt
 * @param[in]  auto_reset
 *
 * @return
 */
sal_err_t sal_event_create(sal_event_t **pevt, bool auto_reset);

/**
 * Destroy the event object
 *
 * @param[in] event
 */
void sal_event_destroy(sal_event_t *event);

/**
 * Set the given event to the signaled state
 *
 * @param[in] event
 */
void sal_event_set(sal_event_t *event);

/**
 * Set the given event to the non-signaled state
 *
 * @param[in] event
 */
void sal_event_reset(sal_event_t *event);

/**
 * The <em>sal_event_wait</em> function returns when the given event is in the
 * signaled state or the <em>timeout</em> interval elapses
 *
 * @param[in] event
 * @param[in] timeout
 *
 * @retval TRUE  The event is in the signaled state
 * @retval FALSE The wait is timed out
 */
bool sal_event_wait(sal_event_t *event, int timeout);

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup event */

#endif /* !__SAL_EVENT_H__ */

