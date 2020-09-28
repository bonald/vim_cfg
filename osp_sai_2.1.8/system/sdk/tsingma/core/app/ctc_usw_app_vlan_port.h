/**
 @file ctc_app_usw_vlan_port.h

 @date 2017-12-21

 @version v5.0

 The file defines vlan port api
*/
#ifndef _CTC_APP_USW_VLAN_PORT_H_
#define _CTC_APP_USW_VLAN_PORT_H_
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_mix.h"
#include "ctc_error.h"

#include "ctc_api.h"
#include "ctcs_api.h"


/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define CTC_APP_VLAN_PORT_MCAST_TUNNEL_ID 4094
#define CTC_APP_VLAN_PORT_BCAST_TUNNEL_ID 4095


/**********************************************************************************
                      Define API function interfaces
 ***********************************************************************************/
extern int32
ctc_usw_app_vlan_port_set_global_cfg(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg);
extern int32
ctc_usw_app_vlan_port_get_global_cfg(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg);
extern int32
ctc_usw_app_vlan_port_get_by_logic_port(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t *p_vlan_port);
extern int32
ctc_usw_app_vlan_port_create_uni(uint8 lchip, ctc_app_uni_t *p_uni);
extern int32
ctc_usw_app_vlan_port_destory_uni(uint8 lchip, ctc_app_uni_t *p_uni);
extern int32
ctc_usw_app_vlan_port_get_uni(uint8 lchip, ctc_app_uni_t *p_uni);
extern int32
ctc_usw_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);
extern int32
ctc_usw_app_vlan_port_destory_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);
extern int32
ctc_usw_app_vlan_port_get_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);
extern int32
ctc_usw_app_vlan_port_update_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);
extern int32
ctc_usw_app_vlan_port_create(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);
extern int32
ctc_usw_app_vlan_port_destory(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);
extern int32
ctc_usw_app_vlan_port_get(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);
extern int32
ctc_usw_app_vlan_port_update(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);
extern int32
ctc_usw_app_vlan_port_create_nni(uint8 lchip, ctc_app_nni_t *p_nni);
extern int32
ctc_usw_app_vlan_port_destory_nni(uint8 lchip, ctc_app_nni_t *p_nni);
extern int32
ctc_usw_app_vlan_port_get_nni(uint8 lchip, ctc_app_nni_t *p_nni);
extern int32
ctc_usw_app_vlan_port_init(uint8 lchip, void *p_param);
extern int32
ctc_usw_app_vlan_port_get_fid_mapping_info(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);
extern int32
ctc_usw_app_vlan_port_alloc_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);
extern int32
ctc_usw_app_vlan_port_free_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);
extern int32
ctc_usw_app_vlan_port_update_nni(uint8 lchip, ctc_app_nni_t* p_nni);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_APP_USW_VLAN_PORT_H_*/

