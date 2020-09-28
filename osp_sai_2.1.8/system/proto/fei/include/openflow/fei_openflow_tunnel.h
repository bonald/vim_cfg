/* Added by weizj for tunnel */
#ifndef __FEI_OPENFLOW_TUNNEL__H__
#define __FEI_OPENFLOW_TUNNEL__H__


/**
 * @brief Fei layer openflow tunnel add port
 */
int32
fei_openflow_tunnel_add_port(glb_openflow_tunnel_info_t* p_data);

/**
 * @brief Fei layer openflow tunnel delete port
 */
int32
fei_openflow_tunnel_del_port(uint32 ifindex, glb_openflow_tunnel_del_type_t type);

/**
 * @brief Fei layer openflow tunnel update decap mode
 */
int32
fei_openflow_tunnel_update_decap_mode(uint8 decap_mode);

/**
 * @brief Fei layer openflow tunnel update vtep ip
 */
int32
fei_openflow_tunnel_update_vtep_ip(uint32 vtep_ip);

#endif
