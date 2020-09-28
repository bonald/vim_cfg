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
*    saivrrp.h
*
* Abstract:
*
*    This module defines SAI vrrp table API
*    The table contains both IPv4 and IPv6 neighbors
*
*/

#if !defined (__SAIVRRP_H_)
#define __SAIVRRP_H_

#include <saitypes.h>

/** \defgroup SAIVRRP SAI - Vrrp specific API definitions.
 *
 *  \{
 */
 
/**
 *  @brief Attribute Id for sai vrrp object
 */
typedef enum _sai_vrrp_attr_t
{
    /** READ-WRITE */
    /** vmac address for the vrrp vip entry*/
    SAI_VRRP_ATTR_VMAC_ADDRESS,

    /** interface mac address for the vrrp vip entry*/
    SAI_VRRP_ATTR_IF_MAC_ADDRESS,
} sai_vrrp_attr_t;

/**
*  @brief vrrp entry
*/
typedef struct _sai_vrrp_vip_entry_t
{
    sai_ip_address_t ip_address;

} sai_vrrp_vip_entry_t;

/**
 * Routine Description:
 *    @brief Create vrrp entry
 *
 * Arguments:
 *    @param[in] vrrp_vip_entry - vrrp vip entry
 *    @param[in] attr_count - number of attributes
 *    @param[in] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
typedef sai_status_t (*sai_create_vrrp_vip_entry_fn)(
    _In_ const sai_vrrp_vip_entry_t* vrrp_vip_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief Remove vrrp vip entry
 *
 * Arguments:
 *    @param[in] vrrp_vip_entry - vrrp vip entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
typedef sai_status_t (*sai_remove_vrrp_vip_entry_fn)(
    _In_ const sai_vrrp_vip_entry_t* vrrp_vip_entry
    );

/**
 *  @brief neighbor table methods, retrieved via sai_api_query()
 */
typedef struct _sai_vrrp_api_t
{
    sai_create_vrrp_vip_entry_fn        create_vrrp_vip_entry;
    sai_remove_vrrp_vip_entry_fn        remove_vrrp_vip_entry;
} sai_vrrp_api_t;

/**
 * \}
 */
#endif // __SAINEIGHBOR_H_

