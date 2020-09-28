

/**
 @file ctc_app_vlan_port.h

 @date 2016-04-13

 @version v5.0

 The file defines vlan port api
*/
#ifndef _CTC_APP_VLAN_PORT_H_
#define _CTC_APP_VLAN_PORT_H_
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



/**
 @brief Set vlan port by logic port

 @param[in] lchip    local chip id

 @param[out] p_glb_cfg    point to ctc_app_global_cfg_t

 @remark  Set global ctl

 @return CTC_E_XXX

*/

int32
ctc_gbx_app_vlan_port_set_global_cfg(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg);


/**
 @brief Get vlan port by logic port

 @param[in] lchip    local chip id

 @param[out] p_glb_cfg    point to ctc_app_global_cfg_t

 @remark  Set global ctl

 @return CTC_E_XXX

*/

int32
ctc_gbx_app_vlan_port_get_global_cfg(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg);

/**
 @brief Get vlan port by logic port

 @param[in] lchip    local chip id

 @param[in] logic_port    logic port

 @param[out] p_vlan_port    point to p_vlan_port

 @remark  Get vlan port by logic port

 @return CTC_E_XXX

*/

int32
ctc_gbx_app_vlan_port_get_by_logic_port(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t *p_vlan_port);


/**
 @brief Create uni port

 @param[in] lchip    local chip id

 @param[in/out] p_uni    point to ctc_app_nni_t

 @remark  Create uni downlin port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_create_uni(uint8 lchip, ctc_app_uni_t *p_uni);

/**
 @brief Destroy uni port

 @param[in] lchip    local chip id

 @param[in] p_uni    point to ctc_app_nni_t

 @remark  Destroy uni downlin port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_destory_uni(uint8 lchip, ctc_app_uni_t *p_uni);



/**
 @brief Get uni port

 @param[in] lchip    local chip id

 @param[in/out] p_uni    point to ctc_app_nni_t

 @remark  Get uni downlin port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_get_uni(uint8 lchip, ctc_app_uni_t *p_uni);




/**
 @brief Create gem port which mapping tunnel value to logic port and gem_gport

 @param[in] lchip    local chip id

 @param[in/out] p_gem_port  point to ctc_app_gem_port_t

 @remark  Mapping tunnel value to logic port and gem_gport

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);

/**
 @brief Destroy gem port which cancel mapping tunnel value to logic port and gem_gport

 @param[in] lchip    local chip id

 @param[in] p_gem_port  point to ctc_app_gem_port_t

 @remark  Cancel mapping tunnel value to logic port and gem_gport

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_destory_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);



/**
 @brief Get gem port which cancel mapping tunnel value to logic port and gem_gport

 @param[in] lchip    local chip id

 @param[in/out] p_gem_port  point to ctc_app_gem_port_t

 @remark  Get mapping tunnel value to logic port and gem_gport

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_get_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);



/**
 @brief Update em port property

 @param[in] lchip    local chip id

 @param[in] p_gem_port  point to ctc_app_gem_port_t

 @remark  Update gem port property

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_update_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port);


/**
 @brief Create vlan port which mapping vlan and service

 @param[in] lchip    local chip id

 @param[in/out] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Mapping vlan and service

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_create(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);

/**
 @brief Destroy vlan port which cancel mapping vlan and service

 @param[in] lchip    local chip id

 @param[in] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Cancel mapping vlan and service

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_destory(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);


/**
 @brief Get vlan port which cancel mapping vlan and service

 @param[in] lchip    local chip id

 @param[in/out] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Get mapping vlan and service

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_get(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);


/**
 @brief Update vlan port which cancel mapping vlan and service

 @param[in] lchip    local chip id

 @param[in/out] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Update mapping vlan and service

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_update(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port);


/**
 @brief Create nni uplink port

 @param[in] lchip    local chip id

 @param[in/out] p_nni    point to ctc_app_nni_t

 @remark  Create nni uplink port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_create_nni(uint8 lchip, ctc_app_nni_t *p_nni);

/**
 @brief Destroy nni uplink port

 @param[in] lchip    local chip id

 @param[in] p_nni    point to ctc_app_nni_t

 @remark  Destroy nni uplnk port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_destory_nni(uint8 lchip, ctc_app_nni_t *p_nni);


/**
 @brief Get nni uplink port

 @param[in] lchip    local chip id

 @param[in/out] p_nni    point to ctc_app_nni_t

 @remark  Get nni uplnk port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_get_nni(uint8 lchip, ctc_app_nni_t *p_nni);




/**
 @brief vlan port ini

 @param[in] lchip    local chip id

 @param[in] p_param    void

 @remark  init vlan port

 @return CTC_E_XXX

*/
int32
ctc_gbx_app_vlan_port_init(uint8 lchip, void *p_param);


int32
ctc_gbx_app_vlan_port_get_fid_mapping_info(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);

int32
ctc_gbx_app_vlan_port_alloc_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);

int32
ctc_gbx_app_vlan_port_free_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);

int32
ctc_gbx_app_vlan_port_update_nni(uint8 lchip, ctc_app_nni_t* p_nni);

#ifdef __cplusplus
}
#endif

#endif  /* _CTC_APP_VLAN_PORT_H_*/



