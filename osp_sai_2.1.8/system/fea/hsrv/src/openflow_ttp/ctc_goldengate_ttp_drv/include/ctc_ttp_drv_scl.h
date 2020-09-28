
#ifndef __CTC_TTP_DRV_SCL_H__
#define __CTC_TTP_DRV_SCL_H__

#define CTC_SCL_ENTRY_ID_MIN    20
#define CTC_SCL_ENTRY_ID_NUM   200

int32
ctc_ttp_drv_scl_entry_id_alloc(uint32 *entry_id);

int32
ctc_ttp_drv_scl_entry_id_free(uint32 entry_id);

int32
ctc_ttp_drv_scl_vlan_mapping_add(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_scl_terminal_mac_flow_add(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_scl_entry_remove(hsrv_openflow_ttp_flow_t *p_flow); 

int32
ctc_ttp_drv_scl_init();

#endif

