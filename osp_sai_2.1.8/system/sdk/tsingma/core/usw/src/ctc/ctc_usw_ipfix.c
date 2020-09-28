#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_ipfix.c

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-17

 @version v3.0


*/


/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_ipfix.h"
#include "sys_usw_ipfix.h"
#include "sys_usw_common.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
ctc_usw_ipfix_init(uint8 lchip, void* p_global_cfg)
{
    ctc_ipfix_global_cfg_t global_cfg;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    if (NULL == p_global_cfg)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_ipfix_global_cfg_t));

        global_cfg.aging_interval = 10;
        global_cfg.bytes_cnt = 9600;
        global_cfg.conflict_export = 0;
        global_cfg.pkt_cnt = 100;
        global_cfg.sample_mode = 0;
        global_cfg.times_interval = 1000; /* 1s */
        p_global_cfg = &global_cfg;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_init(lchip, p_global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipfix_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_deinit(lchip));
    }

    return CTC_E_NONE;
}

extern int32
ctc_usw_ipfix_set_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_set_hash_field_sel(lchip, field_sel));
    }

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_set_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t *ipfix_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_set_global_cfg(lchip, ipfix_cfg));
    }

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_get_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t *ipfix_cfg)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipfix_get_global_cfg(lchip, ipfix_cfg));

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_set_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* flow_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_set_flow_cfg(lchip, flow_cfg));
    }

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_get_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* flow_cfg)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_ipfix_get_flow_cfg(lchip, flow_cfg));

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_register_cb(uint8 lchip, ctc_ipfix_fn_t callback,void *userdata)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_register_cb(lchip, callback, userdata));
    }

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_add_entry(uint8 lchip, ctc_ipfix_data_t* p_data)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_add_entry_by_key(lchip, p_data));
    }

    return CTC_E_NONE;
}
extern int32
ctc_usw_ipfix_delete_entry(uint8 lchip, ctc_ipfix_data_t* p_data)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_data);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ipfix_delete_entry_by_key(lchip, p_data));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_ipfix_traverse(uint8 lchip, ctc_ipfix_traverse_fn fn, ctc_ipfix_traverse_t* p_data)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    return sys_usw_ipfix_traverse(p_data->lchip_id, (void*)fn, p_data, 0);
}

int32
ctc_usw_ipfix_traverse_remove(uint8 lchip, ctc_ipfix_traverse_remove_cmp fn, ctc_ipfix_traverse_t* p_data)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_IPFIX);
    LCHIP_CHECK(lchip);
    return sys_usw_ipfix_traverse(p_data->lchip_id, (void*)fn, p_data, 1);
}

#endif

