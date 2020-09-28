#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_overlay_tunnel.c

 @date 2011-11-07

 @version v2.0


*/
/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"

#include "ctc_overlay_tunnel.h"
#include "sys_usw_overlay_tunnel.h"
#include "sys_usw_common.h"

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
/****************************************************************************
 *
* Function
*
*****************************************************************************/

int32
ctc_usw_overlay_tunnel_init(uint8 lchip, void* overlay_tunnel_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    ctc_overlay_tunnel_global_cfg_t global_cfg;

    LCHIP_CHECK(lchip);
    if (NULL == overlay_tunnel_global_cfg)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_overlay_tunnel_global_cfg_t));
    }
    else
    {
        sal_memcpy(&global_cfg, overlay_tunnel_global_cfg, sizeof(ctc_overlay_tunnel_global_cfg_t));
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_overlay_tunnel_init(lchip, &global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_overlay_tunnel_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_overlay_tunnel_deinit(lchip));
    }

    return CTC_E_NONE;
}


int32
ctc_usw_overlay_tunnel_set_fid(uint8 lchip, uint32 vn_id, uint16 fid )
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_overlay_tunnel_set_fid(lchip, vn_id, fid));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_overlay_tunnel_get_fid(uint8 lchip, uint32 vn_id, uint16* p_fid )
{

    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_overlay_tunnel_get_fid (lchip,  vn_id, p_fid ));

    return CTC_E_NONE;
}

int32
ctc_usw_overlay_tunnel_get_vn_id(uint8 lchip,  uint16 fid, uint32* p_vn_id)
{
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_overlay_tunnel_get_vn_id(lchip, fid, p_vn_id));

    return CTC_E_NONE;
}

int32
ctc_usw_overlay_tunnel_add_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    LCHIP_CHECK(lchip);

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN) && (p_tunnel_param->acl_lkup_num == 0))
    {
        p_tunnel_param->acl_prop[0].tcam_lkup_type = p_tunnel_param->acl_tcam_lookup_type;
        p_tunnel_param->acl_prop[0].class_id = p_tunnel_param->acl_tcam_label;
        p_tunnel_param->acl_prop[0].acl_en = 1;
        p_tunnel_param->acl_prop[0].acl_priority = 0;
        p_tunnel_param->acl_prop[0].direction = 0;
        p_tunnel_param->acl_prop[0].flag = 0;
        p_tunnel_param->acl_prop[0].hash_lkup_type = 0;
        p_tunnel_param->acl_prop[0].hash_field_sel_id = 0;
        p_tunnel_param->acl_lkup_num = 1;
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN) && (p_tunnel_param->acl_lkup_num == 0))
    {
        p_tunnel_param->acl_prop[0].tcam_lkup_type = 0;
        p_tunnel_param->acl_prop[0].class_id = 0;
        p_tunnel_param->acl_prop[0].acl_en = 1;
        p_tunnel_param->acl_prop[0].acl_priority = 0;
        p_tunnel_param->acl_prop[0].direction = 0;
        CTC_SET_FLAG(p_tunnel_param->acl_prop[0].flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP);
        p_tunnel_param->acl_prop[0].hash_lkup_type = p_tunnel_param->acl_hash_lookup_type;
        p_tunnel_param->acl_prop[0].hash_field_sel_id = p_tunnel_param->field_sel_id;
        p_tunnel_param->acl_lkup_num = 1;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_overlay_tunnel_add_tunnel(lchip, p_tunnel_param));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_overlay_tunnel_remove_tunnel(lchip, p_tunnel_param);
    }

    return ret;
}

int32
ctc_usw_overlay_tunnel_remove_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_overlay_tunnel_remove_tunnel(lchip, p_tunnel_param));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_overlay_tunnel_update_tunnel(uint8 lchip, ctc_overlay_tunnel_param_t* p_tunnel_param)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN) && (p_tunnel_param->acl_lkup_num == 0))
    {
        p_tunnel_param->acl_prop[0].tcam_lkup_type = p_tunnel_param->acl_tcam_lookup_type;
        p_tunnel_param->acl_prop[0].class_id = p_tunnel_param->acl_tcam_label;
        p_tunnel_param->acl_prop[0].acl_en = 1;
        p_tunnel_param->acl_prop[0].acl_priority = 0;
        p_tunnel_param->acl_prop[0].direction = 0;
        p_tunnel_param->acl_prop[0].flag = 0;
        p_tunnel_param->acl_prop[0].hash_lkup_type = 0;
        p_tunnel_param->acl_prop[0].hash_field_sel_id = 0;
        p_tunnel_param->acl_lkup_num = 1;
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN) && (p_tunnel_param->acl_lkup_num == 0))
    {
        p_tunnel_param->acl_prop[0].tcam_lkup_type = 0;
        p_tunnel_param->acl_prop[0].class_id = 0;
        p_tunnel_param->acl_prop[0].acl_en = 1;
        p_tunnel_param->acl_prop[0].acl_priority = 0;
        p_tunnel_param->acl_prop[0].direction = 0;
        p_tunnel_param->acl_prop[0].flag = 1;
        p_tunnel_param->acl_prop[0].hash_lkup_type = p_tunnel_param->acl_hash_lookup_type;
        p_tunnel_param->acl_prop[0].hash_field_sel_id = p_tunnel_param->field_sel_id;
        p_tunnel_param->acl_lkup_num = 1;
    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_overlay_tunnel_update_tunnel(lchip, p_tunnel_param));
    }

    return CTC_E_NONE;
}

#endif
