#ifndef _CTC_DKIT_GOLDENGATE_MONITOR_H_
#define _CTC_DKIT_GOLDENGATE_MONITOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"



extern int32
ctc_usw_dkit_monitor_show_queue_id(void* p_para);

extern int32
ctc_usw_dkit_monitor_show_queue_depth(void* p_para);

extern int32
ctc_usw_dkit_monitor_show_sensor_result(void* p_para);

extern int32
ctc_usw_dkit_monitor_sensor_register(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif


