
#ifndef __CTC_TTP_DRV_PORT_H__
#define __CTC_TTP_DRV_PORT_H__

int32
ctc_ttp_drv_port_enable(uint32 ifindex);

int32
ctc_ttp_drv_port_disable(uint32 ifindex);

int32
ctc_ttp_drv_port_set_scl_property(uint32 ifindex, uint32 is_tunnel);

int32
ctc_ttp_drv_port_reset_scl_property(uint32 ifindex, uint32 is_tunnel);

#endif

