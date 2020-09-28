#ifndef __LSRV_POE_H__
#define __LSRV_POE_H__

int32
lsrv_poe_routine_handle();

void
lsrv_poe_event_cb(lai_poe_event_t event, lai_poe_info_t* p_poe);


#endif /* !__LSRV_POE_H__ */




