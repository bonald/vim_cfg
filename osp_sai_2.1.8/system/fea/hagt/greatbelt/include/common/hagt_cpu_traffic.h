#ifndef __HAGT_CPU_TRAFFIC_H__
#define __HAGT_CPU_TRAFFIC_H__
#include "glb_cpu_traffic_define.h"

extern int32
hagt_pdu_debug_dump(void);

extern int32
hagt_cpu_traffic_map_tocpu_reason(glb_packet_tocpu_reason_t hsrv_reason, 
                                  ctc_packet_tocpu_reason_t* p_ctc_reason, 
                                  uint8* p_sub_idx);
extern int32 
hagt_cpu_traffic_init(hagt_master_t* p_hagt_master);

#endif
