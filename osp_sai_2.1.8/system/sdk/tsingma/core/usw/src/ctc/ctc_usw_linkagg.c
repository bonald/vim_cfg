/**
 @file ctc_usw_linkagg.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-10

 @version v2.0

 This file contains linkagg function interface.
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#include "ctc_usw_linkagg.h"
#include "sys_usw_linkagg.h"
#include "sys_usw_common.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
/**
 @brief The function is to init the linkagg module

 @param[]

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_init(uint8 lchip, void* linkagg_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    ctc_linkagg_global_cfg_t linkagg_cfg;

    LCHIP_CHECK(lchip);
    if (NULL == linkagg_global_cfg)
    {
        /*set default value*/
        sal_memset(&linkagg_cfg, 0, sizeof(ctc_linkagg_global_cfg_t));
        linkagg_cfg.linkagg_mode = CTC_LINKAGG_MODE_FLEX;
        linkagg_global_cfg = &linkagg_cfg;
    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_linkagg_init(lchip, linkagg_global_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to set linkagg property

 @param[in] tid the linkagg id wanted to set property

 @param[in] linkagg_prop the property of linkagg

 @param[in] value the property value of linkagg

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_set_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32 value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end,all_chip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_linkagg_set_property(lchip, tid, linkagg_prop, value));
    }
    return ret;
}

/**
 @brief The function is to get linkagg property

 @param[in] tid the linkagg id wanted to get property

 @param[in] linkagg_prop the property of linkagg

 @param[out] p_value pointer to the property value of linkagg

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_get_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32* p_value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
            ret,
            sys_usw_linkagg_get_property(lchip, tid, linkagg_prop, p_value));
    }
    return ret;
}

/**
 @brief The function is to create one linkagg

 @param[in] tid the linkagg id wanted to create

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_create(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_linkagg_grp);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_linkagg_create(lchip, p_linkagg_grp));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_linkagg_destroy(lchip,p_linkagg_grp->tid);
    }

    return ret;
}

/**
 @brief The function is to delete one linkagg

 @param[in] tid the linkagg id wanted to remove

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_destroy(uint8 lchip, uint8 tid)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
            ret,
            sys_usw_linkagg_destroy(lchip, tid));
    }

    return ret;
}

/**
 @brief The function is to add a port to linkagg

 @param[in] tid the linkagg id wanted to operate

 @param[in] gport global port of the member port which wanted to add

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_add_port(uint8 lchip, uint8 tid, uint32 gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
            ret,
            sys_usw_linkagg_add_port(lchip, tid, gport));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_linkagg_remove_port(lchip, tid, gport);
    }

    return ret;
}

/**
 @brief The function is to remove the port from linkagg

 @param[in] tid the linkagg id wanted to operate

 @param[in] gport global port of the member port which wanted to add

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_remove_port(uint8 lchip, uint8 tid, uint32 gport)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret = CTC_E_NONE;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
            ret,
            sys_usw_linkagg_remove_port(lchip, tid, gport));
    }

    return ret;
}

/**
 @brief The function is to replace ports for linkagg

 @param[in] tid the linkagg id wanted to operate

 @param[in] gport global port of the member ports which wanted to replace

 @param[in] mem_num number of the member ports wanted to replace

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_replace_ports(uint8 lchip, uint8 tid, uint32* gport, uint16 mem_num)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_linkagg_replace_ports(lchip, tid, gport, mem_num));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get a local member port of linkagg

 @param[in] tid the linkagg id wanted to operate

 @param[out] p_gport the pointer point to a local member port, will be NULL if none

 @param[out] local_cnt number of local port

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_get_1st_local_port(uint8 lchip_id, uint8 tid, uint32* p_gport, uint16* local_cnt)
{

    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_linkagg_get_1st_local_port(lchip_id, tid, p_gport, local_cnt));

    return CTC_E_NONE;
}

/**
 @brief The function is to get the member ports of a linkagg group

 @param[in] tid the linkagg id wanted to operate

 @param[out] p_gports a global member ports list of linkagg group

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_get_member_ports(uint8 lchip, uint8 tid, uint32* p_gports, uint16* cnt)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_linkagg_show_ports(lchip, tid, p_gports, cnt));

    return CTC_E_NONE;
}

/**
 @brief The function is to set member of linkagg hash key

 @param[in] p_psc the port selection criteria of linkagg hash

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_set_psc(uint8 lchip, ctc_linkagg_psc_t* p_psc)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 all_chip = (sys_usw_chip_get_rchain_en())? 0xff : 1;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_linkagg_set_psc(lchip, p_psc));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get member of linkagg hash key

 @param[in] psc the port selection criteria of linkagg hash

 @return CTC_E_XXX

*/
int32
ctc_usw_linkagg_get_psc(uint8 lchip, ctc_linkagg_psc_t* p_psc)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_linkagg_get_psc(lchip, p_psc));

    return CTC_E_NONE;
}

int32
ctc_usw_linkagg_get_max_mem_num(uint8 lchip, uint16* max_num)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_LINKAGG);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_linkagg_get_max_mem_num(lchip, max_num));

    return CTC_E_NONE;
}

int32
ctc_usw_linkagg_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_linkagg_deinit(lchip));
    }

    return CTC_E_NONE;
}


