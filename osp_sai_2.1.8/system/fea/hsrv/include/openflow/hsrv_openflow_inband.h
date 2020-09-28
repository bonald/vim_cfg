#ifndef __HSRV_OPENFLOW_INBAND_H__
#define __HSRV_OPENFLOW_INBAND_H__


/**
 * @brief Hsrv layer get inband stag
 */
int32
hsrv_openflow_inband_get_stag(uint16 *p_stag);

/**
 * @brief Hsrv layer get inband iloop label
 */
int32
hsrv_openflow_inband_get_iloop_port_label(uint16 ofport, uint32 *p_label);

/**
 * @brief Hsrv layer get inband iloop gport
 */
int32
hsrv_openflow_inband_get_iloop_port_gport(void);

/**
 * @brief Hsrv layer get inband stag
 */
int32
hsrv_openflow_show_inband(FILE *fp);

/**
 * @brief Hsrv layer inband start
 */
int32
hsrv_openflow_inband_start(void);

#endif /* !__HSRV_OPENFLOW_INBAND_H__ */
