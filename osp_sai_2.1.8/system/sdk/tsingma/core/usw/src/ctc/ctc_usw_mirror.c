/**
 @file ctc_usw_mirror.c

 @date 2009-10-21

 @version v2.0

 The file contains all mirror APIs for customers
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "ctc_error.h"
#include "ctc_usw_mirror.h"
#include "sys_usw_mirror.h"
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
 @brief This function is to initialize the mirror module
 */
int32
ctc_usw_mirror_init(uint8 lchip, void* mirror_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_init(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_mirror_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief This function is to set mirror enable on port
*/
int32
ctc_usw_mirror_set_port_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool enable, uint8 session_id)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mirror_set_port_en(lchip, gport, dir, enable, session_id));

    return CTC_E_NONE;
}

/**
 @brief This function is to get the information of port mirror
*/
int32
ctc_usw_mirror_get_port_info(uint8 lchip, uint32 gport, ctc_direction_t dir, bool* enable, uint8* session_id)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mirror_get_port_info(lchip, gport, dir, enable, session_id));

    return CTC_E_NONE;
}

/**
 @brief This function is to set enable mirror on vlan
*/
int32
ctc_usw_mirror_set_vlan_en(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool enable, uint8 session_id)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_set_vlan_en(lchip, vlan_id, dir, enable, session_id));
    }

    return CTC_E_NONE;
}

/**
 @brief This function is to get the information of vlan mirror
*/
int32
ctc_usw_mirror_get_vlan_info(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool* enable, uint8* session_id)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_mirror_get_vlan_info(lchip, vlan_id, dir, enable, session_id));

    return CTC_E_NONE;
}

/**
 @brief This function is to set local mirror destination port
*/
int32
ctc_usw_mirror_add_session(uint8 lchip, ctc_mirror_dest_t* mirror)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_mirror_set_dest(lchip, mirror));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_mirror_unset_dest(lchip, mirror);
    }

    return ret;
}

/**
 @brief Discard some special mirrored packet if enable
*/
int32
ctc_usw_mirror_set_escape_en(uint8 lchip, bool enable)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_rspan_escape_en(lchip, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief Config mac info indicat the mirrored packet is special
*/
int32
ctc_usw_mirror_set_escape_mac(uint8 lchip, ctc_mirror_rspan_escape_t* escape)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_rspan_escape_mac(lchip, escape));
    }

    return CTC_E_NONE;
}

/**
 @brief This function is to remove mirror destination port
*/
int32
ctc_usw_mirror_remove_session(uint8 lchip, ctc_mirror_dest_t* mirror)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_unset_dest(lchip, mirror));
    }

    return CTC_E_NONE;
}

/**
 @brief This function is to set whether packet is mirrored or not when it has been droped
*/
int32
ctc_usw_mirror_set_mirror_discard(uint8 lchip, ctc_direction_t dir, uint16 discard_flag, bool enable)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_set_mirror_discard(lchip, dir, discard_flag, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief This function is to get that whether packet is mirrored when it has been droped
*/
int32
ctc_usw_mirror_get_mirror_discard(uint8 lchip, ctc_direction_t dir, ctc_mirror_discard_t discard_flag, bool* enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_mirror_get_mirror_discard(lchip, dir, discard_flag, enable));

    return CTC_E_NONE;
}

int32
ctc_usw_mirror_set_erspan_psc(uint8 lchip, ctc_mirror_erspan_psc_t* psc)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MIRROR);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_set_erspan_psc(lchip, psc));
    }

    return CTC_E_NONE;
}

