#ifndef __HSRV_OPENFLOW_METER_H__
#define __HSRV_OPENFLOW_METER_H__

struct hsrv_openflow_meter_info_s
{
    uint32  meter_id;
    uint32  policer_id;
    bool    is_bound;
};
typedef struct hsrv_openflow_meter_info_s hsrv_openflow_meter_info_t;


#endif /* !__HSRV_OPENFLOW_METER_H__ */
