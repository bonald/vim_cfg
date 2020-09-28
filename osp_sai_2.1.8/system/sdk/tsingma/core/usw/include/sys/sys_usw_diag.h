
#ifndef _SYS_USW_DIAG_H
#define _SYS_USW_DIAG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_diag.h"
#include "sys_usw_common.h"


#define SYS_DIAG_DROP_REASON_BASE_IPE 0
#define SYS_DIAG_DROP_REASON_BASE_EPE 64

#define SYS_DIAG_DROP_COUNT_SYSTEM 0xFFFF

extern int32
sys_usw_diag_trigger_pkt_trace(uint8 lchip, ctc_diag_pkt_trace_t* p_trace);

extern int32
sys_usw_diag_get_pkt_trace(uint8 lchip, ctc_diag_pkt_trace_result_t* p_rslt);

extern int32
sys_usw_diag_get_drop_info(uint8 lchip, ctc_diag_drop_t* p_drop);

extern int32
sys_usw_diag_init(uint8 lchip, void* init_cfg);

extern int32
sys_usw_diag_deinit(uint8 lchip);

extern int32
sys_usw_diag_set_property(uint8 lchip, ctc_diag_property_t prop, void* p_value);

extern int32
sys_usw_diag_get_property(uint8 lchip, ctc_diag_property_t prop, void* p_value);

#ifdef __cplusplus
}
#endif

#endif  /*_SYS_USW_DIAG_H*/
