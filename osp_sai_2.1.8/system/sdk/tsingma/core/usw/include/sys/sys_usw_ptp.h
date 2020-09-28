#if (FEATURE_MODE == 0)
/**
 @file sys_usw_ptp.h

 @date 2012-9-17

 @version v2.0

  The file defines Macro, stored data structure for PTP module
*/
#ifndef _SYS_USW_PTP_H
#define _SYS_USW_PTP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_ptp.h"
#include "ctc_interrupt.h"

/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/

/****************************************************************************
*
* Data structures
*
****************************************************************************/

/****************************************************************************
 *
* Function
*
****************************************************************************/

/**
 @brief Initialize PTP module and set ptp default config
*/
extern int32
sys_usw_ptp_init(uint8 lchip, ctc_ptp_global_config_t* ptp_global_cfg);

/**
 @brief Set ptp property
*/
extern int32
sys_usw_ptp_set_global_property(uint8 lchip, ctc_ptp_global_prop_t property, uint32 value);

/**
 @brief Get ptp property
*/
extern int32
sys_usw_ptp_get_global_property(uint8 lchip, ctc_ptp_global_prop_t property, uint32* value);

/**
 @brief Get timestamp from free running clock
*/
extern int32
sys_usw_ptp_get_clock_timestamp(uint8 lchip, ctc_ptp_time_t* timestamp);

/**
 @brief Adjust offset for free running clock
*/
extern int32
sys_usw_ptp_adjust_clock_offset(uint8 lchip, ctc_ptp_time_t* offset);

/**
 @brief Set drift for free running clock
*/
extern int32
sys_usw_ptp_set_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift);

/**
 @brief Get drift for free running clock
*/
extern int32
sys_usw_ptp_get_clock_drift(uint8 lchip, ctc_ptp_time_t* p_drift);

/**
 @brief Set PTP device type
*/
extern int32
sys_usw_ptp_set_device_type(uint8 lchip, ctc_ptp_device_type_t device_type);

/**
 @brief Get PTP device type
*/
extern int32
sys_usw_ptp_get_device_type(uint8 lchip, ctc_ptp_device_type_t* device_type);

/**
 @brief Set adjust delay for PTP message delay correct
*/
extern int32
sys_usw_ptp_set_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64 value);

/**
 @brief Set adjust delay for PTP message delay correct
*/
extern int32
sys_usw_ptp_get_adjust_delay(uint8 lchip, uint32 gport, ctc_ptp_adjust_delay_type_t type, int64* value);

/**
 @brief  Set sync interface config
*/
extern int32
sys_usw_ptp_set_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_config);

/**
 @brief  Get sync interface config
*/
extern int32
sys_usw_ptp_get_sync_intf(uint8 lchip, ctc_ptp_sync_intf_cfg_t* p_config);

/**
 @brief  Set toggle time to trigger sync interface output syncColck signal
*/
extern int32
sys_usw_ptp_set_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time);

/**
 @brief  Set toggle time to trigger sync interface output syncColck signal
*/
extern int32
sys_usw_ptp_get_sync_intf_toggle_time(uint8 lchip, ctc_ptp_time_t* p_toggle_time);

/**
 @brief  Get sync interface input time information
*/
extern int32
sys_usw_ptp_get_sync_intf_code(uint8 lchip, ctc_ptp_sync_intf_code_t* g_time_code);

/**
 @brief  Clear sync interface input time information
*/
extern int32
sys_usw_ptp_clear_sync_intf_code(uint8 lchip);

/**
 @brief  Set ToD interface config
*/
extern int32
sys_usw_ptp_set_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* config);

/**
 @brief  Get ToD interface config
*/
extern int32
sys_usw_ptp_get_tod_intf(uint8 lchip, ctc_ptp_tod_intf_cfg_t* config);

/**
 @brief  Set ToD interface output message config
*/
extern int32
sys_usw_ptp_set_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* tx_code);

/**
 @brief  Get ToD interface output message config
*/
extern int32
sys_usw_ptp_get_tod_intf_tx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* tx_code);

/**
 @brief  Get ToD interface input time information
*/
extern int32
sys_usw_ptp_get_tod_intf_rx_code(uint8 lchip, ctc_ptp_tod_intf_code_t* rx_code);

/**
 @brief  Clear ToD interface input time information
*/
extern int32
sys_usw_ptp_clear_tod_intf_code(uint8 lchip);

/**
 @brief  Get timestamp captured by local clock
*/
extern int32
sys_usw_ptp_get_captured_ts(uint8 lchip, ctc_ptp_capured_ts_t* p_captured_ts);


/**
 @brief  Add ptp clock type
*/
extern int32
sys_usw_ptp_add_device_clock(uint8 lchip, ctc_ptp_clock_t* clock);

/**
 @brief  Remove ptp clock type
*/
extern int32
sys_usw_ptp_remove_device_clock(uint8 lchip, ctc_ptp_clock_t* clock);

/**
 @brief  Deinit
*/
extern int32
sys_usw_ptp_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif

