#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_mpls.c

 @date 2010-03-16

 @version v2.0


*/
/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_mpls.h"
#include "ctc_vector.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_common.h"
#include "sys_usw_mpls.h"
#include "sys_usw_nexthop_api.h"
#include "ctc_stats.h"

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
 @brief SDK mpls module initilize

 @param[]

 @return CTC_E_XXX

*/

int32
ctc_usw_mpls_init(uint8 lchip, ctc_mpls_init_t* p_mpls_info)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    ctc_mpls_init_t mpls_init;
    sal_memset(&mpls_init, 0, sizeof(ctc_mpls_init_t));

    LCHIP_CHECK(lchip);
    if (NULL == p_mpls_info)
    {
        p_mpls_info = &mpls_init;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN( sys_usw_mpls_init(lchip, p_mpls_info));
    }

    return  CTC_E_NONE;
}

int32
ctc_usw_mpls_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN( sys_usw_mpls_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief

 @param[in] p_mpls_ilm, parameters used to add mpls ilm entry

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_add_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    int32 ret = CTC_E_NONE;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
                                 ret,
                                 sys_usw_mpls_add_ilm(lchip, p_mpls_ilm));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_mpls_del_ilm(lchip, p_mpls_ilm);
    }

    return ret;
}

/**
@brief update a mpls entry

@param[in] p_mpls_ilm Data of the mpls entry

@remark  Update all properties of a MPLS ilm.

@return CTC_E_XXX

*/
extern int32
ctc_usw_mpls_update_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mpls_update_ilm(lchip, p_mpls_ilm));
    }

    return CTC_E_NONE;
}

/**
 @brief

 @param[in]  p_mpls_ilm of the ilm entry

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_del_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mpls_del_ilm(lchip, p_mpls_ilm));
    }

    return  CTC_E_NONE;
}

/**
 @brief

 @param[in] p_mpls_ilm index of the mpls ilm entry

 @param[out] p_mpls_ilm data of the mpls ilm entry

 @param[out] nh_id nexthop id array of the mpls ilm entry

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_get_ilm(uint8 lchip, uint32* nh_id, ctc_mpls_ilm_t* p_mpls_ilm)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);
    return sys_usw_mpls_get_ilm(lchip, nh_id, p_mpls_ilm);
}


STATIC int32
_ctc_usw_mpls_map_property(ctc_mpls_property_t* p_mpls_pro,
                                                ctc_mpls_property_t* p_mpls_pro_out)
{
    int32 ret = CTC_E_NONE;
    uint32 ctc_type = 0;

    CTC_PTR_VALID_CHECK(p_mpls_pro);

    p_mpls_pro_out->label = p_mpls_pro->label;
    p_mpls_pro_out->space_id = p_mpls_pro->space_id;
    p_mpls_pro_out->value = p_mpls_pro->value;
    p_mpls_pro_out->use_flex = p_mpls_pro->use_flex;
    CTC_PTR_VALID_CHECK(p_mpls_pro_out->value);

    switch (p_mpls_pro->property_type)
    {
        case CTC_MPLS_ILM_QOS_DOMAIN:
            ctc_type = SYS_MPLS_ILM_QOS_DOMAIN;
            break;
        case CTC_MPLS_ILM_APS_SELECT:
            ctc_type = SYS_MPLS_ILM_APS_SELECT;
            break;
        case CTC_MPLS_ILM_OAM_TOCPU:
            ctc_type = SYS_MPLS_ILM_OAM_CHK;
            *((uint32*)p_mpls_pro_out->value) = *((uint32*)p_mpls_pro_out->value)?0x4:0;
            break;
        case CTC_MPLS_ILM_DATA_DISCARD:
            ctc_type = SYS_MPLS_ILM_OAM_CHK;
            *((uint32*)p_mpls_pro_out->value) = *((uint32*)p_mpls_pro_out->value)?0x5:0;
            break;
        case CTC_MPLS_ILM_ROUTE_MAC:
            ctc_type = SYS_MPLS_ILM_ROUTE_MAC;
            break;
        case CTC_MPLS_ILM_LLSP:
            ctc_type = SYS_MPLS_ILM_LLSP;
            break;
        case CTC_MPLS_ILM_OAM_MP_CHK_TYPE:
            ctc_type = SYS_MPLS_ILM_OAM_CHK;
            if ( *((uint32*)p_mpls_pro_out->value) >= CTC_MPLS_ILM_OAM_MP_CHK_TYPE_MAX)
            {
                 ret = CTC_E_INVALID_PARAM;
            }
            break;
         case CTC_MPLS_ILM_ARP_ACTION:
            ctc_type = SYS_MPLS_ILM_ARP_ACTION;
            CTC_MAX_VALUE_CHECK(*((uint32 *)p_mpls_pro_out->value), MAX_CTC_EXCP_TYPE-1);
            break;
         case CTC_MPLS_ILM_QOS_MAP:
            ctc_type = SYS_MPLS_ILM_QOS_MAP;
            break;
         case CTC_MPLS_ILM_STATS_EN:
            ctc_type = SYS_MPLS_STATS_EN;
            break;
         case CTC_MPLS_ILM_DENY_LEARNING_EN:
            ctc_type = SYS_MPLS_ILM_DENY_LEARNING;
            break;
         case CTC_MPLS_ILM_MAC_LIMIT_DISCARD_EN:
            ctc_type = SYS_MPLS_ILM_MAC_LIMIT_DISCARD_EN;
            break;
         case CTC_MPLS_ILM_MAC_LIMIT_ACTION:
            ctc_type = SYS_MPLS_ILM_MAC_LIMIT_ACTION;
            break;
         case CTC_MPLS_ILM_STATION_MOVE_DISCARD_EN:
            ctc_type = SYS_MPLS_ILM_STATION_MOVE_DISCARD_EN;
            break;
         case CTC_MPLS_ILM_METADATA:
            ctc_type = SYS_MPLS_ILM_METADATA;
            break;
         case CTC_MPLS_ILM_DCN_ACTION:
            ctc_type = SYS_MPLS_ILM_DCN_ACTION;
            break;
         case CTC_MPLS_ILM_CID:
            ctc_type = SYS_MPLS_ILM_CID;
            break;
        default:
            ret = CTC_E_INVALID_PARAM;
            break;
    }
    p_mpls_pro_out->property_type = ctc_type;

    return ret;
}

/**
 @brief set a mpls entry property

 @param[in] p_mpls_ilm Data of the mpls entry

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_set_ilm_property(uint8 lchip, ctc_mpls_property_t* p_mpls_pro)
{
    ctc_mpls_property_t mpls_pro;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);
    sal_memset(&mpls_pro, 0, sizeof(ctc_mpls_property_t));

    CTC_ERROR_RETURN(_ctc_usw_mpls_map_property(p_mpls_pro, &mpls_pro));

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro));
    }

    return  CTC_E_NONE;
}


/**
 @brief add mpls stats

 @param[in] stats_index index of the mpls labe stats to be add

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_add_stats(uint8 lchip, ctc_mpls_stats_index_t* p_mpls_pro)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    ctc_mpls_property_t mpls_pro;

    LCHIP_CHECK(lchip);
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    sal_memset(&mpls_pro, 0, sizeof(mpls_pro));

    CTC_PTR_VALID_CHECK(p_mpls_pro);
    mpls_pro.label = p_mpls_pro->label;
    mpls_pro.space_id = p_mpls_pro->spaceid;
    mpls_pro.value = &p_mpls_pro->stats_id;
    CTC_PTR_VALID_CHECK(mpls_pro.value);

    mpls_pro.property_type = SYS_MPLS_STATS_EN;

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro));
    }

    return CTC_E_NONE;
}

/**
 @brief delete mpls stats

 @param[in] stats_index index of the mpls labe stats to be delete

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_remove_stats(uint8 lchip, ctc_mpls_stats_index_t* p_mpls_pro)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint32 stats_id = 0;
    ctc_mpls_property_t mpls_pro;

    LCHIP_CHECK(lchip);
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    sal_memset(&mpls_pro, 0, sizeof(mpls_pro));

    CTC_PTR_VALID_CHECK(p_mpls_pro);
    mpls_pro.label = p_mpls_pro->label;
    mpls_pro.space_id = p_mpls_pro->spaceid;

    mpls_pro.value = &stats_id;
    mpls_pro.property_type = SYS_MPLS_STATS_EN;

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro));
    }

    return CTC_E_NONE;
}

/**
 @brief Add the l2vpn pw entry

 @param[in] p_mpls_pw Information of l2vpn pw entry

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_add_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw)
{
    int32 ret = CTC_E_NONE;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
                                 ret,
                                 sys_usw_mpls_add_l2vpn_pw(lchip, p_mpls_pw));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start, lchip_end)
    {
        sys_usw_mpls_del_l2vpn_pw(lchip, p_mpls_pw);
    }


    return  ret;
}

/**
 @brief Remove the l2vpn pw entry

 @param[in] label VC label of the l2vpn pw entry

 @return CTC_E_XXX

*/
int32
ctc_usw_mpls_del_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_MPLS);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mpls_del_l2vpn_pw(lchip, p_mpls_pw));
    }

    return  CTC_E_NONE;
}

#endif
