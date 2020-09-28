/**
 @file ctc_usw_aps.c

 @date 2010-3-15

 @version v2.0

 Define all Duet2 APIs of APS
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"

#include "ctc_usw_aps.h"
#include "sys_usw_aps.h"
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
 @brief Initialize Duet2 APS module
*/
int32
ctc_usw_aps_init(uint8 lchip, void* aps_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_aps_init(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_aps_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_aps_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief Create APS Bridge Group
*/
int32
ctc_usw_aps_create_aps_bridge_group(uint8 lchip, uint16 aps_bridge_group_id, ctc_aps_bridge_group_t* aps_group)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_create_bridge_group(lchip, aps_bridge_group_id, aps_group));
    }

    return CTC_E_NONE;
}

/**
 @brief Set APS Bridge Group
*/
int32
ctc_usw_aps_set_aps_bridge_group(uint8 lchip, uint16 aps_bridge_group_id, ctc_aps_bridge_group_t* aps_group)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_update_bridge_group(lchip, aps_bridge_group_id, aps_group));
    }

    return CTC_E_NONE;
}


/**
 @brief Remove a APS Bridge Group
*/
int32
ctc_usw_aps_destroy_aps_bridge_group(uint8 lchip, uint16 aps_bridge_group_id)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_remove_bridge_group(lchip, aps_bridge_group_id));
    }

    return CTC_E_NONE;
}

/**
 @brief Set working path of aps bridge group
*/
int32
ctc_usw_aps_set_aps_bridge_working_path(uint8 lchip, uint16 aps_bridge_group_id, uint32 gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_set_bridge_path(lchip, aps_bridge_group_id, gport, TRUE));
    }
    return CTC_E_NONE;
}

/**
 @brief Get working path of aps bridge group
*/
int32
ctc_usw_aps_get_aps_bridge_working_path(uint8 lchip, uint16 aps_bridge_group_id, uint32* gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_bridge_path(lchip, aps_bridge_group_id, gport, TRUE));
    }

    return CTC_E_NONE;
}

/**
 @brief Set protection path of aps bridge group
*/
int32
ctc_usw_aps_set_aps_bridge_protection_path(uint8 lchip, uint16 aps_bridge_group_id, uint32 gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_set_bridge_path(lchip, aps_bridge_group_id, gport, FALSE));
    }

    return CTC_E_NONE;
}

/**
 @brief Get protection path of aps bridge group
*/
int32
ctc_usw_aps_get_aps_bridge_protection_path(uint8 lchip, uint16 aps_bridge_group_id, uint32* gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_bridge_path(lchip, aps_bridge_group_id, gport, FALSE));
    }

    return CTC_E_NONE;
}

/**
 @brief Set aps bridge to working path or protection path
*/
int32
ctc_usw_aps_set_aps_bridge(uint8 lchip, uint16 aps_bridge_group_id, bool protect_en)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_set_protection(lchip, aps_bridge_group_id, protect_en));
    }

    return CTC_E_NONE;
}

/**
 @brief Get aps bridge status
*/
int32
ctc_usw_aps_get_aps_bridge(uint8 lchip, uint16 aps_bridge_group_id, bool* protect_en)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_protection(lchip, aps_bridge_group_id, protect_en));
    }

    return CTC_E_NONE;
}

/**
 @brief Set aps selector from working path or protection path
*/
int32
ctc_usw_aps_set_aps_select(uint8 lchip, uint16 aps_select_group_id, bool protect_en)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_set_protection(lchip, aps_select_group_id, protect_en));
    }

    return CTC_E_NONE;
}

/**
 @brief Get aps selector status
*/
int32
ctc_usw_aps_get_aps_select(uint8 lchip, uint16 aps_select_group_id, bool* protect_en)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_protection(lchip, aps_select_group_id, protect_en));
    }

    return CTC_E_NONE;
}

/**
 @brief create R-APS group
*/
int32
ctc_usw_aps_create_raps_mcast_group(uint8 lchip, uint16 group_id)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_create_raps_mcast_group(lchip, group_id));
    }

    return CTC_E_NONE;
}

/**
 @brief remove R-APS group
*/
int32
ctc_usw_aps_destroy_raps_mcast_group(uint8 lchip, uint16 group_id)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip  = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_remove_raps_mcast_group(lchip, group_id));
    }

    return CTC_E_NONE;
}

/**
 @brief update R-APS group meber
*/
int32
ctc_usw_aps_update_raps_mcast_member(uint8 lchip, ctc_raps_member_t* p_raps_mem)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_lchip = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_APS);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_raps_mem);

    if(CTC_IS_LINKAGG_PORT(p_raps_mem->mem_port))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(p_raps_mem->mem_port, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    all_lchip = (sys_usw_chip_get_rchain_en() && all_lchip)? 0xff : all_lchip;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_ERROR_RETURN(sys_usw_aps_update_raps_mcast_member(lchip, p_raps_mem));
    }

    return CTC_E_NONE;
}

