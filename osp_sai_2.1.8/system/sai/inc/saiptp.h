/*
* Copyright (c) 2014 Microsoft Open Technologies, Inc.
*
*    Licensed under the Apache License, Version 2.0 (the "License"); you may
*    not use this file except in compliance with the License. You may obtain
*    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
*    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
*    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
*    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
*    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
*    See the Apache Version 2.0 License for specific language governing
*    permissions and limitations under the License.
*
*    Microsoft would like to thank the following companies for their review and
*    assistance with these files: Intel Corporation, Mellanox Technologies Ltd,
*    Dell Products, L.P., Facebook, Inc
*
* Module Name:
*
*    saiptp.h
*
* Abstract:
*
*    This module defines SAI PTP API
*
*/

#if !defined (__SAIPTP_H_)
#define __SAIPTP_H_

#include <saitypes.h>

/** \defgroup SAIPTP SAI - PTP specific API definitions.
 *
 *  \{
 */

typedef enum _sai_ptp_device_type_t
{
    /** 
     * @brief PTP device type
     */

    /** Device is Disable */
    SAI_PTP_DEVICE_NONE     = 0, 

    /** Device is OC */
    SAI_PTP_DEVICE_OC       = 1, 

    /** Device is BC */
    SAI_PTP_DEVICE_BC       = 2, 

    /** Device is E2E TC */
    SAI_PTP_DEVICE_E2E_TC   = 3, 

    /** Device is P2P TC */
    SAI_PTP_DEVICE_P2P_TC   = 4, 

} sai_ptp_device_type_t;

/**
 *  @brief Attribute Id in sai_set_ptp_attribute_fn() and
 *  sai_get_ptp_attribute_fn() calls
 */
typedef enum _sai_ptp_attr_t
{
    /** READ-ONLY */
    SAI_PTP_ATTR_GPORT,

    /** READ-WRITE */

    /** PTP device type [sai_ptp_device_type_t]
      */
    SAI_PTP_ATTR_DEVICE_TYPE,

    /** PTP adjust offset [_sai_s32_list_t]
      * the first s32 is seconds, the second s32 is nanoseconds
      */
    SAI_PTP_ATTR_ADJUST_OFFSET,
    
    /*new add glb*/
    SAI_PTP_ATTR_ADJUST_FFO,
    SAI_PTP_ATTR_SYSTEM_TIME,
    SAI_PTP_ATTR_HARDWARE_SECOND,

    /** PTP timestamp [_sai_s32_list_t]
      * the first s32 is seconds, the second s32 is nanoseconds
      */
    SAI_PTP_ATTR_TIMESTAMP,

    /*new add port*/
    SAI_PTP_ATTR_PORT_PATH_DELAY,
    SAI_PTP_ATTR_PORT_INGRESS_LATENCY_CORRECTION,
    SAI_PTP_ATTR_PORT_EGRESS_LATENCY_CORRECTION,
    SAI_PTP_ATTR_PORT_ASYMMETRY_CORRECTION,
    SAI_PTP_ATTR_PORT_ENABLE,
    
    
    /** Custom range base value */
    SAI_PTP_ATTR_CUSTOM_RANGE_BASE  = 0x10000000

} sai_ptp_attr_t;


/**
 * Routine Description:
 *    @brief Set PTP attribute Value
 *
 * Arguments:
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_set_ptp_attribute_fn)(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t  *attr
    );

/**
 * Routine Description:
 *    @brief Get PTP attribute Value
 *
 * Arguments:
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_get_ptp_attribute_fn)(
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    );

/**
 * @brief PTP methods table retrieved with sai_api_query()
 */
typedef struct _sai_ptp_api_t
{
    sai_set_ptp_attribute_fn       set_ptp_attribute;
    sai_get_ptp_attribute_fn       get_ptp_attribute;
} sai_ptp_api_t;

/**
 *\}
 */
#endif // __SAIPTP_H_
