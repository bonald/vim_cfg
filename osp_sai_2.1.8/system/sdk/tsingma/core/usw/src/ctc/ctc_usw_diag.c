
#include "ctc_error.h"
#include "ctc_usw_diag.h"
#include "sys_usw_diag.h"


int32
ctc_usw_diag_init(uint8 lchip, void* diag_init_cfg)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    ctc_diag_global_cfg_t   global_cfg;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip);
    if (NULL == diag_init_cfg)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_diag_global_cfg_t));
        diag_init_cfg                 = &global_cfg;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_diag_init(lchip, diag_init_cfg));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_diag_deinit(uint8 lchip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_diag_deinit(lchip));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_diag_trigger_pkt_trace(uint8 lchip_id, ctc_diag_pkt_trace_t *p_trace)
{
    uint8 lchip = lchip_id;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_diag_trigger_pkt_trace(lchip_id, p_trace));
    return CTC_E_NONE;
}

int32
ctc_usw_diag_get_pkt_trace(uint8 lchip_id, ctc_diag_pkt_trace_result_t* p_rslt)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_diag_get_pkt_trace(lchip_id, p_rslt));
    return CTC_E_NONE;
}


int32
ctc_usw_diag_get_drop_info(uint8 lchip_id, ctc_diag_drop_t* p_drop)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_diag_get_drop_info(lchip_id, p_drop));
    return CTC_E_NONE;
}


int32
ctc_usw_diag_set_property(uint8 lchip_id, ctc_diag_property_t prop, void* p_value)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_diag_set_property(lchip_id, prop, p_value));
    return CTC_E_NONE;
}

int32
ctc_usw_diag_get_property(uint8 lchip_id, ctc_diag_property_t prop, void* p_value)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_DIAG);
    LCHIP_CHECK(lchip_id);
    CTC_ERROR_RETURN(sys_usw_diag_get_property(lchip_id, prop, p_value));
    return CTC_E_NONE;
}


