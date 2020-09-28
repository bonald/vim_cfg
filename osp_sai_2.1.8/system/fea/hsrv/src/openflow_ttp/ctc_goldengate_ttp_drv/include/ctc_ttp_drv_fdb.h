
#ifndef __CTC_TTP_DRV_FDB_H__
#define __CTC_TTP_DRV_FDB_H__

int32
ctc_ttp_drv_fdb_add_entry(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_fdb_remove_entry(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_fdb_add_default_entry(uint16 fid);

int32
ctc_ttp_drv_fdb_del_default_entry(uint16 fid);

#endif

