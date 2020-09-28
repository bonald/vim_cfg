
#ifndef __CTC_TTP_DRV_IPUC_H__
#define __CTC_TTP_DRV_IPUC_H__

int32
ctc_ttp_drv_ipuc_add_entry(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_ipuc_remove_entry(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_ipuc_add_default_entry(uint16 vrf_id);

int32
ctc_ttp_drv_ipuc_del_default_entry(uint16 vrf_id);

#endif

