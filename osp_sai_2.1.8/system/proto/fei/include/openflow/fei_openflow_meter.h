/* Added by weizj for meter */
#ifndef __FEI_OPENFLOW_METER_H__
#define __FEI_OPENFLOW_METER_H__


/**
 * @brief Fei layer add meter entry
 */
int32
fei_meter_add_meter_entry(glb_openflow_meter_t* p_meter_info);

/**
 * @brief Fei layer update meter entry
 */
int32
fei_meter_update_meter_entry(glb_openflow_meter_t* p_meter_info, uint8 old_stats_enable);

/**
 * @brief Fei layer delete meter entry
 */
int32
fei_meter_delete_meter_entry(glb_openflow_meter_t* p_meter_info);

/**
 * @brief Fei layer update meter ipg status
 */
int32
fei_meter_update_meter_ipg_status(glb_openflow_meter_ipg_t* p_status);

/**
 * @brief Fei layer clear all stats
 */
int32
fei_meter_clear_all_stats(uint32 meter_id);

#endif /* !__FEI_OPENFLOW_METER_H__ */
