/* Added by weizj for tunnel */
#ifndef __HAGT_OPENFLOW_TUNNEL_H__
#define __HAGT_OPENFLOW_TUNNEL_H__


#define HAGT_OPENFLOW_TUNNEL_INVALID_FID          0
#define HAGT_OPENFLOW_TUNNEL_INVALID_VNI          0


/**
 * @brief Hagt layer openflow tunnel msg callback
 */
int32
hagt_openflow_tunnel_register_callback(void);

/**
 * @brief Hagt layer openflow tunnel set fid
 */
int32
hagt_openflow_tunnel_set_fid(uint32 vni, uint16 fid);

#endif