
#ifndef __CTC_TTP_DRV_POLICER_H__
#define __CTC_TTP_DRV_POLICER_H__

void
ctc_ttp_drv_policer_id_alloc(uint32 *policer_id);

void
ctc_ttp_drv_policer_id_free(uint32 policer_id);

int32
ctc_ttp_drv_policer_set(glb_openflow_meter_t  *p_config, uint32 policer_id);

int32
ctc_ttp_drv_policer_remove(uint32 policer_id);

int32
ctc_ttp_drv_policer_get_stats(uint32 policer_id, hsrv_openflow_ttp_meter_stats_t *p_meter_stats);

int32
ctc_ttp_drv_policer_clear_stats(uint32 policer_id);

#endif

