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
*    saiisolation.h
*
* Abstract:
*
*    This module defines SAI isolation API
*
*/

#if !defined (__SAIISOLATION_H_)
#define __SAIISOLATION_H_

#include <saitypes.h>

/** \defgroup SAIISOLATION SAI - ISOLATION specific API definitions.
 *
 *  \{
 */

/**
 * @brief SAI drop packet type of isolation group
 */
typedef enum _sai_isolaton_group_drop_pkt_type_t
{
    /** unknown unicast traffic in the isolation group*/
    SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_UC = 1,

    /** known unicast traffic in the isolation group*/
    SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_UC,

    /** unknown mcast traffic in the isolation group*/
    SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_UNKNOWN_MC,

    /** unknown mcast traffic in the isolation group*/
    SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_KNOWN_MC,

    /** bcast traffic in the isolation group*/
    SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_BC,

    /** all type of traffic in the isolation group*/
    SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL,
        
} sai_isolaton_group_drop_pkt_type_t;

/**
 * @brief SAI mode of isolation group
 */
typedef enum _sai_isolaton_group_mode_t
{
    /** isolation group mode for isolation usage*/
    SAI_ISOLATION_GROUP_MODE_ISOLATION = 1,

    /** isolation group mode for MLAG usage*/
    SAI_ISOLATION_GROUP_MODE_MLAG,

    /** isolation group mode for PVLAN usage*/
    SAI_ISOLATION_GROUP_MODE_PVLAN,
        
} sai_isolaton_group_mode_t;

/**
 * @brief SAI attributes for isolation group
 */
typedef enum _sai_isolation_group_attr_t
{
    /** READ-ONLY */

    /** SAI port list [sai_object_list_t], port list to which the traffic from the isolation group will be dropped */
    SAI_ISOLATION_ATTR_EGRESS_BLOCK_PORT_LIST,

    /** SAI port list [sai_object_list_t] , port list from which the traffic to the isolation group will be dropped */
    SAI_ISOLATION_ATTR_INRESS_BLOCK_PORT_LIST,

    /** READ_WRITE */
    /** SAI isolation group drop type[sai_isolaton_group_drop_pkt_type_t],
     * Default is SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL*/
    SAI_ISOLATION_ATTR_DROP_PACKET_TYPE,

    /** MANDATORY_ON_CREATE|CREATE_ONLY */
    /** SAI isolation group mode[sai_isolaton_group_mode_t],
     * Default is SAI_ISOLATION_GROUP_MODE_ISOLATION*/
    SAI_ISOLATION_ATTR_MODE,

    /** Custom range base value */
    SAI_ISOLATION_ATTR_CUSTOM_RANGE_BASE  = 0x10000000
    
} sai_isolation_group_attr_t;

/**
 * @brief Create isolation group.
 *
 * @param[out] group_id isolation group id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_create_isolation_group_fn)(
        _Out_ sai_object_id_t *group_id,
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list);

/**
 * @brief Remove isolation group.
 *
 * @param[in] group_id Port Isolation Group id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_remove_isolation_group_fn)(
        _In_ sai_object_id_t group_id);

/**
 * @brief Set isolation group attributes.
 *
 * @param[in] group_id Port isolation group id
 * @param[in] attr Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_set_isolation_group_attribute_fn)(
        _In_ sai_object_id_t group_id,
        _In_ const  sai_attribute_t *attr);

/**
 * @brief Get isolation group attributes.
 *
 * @param[in] group_id Port isolation group id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Value of attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_get_isolation_group_attribute_fn)(
        _In_ sai_object_id_t group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list);

/**
 * @brief ISOLATION method table retrieved with sai_api_query()
 */
typedef struct _sai_isolation_api_t
{
    sai_create_isolation_group_fn create_isolation_group;
    sai_remove_isolation_group_fn remove_isolation_group;
    sai_set_isolation_group_attribute_fn set_isolation_group_attribute;
    sai_get_isolation_group_attribute_fn get_isolation_group_attribute;
} sai_isolation_api_t;

/**
 *\}
 */
#endif // __SAIISOLATION_H_

