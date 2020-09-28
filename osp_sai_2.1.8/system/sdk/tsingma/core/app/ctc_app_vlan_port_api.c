/**
 @file ctc_app_vlan_port_api.c

 @date 2017-07-12

 @version v2.0


*/
/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_app.h"
#include "ctc_app_index.h"
#include "ctc_api.h"
#include "ctc_app_vlan_port_api.h"
#include "dal.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
ctc_app_vlan_port_api_t *ctc_app_vlan_port_api = NULL;
#if defined(GREATBELT) || defined(DUET2)
extern ctc_app_vlan_port_api_t ctc_gb_dt2_app_vlan_port_api;
#endif
#if defined(TSINGMA)
extern ctc_app_vlan_port_api_t ctc_usw_app_vlan_port_api;
#endif

/****************************************************************************
 *
* Function
*
*****************************************************************************/
int32 ctc_app_vlan_port_install_api(uint8 lchip)
{
    uint32 dev_id = 0;
    dal_get_chip_dev_id(lchip, &dev_id);
    switch (dev_id)
    {
#if defined(GREATBELT) || defined(DUET2)
        case DAL_GREATBELT_DEVICE_ID:
        case DAL_DUET2_DEVICE_ID:
            ctc_app_vlan_port_api = &ctc_gb_dt2_app_vlan_port_api;
            break;
#endif
#if defined(TSINGMA)
        case DAL_TSINGMA_DEVICE_ID:
            ctc_app_vlan_port_api = &ctc_usw_app_vlan_port_api;
            break;
#endif
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }
    return CTC_E_NONE;
}
int32 ctc_app_vlan_port_uninstall_api(uint8 lchip)
{
    ctc_app_vlan_port_api = NULL;
    return CTC_E_NONE;
}

int32 ctc_app_vlan_port_init(uint8 lchip, void* p_param)
{
    ctc_app_vlan_port_install_api(lchip);
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_init, lchip, p_param);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_init);

int32 ctc_app_vlan_port_get_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get_nni, lchip, p_nni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get_nni);

int32 ctc_app_vlan_port_update_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_update_nni, lchip, p_nni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_update_nni);

int32 ctc_app_vlan_port_destory_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_destory_nni, lchip, p_nni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_destory_nni);

int32 ctc_app_vlan_port_create_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_create_nni, lchip, p_nni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_create_nni);

int32 ctc_app_vlan_port_update(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_update, lchip, p_vlan_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_update);

int32 ctc_app_vlan_port_get(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get, lchip, p_vlan_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get);

int32 ctc_app_vlan_port_destory(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_destory, lchip, p_vlan_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_destory);

int32 ctc_app_vlan_port_create(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_create, lchip, p_vlan_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_create);

int32 ctc_app_vlan_port_update_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_update_gem_port, lchip, p_gem_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_update_gem_port);

int32 ctc_app_vlan_port_get_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get_gem_port, lchip, p_gem_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get_gem_port);

int32 ctc_app_vlan_port_destory_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_destory_gem_port, lchip, p_gem_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_destory_gem_port);

int32 ctc_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_create_gem_port, lchip, p_gem_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_create_gem_port);

int32 ctc_app_vlan_port_get_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get_uni, lchip, p_uni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get_uni);

int32 ctc_app_vlan_port_destory_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_destory_uni, lchip, p_uni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_destory_uni);

int32 ctc_app_vlan_port_create_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_create_uni, lchip, p_uni);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_create_uni);

int32 ctc_app_vlan_port_get_by_logic_port(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t* p_vlan_port)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get_by_logic_port, lchip, logic_port, p_vlan_port);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get_by_logic_port);

int32 ctc_app_vlan_port_get_global_cfg(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get_global_cfg, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get_global_cfg);

int32 ctc_app_vlan_port_set_global_cfg(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_set_global_cfg, lchip, p_glb_cfg);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_set_global_cfg);

int32 ctc_app_vlan_port_get_fid_mapping_info(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_get_fid_mapping_info, lchip, p_port_fid);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_get_fid_mapping_info);

int32 ctc_app_vlan_port_alloc_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_alloc_fid, lchip, p_port_fid);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_alloc_fid);

int32 ctc_app_vlan_port_free_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    CTC_PTR_VALID_CHECK(ctc_app_vlan_port_api);
    CTC_API_ERROR_RETURN(ctc_app_vlan_port_api->ctc_app_vlan_port_free_fid, lchip, p_port_fid);
}
CTC_EXPORT_SYMBOL(ctc_app_vlan_port_free_fid);

