/* Added by weizj for inband */
#ifndef __FEI_OPENFLOW_INBAND__H__
#define __FEI_OPENFLOW_INBAND__H__


/**
 * @brief Adapter layer inband update stag
 */
int32
fei_inband_update_stag(glb_openflow_inband_t* p_config);

/**
 * @brief Adapter layer inband create vif
 */
int32
fei_inband_create_vif(uint32 ifindex, char* name, uint16 gport);

/**
 * @brief Adapter layer inband destroy vif
 */
int32
fei_inband_destroy_vif(uint32 ifindex, char* name);

#endif
