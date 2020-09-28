
#ifndef __CTC_TTP_DRV_STATS_H__
#define __CTC_TTP_DRV_STATS_H__

int32
ctc_ttp_drv_stats_create(hsrv_openflow_ttp_stats_type_t stats_type, uint32 *stats_id);

int32
ctc_ttp_drv_stats_destory(uint32 stats_id);

int32
ctc_ttp_drv_stats_clear(uint32 stats_id);

int32
ctc_ttp_drv_stats_get(hsrv_openflow_ttp_stats_t *stats_info, uint32 stats_id);

#endif

