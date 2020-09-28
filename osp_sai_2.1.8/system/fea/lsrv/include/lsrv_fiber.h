#ifndef __LSRV_FIBER_H__
#define __LSRV_FIBER_H__

int32
lsrv_fiber_routine_handle();

void
lsrv_fiber_event_cb(lai_fiber_event_t  event,lai_fiber_info_t* p_fiber);

#endif /* !__LSRV_FIBER_H__ */


