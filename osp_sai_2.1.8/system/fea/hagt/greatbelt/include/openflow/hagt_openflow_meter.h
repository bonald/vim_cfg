/* Added by weizj for meter */
#ifndef __HAGT_OPENFLOW_METER_H__
#define __HAGT_OPENFLOW_METER_H__


/**
 * @brief Hagt layer openflow meter get stats
 */
int32
hagt_openflow_meter_get_stats(uint32 meter_id, glb_qos_policer_stats_t* p_meter_stats);

/**
 * @brief Hagt layer openflow meter clear stats
 */
int32
hagt_openflow_meter_clear_stats(uint32 meter_id);

/**
 * @brief Hagt layer openflow meter msg callback
 */
int32
hagt_openflow_meter_register_callback(void);

/**
 * @brief Hagt layer openflow meter init
 */
void
hagt_openflow_meter_init(ctc_qos_global_cfg_t* p_qos_cfg);

#endif
