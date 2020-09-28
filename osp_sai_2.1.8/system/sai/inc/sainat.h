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
*    sainat.h
*
* Abstract:
*
*    This module defines SAI nat table API
*
*/

#if !defined (__SAINAT_H_)
#define __SAINAT_H_

#include <saitypes.h>

/**
 *  @brief Attribute Id for sai nat object
 */
typedef enum _sai_nat_forward_attr_t
{
    /** READ-WRITE */
    /**  */
    SAI_NAT_ATTR_INSIDE_INTERFACE,

    SAI_NAT_ATTR_ENTRY_HIT,

    /**  */
    SAI_NAT_ATTR_XXXX,
} sai_nat_forward_attr_t;

/* 0x01, 0x02, 0x04....*/
typedef enum _sai_nat_forward_flag_t
{
    SAI_NAT_FWD_FLAG_USE_TCP_PORT      = 0x01 << 0,
    SAI_NAT_FWD_FLAG_MAX               = 0xFF

}sai_nat_forward_flag_t;

/**
*  @brief nat forward entry
*/
typedef struct _sai_nat_forward_entry_t
{
    sai_uint16_t      flags;
    sai_uint16_t      vrf_id;
    sai_ip_address_t  inside_local_ip;
    sai_ip_address_t  inside_global_ip;
    sai_uint16_t      inside_local_port;
    sai_uint16_t      inside_global_port;
} sai_nat_forward_entry_t;

/**
 * Routine Description:
 *    @brief Create nat forward entry
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *    @param[in] attr_count - number of attributes
 *    @param[in] attrs - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
typedef sai_status_t (*sai_create_nat_forward_entry_fn)(
    _In_ const sai_nat_forward_entry_t* nat_fwd_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *    @brief Remove nat forward entry
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
typedef sai_status_t (*sai_remove_nat_forward_entry_fn)(
    _In_ const sai_nat_forward_entry_t* nat_fwd_entry
    );

/**
 * Routine Description:
 *    @brief set nat forward entry attribute
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
typedef sai_status_t (*sai_set_nat_forward_attribute_fn)(
    _In_ const sai_nat_forward_entry_t* nat_fwd_entry,
    _In_ const sai_attribute_t *attr
    );

/**
 * Routine Description:
 *    @brief get nat forward entry attribute
 *
 * Arguments:
 *    @param[in] nat_forward_entry - nat forward entry
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
typedef sai_status_t (*sai_get_nat_forward_attribute_fn)(
    _In_ const sai_nat_forward_entry_t* nat_fwd_entry,
    _In_ sai_attribute_t *attr
    );

/**
 *  @brief nat table methods, retrieved via sai_api_query()
 */
typedef struct _sai_nat_api_t
{
    sai_create_nat_forward_entry_fn        create_nat_forward_entry;
    sai_remove_nat_forward_entry_fn        remove_nat_forward_entry;
    sai_set_nat_forward_attribute_fn       set_nat_forward_attribute;
    sai_get_nat_forward_attribute_fn       get_nat_forward_attribute;    
} sai_nat_api_t;

/**
 * \}
 */
#endif // __SAINEIGHBOR_H_

