#ifndef __HSRV_OPENFLOW_TTP_METER_H__
#define __HSRV_OPENFLOW_TTP_METER_H__

#define HSRV_OPENFLOW_METER_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_METER, size)
#define HSRV_OPENFLOW_METER_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_METER, ptr);  \
    ptr = NULL; \
}while(0)

struct hsrv_openflow_meter_info_s
{
    uint32  meter_id;
    uint32  policer_id;
    uint32  stats_id;
    uint8   stats_en;
};
typedef struct hsrv_openflow_meter_info_s hsrv_openflow_ttp_meter_info_t;

hsrv_openflow_ttp_meter_info_t*
hsrv_openflow_get_meter_info(uint32 meter_id);

int32
hsrv_openflow_meter_start(void);

#endif
