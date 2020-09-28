
#ifndef __CTC_TTP_DRV_ACL_H__
#define __CTC_TTP_DRV_ACL_H__

#define CTC_ACL_ENTRY_ID_MIN    11
#define CTC_ACL_ENTRY_ID_NUM   4000

void
ctc_ttp_drv_acl_entry_id_alloc(uint32 *entry_id);

void
ctc_ttp_drv_acl_entry_id_free(uint32 entry_id);

int32
ctc_ttp_drv_acl_add_entry(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_acl_remove_entry(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_acl_init();

#endif

