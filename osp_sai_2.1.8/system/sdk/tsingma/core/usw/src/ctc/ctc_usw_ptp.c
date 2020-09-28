#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_ptp.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2012-9-17

 @version v2.0

  This file contains PTP(IEEE1588) associated APIs's implementation
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_ptp.h"
#include "ctc_error.h"
#include "ctc_usw_ptp.h"
#include "sys_usw_ptp.h"
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
 @brief Initialize PTP module and set ptp default config

 @param[in] lchip  Local chip ID

 @param[in]  p_ptp_cfg  The configuration of PTP

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_init(uint8 lchip, ctc_ptp_global_config_t* p_ptp_cfg)
{
    ctc_ptp_global_config_t ptp_cfg;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);

    /* set default config*/
    if (NULL == p_ptp_cfg)
    {
        sal_memset(&ptp_cfg, 0, sizeof(ptp_cfg));
        ptp_cfg.cache_aging_time = 60;
        p_ptp_cfg = &ptp_cfg;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_init(lchip, p_ptp_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief De-Initialize ptp module

 @param[in] lchip  Local chip ID

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief Set ptp property

 @param[in] lchip  Local chip ID

 @param[in]  property  Enumeration of PTP property

 @param[in]  value  Property value

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_global_property(uint8 lchip, ctc_ptp_global_prop_t property, uint32 value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_global_property(lchip, property, value));
    }

    return CTC_E_NONE;
}

/**
 @brief Get ptp property

 @param[in] lchip  Local chip ID

 @param[in]  property  Enumeration of PTP property

 @param[out]  value  Property value

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_global_property(uint8 lchip, ctc_ptp_global_prop_t property, uint32* value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_global_property(lchip, property, value));
    }

    return CTC_E_NONE;
}

/**
 @brief Get local clock timestamp

 @param[in] lchip  Local chip ID

 @param[out]  p_timestamp  Timestamp read from free running clock

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_clock_timestamp(uint8 lchip, ctc_ptp_time_t* p_timestamp)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_clock_timestamp(lchip, p_timestamp));
    }

    return CTC_E_NONE;
}

/**
 @brief Adjust offset for local clock

 @param[in] lchip  Local chip ID

 @param[in] p_offset  Offset different from master clock

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_adjust_clock_offset(uint8 lchip, ctc_ptp_time_t* p_offset)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_adjust_clock_offset(lchip, p_offset));
    }

    return CTC_E_NONE;
}

/**
 @brief Set drift for local clock

 @param[in] lchip  Local chip ID

 @param[in]  p_drift  The drift parameter defined as amount of time to drift per 1 second

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_clock_drift(lchip, p_drift));
    }

    return CTC_E_NONE;
}

/**
 @brief Get drift value

 @param[in] lchip  Local chip ID

 @param[out]  p_drift  The drift parameter defined as amount of time to drift per 1 second

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_clock_drift(lchip, p_drift));
    }

    return CTC_E_NONE;
}

/**
 @brief Set PTP device type

 @param[in] lchip  Local chip ID

 @param[in]  device_type  The type of PTP device

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_device_type(uint8 lchip, ctc_ptp_device_type_t device_type)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_device_type(lchip, device_type));
    }

    return CTC_E_NONE;
}

/**
 @brief Get PTP device type

 @param[in] lchip  Local chip ID

 @param[out]  p_device_type  The type of PTP device

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_device_type(uint8 lchip, ctc_ptp_device_type_t* p_device_type)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_device_type(lchip, p_device_type));
    }

    return CTC_E_NONE;
}

/**
 @brief Set adjust delay for PTP message delay correct

 @param[in] lchip  Local chip ID

 @param[in]  gport  Global port ID

 @param[in]  type  Enum of delay type

 @param[in]  value  The value of delay,unit:nanosecond

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64 value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_adjust_delay(lchip, gport, type, value));
    }

    return CTC_E_NONE;
}

/**
 @brief Set adjust delay for PTP message delay correct

 @param[in] lchip  Local chip ID

 @param[in]  gport  Global port ID

 @param[in]  type  Enum of delay type

 @param[out]  value  The value of delay,unit:nanosecond

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64* value)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_adjust_delay(lchip, gport, type, value));
    }

    return CTC_E_NONE;
}

/**
 @brief  Set Sync Interface config

 @param[in] lchip  Local chip ID

 @param[in]  p_sync_cfg  Configuration of Sync Interface

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_sync_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_sync_intf(lchip, p_sync_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get Sync Interface config

 @param[in] lchip  Local chip ID

 @param[in]  p_sync_cfg  Configuration of Sync Interface

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_sync_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_sync_intf(lchip, p_sync_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief  Set toggle time to trigger Sync Interface output syncColck signal

 @param[in] lchip  Local chip ID

 @param[in]  p_toggle_time  Toggle time to trigger Sync Interface output syncColck signal

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_sync_intf_toggle_time(lchip, p_toggle_time));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get toggle time to trigger Sync Interface output syncColck signal

 @param[in] lchip  Local chip ID

 @param[out]  p_toggle_time  Toggle time to trigger Sync Interface output syncColck signal

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_sync_intf_toggle_time(lchip, p_toggle_time));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get Sync Interface input time code

 @param[in] lchip  Local chip ID

 @param[out]  p_time_code  The time information receive by Sync Interface

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_sync_intf_code(uint8 lchip, ctc_ptp_sync_intf_code_t* p_time_code)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_sync_intf_code(lchip, p_time_code));
    }

    return CTC_E_NONE;
}

/**
 @brief  Clear Sync Interface input time code

 @param[in] lchip  Local chip ID

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_clear_sync_intf_code(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_clear_sync_intf_code(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief  Set ToD Interface config

 @param[in] lchip  Local chip ID

 @param[in]  p_tod_cfg  The configuration of ToD Interface

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* p_tod_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_tod_intf(lchip, p_tod_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get ToD Interface config

 @param[in] lchip  Local chip ID

 @param[out]  p_tod_cfg  The configuration of ToD Interface

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* p_tod_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_tod_intf(lchip, p_tod_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief  Set ToD Interface output code config

 @param[in] lchip  Local chip ID

 @param[in]  p_tx_code  The configuration of ToD Interface output code

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_set_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* p_tx_code)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_set_tod_intf_tx_code(lchip, p_tx_code));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get ToD Interface output code config

 @param[in] lchip  Local chip ID

 @param[out]  p_tx_code  The configuration of ToD Interface output code

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* p_tx_code)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_tod_intf_tx_code(lchip, p_tx_code));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get ToD Interface input time code

 @param[in] lchip  Local chip ID

 @param[out]  p_rx_code  The time code received by ToD Interface

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_tod_intf_rx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* p_rx_code)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_tod_intf_rx_code(lchip, p_rx_code));
    }

    return CTC_E_NONE;
}

/**
 @brief  Clear ToD Interface input time code

 @param[in] lchip  Local chip ID

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_clear_tod_intf_code(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_clear_tod_intf_code(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief  Get timestamp captured by local clock

 @param[in] lchip  Local chip ID

 @param[in]  p_captured_ts  Timestamp source type and sequence id

 @param[out]  p_captured_ts  Captured timestamp

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_get_captured_ts(uint8 lchip, ctc_ptp_capured_ts_t* p_captured_ts)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_get_captured_ts(lchip, p_captured_ts));
    }

    return CTC_E_NONE;
}

/**
 @brief  Add ptp clock type

 @param[in] lchip  Local chip ID

 @param[in] clock  Ptp clock type configuration

 @remark  Add ptp clock configuration, refer ctc_ptp_clock_t

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_add_device_clock(uint8 lchip, ctc_ptp_clock_t* clock)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_add_device_clock(lchip, clock));
    }

    return CTC_E_NONE;
}

/**
 @brief  Remove ptp clock type

 @param[in] lchip  Local chip ID

 @param[in] clock  Ptp clock type configuration, only use clock id

 @remark  Remove ptp clock type of given ptp id in ctc_ptp_clock_t

 @return CTC_E_XXX

*/
int32
ctc_usw_ptp_remove_device_clock(uint8 lchip, ctc_ptp_clock_t* clock)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PTP);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_ptp_remove_device_clock(lchip, clock));
    }

    return CTC_E_NONE;
}

#endif

