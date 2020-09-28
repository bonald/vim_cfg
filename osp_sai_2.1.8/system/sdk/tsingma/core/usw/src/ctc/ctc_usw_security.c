/**
 @file ctc_usw_security.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-2-4

 @version v2.0

   This file define ctc functions of SDK.
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_usw_security.h"
#include "sys_usw_security.h"
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
/**
 @brief  Init security module

 @return CTC_E_XXX

*/
int32
ctc_usw_security_init(uint8 lchip, void* security_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_security_init(lchip));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_security_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_security_deinit(lchip));
    }
    return CTC_E_NONE;
}

/**
 @brief  Configure Port based security

 @param[in] gport  global port

 @param[in] enable  if set,packet will be discard when srcport is mismatch with FDB

 @return CTC_E_XXX

*/
int32
ctc_usw_mac_security_set_port_security(uint8 lchip, uint32 gport, bool enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);

    CTC_ERROR_RETURN(sys_usw_mac_security_set_port_security(lchip, gport, enable));

    return CTC_E_NONE;
}

/**
 @brief  Get configure Port based security

 @param[in] gport   global port

 @param[out] p_enable if set,packet will be discard when srcport is mismatch with FDB

 @return CTC_E_XXX

*/
int32
ctc_usw_mac_security_get_port_security(uint8 lchip, uint32 gport, bool* p_enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);

    CTC_ERROR_RETURN(sys_usw_mac_security_get_port_security(lchip, gport, p_enable));

    return CTC_E_NONE;
}

/**
 @brief  Configure port based  mac limit

 @param[in] gport       global port

 @param[in] action      refer to ctc_maclimit_action_t

 @return CTC_E_XXX
*/
int32
ctc_usw_mac_security_set_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);

    CTC_ERROR_RETURN(sys_usw_mac_security_set_port_mac_limit(lchip, gport, action));

    return CTC_E_NONE;
}

/**
 @brief  get the configure  of port based  mac limit

 @param[in] gport       global port

 @param[in] action      refer to ctc_maclimit_action_t

 @return CTC_E_XXX
*/
int32
ctc_usw_mac_security_get_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);

    CTC_ERROR_RETURN(sys_usw_mac_security_get_port_mac_limit(lchip, gport, action));

    return CTC_E_NONE;
}

/**
 @brief Configure vlan based  mac limit

 @param[in] vlan_id     vlan id

 @param[in] action      refer to ctc_maclimit_action_t

 @return CTC_E_XXX
*/
int32
ctc_usw_mac_security_set_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t action)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mac_security_set_vlan_mac_limit(lchip, vlan_id, action));
    }

    return CTC_E_NONE;
}

/**
 @brief Get the configure  of vlan based  mac limit

 @param[in] vlan_id     vlan id

 @param[in] action      refer to ctc_maclimit_action_t

 @return CTC_E_XXX
*/
int32
ctc_usw_mac_security_get_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_mac_security_get_vlan_mac_limit(lchip, vlan_id, action));

    return CTC_E_NONE;
}

/**
 @brief Configure system based  mac limit

 @param[in] action      refer to ctc_maclimit_action_t

 @return CTC_E_XXX
*/
int32
ctc_usw_mac_security_set_system_mac_limit(uint8 lchip, ctc_maclimit_action_t action)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mac_security_set_system_mac_limit(lchip, action));
    }

    return CTC_E_NONE;
}

/**
 @brief Get the configure of system based  mac limit

 @param[out] action      refer to ctc_maclimit_action_t

 @return CTC_E_XXX
*/
int32
ctc_usw_mac_security_get_system_mac_limit(uint8 lchip, ctc_maclimit_action_t* action)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_mac_security_get_system_mac_limit(lchip, action));

    return CTC_E_NONE;
}

/**
 @brief Configure learn limit

 @param[in] p_learn_limit    refer to ctc_security_learn_limit_t

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_set_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learn_limit)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learn_limit);
    if ((CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM == p_learn_limit->limit_type)
        || (CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN == p_learn_limit->limit_type)
        || ((CTC_SECURITY_LEARN_LIMIT_TYPE_PORT == p_learn_limit->limit_type) && CTC_IS_LINKAGG_PORT(p_learn_limit->gport)))
    {
        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
        {
            CTC_ERROR_RETURN(sys_usw_mac_security_set_learn_limit(lchip, p_learn_limit));
        }
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(p_learn_limit->gport, lchip);
        CTC_ERROR_RETURN(sys_usw_mac_security_set_learn_limit(lchip, p_learn_limit));
    }

    return CTC_E_NONE;
}

/**
 @brief Get the configure of learn limit

 @param[in] p_learn_limit    refer to ctc_security_learn_limit_t

 @return CTC_E_XXX
*/
extern int32
ctc_usw_mac_security_get_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learn_limit)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learn_limit);

    if ((CTC_SECURITY_LEARN_LIMIT_TYPE_PORT == p_learn_limit->limit_type) && !CTC_IS_LINKAGG_PORT(p_learn_limit->gport))
    {
        SYS_MAP_GPORT_TO_LCHIP(p_learn_limit->gport, lchip);
    }

    CTC_ERROR_RETURN(sys_usw_mac_security_get_learn_limit(lchip, p_learn_limit, NULL));

    return CTC_E_NONE;
}

/**
 @brief  Add ip source guard entry

 @param[in] p_elem  ip source guard property

 @return CTC_E_XXX

*/
int32
ctc_usw_ip_source_guard_add_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_elem);

    if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT)
        || CTC_IS_LINKAGG_PORT(p_elem->gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(p_elem->gport,lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                    sys_usw_ip_source_guard_add_entry(lchip, p_elem));
    }

    /*roll back if error exist*/
    if(1 == all_lchip)
    {
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_ip_source_guard_remove_entry(lchip, p_elem);
        }
    }

    return ret;
}

/**
 @brief  Remove ip source guard entry

 @param[in] p_elem  ip source guard property

 @return CTC_E_XXX

*/
int32
ctc_usw_ip_source_guard_remove_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_elem);

    if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT)
        || CTC_IS_LINKAGG_PORT(p_elem->gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(p_elem->gport,lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;

    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_ERROR_RETURN(sys_usw_ip_source_guard_remove_entry(lchip, p_elem));
    }

    return CTC_E_NONE;
}

/**
 @brief  Set configure of storm ctl

 @param[in] stmctl_cfg   configure of storm ctl

 @return CTC_E_XXX

*/
int32
ctc_usw_storm_ctl_set_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* stmctl_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(stmctl_cfg);
    if (CTC_SECURITY_STORM_CTL_OP_PORT == stmctl_cfg->op)
    {
        SYS_MAP_GPORT_TO_LCHIP(stmctl_cfg->gport, lchip);
        CTC_ERROR_RETURN(sys_usw_storm_ctl_set_cfg(lchip, stmctl_cfg));

    }
    else
    {
        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
        {
            CTC_ERROR_RETURN(sys_usw_storm_ctl_set_cfg(lchip, stmctl_cfg));
        }
    }

    return CTC_E_NONE;
}

/**
 @brief  Get configure of storm ctl

 @param[out] stmctl_cfg  configure of storm ctl

 @return CTC_E_XXX

*/
int32
ctc_usw_storm_ctl_get_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* stmctl_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(stmctl_cfg);
    if (CTC_SECURITY_STORM_CTL_OP_PORT == stmctl_cfg->op)
    {
        SYS_MAP_GPORT_TO_LCHIP(stmctl_cfg->gport, lchip);
        CTC_ERROR_RETURN(sys_usw_storm_ctl_get_cfg(lchip, stmctl_cfg));
    }
    else
    {
        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
        {
            CTC_ERROR_RETURN(sys_usw_storm_ctl_get_cfg(lchip, stmctl_cfg));
        }
    }
    return CTC_E_NONE;
}

/**
 @brief  Set global configure of storm ctl

 @param[out] p_glb_cfg  global configure of storm ctl

 @return CTC_E_XXX

*/
int32
ctc_usw_storm_ctl_set_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_storm_ctl_set_global_cfg(lchip, p_glb_cfg));
    }
    return CTC_E_NONE;
}

/**
 @brief  Get global configure of storm ctl

 @param[out] p_glb_cfg  global configure of storm ctl

 @return CTC_E_XXX

*/
int32
ctc_usw_storm_ctl_get_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_storm_ctl_get_global_cfg(lchip, p_glb_cfg));

    return CTC_E_NONE;
}

/**
 @brief  Set route is obey isolated enable

 @param[in] enable  a boolean value denote route-obey-iso is enable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_isolation_set_route_obey_isolated_en(uint8 lchip, bool enable)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_port_isolation_set_route_obey_isolated_en(lchip, enable));
    }
    return CTC_E_NONE;
}

/**
 @brief  Get route is obey isolated enable

 @param[out] p_enable  a boolean value denote route-obey-iso is enable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_isolation_get_route_obey_isolated_en(uint8 lchip, bool* p_enable)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_SECURITY);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_port_isolation_get_route_obey_isolated_en(lchip, p_enable));
    }
    return CTC_E_NONE;
}

