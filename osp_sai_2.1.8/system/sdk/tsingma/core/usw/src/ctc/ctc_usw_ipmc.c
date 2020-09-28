#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file ctc_usw_ipmc.c

 @date 2010-01-25

 @version v2.0


*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_ipmc.h"
#include "sys_usw_common.h"
#include "sys_usw_ipmc.h"
#include "sys_usw_nexthop_api.h"

/****************************************************************************
 *
* Function
*
*****************************************************************************/

/**
 @brief Init ipmc module, including global variable, ipmc database, etc.

 @param[in] ipmc_global_cfg   ipmc global config information, use struct ctc_ipmc_global_cfg_t to set max vrf id (ipv4, ipv6)

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_init(uint8 lchip, void* ipmc_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end   = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipmc_init(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipmc_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end   = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipmc_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief Add an ipmc multicast group

 @param[in] p_group     An pointer reference to an ipmc group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_add_group(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_ipmc_add_group(lchip, p_group));
    }

    /* rollback if error exist */
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_ipmc_remove_group(lchip, p_group);
    }

    return ret;
}

/**
 @brief Remove an ipmc group

 @param[in] p_group     An pointer reference to an ipmc group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_remove_group(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
            ret,
            sys_usw_ipmc_remove_group(lchip, p_group));
    }

    return ret;
}

/**
 @brief Add members to an ipmc group

 @param[in] p_group     An pointer reference to an ipmc group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_add_member(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;
    uint8   index      = 0;
    uint8   old_member = 0;
    uint8   normal_member = 0;
    uint8   reroute_member = 0;
    sys_nh_info_dsnh_t      sys_nh_info;
    ctc_ipmc_member_info_t  *pold_member_info_ptr = NULL;
    ctc_ipmc_member_info_t  *pipmc_member         = NULL;
    ctc_ipmc_member_info_t  *pipmc_reroute_member = NULL;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);

    sal_memset(&sys_nh_info,0,sizeof(sys_nh_info));

    pold_member_info_ptr = mem_malloc(MEM_IPMC_MODULE, sizeof(ctc_ipmc_member_info_t)
                                        * CTC_IPMC_MAX_MEMBER_PER_GROUP);
    pipmc_member = mem_malloc(MEM_IPMC_MODULE, sizeof(ctc_ipmc_member_info_t)
                                        * CTC_IPMC_MAX_MEMBER_PER_GROUP);
    pipmc_reroute_member = mem_malloc(MEM_IPMC_MODULE, sizeof(ctc_ipmc_member_info_t)
                                        * CTC_IPMC_MAX_MEMBER_PER_GROUP);

    if(!pold_member_info_ptr || !pipmc_member || !pipmc_reroute_member)
    {
        ret = CTC_E_NO_MEMORY;
        goto out;
    }

    sal_memset(pold_member_info_ptr,0,sizeof(ctc_ipmc_member_info_t)
                * CTC_IPMC_MAX_MEMBER_PER_GROUP);
    sal_memset(pipmc_member,0,sizeof(ctc_ipmc_member_info_t)
                * CTC_IPMC_MAX_MEMBER_PER_GROUP);
    sal_memset(pipmc_reroute_member,0,sizeof(ctc_ipmc_member_info_t)
                * CTC_IPMC_MAX_MEMBER_PER_GROUP);

    old_member = p_group->member_number;
    sal_memcpy(pold_member_info_ptr,
        p_group->ipmc_member,
        sizeof(ctc_ipmc_member_info_t) * CTC_IPMC_MAX_MEMBER_PER_GROUP);

    for(index = 0; index < old_member;index ++)
    {
        if(p_group->ipmc_member[index].is_nh && sys_usw_nh_get_nhinfo(lchip,
            p_group->ipmc_member[index].nh_id,&sys_nh_info, 0) < 0)
        {
            reroute_member = 0;
            normal_member  = old_member;
            sal_memcpy(pipmc_member,
                p_group->ipmc_member,
                sizeof(ctc_ipmc_member_info_t) * CTC_IPMC_MAX_MEMBER_PER_GROUP);
            break;
        }

        if(sys_nh_info.re_route)
        {
            sal_memcpy(&pipmc_reroute_member[reroute_member],
                &p_group->ipmc_member[index],
                sizeof(ctc_ipmc_member_info_t));
            reroute_member++;
        }else{
            sal_memcpy(&pipmc_member[normal_member],
                &p_group->ipmc_member[index],
                sizeof(ctc_ipmc_member_info_t));
            normal_member++;
        }
    }

    if(reroute_member)
    {
        lchip_start = 0;
        lchip_end   = 1;

        p_group->member_number = reroute_member;
        sal_memcpy(p_group->ipmc_member,
            pipmc_reroute_member,
            sizeof(ctc_ipmc_member_info_t) * CTC_IPMC_MAX_MEMBER_PER_GROUP);

        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
        {
            CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
                ret,
                sys_usw_ipmc_add_member(lchip, p_group));
        }

        /* rollback if error exist */
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_ipmc_remove_member(lchip, p_group);
        }
    }

    if(normal_member && (CTC_E_NONE == ret))
    {
        p_group->member_number = normal_member;
        sal_memcpy(p_group->ipmc_member,
            pipmc_member,
            sizeof(ctc_ipmc_member_info_t) * CTC_IPMC_MAX_MEMBER_PER_GROUP);

        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
        {
            CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
                ret,
                sys_usw_ipmc_add_member(lchip, p_group));
        }

        /* rollback if error exist */
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_ipmc_remove_member(lchip, p_group);
        }

        if((CTC_E_NONE != ret) && reroute_member)
        {
            lchip_start = 0;
            lchip_end   = 1;

            p_group->member_number = reroute_member;
            sal_memcpy(p_group->ipmc_member,
                pipmc_reroute_member,
                sizeof(ctc_ipmc_member_info_t) * CTC_IPMC_MAX_MEMBER_PER_GROUP);
            CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
            {
                sys_usw_ipmc_remove_member(lchip, p_group);
            }
        }
    }

    p_group->member_number = old_member;
    sal_memcpy(p_group->ipmc_member,
        pold_member_info_ptr,
        sizeof(ctc_ipmc_member_info_t) * CTC_IPMC_MAX_MEMBER_PER_GROUP);

out:
    mem_free(pold_member_info_ptr);
    mem_free(pipmc_member);
    mem_free(pipmc_reroute_member);

    return ret;
}

/**
 @brief Remove members from an ipmc group

 @param[in] p_group     An pointer reference to an ipmc group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_remove_member(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_ipmc_remove_member(lchip, p_group));
    }

    return ret;
}

/**
 @brief Update the RPF interface of the group

 @param[in] p_group     An pointer reference to an ipmc group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_update_rpf(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipmc_update_rpf(lchip, p_group));
    }

    return CTC_E_NONE;
}

/**
 @brief Add the rp interface

 @param[in] p_rp     An pointer reference to an rp group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_add_rp_intf(uint8 lchip, ctc_ipmc_rp_t * p_rp)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_ipmc_add_rp_intf(lchip, p_rp));
    }

    /* rollback if error exist */
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_ipmc_remove_rp_intf(lchip, p_rp);
    }

    return ret;
}

/**
 @brief Remove the RP interface

 @param[in] p_rp     An pointer reference to an rp group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_remove_rp_intf(uint8 lchip, ctc_ipmc_rp_t* p_rp)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_ipmc_remove_rp_intf(lchip, p_rp));
    }

    return ret;
}

/**
 @brief Get info of the group

 @param[in] p_group     An pointer reference to an ipmc group

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_get_group_info(uint8 lchip, ctc_ipmc_group_info_t* p_group)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipmc_get_group_info(lchip, p_group));

    return CTC_E_NONE;
}

/**
 @brief Add the default action for both ipv4 and ipv6 ipmc.

 @param[in] ip_version     IPv4 or IPv6.

 @param[in] type           Indicates send to cpu or drop only.

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_add_default_entry(uint8 lchip, uint8 ip_version, ctc_ipmc_default_action_t type)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipmc_add_default_entry(lchip, ip_version, type));
    }

    return CTC_E_NONE;
}

/**
 @brief Set ipv4 and ipv6 force route register for force mcast ucast or bridge.

 @param[in] p_data   An point reference to ctc_ipmc_force_route_t.

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_set_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipmc_set_mcast_force_route(lchip, p_data));
    }

    return CTC_E_NONE;
}

/**
 @brief Get ipv4 and ipv6 mcast force route.

 @param[in, out] p_data   An point reference to ctc_ipmc_v4_force_route_t.

 @return CTC_E_XXX

*/
int32
ctc_usw_ipmc_get_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipmc_get_mcast_force_route(lchip, p_data));

    return CTC_E_NONE;
}

int32
ctc_usw_ipmc_set_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8 hit)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipmc_set_entry_hit(lchip, p_group, hit));
    }
    return CTC_E_NONE;

}

int32
ctc_usw_ipmc_get_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8* hit)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPMC);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipmc_get_entry_hit(lchip, p_group, hit));

    return CTC_E_NONE;
}
#endif

