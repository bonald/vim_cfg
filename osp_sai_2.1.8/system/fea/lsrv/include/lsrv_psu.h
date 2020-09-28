#ifndef __LSRV_PSU_H__
#define __LSRV_PSU_H__

int32
lsrv_psu_routine_handle();

void
lsrv_psu_event_cb(lai_psu_event_t  event,lai_psu_info_t* p_psu);


#endif /* !__LSRV_PSU_H__ */




