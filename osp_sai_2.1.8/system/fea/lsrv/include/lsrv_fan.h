#ifndef __LSRV_FAN_H__
#define __LSRV_FAN_H__



void
lsrv_fan_event_cb(lai_fan_event_t  event,lai_fan_info_t* p_fan);

int32
lsrv_fan_routine_handle();
#endif /* !__LSRV_FAN_H__ */


