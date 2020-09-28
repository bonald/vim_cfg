/**
 @file ctc_usw_ftm.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-11

 @version v2.0

 memory alloc init
*/

#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_alloc.h"
#include "ctc_debug.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"


/**
 @brief

 @param[in] ctc_profile_info  allocation profile information

 @return CTC_E_XXX

*/
int32
ctc_usw_ftm_mem_alloc(uint8 lchip, ctc_ftm_profile_info_t* ctc_profile_info)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ftm_mem_alloc(lchip, ctc_profile_info));
    }

    return CTC_E_NONE;
}



int32
ctc_usw_ftm_mem_free(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        sys_usw_ftm_mem_free(lchip);
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ftm_get_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly)
{
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ftm_get_hash_poly(lchip, hash_poly));
    return CTC_E_NONE;
}

int32
ctc_usw_ftm_set_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly)
{
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ftm_set_hash_poly(lchip, hash_poly));
    return CTC_E_NONE;
}

int32
ctc_usw_ftm_set_profile_specs(uint8 lchip, uint32 spec_type, uint32 value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ftm_set_profile_specs(lchip, spec_type, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ftm_get_profile_specs(uint8 lchip, uint32 spec_type, uint32* value)
{
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ftm_get_profile_specs(lchip, spec_type, value));

    return CTC_E_NONE;
}
int32 ctc_usw_ftm_mem_change(uint8 lchip, ctc_ftm_change_profile_t* p_profile)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ftm_mem_change(lchip, p_profile));
    }

    return CTC_E_NONE;
}

