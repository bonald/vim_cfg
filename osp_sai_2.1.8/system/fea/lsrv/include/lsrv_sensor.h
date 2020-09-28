#ifndef __LSRV_SENSOR_H__
#define __LSRV_SENSOR_H__

int32
lsrv_sensor_routine_handle();


void
lsrv_sensor_event_cb(lai_thermal_event_t  event,lai_thermal_info_t* p_sensor);
#endif /* !__LSRV_SENSOR_H__ */





