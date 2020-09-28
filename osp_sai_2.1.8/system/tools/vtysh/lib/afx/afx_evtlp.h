#ifndef __AFX_EVTLP_H__
#define __AFX_EVTLP_H__

/*
 * Event Loop
 */
int afx_event_loop_create();
void afx_event_loop_destroy();
int afx_event_loop_run();
void afx_event_loop_exit();

/*
 * IO Directions
 */
typedef enum
{
    AFX_IO_IN,
    AFX_IO_OUT,
    AFX_IO_ANY
} afx_io_dir_t;

/*
 * Multiplexed IO
 */
typedef struct afx_mio afx_mio_t;
typedef void (*afx_mio_cb_t)(int fd, afx_io_dir_t dir, void *arg);

int
afx_mio_create(afx_mio_t **pp_mio,
               int fd,
               afx_io_dir_t dir,
               afx_mio_cb_t cb,
               void *arg);

void afx_mio_destroy(afx_mio_t *p_mio);

/*
 * Timers
 */
typedef struct afx_timer afx_timer_t;
typedef void (*afx_timer_cb_t)(void *arg);

int
afx_timer_create(afx_timer_t **pp_timer,
                 afx_timer_cb_t cb,
                 void *arg);

int afx_timer_destroy(afx_timer_t *p_timer);
int afx_timer_start(afx_timer_t *p_timer, uint32_t timeout);
int afx_timer_stop(afx_timer_t *p_timer);

#endif /* !__AFX_EVTLP_H__ */
